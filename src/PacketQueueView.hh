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
 *  Copyright (c) 2002-2004 Julian Missig
 */

#ifndef INCL_PACKETQUEUEVIEW_HH
#define INCL_PACKETQUEUEVIEW_HH

#include "PacketQueue.hh"

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/object.h>

namespace Gabber {

class PacketQueueView : public SigC::Object
{
public:
    PacketQueueView(Gtk::ScrolledWindow& scroll, Gtk::TreeView& view);
    void clear();
private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> jid;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
        ModelColumns()
        { add(icon); add(name); add(jid); }
    };

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _store;
    Gtk::ScrolledWindow& _scroll;
    Gtk::TreeView& _view;
    bool _is_visible;
    int  _row_height;
    int  _extra_height;
    int  _rows_visible;

    void on_packet_queued(const std::string& jid, const std::string& icon);
    void on_queue_emptied(const std::string& jid);
    void on_queue_changed(const PacketQueue::QueueInfo& jid_next, 
                          const PacketQueue::QueueInfo& first);
    void on_row_activated(const Gtk::TreeModel::Path& path, 
                          Gtk::TreeViewColumn* column);
};

}; //namespace Gabber

#endif // INCL_PACKETQUEUEVIEW_HH
