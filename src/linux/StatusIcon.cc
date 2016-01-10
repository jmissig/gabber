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

#include "StatusIcon.hh"

#include "GabberApp.hh"
#include "GabberWin.hh"
#include "JabberConnection.hh"
#include "PacketQueue.hh"
#include "ResourceManager.hh"

#include "intl.h"
#include <jabberoo/presence.hh>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>

namespace Gabber {

StatusIcon::StatusIcon(GabberWin* gwin)
     : _gwin(gwin)
{
     // actually, apparently it doesn't matter whether there is a dock or not
     // the docklet will auto-appear when it's time
     init();
     init_queue();
}

StatusIcon::~StatusIcon()
{
    // Gtkmm cleans up for us.
}

void StatusIcon::init()
{
     _status_icon_egg = egg_tray_icon_new(_("Gabber"));
     _status_icon = Glib::wrap(GTK_CONTAINER(_status_icon_egg));
//     _status_icon ->signal_delete_event().connect(slot(*this, &StatusIcon::on_delete_event));

     // Add the event box
     _evt_status  = new Gtk::EventBox();
//     _evt_status  ->set_shadow_type(Gtk::SHADOW_NONE);
     _status_icon->add(*_evt_status);
     _evt_status ->add_events(Gdk::BUTTON_PRESS_MASK);
     _evt_status ->signal_button_press_event().connect(SigC::slot(*this, &StatusIcon::on_button_press));


     _img_status = new Gtk::Image(ResourceManager::getSingleton().getPixbuf("offline.png"));
     _evt_status ->add(*_img_status);
     _img_status ->show();
     _evt_status ->show();
     _status_icon->show();

     // Now that we've established all the widgets and whatnot
     // let's hook up to the events we want
     G_App.getSession().evtMyPresence.connect(SigC::slot(*this, &StatusIcon::on_my_presence));
     //_app.evtConnecting.connect(  SigC::slot(*this, &GabberWin::on_evtConnecting));
     //_app.evtConnected.connect(   SigC::slot(*this, &GabberWin::on_evtConnected));
     //_app.evtDisconnected.connect(SigC::slot(*this, &GabberWin::on_evtDisconnected));
}

void StatusIcon::init_queue()
{
     _queue_icon_egg = egg_tray_icon_new(_("Gabber Message Queue"));
     _queue_icon = Glib::wrap(GTK_CONTAINER(_queue_icon_egg));
     
     // Add the event box
     _evt_queue = new Gtk::EventBox();
     _queue_icon->add(*_evt_queue);
     _evt_queue ->add_events(Gdk::BUTTON_PRESS_MASK);
     _evt_queue ->signal_button_press_event().connect(SigC::slot(*this, &StatusIcon::on_queue_button_press));
     
     _img_queue = new Gtk::Image(ResourceManager::getSingleton().getPixbuf("message-standalone.png"));
     _evt_queue ->add(*_img_queue);
     _img_queue ->show();
     _evt_queue ->show();
     
     // we only show the queue icon when necessary
     _queue_icon->hide();
     
     // Hook up the events we want
     G_App.getPacketQueue().packet_queued_event.connect(SigC::slot(*this, &StatusIcon::on_packet_queued));
     G_App.getPacketQueue().queue_changed_event.connect(SigC::slot(*this, &StatusIcon::on_queue_changed));
     G_App.getPacketQueue().queue_emptied_event.connect(SigC::slot(*this, &StatusIcon::on_queue_emptied));

     // Set the tooltip
     Glib::ustring tooltip = _("Pending Message");
     _tips.set_tip(*_evt_queue, tooltip);
}

void StatusIcon::on_my_presence(const jabberoo::Presence& p)
{
     // Set the labels
     std::string filename;
     if((p.getShow() == jabberoo::Presence::stInvalid) ||
        (p.getShow() == jabberoo::Presence::stOffline) ||
        p.getShow_str().empty())
          filename = "offline.png";
     else
          filename = p.getShow_str() + ".png";
     _img_status->set(ResourceManager::getSingleton().getPixbuf(filename));

     // Set the tooltip
     Glib::ustring tooltip;
     tooltip += Gabber::Util::getShowName(p.getShow());
     if (!p.getStatus().empty())
          tooltip += ": " + p.getStatus();
     _tips.unset_tip(*_evt_status);
     _tips.set_tip(*_evt_status, tooltip);
}

bool StatusIcon::on_button_press(GdkEventButton* e)
{
     // Determine if we should show/hide the main window or popup menu
     if (e->type == GDK_BUTTON_PRESS)
     {
	  if (e->button == 1)
               _gwin->toggle_visibility();
          else if (e->button == 3)
          {
               // XXX: popup menu
          }
     }
     return false;
}

bool StatusIcon::on_queue_button_press(GdkEventButton* e)
{
     if (e->type == GDK_BUTTON_PRESS)
     {
          if (e->button == 1)
          {
               // Pop the Top!
               G_App.getPacketQueue().pop();
          }
     }
     return false;
}

void StatusIcon::on_packet_queued(const std::string& who, const std::string& icon)
{
    if (_queued)
        return;

     _queue_icon->show();
     
     _img_queue->set(ResourceManager::getSingleton().getPixbuf(icon));

     // TODO: set tooltip to the roster name

     _queued = true;
}

void StatusIcon::on_queue_changed(const PacketQueue::QueueInfo& next_pack, 
                                  const PacketQueue::QueueInfo& top_pack)
{
// supposedly guaranteed not empty
//     if (G_App.getPacketQueue().empty())
//          return;
     
     // packet_queued enables
//     _queue_icon->show();

     _img_queue->set(ResourceManager::getSingleton().getPixbuf(top_pack.icon));
     // TODO: set tooltip to the roster name
}

void StatusIcon::on_queue_emptied(const std::string& who)
{
    if (G_App.getPacketQueue().empty())
    {
        _queue_icon->hide();
        _queued = false;
    }
    else
    {
        _img_queue->set(ResourceManager::getSingleton().getPixbuf(
            G_App.getPacketQueue().begin()->icon));
            // TODO: set the tooltip to the roster name
    }
}

} // namespace Gabber
