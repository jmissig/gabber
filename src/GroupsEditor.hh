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
 *  Copyright (c) 2003 Julian Missig
 */

#ifndef INCL_GROUPSEDITOR_HH
#define INCL_GROUPSEDITOR_HH

#include "BaseGabberWindow.hh"

#include <jabberoo/roster.hh>

#include <gtkmm/treemodel.h>

namespace Gabber {

class JabberConnection;
class PrettyJid;

/**
 * Edit Groups Dialog.
 * This dialog is where the user can edit all the groups for 
 * a specific contact.
 */
class EditGroupsDlg : public BaseGabberWindow
{
public:
     /**
      * Display an EditGroupsDlg for the given JabberID.
      * @param conn The JabberConnection (GabberApp) we're using.
      * @param parent The parent Window.
      * @param jid The JabberID to get info for.
      */
     static void display(JabberConnection& conn, Gtk::Window& parent,
			 const Glib::ustring& jid);

protected:
     EditGroupsDlg(JabberConnection& conn, Gtk::Window& parent,
		   const Glib::ustring& jid);
     void get_groups();
     void on_available_cursor_changed();
     void on_Add_clicked();
     void on_Remove_clicked();

private:
     class ModelColumns : public Gtk::TreeModel::ColumnRecord
     {
     public:
	  ModelColumns()
	       { add(group); }
	  Gtk::TreeModelColumn<Glib::ustring> group;
     };

     JabberConnection& _conn;
     Glib::ustring     _jid;
     jabberoo::Roster::Item _item;

     Gtk::Button*      _btnAdd;
     Gtk::Button*      _btnRemove;
     Gtk::Entry*       _entGroup;

     Gtk::TreeView*    _available_treeview;
     Glib::RefPtr<Gtk::ListStore> _available_store;
     Gtk::TreeView*    _current_treeview;
     Glib::RefPtr<Gtk::ListStore> _current_store;
     ModelColumns      _columns;
}; // class EditGroupsDlg

} // namespace Gabber

#endif // INCL_GROUPSEDITOR_HH
