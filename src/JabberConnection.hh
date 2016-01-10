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
 *  Copyright (c) 2002-2004 Julian Missig
 */

#ifndef INCL_JABBER_CONNECTION_HH
#define INCL_JABBER_CONNECTION_HH

#include "TCPTransmitter.hh"
#include "PacketQueue.hh"
#include <jabberoo/jabberoo.hh>

#include <sigc++/object.h>
#include <sigc++/connection.h>

namespace Gabber
{

/**
 * A Jabber Connection.
 * This provides a nice wrapper around jabberoo + TCPTransmitter.
 * GabberApp is a JabberConnection.
 * Disconnection happens on deconstruction.
 */
class JabberConnection
     : public SigC::Object
{
public:
     /**
      * Construct a JabberConnection. This does not initiate connection.
      */
     JabberConnection();
     virtual ~JabberConnection();

     /**
      * Start the connection sequence.
      * First Gabber will attempt to connect to the server,
      * throwing evtConnecting, then it will attempt to
      * log in, throwing evtLoggingIn. When the process is
      * completed and a session has been established,
      * JabberConnection will throw the evtConnected signal.
      * @param login Whether Jabberoo should attempt to log in.
      */
     void connect(bool login = true);

     // Accessors
     /**
      * Whether or not we are currently connected.
      * @return whether we are connected and logged in.
      */
     bool               is_connected()     { return _connected; }

     /**
      * The current presence packet.
      * The JabberConnection has this copy of the most recently
      * sent presence packet.
      * @return current presence.
      */
     const jabberoo::Presence& get_my_presence();

     /**
      * Access to the jabberoo::Session.
      * @return jabberoo::Session object being used by Gabber
      */ 
     jabberoo::Session& getSession()       { return *_session; }
     /**
      * Access to the TCPTransmitter.
      * @return TCPTransmitter object being used by Gabber
      */
     TCPTransmitter&    getTransmitter()   { return *_transmitter; }
     PacketQueue& getPacketQueue() { return *_pqueue; }

     /**
      * Set the session password.
      */
    void set_password(const std::string& password)
    { _connection_info.password = password; }

     // Signals
     /**
      * Thrown when Gabber is attempting connect to the server.
      * The next signal is evtLoggingIn;
      */
     SigC::Signal0<void> evtConnecting;
     /**
      * Thrown when Gabber is attempting to log in.
      * The next signal is evtConnected;
      */
     SigC::Signal0<void> evtLoggingIn;
     /**
      * Thrown when Gabber has successfully connected (and logged in).
      * The first thing you should do after receiving this signal is send presence. Probably.
      */
     SigC::Signal0<void> evtConnected;
     /**
      * Thrown when Gabber has disconnected.
      */
     SigC::Signal0<void> evtDisconnected;
     /**
      * Thrown when Gabber is reconnecting to the server (just like evtConnecting).
      * evtDisconnected will be thrown when Gabber is disconnected,
      * then this signal will be thrown if Gabber is attempting to reconnect.
      * evtConnected will be thrown when Gabber has connected and logged in again.
      */
     SigC::Signal0<void> evtReconnecting;

protected:
     // Debugging event handlers
     void on_XML_ParserError(int error_code, const string& error_msg);
     // Transmitter event handlers
     void on_transmitter_connected();
     void on_transmitter_disconnected();
     void on_transmitter_reconnect();
     void on_transmitter_error(const string & emsg);
     // Session event handlers
     void on_session_connected(const judo::Element& t);
     void on_session_roster();
     void on_server_disco(const jabberoo::DiscoDB::Item* item);
     void on_serverchild_disco(const jabberoo::DiscoDB::Item* item);
     void on_session_disconnected();
     void on_session_my_presence(const jabberoo::Presence& p);
     void on_session_version(string& name, string& version, string& os);
     void on_session_time(string& UTF8Time, string& UTF8TimeZone);

protected:
    struct ConnectionInfo
    {
        std::string server;
        int port;
        std::string username;
        std::string resource;
        std::string password;
        bool ssl;
        bool reconnect;
    };
        
    ConnectionInfo      _connection_info;
    TCPTransmitter*    _transmitter;
    PacketQueue*       _pqueue;
    jabberoo::Session* _session;
    jabberoo::Presence _currPresence;

private:
    bool               _connected;
    bool               _login;
    SigC::Connection   _roster_conn;
};

}; // namespace Gabber

#endif // INCL_JABBER_CONNECTION_HH
