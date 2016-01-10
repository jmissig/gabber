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

#include "RosterView.hh"

#include "GabberApp.hh"
#include "ContactInfo.hh"
#include "Environment.hh"
#include "JabberConnection.hh"
#include "GabberUtility.hh"
#include "Menus.hh"
#include "FileTransferSendDlg.hh"
#include "Configurator.hh"
#include "ConfigPaths.hh"
#include "GroupsEditor.hh"
#include "LogViewer.hh"
#include "GabberUtility.hh"

#include "intl.h"
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treerowreference.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/window.h>

namespace Gabber {

// -------------------------------------------------------------------
//
// GabberWin's Roster View
//
// -------------------------------------------------------------------

RosterView::RosterView(JabberConnection& jc, Gtk::TreeView* treeview, 
    Glib::RefPtr<RosterController> rostercontroller, Gtk::Window* parent) : 
    _jabber_connection(jc), _treeview(treeview), 
    _rostercontroller(rostercontroller), _parent(parent)
     
{
     // Set the model
     _treeview->set_model(_rostercontroller);

     _rselection = _treeview->get_selection();
     _rselection ->set_mode(Gtk::SELECTION_BROWSE);

     add_columns();
     _treeview->signal_cursor_changed().connect(
             SigC::slot(*this, &RosterView::on_signal_cursor_changed));

     // Connect to the row activation
     _treeview->signal_row_activated().connect(SigC::slot(*this,
                 &RosterView::on_row_activated));

     // Connect to a button press signal the gtkmm way (Doesn't work): 
     _treeview->signal_button_press_event().connect(
             SigC::slot(*this, &RosterView::on_button_press_event), false);

     _treeview->signal_event().connect(
             SigC::slot(*this, &RosterView::on_event));
    _treeview->signal_row_expanded().connect(
        SigC::slot(*this, &RosterView::on_row_expanded));
    _treeview->signal_row_collapsed().connect(
        SigC::slot(*this, &RosterView::on_row_collapsed));

    // Setup the menu
    _user_menu = new Popups::User();
    Popups::User::getSingleton().getItem("get_info")->signal_activate().connect(SigC::slot(*this, &RosterView::on_view_info));
    Popups::User::getSingleton().getItem("delete_this_contact")->signal_activate().connect(SigC::slot(*this, &RosterView::on_delete_contact));
    Popups::User::getSingleton().getItem("send_file")->signal_activate().connect(SigC::slot(*this, &RosterView::on_send_file));
    Popups::User::getSingleton().getItem("edit_groups")->signal_activate().connect(SigC::slot(*this, &RosterView::on_edit_groups));
    Popups::User::getSingleton().getItem("read_log")->signal_activate().connect(SigC::slot(*this, &RosterView::on_read_log));

    _rostercontroller->signal_row_inserted().connect(SigC::slot(*this,
        &RosterView::on_row_inserted));

    // Configure the collapsed groups
    Configurator& config = GabberApp::getSingleton().getConfigurator();
    const std::list<Glib::ustring>&
        closed_groups(config.get_string_list(Keys::acct.closed_groups));
    _closed_groups.insert(closed_groups.begin(), closed_groups.end());
}

RosterView::~RosterView()
{
    delete _user_menu;
}

void RosterView::add_columns()
{
     // Show Icon and Name
    {
        Gtk::TreeView::Column* pColumn = Gtk::manage( new
            Gtk::TreeView::Column("Name") );

        pColumn->pack_start(_rostercontroller->columns.icon, false);
        pColumn->pack_start(_rostercontroller->columns.name, true);

        std::vector<Gtk::CellRenderer*> rends = pColumn->get_cell_renderers();
        pColumn->clear_attributes(*rends[1]);
        pColumn->add_attribute(*rends[1], "markup", 1);
        Glib::RefPtr<Gtk::Style> style = _treeview->get_style();
        _rostercontroller->set_default_colors(style->get_fg(Gtk::STATE_NORMAL),
            style->get_base(Gtk::STATE_NORMAL),
            style->get_fg(Gtk::STATE_SELECTED),
            style->get_base(Gtk::STATE_SELECTED));
        pColumn->add_attribute(*rends[1], "foreground-gdk", 5);
        pColumn->add_attribute(rends[0]->property_visible(),
                               _rostercontroller->columns.use_icon);

        _treeview->append_column(*pColumn);
    }
}

bool RosterView::on_event(GdkEvent* e)
{ 
    if(e->type == GDK_BUTTON_PRESS && e->button.button == 3) 
    {
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col;
        int cx, cy;
        bool ret = _treeview->get_path_at_pos((int)e->button.x, 
            (int)e->button.y, path, col, cx, cy);
        if ( ret )
        {
            // See if we snagged us a user or group.
            Gtk::TreeModel::iterator iter = _rostercontroller->get_iter(path);
            if ( !iter->get_value(_rostercontroller->columns.is_group) )
            {
                Popups::User::getSingleton().jid_popup(
                    iter->get_value(_rostercontroller->columns.jid), 
                    e->button.button, e->button.time);
            }
        }
    }

    return false;
}

void RosterView::on_row_activated(const Gtk::TreeModel::Path& path, 
        Gtk::TreeViewColumn* column)
{
    Gtk::TreeModel::iterator iter = _rostercontroller->get_iter(path);
    // It's a group, toggle its state
    if (iter->get_value(_rostercontroller->columns.is_group))
    {
        Glib::ustring name = iter->get_value(_rostercontroller->columns.name);
        if (_treeview->row_expanded(path))
        {
            _treeview->collapse_row(path);
        }
        else
        {
            _treeview->expand_row(path, false);
        }
        return;
    }

    Glib::ustring jid = iter->get_value(_rostercontroller->columns.jid);
    // Do what the menu wants us to do!
    if (G_App.getPacketQueue().isQueued(jid))
    {
        G_App.getPacketQueue().pop(jid);
    }
    else
    {
        Popups::User::getSingleton().default_activate(jid);
    }
}

bool RosterView::on_button_press_event(GdkEventButton* b) { 
    return false;
}

// This will be called when a row in the roster view is highlighted
void RosterView::on_signal_cursor_changed()
{
    Gtk::TreeRow row    = *(_treeview->get_selection()->get_selected());
    Glib::ustring roster_jid;
    row.get_value(0, roster_jid);
    _rostercontroller->set_selected_jid(roster_jid);
}

void RosterView::on_view_info()
{
     const Glib::ustring& cur_jid = Popups::User::getSingleton().getSelectedJID();
     ContactInfoDlg::display(_jabber_connection, *_parent, cur_jid);
}

void RosterView::on_send_file()
{
    const Glib::ustring& cur_jid = Popups::User::getSingleton().getSelectedJID();
    new FileTransferSendDlg(cur_jid);
}

void RosterView::on_edit_groups()
{
     const Glib::ustring& cur_jid = Popups::User::getSingleton().getSelectedJID();
     EditGroupsDlg::display(_jabber_connection, *_parent, cur_jid);
}

void RosterView::on_read_log()
{
    LogViewer* lw = new LogViewer(Popups::User::getSingleton().getSelectedJID());
    lw->show();
}

void RosterView::on_delete_contact()
{
    jabberoo::Session& session = _jabber_connection.getSession();
    jabberoo::Roster& roster = _jabber_connection.getSession().roster();
    const Glib::ustring& cur_jid = Popups::User::getSingleton().getSelectedJID();
    jabberoo::Roster::Item item = roster[cur_jid];
    jabberoo::Roster::Subscription s10n = item.getSubsType();

    // Lookup nickname
    Glib::ustring nickname;
    // XXX: Maybe this should be in Popups::User::getSingleton() as well?
    // attempt to look it up
    try {
	 nickname = _jabber_connection.getSession().roster()[cur_jid].getNickname();
    } catch (jabberoo::Roster::XCP_InvalidJID& e) {
	 // the default nickname is the username
	 nickname = jabberoo::JID::getUser(cur_jid);
    }

    // Confirmation Dialog
    Util::MessageDialog dlg(*_parent, 
			   Util::substitute(_("Are you sure you want to remove %s (%s) from your Contact List?"), nickname, cur_jid),
			   Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
    dlg.set_title(Util::substitute(_("Remove %s from Contact List"), nickname));
    dlg.add_button(_("_Don't Remove"), Gtk::RESPONSE_NO);
    dlg.add_button(Gtk::Stock::REMOVE, Gtk::RESPONSE_YES);

    int ret = dlg.run();
    dlg.hide();
    if (ret == Gtk::RESPONSE_YES)
    {
	 if (s10n == jabberoo::Roster::rsFrom || s10n == jabberoo::Roster::rsBoth)
	 {
	      session << jabberoo::Presence(cur_jid,
					    jabberoo::Presence::ptUnsubscribed);
	 }
	 roster.deleteUser(cur_jid);
    }
}

void RosterView::on_row_inserted(const Gtk::TreeModel::Path& path, 
                                 const Gtk::TreeModel::iterator& iter)
{
    if ( path.size() > 1 )
    {
        const Gtk::TreeModel::iterator& piter(iter->parent());
        const Gtk::TreeModel::Path& ppath(_rostercontroller->get_path(piter));
        std::string pname(piter->get_value(_rostercontroller->columns.name));
        if (!_treeview->row_expanded(ppath) && 
            (_closed_groups.find(pname) == _closed_groups.end()) )
        {
            _treeview->expand_row(ppath, false);
        }
    }
}

void RosterView::on_row_expanded(const Gtk::TreeModel::iterator& iter,
                                 const Gtk::TreeModel::Path& path)
{
    _closed_groups.erase(iter->get_value(_rostercontroller->columns.name));
    typedef std::list<Glib::ustring> slGu_t;
    slGu_t closed_groups;
    for (ssGu_t::iterator it = _closed_groups.begin(); 
         it != _closed_groups.end(); ++it)
    {
        closed_groups.push_back(*it);
    }
    Configurator& config = GabberApp::getSingleton().getConfigurator();
    config.set(Keys::acct.closed_groups, closed_groups);

}

void RosterView::on_row_collapsed(const Gtk::TreeModel::iterator& iter,
                                  const Gtk::TreeModel::Path& path)
{
    _closed_groups.insert(iter->get_value(_rostercontroller->columns.name));
    typedef std::list<Glib::ustring> slGu_t;
    slGu_t closed_groups;
    for (ssGu_t::iterator it = _closed_groups.begin(); 
         it != _closed_groups.end(); ++it)
    {
        closed_groups.push_back(*it);
    }
    Configurator& config = GabberApp::getSingleton().getConfigurator();
    config.set(Keys::acct.closed_groups, closed_groups);

}

} // namespace Gabber
