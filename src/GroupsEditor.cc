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

#include "GroupsEditor.hh"

#include "JabberConnection.hh"

#include "GabberUtility.hh"
#include "GabberWidgets.hh"

#include "intl.h"

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include <vector>

namespace Gabber {

void EditGroupsDlg::display(JabberConnection& conn, Gtk::Window& parent,
			    const Glib::ustring& jid)
{
     EditGroupsDlg* e = new EditGroupsDlg(conn, parent, jid);
     e->show();
}

EditGroupsDlg::EditGroupsDlg(JabberConnection& conn, Gtk::Window& parent,
			     const Glib::ustring& jid)
     : BaseGabberWindow("EditGroups_dlg"), _conn(conn), _jid(jid),
       _item(conn.getSession().roster()[jid])
{
     // Set the dialog's parent
     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(parent);

     get_widget("Add_btn", _btnAdd);
     _btnAdd->signal_clicked().connect(SigC::slot(*this, &EditGroupsDlg::on_Add_clicked));
     get_widget("Remove_btn", _btnRemove);
     _btnRemove->signal_clicked().connect(SigC::slot(*this, &EditGroupsDlg::on_Remove_clicked));
    
     get_widget("NewGroup_ent", _entGroup);
     
     Gtk::Button* b;
     get_widget("Close_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &EditGroupsDlg::close));

     PrettyJID* pj = manage(new PrettyJID(_jid, "", PrettyJID::dtNickJID, false));
     pj->show();

     Gtk::HBox* hb;
     get_widget("JIDInfo_hbox", hb);
     hb->pack_start(*pj, true, true, 0);

     // Set the window title
     _thisWindow->set_title(Util::substitute(_("%s's Groups"), pj->get_nickname()));

     // Attempt to size the group lists in a similar fashion
     Gtk::Frame* f;
     Glib::RefPtr<Gtk::SizeGroup> client_grp = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
     get_widget("Available_frm", f);
     client_grp->add_widget(*f);
     get_widget("Current_frm", f);
     client_grp->add_widget(*f);

     // Available Groups
     get_widget("Available_treeview", _available_treeview);
     _available_store = Gtk::ListStore::create(_columns);
     _available_treeview->set_model(_available_store);

     Gtk::TreeView::Column* column = 
	  Gtk::manage(new Gtk::TreeView::Column(_("Available Groups")));
     column->pack_start(_columns.group);
     _available_treeview->append_column(*column);

     _available_treeview->signal_cursor_changed().connect(SigC::slot(*this, &EditGroupsDlg::on_available_cursor_changed));
     
     // Current Groups
     get_widget("Current_treeview", _current_treeview);
     _current_store = Gtk::ListStore::create(_columns);
     _current_treeview->set_model(_current_store);

     column =
          Gtk::manage(new Gtk::TreeView::Column(_("Current Groups")));
     column->pack_start(_columns.group);
     _current_treeview->append_column(*column);

     get_groups();
}

void EditGroupsDlg::get_groups()
{
     _available_store->clear();
     _current_store->clear();
     
     // Build a list of available groups
     typedef map<string, set<string> > GMAP;
     const GMAP grps = _conn.getSession().roster().getGroups();
     for (GMAP::const_iterator it = grps.begin(); it != grps.end(); ++it)
     {
	  // Don't display virtual groups
	  if (it->first == "Unfiled" || it->first == "Pending" || it->first == "Agents")
	       continue;
	  
	  Gtk::TreeModel::Row row = *_available_store->append();
	  row[_columns.group] = it->first;
     }

     // Load current group information
     for (jabberoo::Roster::Item::iterator it = _item.begin(); it != _item.end(); ++it)
     {
	  // Don't display virtual groups
	  if (*it == "Unfiled" || *it == "Pending" || *it == "Agents")
	  {
	       // XXX: fix a bug.. we'll keep this in for a little bit. can remove later.
	       _item.delFromGroup("Unfiled");
	       continue;
	  }
	
	  Gtk::TreeModel::Row row = *_current_store->append();
          row[_columns.group] = *it;
     }
}

void EditGroupsDlg::on_available_cursor_changed()
{
     Glib::ustring groupname = "";
     Gtk::TreeRow row = *(_available_treeview->get_selection()->get_selected());
     row.get_value(0, groupname);
     _entGroup->set_text(groupname);
}

void EditGroupsDlg::on_Add_clicked()
{
     _item.addToGroup(_entGroup->get_text());
     _conn.getSession().roster() << _item;
     get_groups();
}

void EditGroupsDlg::on_Remove_clicked()
{
     Glib::ustring groupname = "";
     Gtk::TreeRow row = *(_current_treeview->get_selection()->get_selected());
     row.get_value(0, groupname);
     if (!groupname.empty())
     {
          _item.delFromGroup(groupname);
          _conn.getSession().roster() << _item;
          get_groups();
     }
}


} // namespace Gabber
