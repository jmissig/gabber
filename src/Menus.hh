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

#ifndef INCL_MENUS_HH
#define INCL_MENUS_HH

#include "Singleton.hh"
#include "Environment.hh"
#include <glade/glade-xml.h>
#include <gtkmm/menu.h>

namespace Gabber
{

/**
 * Menus that are independent of other widgets
 */
namespace Popups {

/**
 * Handles the base popup menu methods
 */
class Base : public Gtk::Menu {
public:
    /**
     * Loads the menu and sets up intial state
     * @param menu_name The glade name to load.
     */
    Base(const Glib::ustring& menu_name);
    virtual ~Base();
    /**
     * Adds a menu item to the beginning of the menu.
     * @param mi The menu item to append
     */
    void addItem(Gtk::MenuItem* mi);
    /**
     * Retrieves a menu item
     * This can only access menu items that are in the glade file, not ones
     * added using addItem()
     * @param item_name The name of the menu item in glade.
     * @returns Gtk::MenuItem* The selected menu item.
     */
    Gtk::MenuItem* getItem(const Glib::ustring& item_name);
private:
    GladeXML* _xml;
    int _next_item_pos;
};

/**
 * Popup menu for the roster users
 */
class User : public Singleton<User>, public Base {
public:
    User();
     void default_activate(const Glib::ustring& jid);
     void jid_popup(const Glib::ustring& jid, guint button, guint32 active_time);

    /**
     * Return the currently selected JID
     * @returns const Glib::ustring& The currently selected JID
     */
    const Glib::ustring& getSelectedJID() { return _current_jid; }

    static User& getSingleton();
private:
    Glib::ustring _current_jid; // The JID that was most recently highlighted/selected
};

/**
 * Action menu in Gabber window.
 */
class ActionMenu : public Singleton<ActionMenu>, public Base {
public:
     ActionMenu();
     
     static ActionMenu& getSingleton();
private:
};

}; // namespace Popups

}; // namespace Gabber
#endif // INCL_MENUS_HH
