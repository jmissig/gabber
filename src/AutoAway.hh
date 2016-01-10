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
 *  Gabber
 *  Copyright (C) 1999-2002 Dave Smith & Julian Missig
 *  Ported to Gabber 2
 *  Copyright (c) 2003 Julian Missig
 */


#ifndef INCL_AUTO_AWAY_HH
#define INCL_AUTO_AWAY_HH

#include "fwd.h"

#include <jabberoofwd.h>
#include <jabberoo/presence.hh>

#include <sigc++/connection.h>
#include <sigc++/object_slot.h>

// Platform-specific idle tracking
#include "PlatformFuncs.hh"

namespace Gabber
{

class GabberWin;

class AutoAway
     : public SigC::Object
{
public:
     AutoAway(GabberWin* gwin);
     ~AutoAway();
protected:
     void on_my_presence_event(const jabberoo::Presence& p);
     void on_session_connected(const judo::Element& t);
     void on_session_disconnected();
     void on_session_last(std::string &idletime);
     bool auto_away_timer();
     void AutoAway::update_statusbar_aatime_msg(unsigned long idle_time);

     void set_away(jabberoo::Presence::Show show);
     void set_back();
private:
     GabberWin* _gwin;
     bool   _autoaway;
     bool   _enabled;

     jabberoo::Presence::Show _curshow;

     SigC::Connection _timer;

     PlatformIdleTracker _idle;

}; // class AutoAway

}; // namespace Gabber

#endif // INCL_AUTO_AWAY_HH
