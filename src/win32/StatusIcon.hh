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

#include "GabberWin.hh"
#include "PacketQueue.hh"

#include <gdkmm.h> // XXX: what exactly to include?
#include <gtkmm/container.h>
#include <gtkmm/tooltips.h>

#define WM_TRAYMESSAGE WM_USER

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

    void clicked();
protected:
     void init();
     void on_my_presence(const jabberoo::Presence& p);
    void on_packet_queued(const std::string& jid, const std::string& icon);
    void on_queue_changed(const PacketQueue::QueueInfo& jid_first,
                          const PacketQueue::QueueInfo& first);
    void on_queue_emptied(const std::string& jid);
    bool swapIcons();
private:
     GabberWin*      _gwin;
     HICON           _prev_icon;
     HICON           _queue_icon;
     std::string      _queue_msg;
     HICON           _normal_icon;
     std::string     _normal_msg;
     SigC::Connection _timer;
     bool              _blinking;

    HWND systray_create_hiddenwin();
    void systray_init_icon(HWND hWnd, HICON icon);
    void systray_change_icon(HICON icon, const char* text);
    void systray_remove_nid(void);
     
}; // class StatusIcon

}; // namespace Gabber

#endif // INCL_STATUSICON_HH
