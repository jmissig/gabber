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

#ifndef INCL_STATUSICON_HH
#define INCL_STATUSICON_HH

#include "fwd.h"

#include <gdkmm.h> // XXX: what exactly to include?
extern "C" {
#include "eggtrayicon.h"
}

#include <gtkmm/container.h>
#include <gtkmm/tooltips.h>

#include "PacketQueue.hh"

namespace Gabber {

class GabberWin;

/**
 * A tray icon indicating the current status.
 * This is for the main GabberApp and GabberWin.
 * Additional connections will have to implement 
 * their own.
 */
class StatusIcon
     : public SigC::Object
{
public:
     StatusIcon(GabberWin* gwin);
     ~StatusIcon();
protected:
     void init();
     void init_queue();
     void on_my_presence(const jabberoo::Presence& p);
     bool on_button_press(GdkEventButton* e);
     bool on_queue_button_press(GdkEventButton* e);
     void on_packet_queued(const std::string& who, const std::string& icon);
     void on_queue_changed(const PacketQueue::QueueInfo& next_pack, const PacketQueue::QueueInfo& top_pack);
     void on_queue_emptied(const std::string& who);
private:
     GabberWin*      _gwin;
     EggTrayIcon*    _status_icon_egg;
     Gtk::Container* _status_icon;
     Gtk::EventBox*  _evt_status;
     Gtk::Image*     _img_status;
     
     EggTrayIcon*    _queue_icon_egg;
     Gtk::Container* _queue_icon;
     Gtk::EventBox*  _evt_queue;
     Gtk::Image*     _img_queue;
     bool            _queued;
     
     Gtk::Tooltips   _tips;
     
}; // class StatusIcon

}; // namespace Gabber

#endif // INCL_STATUSICON_HH
