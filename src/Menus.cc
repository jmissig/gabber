
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

#include "Menus.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"
#include <JID.hh>

#include <gtk/gtkmenu.h>

namespace Gabber {

namespace Popups
{

Base::Base(const Glib::ustring& menu_name) : Gtk::Menu(
        GTK_MENU(glade_xml_get_widget( 
            (_xml = glade_xml_new(Glib::build_filename(
                G_App.getConfigurator().get_string(Keys::paths.datadir), 
                menu_name + ".glade").c_str(), menu_name.c_str(), "" )),
            menu_name.c_str()))), _next_item_pos(0)
    { }
Base::~Base() { }

void Base::addItem(Gtk::MenuItem* mi) {
    std::cout << "Adding new item" << std::endl;
    mi->show();
    insert(*mi, _next_item_pos);
    _next_item_pos++;
}

Gtk::MenuItem* Base::getItem(const Glib::ustring& item_name)
{
    return static_cast<Gtk::MenuItem*>(Glib::wrap(glade_xml_get_widget(_xml,
                                                        item_name.c_str())));
}

/* Popups::User */

void User::default_activate(const Glib::ustring& jid)
{
     _current_jid = jid;
     items().front().activate();
}

void User::jid_popup(const Glib::ustring& jid, guint button, guint32 active_time)
{
     _current_jid = jid;
     popup(button, active_time);
}

User::User() : Base("Roster_menu_user") { }

User& User::getSingleton(void)
{
    return Singleton<User>::getSingleton();
}

/* Popups::ActionMenu */

ActionMenu::ActionMenu() : Base("Roster_Action_menu") { }

ActionMenu& ActionMenu::getSingleton(void)
{
     return Singleton<ActionMenu>::getSingleton();
}

} // namespace Popups

} // namespace Gabber
