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
#ifndef INCL_GABBER_APP_HH
#define INCL_GABBER_APP_HH

#include "fwd.h"

#include "Singleton.hh"
#include "JabberConnection.hh"
#include "Configurator.hh"
#include "GabberUtility.hh"

#include <jabberoo/discoDB.hh>
#include <gtkmm/main.h>


/**
 * Primary Gabber Namespace
 */
namespace Gabber
{

class ModuleManager;
class AutoAway;
class StatusIcon;

/**
 * The main Gabber application object.
 * This class is instantiated by main and is
 * in control of the Gtk::Main loop.
 * It is also the direct control over TCPTransmitter
 * and jabberoo::Session. It therefore has functions
 * such as login and logout which are preferred to
 * accessing jabberoo::Session and TCPTransmitter 
 * directly.
 */
class GabberApp
     : public Singleton<GabberApp>, public JabberConnection
{
public:
     /**
      * The constructor intended for main
      */
     GabberApp(int argc, char** argv);
     ~GabberApp();

     // Modifiers
     /**
      * Start the Gtk::Main loop.
      * This should should only be started by main.
      */
     void run();
     /**
      * Ask the user for their password.
      * Pops up a dialog asking for the account password if they
      * want to autoconnect but don't have a password saved.
      * This will continue the login procedure accordingly
      * (i.e., login if there's a password and drop to login
      * dialog otherwise.)
      */
     void get_password();
     /**
      * Initialize the main Gabber Window.
      * This will make G_Win no longer null.
      */
     void init_win();
     /**
      * Cleanly disconnect, sending offline presence and all.
      */
     void clean_disconnect();
     /**
      * Quit the application, kill the Gtk::Main loop, etc.
      */
     void quit();

     /**
     * Get a reference to the resource manager
     */
     ResourceManager& getResourceManager() const
     { return *_resource_manager; }

     /**
     * Get a reference to the log manager.
     */
     LogManager& getLogManager() const
     { return *_log_manager; }

     /**
     * Get a reference to the configurator.
     */
     Configurator& getConfigurator() const
     { return *_configurator; }

     FileTransferManager& getFileTransferManager() const
     { return *_ft_manager; }

     /**
     * Return a reference to the agent list
     */
     Util::JIDList& getAgentList()
     { return _agents; }

     /**
     * This signal is fired whenever we need to display a groupchat join dialog
     * There are issues with how this is implemented right now, such as multiple
     * handlers could hook up to it, but we really only want one to do so.
     */
     SigC::Signal0<void> sigJoinGCDlg;

     static GabberApp& getSingleton();
protected:
     // Session event handlers
     void on_XML_transmit(const char* XML);
     void on_XML_recv(const char* XML);
     void on_evtDisconnected();
     void on_session_auth_error(int ErrorCode, const char* ErrorMsg);
     void on_presence_request(const jabberoo::Presence& pres);
     void on_presence_unsubscribed(const jabberoo::Presence& pres);
     void on_discodb_filter(const jabberoo::DiscoDB::Item& item);
     void on_queue_flushing();

private:
    Gtk::Main           _GtkMain;
    Configurator*       _configurator;
    GabberWin*          _winMain;
    AutoAway*           _autoaway;
    StatusIcon*         _status_icon;
    ResourceManager*    _resource_manager;
    ModuleManager*      _module_manager;
    LogManager*         _log_manager;
    FileTransferManager* _ft_manager;
    bool _shutting_down;

    Util::JIDList    _agents;

    void finalize(void);
};

/**
* Easier access for all you pansy typists
*/
#define G_App GabberApp::getSingleton()

}; // namespace Gabber

#endif // INCL_GABBER_APP_HH
