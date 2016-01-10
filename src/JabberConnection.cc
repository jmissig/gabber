/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 *  Gabber 2
 *  Based on Gabber, Copyright (c) 1999-2002 Dave Smith & Julian Missig
 *  Copyright (c) 2002 Julian Missig
 */

#include "JabberConnection.hh"
#include "Environment.hh"
#include "ConfigPaths.hh"
#include "PacketQueue.hh"

#include <glibmm/convert.h>

#ifndef WIN32
#  include <sys/utsname.h>
#endif

namespace Gabber {

// -------------------------------------------------------------------
//
// Generic Jabber Connection = Jabberoo + TCPTransmitter
//
// -------------------------------------------------------------------
JabberConnection::JabberConnection() :
     _currPresence("", jabberoo::Presence::ptUnavailable, jabberoo::Presence::stOffline)
{
     // Instiatiate objects
     _session     = new jabberoo::Session;
     _pqueue      = new PacketQueue(*_session);
     _transmitter = new TCPTransmitter;

     // Debugging signals
     _session->evtXMLParserError.connect(SigC::slot(*this, &JabberConnection::on_XML_ParserError));

     // Transmitter signals
     _transmitter->evtConnected.connect(SigC::slot(*this, &JabberConnection::on_transmitter_connected));
     _transmitter->evtDisconnected.connect(SigC::slot(*this, &JabberConnection::on_transmitter_disconnected));
     _transmitter->evtReconnect.connect(SigC::slot(*this, &JabberConnection::on_transmitter_reconnect));
     _transmitter->evtError.connect(SigC::slot(*this, &JabberConnection::on_transmitter_error));
     _transmitter->evtDataAvailable.connect(SigC::slot(*_session, &jabberoo::Session::push));

     // Session signals
     _session->evtTransmitXML.connect(SigC::slot(*_transmitter, &TCPTransmitter::send));
     _session->evtConnected.connect(SigC::slot(*this, &JabberConnection::on_session_connected));
     _session->evtDisconnected.connect(SigC::slot(*this, &JabberConnection::on_session_disconnected));
     _session->evtMyPresence.connect(SigC::slot(*this, &JabberConnection::on_session_my_presence));
     _session->evtOnVersion.connect(SigC::slot(*this, &JabberConnection::on_session_version));

     _session->evtOnTime.connect(SigC::slot(*this, &JabberConnection::on_session_time));
}

JabberConnection::~JabberConnection()
{
     // <aolvoice>Goodbye</aolvoice>
     delete _transmitter;
     delete _session;
     _session = NULL; // Just in case delete _transmitter sends a signal
     delete _pqueue;
}

void JabberConnection::connect(bool login)
{
     // Whether or not we want to attempt to log in
     _login = login;

     // set the proxy type
     // _transmitter->setProxy...

     // tell stuff we're connecting to the server
     evtConnecting();

     // Connect the transmitter
     _transmitter->connect(_connection_info.server,
                           _connection_info.port,
                           _connection_info.ssl,
                           _connection_info.reconnect);

     cout << "JabberConnection::connect" << endl;
     
     // Start polling to attempt to keep the connection open
     _transmitter->startPoll();

     // JabberConnection::on_transmitter_connected is next
}

const jabberoo::Presence& JabberConnection::get_my_presence()
{
     return _currPresence;
}

// -------------------------------------
// Debugging event handlers
// -------------------------------------
void JabberConnection::on_XML_ParserError(int error_code, const string& error_msg)
{
     // Display an error explaining that there was an error parsing the XML
     // TODO: see above ;)
     cerr << "XML Parser error " << error_code << " " << error_msg << "\nWe should have a dialog for this.\n" << endl;
     
     // Try to disconnect as gracefully as possible
     // TODO: set presence to offline -- is that even possible after parser error?
     if (!_session->disconnect())
	  _transmitter->disconnect();
}


// -------------------------------------
// Transmitter event handlers
// -------------------------------------
void JabberConnection::on_transmitter_connected()
{
     cout << "JabberConnection::on_transmitter_connected" << endl;
     // Figure out the auth method
     // Default to auto-select: 0k, digest, plaintext
     jabberoo::Session::AuthType atype = jabberoo::Session::atAutoAuth;
     // if they want to force one type, do so

     // Tell them we're logging in
     evtLoggingIn();
     // Send the credentials and attempt to log in
     _session->connect(_connection_info.server,
		       atype,
		       _connection_info.username,
		       _connection_info.resource,
		       _connection_info.password,
		       false,
		       _login);
}

void JabberConnection::on_transmitter_disconnected()
{
     // only do this if the session exists
     if (_session)
     {
          // Tell the session to disconnect
          cout << "transmitter DISCONNECTED" << endl;
          _session->disconnect();
     }
     
     // Tell the user we've been disconnected
     // this is *not* thrown for on_session_disconnected because
     // that event will end up causing this one anyway
     evtDisconnected();
}

void JabberConnection::on_transmitter_reconnect()
{
     // Tell the user we're reconnecting
     evtReconnecting();
}

void JabberConnection::on_transmitter_error(const string & emsg)
{
     cout << "Transmitter error. Disconnected: " << emsg << endl;
     _transmitter->disconnect();
}

// -------------------------------------
// Session event handlers
// -------------------------------------
void JabberConnection::on_session_connected(const judo::Element& t)
{
     cout << "JabberConnection::on_session_connected" << endl;
     _connected = true;
     // We are technically connected at this point
     // however, we really don't want things to go on
     // until /after/ the roster has been loaded
     _roster_conn = _session->evtOnRoster.connect(SigC::slot(*this, &JabberConnection::on_session_roster));
}

void JabberConnection::on_session_roster()
{
     // We only want to receive the OnRoster signal /once/
     // more than that is just excessive
     _roster_conn.disconnect();

     // Now we're connected and ready to go
     evtConnected();

     // Disco our server
     _session->discoDB().cache(_connection_info.server, SigC::slot(*this, &JabberConnection::on_server_disco), false);
     // and again for features
     _session->discoDB().cache(_connection_info.server, SigC::slot(*this, &JabberConnection::on_server_disco), true);
     
     _pqueue->loadFromDisk();
}

void JabberConnection::on_server_disco(const jabberoo::DiscoDB::Item* item)
{
     // in theory we could do things here like enable/disable specific features
     // based upon what the server supports
     
     // right now we'll query all the children so we know what they are
     for (jabberoo::DiscoDB::Item::const_iterator it = item->begin(); it != item->end(); ++it)
     {
          _session->discoDB().cache((*it)->getJID(), SigC::slot(*this, &JabberConnection::on_serverchild_disco), false);
     }
}

void JabberConnection::on_serverchild_disco(const jabberoo::DiscoDB::Item* item)
{
     // nadda
}

void JabberConnection::on_session_disconnected()
{
     cout << "session disconnected" << endl;
     // Disconnect the socket...
     _transmitter->disconnect();
}

void JabberConnection::on_session_my_presence(const jabberoo::Presence& p)
{
     _currPresence = jabberoo::Presence(p.getBaseElement());
}

void JabberConnection::on_session_version(string& name, string& version, string& os)
{
     name    = ENV_VARS.package;
     version = ENV_VARS.version;
#ifdef WIN32
     os = "Win32";
     // XXX Make me more informative
#else
     // Run uname
     struct utsname osinfo;
     uname(&osinfo);
     os = string(osinfo.sysname) + " " + string(osinfo.machine);
#endif
//     creator = "Julian Missig and Thomas Muldowney";
}

void JabberConnection::on_session_time(string& UTF8Time, string& UTF8TimeZone)
{
#ifndef WIN32
     UTF8Time = Glib::locale_to_utf8(UTF8Time);
     UTF8TimeZone = Glib::locale_to_utf8(UTF8TimeZone);
#endif
}

} // namespace Gabber
