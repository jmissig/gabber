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

#ifndef INCL_ROSTER_VIEW_HH
#define INCL_ROSTER_VIEW_HH

#include "fwd.h"
#include "RosterController.hh"
#include "JabberConnection.hh"
#include "Menus.hh"

#include <gtkmm/treemodel.h>

namespace Gabber
{

/**
 * Controls the view of the Roster
 */
class RosterView
     : public SigC::Object
{
public:
    RosterView(JabberConnection& jc, Gtk::TreeView* treeview,
               Glib::RefPtr<RosterController> rostercontroller, 
               Gtk::Window* parent);
    ~RosterView();

protected:
    void add_columns();
    bool on_button_press_event(GdkEventButton* b);
    void on_row_activated(const Gtk::TreeModel::Path& path, 
                          Gtk::TreeViewColumn* column);
    void on_signal_cursor_changed();
    bool on_event(GdkEvent* e);
     void on_view_info();
    void on_send_file();
    void on_edit_groups();
    void on_read_log();
    void on_delete_contact();
    // TreeModel signals
    void on_row_inserted(const Gtk::TreeModel::Path& path, 
                         const Gtk::TreeModel::iterator& iter);
    void on_row_expanded(const Gtk::TreeModel::iterator& iter,
                         const Gtk::TreeModel::Path& path);
    void on_row_collapsed(const Gtk::TreeModel::iterator& iter,
                          const Gtk::TreeModel::Path& path);
private:
    JabberConnection& _jabber_connection;
    Gtk::TreeView*       _treeview;
    Glib::RefPtr<RosterController>        _rostercontroller;
     Gtk::Window*        _parent;
    Glib::RefPtr<Gtk::TreeSelection> _rselection;
    Popups::User* _user_menu; // The one and only
    typedef std::set<Glib::ustring> ssGu_t;
    ssGu_t _closed_groups;
};

}; // namespace Gabber

#endif // INCL_ROSTER_VIEW_HH
