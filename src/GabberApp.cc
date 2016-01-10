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

#include "GabberApp.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Environment.hh"
#include "ConfigPaths.hh"
#include "GabberDialog.hh" // for password dialog
#include "GabberWidgets.hh" // for unsubscribed dialog
#include "PrefsInterface.hh" // for LoginDlg
#include "ModuleManager.hh"
#include "S10nRequestDlg.hh"
#include "GabberWin.hh"
#include "ResourceManager.hh"
#include "LogManager.hh"
#include "PlatformFuncs.hh"
#include "S5B.hh"
#include "FileTransferDlg.hh"
#include "StreamInitiation.hh"
#include "FileTransferManager.hh"
#include "FileTransferRecvDlg.hh"
#include "GabberUtility.hh"

#include "AutoAway.hh"
#include "StatusIcon.hh"

#include "intl.h"

#include <jabberoo/presence.hh>

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/image.h>

using namespace Gabber;
using namespace jabberoo;

// -------------------------------------------------------------------
//
// Ye Olde Gabber App - The Hub
//
// -------------------------------------------------------------------
GabberApp::GabberApp(int argc, char** argv)
     : _GtkMain(argc, argv), _winMain(NULL), _shutting_down(false)
{
     cout << ENV_VARS.package << " " << ENV_VARS.version << endl;

     _configurator = create_configurator();
     _log_manager = new LogManager();

     // Instiatiate objects
     _resource_manager = new ResourceManager();
     _module_manager = new ModuleManager();
     _ft_manager = new FileTransferManager();

     // Filter for agents
    _session->discoDB().signal_cache_updated.connect(
        SigC::slot(*this, &GabberApp::on_discodb_filter));

     // Session signals
     _session->evtTransmitXML.connect(SigC::slot(*this, &GabberApp::on_XML_transmit));
     _session->evtRecvXML.connect(SigC::slot(*this, &GabberApp::on_XML_recv));
//     _session->evtIQ.connect(slot(*this, &GabberApp::on_session_iq));
     _session->evtAuthError.connect(SigC::slot(*this, &GabberApp::on_session_auth_error));
     _session->evtPresenceRequest.connect(SigC::slot(*this, &GabberApp::on_presence_request));
     _session->evtPresenceUnsubscribed.connect(SigC::slot(*this, &GabberApp::on_presence_unsubscribed));
//     _session->evtUnknownPacket.connect(slot(*this, &GabberApp::on_session_unknown_packet));
//     _session->evtPresence.connect(slot(*this, &GabberApp::on_session_presence));
//     _session->evtMyPresence.connect(slot(*this, &GabberApp::on_session_my_presence));
     evtDisconnected.connect(SigC::slot(*this, &GabberApp::on_evtDisconnected)); // Only for cleaning up _agents and the like

    // Add all the features we have
    _session->addFeature("jabber:x:event");
    // _session->addFeature("jabber:x:roster");
    // _session->addFeature("jabber:x:conference");
    _session->addFeature("gc-1.0");
    _session->addFeature("jabber:iq:oob");
    _session->addFeature("http://jabber.org/protocol/si");
    _session->addFeature("http://jabber.org/protocol/si/profile/file-transfer");
    _session->addFeature("http://jabber.org/protocol/bytestreams");

     // Check if we should do first-time wizard
     // if (config ...
     // otherwise, do they want to autoconnect?
     if (_configurator->get_bool(Keys::acct.autoconnect))
     {
          if (_configurator->get_bool(Keys::acct.savepassword))
          {
               init_win();
               set_password(_configurator->get_string(Keys::acct.password));
               connect();
          }
          else
          {
               get_password();
          }
     }
     else
     {
          // ok, just give them the login dialog
          LoginDlg::execute();
     }
}

GabberApp::~GabberApp()
{
     delete _ft_manager;
     delete _resource_manager;
     delete _module_manager;
     delete _log_manager;
}

void GabberApp::run()
{
     // Let the gnome app start processing messages
     Gtk::Main::run();
}

void GabberApp::get_password()
{
     Gtk::Window* mainwin = NULL;
     if (_winMain)
          mainwin = _winMain->getGtkWindow();
     
     GabberDialog dlg("Password_dlg", "Gabber_win", mainwin);
     Gtk::Label* lblMain;
     dlg.get_widget("Password_Main_lbl", lblMain);
     lblMain->set_markup("<span weight='bold' size='larger'>"
                         + string(_("Password required")) + "</span>\n\n"
                         + Util::substitute(_("Enter your password for JabberID <b>%s@%s</b>."),
                                            _configurator->get_string(Keys::acct.username),
                                            _configurator->get_string(Keys::acct.server)));
     dlg.show();
     
     int resp = dlg.run();
     dlg.hide();
     if (resp == Gtk::RESPONSE_OK)
     {
          init_win();
          Gtk::Entry* entPass;
          dlg.get_widget("Password_ent", entPass);
          set_password(entPass->get_text());
          connect();
     }
     else
     {
          LoginDlg::execute();
     }
}

void GabberApp::init_win()
{
    // Setup all our connection info
    _connection_info.server = _configurator->get_string(Keys::acct.server);
    _connection_info.port = _configurator->get_int(Keys::acct.port);
    _connection_info.username = _configurator->get_string(Keys::acct.username);
    _connection_info.resource = _configurator->get_string(Keys::acct.resource);
    // This is now handled by the set_password function so it doesn't have to be
    // saved in GConf
    //_connection_info.password = _configurator->get_string(Keys::acct.password);
    _connection_info.ssl = _configurator->get_bool(Keys::acct.ssl);
    _connection_info.reconnect = _configurator->get_bool(Keys::acct.reconnect);

    // If we don't have an existing main window
    if (_winMain == NULL)
    {
         // Create one
         _winMain  = new GabberWin(*this);
         _autoaway = new AutoAway(_winMain); // hook up autoaway to this window
         _status_icon = new StatusIcon(_winMain);
    }
    _module_manager->loadAll();
}

void GabberApp::clean_disconnect()
{
    // Send out status
    *_session << jabberoo::Presence("", jabberoo::Presence::ptUnavailable,
    jabberoo::Presence::stOffline, _("Logged Out"));

    // Try disconnecting Session.. and if that fails, the Transmitter
    if (!_session->disconnect())
    {
       _transmitter->disconnect();
    }
}

void GabberApp::finalize()
{
     // Cleanup GUI stuff before the msg loop goes boom
     delete _autoaway;
     delete _status_icon;
     delete _winMain;

     // Stop gtk/gnome message loop
     Gtk::Main::quit();
 }

void GabberApp::quit()
{
    _shutting_down = true;
    if(_session && 
       (_session->getConnState() != jabberoo::Session::csNotConnected))
    {
        clean_disconnect();
    }
    else
    {
        finalize();
    }
}

// -------------------------------------
// Session event handlers
// -------------------------------------
void GabberApp::on_XML_transmit(const char* XML)
{
#ifdef SESSION_DEBUG
    _log_manager->debug() << jutil::getTimeStamp() << " send " 
        << XML << endl;
#endif // SESSION_DEBUG
}

void GabberApp::on_XML_recv(const char* XML)
{
#ifdef SESSION_DEBUG
    _log_manager->debug() << jutil::getTimeStamp() << " recv " 
        << XML << endl;
#endif // SESSION_DEBUG
}

void GabberApp::on_evtDisconnected()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
     // this is just for handling things that GabberApp set up internally
     // which should change on a disconnect
     _agents.clear();

     if(_shutting_down)
     {
         finalize();
     }
}

void GabberApp::on_session_auth_error(int ErrorCode, const char* ErrorMsg)
{
     // TODO: handle these errors differently when in registration phase
     // perhaps disconnect this handler and use a separate one?

     Util::MessageDialog* dlg;
     
     switch (ErrorCode)
     {
          case 401:
               dlg = new Util::MessageDialog(_("<b>Could not log in: Authorization failed</b>\nYour username or password may be incorrect."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("Authorization failed"));
               dlg->run();
               dlg->hide();
               get_password();
	       delete dlg;
               return;
          case 402:
          case 407:
               dlg = new Util::MessageDialog(_("<b>Could not log in: External registration required</b>\nThis server requires some sort of external registration or payment. Try looking at the server's web site."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("External registration required"));
               dlg->run();
               dlg->hide();
               LoginDlg::execute();
	       delete dlg;
               return;
          case 406:
               dlg = new Util::MessageDialog(_("<b>Could not log in: Required information not provided</b>\nAdditional authentication is required. Make sure the password is not blank. It is possible Gabber does not support the authentication required for this server."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("Required information not provided"));
               dlg->run();
               dlg->hide();
               LoginDlg::execute();
	       delete dlg;
               return;
          case 409:
               dlg = new Util::MessageDialog(_("<b>Could not log in: Resource conflict</b>\nYou have another connection using the same resource. Choose a different resource."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("Resource conflict"));
               dlg->run();
               dlg->hide();
               LoginDlg::execute();
	       delete dlg;
               return;
          case 500:
               dlg = new Util::MessageDialog(_("<b>Could not log in: Internal server error</b>\nThis server encountered an error while attempting to authenticate. Try again later. It is possible Gabber does not support the authentication required for this server."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("Internal server error"));
               dlg->run();
               dlg->hide();
               LoginDlg::execute();
	       delete dlg;
               return;
          default:
               dlg = new Util::MessageDialog(_("<b>Could not log in</b>\nAn unknown error occurred while attempting to log in."), Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
               dlg->set_title(_("Internal server error"));
               dlg->run();
               dlg->hide();
               LoginDlg::execute();
	       delete dlg;
               return;
     }
}

void GabberApp::on_presence_request(const jabberoo::Presence& pres)
{
     std::string jid = pres.getFrom();
     std::string reason = pres.getStatus();
     jabberoo::Presence::Type type = pres.getType();

     if (type == jabberoo::Presence::ptSubRequest)
     {
          // If this has already been queued
          // or if we want messages auto-displayed and are currently free for chat or available
          // we can view it now, else queue
          if (pres.getBaseElement().cmpAttrib("gabber:queued", "true")
              || (_configurator->get_bool(Keys::intrface.messages_autodisplay)
                  && (_currPresence.getShow() == jabberoo::Presence::stChat
                      || _currPresence.getShow() == jabberoo::Presence::stOnline)
                  )
              )
          {
               // Other user wants access to our presence
               S10nRequestDlg::display(*this, *_winMain->getGtkWindow(), jid, reason);
          }
          else
          {
               _pqueue->push(new judo::Element(pres.getBaseElement()), "presence-s10n.png", "PresenceRequest");
          }
     }
     else if (type == jabberoo::Presence::ptUnsubRequest)
     {
          // Other user no longer wants our presence
          jabberoo::Presence p(jid, jabberoo::Presence::ptUnsubscribed);
          *_session << p;
     }
}

void GabberApp::on_presence_unsubscribed(const jabberoo::Presence& pres)
{
     // Other user denies our presence viewing abilities
     PrettyJID pjid(pres.getFrom(), "", PrettyJID::dtNickJID, false, false);
     
     // If they're still on our roster, then they just did this to us.
     if (!pjid.is_on_roster())
          return;
     
     // b& for sass
     
     Gtk::Window* mainwin = NULL;
     if (_winMain)
          mainwin = _winMain->getGtkWindow();
     
     GabberDialog dlg("Unsubscribed_dlg", "Gabber_win", mainwin);
     dlg.getDialog()->set_title(Util::substitute(_("%s Removed You"), pjid.get_nickname()));
     Gtk::HBox* hbox;
     dlg.get_widget("PrettyJID_hbox", hbox);
     hbox->add(pjid);
     pjid.show();
     dlg.show();
     
     int resp = dlg.run();
     dlg.hide();
     // Remove Contact
     if (resp == Gtk::RESPONSE_OK)
     {
          *_session << jabberoo::Presence(pres.getFrom(), jabberoo::Presence::ptUnsubscribed);
          _session->roster().deleteUser(pres.getFrom());
     }
     // Re-subscribe
     else if (resp == Gtk::RESPONSE_APPLY)
     {
          *_session << jabberoo::Presence(pres.getFrom(), jabberoo::Presence::ptSubRequest);
     }
     // Leave Contact
}

void GabberApp::on_discodb_filter(const DiscoDB::Item& item)
{
     const DiscoDB::Item::FeatureList& features = item.getFeatureList();
     // XXX: for NOW we check for jabber:iq:register
     // in the future we may want to change this
     // to search for jabber:iq:gateway?
     if (std::find(features.begin(), features.end(), 
                   "jabber:iq:register") != features.end())
     {
          try {
               G_App.getSession().roster()[item.getJID()].getNickname();
               _agents.push_back(item.getJID());
               cerr << item.getJID() << " added to agents list" << endl;
          } catch (jabberoo::Roster::XCP_InvalidJID& e) {
               cerr << item.getJID() << " not added to agents list -- not in roster" << endl;
          }
     }
     else
          cerr << item.getJID() << " not added to agents list -- lacks jabber:iq:register" << endl;
}

void GabberApp::on_queue_flushing()
{
    PacketQueue::queue_iterator it = _pqueue->begin();
    while (it != _pqueue->end())
    {
        PacketQueue::queue_iterator next = it;
        ++next;

        if (it->type == "PresenceRequest")
        {
            _pqueue->pop(it);
        }

        it = next;
    }
}

GabberApp& GabberApp::getSingleton(void)
{
    return Singleton<GabberApp>::getSingleton();
}

