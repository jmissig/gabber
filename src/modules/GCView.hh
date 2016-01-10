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

#ifndef INCL_GCVIEW_HH
#define INCL_GCVIEW_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

#include <jabberoo/presence.hh>
#include <jabberoo/XPath.h>
#include <gtkmm/treemodel.h>


namespace Gabber {

class GCView : public BaseGabberWindow
{
public:
    GCView(GCViewManager& mgr, const Glib::ustring& jid, 
           const Glib::ustring& nickname);
    ~GCView();

    void close();
protected:
    // callbacks
    void on_presence_node(const judo::Element& node);
    void on_message_node(const judo::Element& node);
    bool on_window_event(GdkEvent* ev);
    bool on_subject_key_released(GdkEventKey* ev);
    bool on_message_key_released(GdkEventKey* ev);
private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        { add(nick); add(icon); }

        Gtk::TreeModelColumn<Glib::ustring> nick;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
    };

    typedef std::map<std::string, jabberoo::Presence::Show> GCUserMap;
    GCViewManager&                          _mgr;
    Glib::ustring                           _room_jid;
    Glib::ustring                           _nickname;
    jabberoo::Session&                      _session;
    judo::XPath::Query*                     _pres_xpath;
    judo::XPath::Query*                     _msg_xpath;
    PrettyText*                             _txtChatView;
    Gtk::Label*                             _lblRoom;
    Gtk::TextView*                          _txtMessage;
    Gtk::Entry*                             _subject_ent;
    Gtk::TreeView*                          _users_treeview;
    Glib::RefPtr<Gtk::ListStore>            _users_store;
    GCUserMap                               _users;
    ModelColumns                            _columns;
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > _icons;

    // Send the message
    void send_message();

    // Refresh the view of users in the chat
    void update_userlist();
}; // class GCView

}; // namespace Gabber

#endif // INCL_GCVIEW_HH
