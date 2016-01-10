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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "GabberWin.hh"

#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "RosterView.hh"
#include "RosterController.hh"
#include "Menus.hh"
#include "PrefsInterface.hh"
#include "PacketQueueView.hh"

#include "AddContactDlg.hh"
#include "ContactInfo.hh"

#include "intl.h"
#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/menu.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

using namespace jabberoo;

namespace Gabber {

GabberWin::GabberWin(GabberApp& app) : 
    BaseGabberWindow("Gabber_win"), _app(app), 
    _prevPresence("", Presence::ptAvailable, Presence::stOnline)
{
//     _thisWindow->realize();
     _thisWindow->hide();
          
     Configurator& config = GabberApp::getSingleton().getConfigurator();     
#ifndef WIN32
     // FIXME: Maybe support Gnome Session Management and tell the window manager to save position there.
     // this would have to somehow be automatically compiled in or not
     // and if not, then we do this crap anyway:
     int tempx = config.get_int(Keys::intrface.contactlist_size_width);
     int tempy = config.get_int(Keys::intrface.contactlist_size_height);
     _thisWindow->set_default_size(tempx, tempy);
     _thisWindow->resize(tempx, tempy);
     
     tempx = config.get_int(Keys::intrface.contactlist_pos_x);
     tempy = config.get_int(Keys::intrface.contactlist_pos_y);
     if (tempx >= 0 && tempy >= 0)
          _thisWindow->move(tempx, tempy);
#endif
     
     // Setup roster
     bool hide_offline = config.get_bool(Keys::intrface.contactlist_hideoffline);
     
     _rostercontroller = RosterController::create(_app.getSession(), hide_offline);

     Gtk::TreeView* tv;
     get_widget("Roster_treeview", tv);
     _rosterview  = new RosterView(_app, tv, _rostercontroller, _thisWindow);

     // Setup Packet Queue View
     Gtk::ScrolledWindow* sw;
     get_widget("PacketQueue_scroll", sw);
     get_widget("PacketQueue_treeview", tv);
     _packetqueueview = new PacketQueueView(*sw, *tv);
     
     // Initialize pointers
     get_widget("Presence_optmenu", _optPres);
     
     // Status area
     get_widget("StatusMsg_scroll", _scrollStatus);
     get_widget("StatusMsg_lbl", _lblStatus);
     get_widget("StatusMsg_txt", _txtStatus);
     get_widget("StatusBack_btn", _btnBack);

     _txtStatus->signal_event().connect(SigC::slot(*this, &GabberWin::on_status_txt_event));
     _txtStatus->signal_focus_out_event().connect(SigC::slot(*this, &GabberWin::on_status_txt_unfocused));
     _btnBack->signal_clicked().connect(SigC::slot(*this, &GabberWin::set_back));

     get_widget("Gabber_statusbar", _statusbar);

     // Presence events
     _app.getSession().evtMyPresence.connect(SigC::slot(*this, &GabberWin::on_my_presence_event));
     _app.getSession().evtPresence.connect(SigC::slot(*this, &GabberWin::on_evtPresence));

     _app.evtConnecting.connect(  SigC::slot(*this, &GabberWin::on_evtConnecting));
     _app.evtConnected.connect(   SigC::slot(*this, &GabberWin::on_evtConnected));
     _app.evtDisconnected.connect(SigC::slot(*this, &GabberWin::on_evtDisconnected));

     // Hook up all the menus
     init_menus();
     
     show();
}

GabberWin::~GabberWin()
{
     Configurator& config = GabberApp::getSingleton().getConfigurator();   
     int tempx, tempy;
     _thisWindow->get_position(tempx, tempy);
     config.set(Keys::intrface.contactlist_pos_x, tempx);
     config.set(Keys::intrface.contactlist_pos_y, tempy);
     _thisWindow->get_size(tempx, tempy);
     config.set(Keys::intrface.contactlist_size_width, tempx);
     config.set(Keys::intrface.contactlist_size_height, tempy);
     
     delete _action_menu;
     delete _packetqueueview;
     delete _rosterview;
}

void GabberWin::init_menus() {
    Gtk::MenuItem* mi;
    // Gabber
    get_widget("Quit_item", mi);
    mi->signal_activate().connect(SigC::slot(*this, &GabberWin::close));
    get_widget("Login_item", mi);
    mi->set_sensitive(true);
    mi->signal_activate().connect(SigC::slot(&LoginDlg::execute));
    get_widget("Logout_item", mi);
    mi->set_sensitive(false);
    mi->signal_activate().connect(SigC::slot(_app, &GabberApp::clean_disconnect));
    get_widget("ConnectionSettings_item", mi);
    mi->set_sensitive(false);
    mi->signal_activate().connect(SigC::slot(&LoginDlg::execute_as_settings));
    get_widget("MyInfo_item", mi);
    mi->set_sensitive(false);
    mi->signal_activate().connect(SigC::slot(&MyInfoDlg::execute));
    get_widget("Preferences_item", mi);
    mi->signal_activate().connect(SigC::slot(&PrefsWin::execute));
    // Actions
    _action_menu = new Popups::ActionMenu();
    _action_menu->getItem("AddContact_item")->signal_activate().connect(SigC::slot(*this, &GabberWin::on_add_contacts_activate));
    _action_menu->getItem("JoinGroupChat_item")->signal_activate().connect(SigC::slot(*this,
        &GabberWin::on_joingc_activate));
    get_widget("Actions_menu", mi);
    mi->set_submenu(*_action_menu);
    
    // Roster Stuff
    Gtk::CheckMenuItem* cmi;
    get_widget("OfflineContacts_item", cmi);
    cmi->signal_activate().connect(SigC::slot(*this,
        &GabberWin::on_roster_hideoffline_activate));
    cmi->set_active(_app.getConfigurator().get_bool(Keys::intrface.contactlist_hideoffline));
    
    // Help menu
    get_widget("About_item", mi);
    mi->signal_activate().connect(SigC::slot(*this,
          &GabberWin::on_about_activate));

    // Presence option menu
    _optPres->set_sensitive(false);
    _optPres->get_menu()->set_sensitive(false);
    _optPres->set_history(1);
    _optPres_changed = _optPres->signal_changed().connect(SigC::slot(*this,
                &GabberWin::on_pres_menu_changed));
}

void GabberWin::close() 
{
     _app.quit();
     //BaseGabberWindow::close();
}

void GabberWin::push_statusbar_msg(const Glib::ustring& msg, int millisecs)
{
     // pop!
     _statusbar->pop();
     // disconnect an existing timer (we just popped)
     if (_statusbar_pop_timer.connected())
	  _statusbar_pop_timer.disconnect();

     // push the new message
     _statusbar->push(msg);
     
     // If we want to pop in a certain number of milliseconds,
     if (millisecs != 0)
     {
          // hook up the new timer
          _statusbar_pop_timer = Glib::signal_timeout().connect(SigC::slot(*this, &GabberWin::pop_statusbar), millisecs);
     }
}

bool GabberWin::pop_statusbar()
{
     // pop!
     _statusbar->pop();

     // disconnect an existing timer
     if (_statusbar_pop_timer.connected())
	  _statusbar_pop_timer.disconnect();

     return false;
}

void GabberWin::set_back()
{
     _app.getSession() << _prevPresence;
     
     if (_app.getConfigurator().get_bool(Keys::intrface.messages_autodisplay))
     {
          _app.getPacketQueue().flush();
     }
}

void GabberWin::toggle_visibility()
{
     if (_thisWindow->is_visible())
     {
	  // Save the window position
	  _thisWindow->get_position(_gwin_x, _gwin_y);

	  // Hide it
	  _thisWindow->hide();
     }
     else
     {
	  // restore the position we saved
	  _thisWindow->move(_gwin_x, _gwin_y);

	  // Show it
	  _thisWindow->show();
     }
}

void GabberWin::on_evtConnecting()
{
     cout << "GabberWin::on_evtConnecting" << endl;
     push_statusbar_msg(_("Connecting..."));

     Gtk::MenuItem* mi;
     get_widget("Login_item", mi);
     mi->set_sensitive(false); // don't let the user try to log in again.
     get_widget("Logout_item", mi);
     mi->set_sensitive(true); // allow the user the option to log out.
     get_widget("ConnectionSettings_item", mi);
     mi->set_sensitive(true);
     get_widget("MyInfo_item", mi);
     mi->set_sensitive(true);
     
}

void GabberWin::on_evtConnected()
{
     cout << "GabberWin::on_evtConnected" << endl;
     push_statusbar_msg(_("Connected"), 5000);

     // We can now set presence. Yey.
     _optPres->set_sensitive(true);
     _optPres->get_menu()->set_sensitive(true);

     gchar* priority = g_strdup_printf("%d", _app.getConfigurator().get_int(Keys::acct.priority));
     _app.getSession() << Presence("", Presence::ptAvailable,
                                   Presence::stOnline, std::string(),
                                   std::string(priority));
     g_free(priority);

     
    // And now.. the HACKS
    // This is one of the oldest HACKS in Gabber's existence
    // and yet, Jabber still has yet to provide a nice way around this.
    // How do we identify which roster items are Gateways without browsing to everyone?
    // ...
    // ...
    // That's right--we guess. If it doesn't have an @, then hey, maybe it's a Gateway.
    // this hack brought to you by Julian Missig, maker of fine Jabber hacks since 0.7.

    // Iterate over the whole roster
    for (Roster::iterator it = _app.getSession().roster().begin();
         it != _app.getSession().roster().end(); ++it)
    {
        // there's no @
        if(it->getJID().find("@") == string::npos)
        {
            // so maybe we have a Gateway?
            // this time around we can at least check..
            _app.getSession().discoDB().cache(it->getJID(), 
                SigC::slot(*this, &GabberWin::on_discoDB_cache));
        }
    }
}

void GabberWin::on_discoDB_cache(const jabberoo::DiscoDB::Item* item)
{
    std::cout << "Cached agent: " << item->getJID() << std::endl;
     // XXX: This doesn't seem to get called when it falls back on browse?
     // I don't think we particularly need to do anything here yet, but still...
}

void GabberWin::on_evtDisconnected()
{
     push_statusbar_msg(_("Disconnected"));

     Gtk::MenuItem* mi;
     get_widget("Login_item", mi);
     mi->set_sensitive(true); // let the user try to log in again.
     get_widget("Logout_item", mi);
     mi->set_sensitive(false); // don't allow the user the option to log out.
     get_widget("ConnectionSettings_item", mi);
     mi->set_sensitive(false);
     get_widget("MyInfo_item", mi);
     mi->set_sensitive(false);

     // We can't set presence anymore
     _optPres->set_sensitive(false);
     _optPres->get_menu()->set_sensitive(false);

     _packetqueueview->clear();

     _rostercontroller->clear();
}

void GabberWin::on_my_presence_event(const Presence& p)
{
     // temporarily disable presence changing signal
     _optPres_changed.disconnect();
     
     // Set the presence menu
     switch(p.getShow())
     {
     case Presence::stChat:
	  _optPres->set_history(0);
       _btnBack->hide();
       _prevPresence = Presence(p.getBaseElement());
	  break;
     case Presence::stAway:
	  _optPres->set_history(3);
       _btnBack->show();
	  break;
     case Presence::stXA:
	  _optPres->set_history(4);
       _btnBack->show();
	  break;
     case Presence::stDND:
	  _optPres->set_history(2);
       _btnBack->show();
	  break;
     case Presence::stOffline:
       _optPres->set_history(1);
       _btnBack->hide();   
     default:
	  _optPres->set_history(1);
       _btnBack->hide();
       _prevPresence = Presence(p.getBaseElement());
	  break;
     };

     // Figure out how tall the status label should be

     // Figure out the line height
     // THEME/FONT SPECIFIC
//     Glib::RefPtr<Pango::Layout> playout = Pango::Layout::create(_txtStatus->get_pango_context());
//     Pango::Rectangle inkrect, logicalrect;
//     playout->get_pixel_extents(inkrect, logicalrect);

     // Set the label height to be line height times multiplier
//     _txtStatus->set_size_request(-1, logicalrect.get_height() * multiplier);

     // Set the presence status label
     _lblStatus->set_label(p.getStatus());

     if (p.getStatus().empty())
     {
          _lblStatus->hide();
     }
     else
     {
          _lblStatus->show();
     }
     
     push_statusbar_msg(Util::substitute(_("You are now %s"), Util::getShowName(p.getShow())), 5000);

     // re-enable presence changing signal
     _optPres_changed = _optPres->signal_changed().connect(SigC::slot(*this, &GabberWin::on_pres_menu_changed));
}

void GabberWin::on_evtPresence(const Presence& p, Presence::Type prev_type)
{
     Glib::ustring nickname;
     // attempt to lookup nickname in the roster
     try {
	  nickname = _app.getSession().roster()[JID::getUserHost(p.getFrom())].getNickname();
     } catch (Roster::XCP_InvalidJID& e) {
          // attempt to lookup nickname in the roster using full jid
          try {
               nickname = _app.getSession().roster()[p.getFrom()].getNickname();
          } catch (Roster::XCP_InvalidJID& e) {
               // this presence isn't in roster. screw it.
               return;
          }
     }

     try {
          // Walk the list of resources in the presencedb
          PresenceDB::range r = _app.getSession().presenceDB().equal_range(p.getFrom());
          for (PresenceDB::const_iterator it = r.first; it != r.second; it++)
	  {
	       const Presence& pit = *it;
               
               // If this presence is a NA presence, then skip it
	       if (pit.getType() == Presence::ptUnavailable || pit.getType() == Presence::ptError)
		    continue;
               
               // If this is the presence packet we got, skip it
               if (pit.getFrom() == p.getFrom())
                    continue;
           
               // we found another resource available
               // which means this user did not go offline
               // and we've already told them they are online
               return;
          }
     } catch (PresenceDB::XCP_InvalidJID& e) {
     }

     if ((prev_type == Presence::ptUnavailable || prev_type == Presence::ptError)
	 && (p.getType() == Presence::ptAvailable))
     {
	  push_statusbar_msg(Util::substitute(_("%s came online"), nickname), 5000);
     }
     else if ((prev_type == Presence::ptAvailable)
	      && (p.getType() == Presence::ptUnavailable || p.getType() == Presence::ptError))
     {
	  push_statusbar_msg(Util::substitute(_("%s went offline"), nickname), 5000);
     }
}

void GabberWin::on_add_contacts_activate()
{
     // shake that thing
     AddContactDlg::display(_app, *_thisWindow);
}

void GabberWin::on_joingc_activate()
{
    // We actually pass this off to the modules
    _app.sigJoinGCDlg();
}

void GabberWin::on_roster_hideoffline_activate()
{
    Gtk::CheckMenuItem* mi;
    get_widget("OfflineContacts_item", mi);
    _rostercontroller->setHideOffline(mi->get_active());
    _app.getConfigurator().set(Keys::intrface.contactlist_hideoffline, mi->get_active());
}

void GabberWin::on_about_activate()
{
     Gtk::Dialog dlg;
     Gtk::Button okbutton(Gtk::StockID("gtk-ok"));
     Gtk::Label label;
     okbutton.set_flags(Gtk::CAN_FOCUS);
     okbutton.set_relief(Gtk::RELIEF_NORMAL);
     dlg.get_action_area()->property_layout_style().set_value(Gtk::BUTTONBOX_END);
     dlg.get_action_area()->pack_start(okbutton);
     label.set_alignment(0.5,0.5);
     label.set_padding(5,5);
     label.set_justify(Gtk::JUSTIFY_CENTER);
     label.set_line_wrap(true);
     label.set_use_markup(true);
     Glib::ustring translator = _("translator_credits");
     if (translator == "translator_credits")
          translator = Glib::ustring();
     else
          translator = Util::substitute(_("Generously translated by: %s\n"), translator);
     label.set_markup("<span size='xx-large'>" + ENV_VARS.package + " " + ENV_VARS.version + "</span>\n\n" 
                      + _("Extensible easy-to-use Jabber client.") + "\n\n"
                      + _("User Interface Design &amp; Programming: Julian Missig") + "\n"
                      + _("Extensive Programming: Thomas Muldowney") + "\n"
                      + translator
                      + "\n<span size='small'>Copyright © Julian Missig 2002-2004</span>");
     dlg.get_vbox()->set_homogeneous(false);
     dlg.get_vbox()->set_spacing(0);
     dlg.get_vbox()->pack_start(label, Gtk::PACK_SHRINK, 0);
     dlg.property_window_position().set_value(Gtk::WIN_POS_NONE);
     dlg.set_resizable(false);
     okbutton.show();
     label.show();
     dlg.show();
     dlg.set_title(_("About Gabber"));
     okbutton.signal_clicked().connect(SigC::slot(dlg, &Gtk::Widget::hide));
     dlg.run();
}

void GabberWin::on_pres_menu_changed()
{
     // The text view is now editable
     _lblStatus->hide();
     _btnBack->hide();

     Glib::RefPtr<Gtk::TextBuffer> statbuff = _txtStatus->get_buffer();
     // Set a default status message
     switch(_optPres->get_history())
     {
          case 1:
               statbuff->set_text(Glib::ustring());
               break;
          case 0:
               statbuff->set_text(_("Talk to me!"));
               break;
          case 3:
               statbuff->set_text(_("I'm not here, I'll be back soon."));
               break;
          case 4:
               statbuff->set_text(_("I'll be away for a while."));
               break;
          case 2:
               statbuff->set_text(_("Sorry, I'm busy."));
               break;
          case 5:
               statbuff->set_text(_("Invisible to everyone else."));
               break;
     };

     // select the text
     statbuff->move_mark(statbuff->get_selection_bound(), statbuff->begin());
     _scrollStatus->show();
     
     // Focus on the text view
     _txtStatus->grab_focus();
}

bool GabberWin::on_status_txt_event(GdkEvent* ev) 
{
    if (ev->type != GDK_KEY_PRESS)
    return false;

    GdkEventKey* e = (GdkEventKey*)ev;

    // If they pressed the Keypad enter, make it act like a normal enter
    if (e->keyval == GDK_KP_Enter)
        e->keyval = GDK_Return;

    if (e->keyval == GDK_Return)
    {
        //enter a newline if shift-return is used
        if (e->state & GDK_SHIFT_MASK)
        {
            //unset the shift bit. shift-return seems to have a special meaning for the widget
            e->state ^= GDK_SHIFT_MASK;
            return false;
        }

        // Ctrl-Return sets status so we don't need to check for it

        set_status_from_status_txt();
        return true;
    }

    return false;
}

bool GabberWin::on_status_txt_unfocused(GdkEventFocus* e)
{
     if (_scrollStatus->is_visible())
	  set_status_from_status_txt();

     return false;
}

void GabberWin::set_status_from_status_txt()
{
     Glib::RefPtr<Gtk::TextBuffer> tb = _txtStatus->get_buffer();
     Glib::ustring statusmsg = tb->get_text(tb->begin(), tb->end());

     gchar* priority = g_strdup_printf("%d", _app.getConfigurator().get_int(Keys::acct.priority));
     
     // Send the appropriate presence
     switch(_optPres->get_history())
     {
     case 1:
	  _app.getSession() << Presence("", Presence::ptAvailable,
                                 Presence::stOnline, statusmsg,
                                 std::string(priority));
	  break;
     case 0:
	  _app.getSession() << Presence("", Presence::ptAvailable,
                                 Presence::stChat, statusmsg,
                                 std::string(priority));
	  break;
     case 3:
	  _app.getSession() << Presence("", Presence::ptAvailable,
                                 Presence::stAway, statusmsg,
                                 std::string(priority));
	  break;
     case 4:
	  _app.getSession() << Presence("", Presence::ptAvailable,
                                 Presence::stXA, statusmsg,
                                 std::string(priority));
	  break;
     case 2:
	  _app.getSession() << Presence("", Presence::ptAvailable,
                                 Presence::stDND, statusmsg,
                                 std::string(priority));
	  break;
     case 5:
	  _app.getSession() << Presence("", Presence::ptInvisible,
                                 Presence::stOnline, statusmsg,
                                 std::string(priority));
	  break;
     };

     g_free(priority);

     // Text view is no longer editable
     _scrollStatus->hide();
     _lblStatus->show();
}

} // namespace Gabber
