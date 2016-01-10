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

#include "PacketQueueView.hh"
#include "GabberApp.hh"
#include "ResourceManager.hh"

#include <jabberoo/roster.hh>

#include <gtkmm/treeview.h>

#include <sigc++/slot.h>

using namespace Gabber;

PacketQueueView::PacketQueueView(Gtk::ScrolledWindow& scroll, 
    Gtk::TreeView& view) : _scroll(scroll), _view(view), _is_visible(false),
    _row_height(0), _rows_visible(0)
{
    _store = Gtk::ListStore::create(_columns);
    _view.set_model(_store);

    Gtk::TreeView::Column* pColumn = Gtk::manage( 
        new Gtk::TreeView::Column("Incoming Messages") );

    pColumn->pack_start(_columns.icon, false);
    pColumn->pack_start(_columns.name, true);

    _view.append_column(*pColumn);

    _view.signal_row_activated().connect(
        SigC::slot(*this, &PacketQueueView::on_row_activated));

    PacketQueue& pq(G_App.getPacketQueue());
    pq.packet_queued_event.connect(
        SigC::slot(*this, &PacketQueueView::on_packet_queued));
    pq.queue_emptied_event.connect(
        SigC::slot(*this, &PacketQueueView::on_queue_emptied));
    pq.queue_changed_event.connect(
        SigC::slot(*this, &PacketQueueView::on_queue_changed));
}

void PacketQueueView::clear()
{
    _store->clear();
}

void PacketQueueView::on_packet_queued(const std::string& jid, const std::string& icon)
{
    std::string userhost = jabberoo::JID::getUserHost(jid);

    typedef Gtk::TreeModel::Children type_children;
    type_children children = _store->children();
    for (type_children::iterator it = children.begin(); 
         it != children.end(); ++it)
    {
        Gtk::TreeModel::Row row = *it;
        if (row[_columns.jid] == userhost)
        {
            // Only need them in there once
            return;
        }
    }
    
    // XXX Actually check the type and get the correct icon
    Gtk::TreeModel::iterator mit = _store->append();
    Gtk::TreeModel::Row row = *mit;

    std::string name(userhost);
    jabberoo::Roster& roster(G_App.getSession().roster());
    if (roster.containsJID(jid))
    {
        std::string nick = roster[userhost].getNickname();
        if (!nick.empty())
            name = nick;
    }
    row[_columns.name] = name;
    row[_columns.jid] = userhost;
    row[_columns.icon] = G_App.getResourceManager().getPixbuf(icon);
    ++_rows_visible;

    if (!_is_visible)
    {
        _scroll.show();
        _is_visible = true;

        if (_row_height == 0)
        {
            Gtk::TreeView::Column* pColumn = _view.get_column(0);
            Gdk::Rectangle tangle;
            int zero(0);
            pColumn->cell_get_size(tangle, zero, zero, zero, _row_height);

            _extra_height = _scroll.get_border_width();
            // Just a bit of padding of our own
            _extra_height += 4;
        }
    }

    int height = _rows_visible <= 3 ? _rows_visible : 3;
    _scroll.set_size_request(-1, (height * _row_height) + _extra_height);
    _scroll.queue_resize();

}

void PacketQueueView::on_queue_emptied(const std::string& jid)
{
    std::string userhost = jabberoo::JID::getUserHost(jid);
    typedef Gtk::TreeModel::Children type_children;
    type_children children = _store->children();
    for (type_children::iterator it = children.begin(); 
         it != children.end(); ++it)
    {
        Gtk::TreeModel::Row row = *it;
        if (row[_columns.jid] == userhost)
        {
            // We don't need to add them multiple times
            _store->erase(it);
            --_rows_visible;
            break;
        }
    }

    if (children.empty())
    {
        _scroll.hide();
        _is_visible = false;
    }
    else
    {
        int height = _rows_visible <= 3 ? _rows_visible : 3;
        _scroll.set_size_request(-1, (height * _row_height) + _extra_height);
        _scroll.queue_resize();
    }
}

void PacketQueueView::on_queue_changed(const PacketQueue::QueueInfo& jid_next, 
        const PacketQueue::QueueInfo& first)
{
    std::string userhost = jabberoo::JID::getUserHost(jid_next.jid);

    typedef Gtk::TreeModel::Children type_children;
    type_children children = _store->children();
    for (type_children::iterator it = children.begin(); 
         it != children.end(); ++it)
    {
        Gtk::TreeModel::Row row = *it;
        if (row[_columns.jid] == userhost)
        {
            // Update the icon and move on
            row[_columns.icon] =
                G_App.getResourceManager().getPixbuf(jid_next.icon);
            return;
        }
    }
}

void PacketQueueView::on_row_activated(const Gtk::TreeModel::Path& path, 
        Gtk::TreeViewColumn* column)
{
    Gtk::TreeModel::iterator iter = _store->get_iter(path);
    std::string jid = iter->get_value(_columns.jid);
    G_App.getPacketQueue().pop(jid);
}

