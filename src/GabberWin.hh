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
 *  Copyright (c) 2002-2003 Julian Missig
 */

#ifndef INCL_GABBER_WIN_HH
#define INCL_GABBER_WIN_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"
#include "RosterController.hh"
#include "Menus.hh"

#include <jabberoofwd.h>
#include <discoDB.hh>

#include <gtkmm/statusbar.h>

namespace Gabber
{
/**
 * The main Gabber window.
 */
class GabberWin : public BaseGabberWindow
{
public:
     /**
      * Create the Gabber window.
      * This should be done by GabberApp
      * @param app The GabberApp instance we are creating this window for.
      */
     GabberWin(GabberApp& app);
     ~GabberWin();

     /**
      * Push a message to the status bar.
      * This will clear the existing message and push a new one.
      * If millisecs is set, the message will disappear after the 
      * given amount of time.
      * @param msg The message to display.
      * @param millisecs How long the message should be displayed.
      */
     void push_statusbar_msg(const Glib::ustring& msg, int millisecs = 0);
     
     bool pop_statusbar();
     
     /**
      * Set our presence back to available, whatever exactly it was.
      * This is to be used when returning from auto-away or when
      * "I'm Back" is clicked. GabberWin remembers the previous
      * Presence::ptAvailable or Presence::ptChat and sets us to that
      * presence.
      */
      void set_back();
      
     /**
      * Toggle the visibility of the main window.
      */
     void toggle_visibility();

protected:

     /**
      * Connect all the signals from the menu items.
      * Should only be called once by this GabberWin.
      */
     void init_menus();
     /**
      * Close the GabberWin, quitting the GabberApp as well.
      */
     void close();

     // Callbacks
     void on_evtConnecting();
     void on_evtConnected();
     void on_discoDB_cache(const jabberoo::DiscoDB::Item* item);
     void on_evtDisconnected();
     void on_my_presence_event(const jabberoo::Presence& p);
     void on_evtPresence(const jabberoo::Presence& p, jabberoo::Presence::Type prev_type);
     // Menu items
     void on_add_contacts_activate();
     void on_joingc_activate();
     void on_roster_hideoffline_activate();
     void on_about_activate();

    // Widget Callbacks
     void on_pres_menu_changed();
     bool on_status_txt_event(GdkEvent* e);
     bool on_status_txt_unfocused(GdkEventFocus* e);
     void set_status_from_status_txt();
private:
     GabberApp&           _app;
     Glib::RefPtr<RosterController>  _rostercontroller;
     RosterView*          _rosterview;
     PacketQueueView*     _packetqueueview;
     Popups::ActionMenu*  _action_menu;
     Gtk::OptionMenu*     _optPres;
     jabberoo::Presence   _prevPresence;
     Gtk::ScrolledWindow* _scrollStatus;
     Gtk::Label*		 _lblStatus;
     Gtk::TextView*       _txtStatus;
     Gtk::Button*         _btnBack;
     Gtk::Statusbar*      _statusbar;
     int                  _gwin_x;
     int                  _gwin_y;

     SigC::Connection     _optPres_changed;
     SigC::Connection     _statusbar_pop_timer;
}; // class GabberWin

}; // namespace Gabber

#endif // INCL_GABBER_WIN_HH
