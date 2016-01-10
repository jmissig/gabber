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

#include "AutoAway.hh"

#include "intl.h"
#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "GabberWin.hh"

// Platform-specific idle tracking
#include "PlatformFuncs.hh"

namespace Gabber {

AutoAway::AutoAway(GabberWin* gwin)
     : _gwin(gwin), _autoaway(false), _enabled(false)
{
     if (!_idle.init())
     {
          cerr << "Idle tracking initialization failed, auto-away disabled." << endl;
     }
     else
     {
          _enabled = true;
          G_App.getSession().evtMyPresence.connect(SigC::slot(*this, &AutoAway::on_my_presence_event));
          G_App.getSession().evtConnected.connect(SigC::slot(*this, &AutoAway::on_session_connected));
          G_App.getSession().evtDisconnected.connect(SigC::slot(*this, &AutoAway::on_session_disconnected));
          G_App.getSession().evtOnLast.connect(SigC::slot(*this, &AutoAway::on_session_last));
     }
}

AutoAway::~AutoAway()
{
}

void AutoAway::on_my_presence_event(const jabberoo::Presence& p)
{
     _curshow = p.getShow();
}

void AutoAway::on_session_connected(const judo::Element& t)
{
     _autoaway = false;
     // we check 
     _timer = Glib::signal_timeout().connect(SigC::slot(*this, &AutoAway::auto_away_timer), 1000);
}

void AutoAway::on_session_disconnected()
{
     _timer.disconnect();
}

bool AutoAway::auto_away_timer()
{
     unsigned long idle_time = 0;
     Configurator& cf = G_App.getConfigurator();

     if (!cf.get_bool(Keys::autoaway.enable))
	  return true;

     idle_time = _idle.get_seconds();

     unsigned int aatime = cf.get_int(Keys::autoaway.away_after);
     unsigned int natime = cf.get_int(Keys::autoaway.xa_after);
     signed long away_time = (idle_time - (aatime * 60));

     if (!aatime && !natime)
          return true;

     // idle_time is in milliseconds, assume that natime will be longer than away time
     if (natime && (idle_time > natime * 60))
     {
	  if (!_autoaway && (_curshow == jabberoo::Presence::stOnline || _curshow == jabberoo::Presence::stChat))
	  {
	       set_away(jabberoo::Presence::stXA);
	       _gwin->push_statusbar_msg(_("Moving to extended away"), 1000);
	  }
	  
          // Only display the timer if we're automatically away, not away set by the user
	  if (_autoaway && _curshow == jabberoo::Presence::stXA)
	       update_statusbar_aatime_msg(away_time);  // Update the status bar
     }

     else if (aatime && (idle_time > aatime * 60))
     {
	  if (!_autoaway && (_curshow == jabberoo::Presence::stOnline || _curshow == jabberoo::Presence::stChat))
	  {
	       set_away(jabberoo::Presence::stAway);
	       _gwin->push_statusbar_msg(_("Starting auto-away"), 1000);
	  }

	  else if (_autoaway)
	       update_statusbar_aatime_msg(away_time);  // Update the status bar
     }
     else if (_autoaway)
     {
          set_back();

	  // Update the status bar
	  _gwin->push_statusbar_msg(_("Returning from auto-away"), 7000);
     }
     return true;
}

void AutoAway::update_statusbar_aatime_msg(unsigned long idle_time)
{
     // Manipulate the seconds of idle_time into 'normal' units
     int days, hrs, mins, secs, seconds;
     seconds = idle_time; // meh. it gives warnings. ah well. FIXME: make this not give warnings ;)
     days = (seconds / (3600 * 24));
     hrs  = ((seconds / 3600) - (days * 24));
     mins = ((seconds / 60) - (days * 24 * 60) - (hrs * 60));
     secs = (seconds - (days * 24 * 60 * 60) - (hrs * 60 * 60) - (mins * 60));
     char *tdays, *thrs, *tmins, *tsecs;
     tdays = g_strdup_printf("%d", days);
     thrs  = g_strdup_printf("%d", hrs);
     tmins = g_strdup_printf("%d", mins);
     tsecs = g_strdup_printf("%d", secs);
     
     // Format the string to be displayed
     string aamsg = _("Auto away for ");
     if (days > 0)
	  aamsg += string(tdays) + " " + _("d") + " ";
     else
	  g_free(tdays);
     
     if (hrs > 0)
	  aamsg += string(thrs) + ":" + string(tmins) + ":" + string(tsecs);
     else
     {
	  aamsg += string(tmins);

	  if (secs < 10)
	       aamsg += ":0" + string(tsecs);
	  else
	       aamsg += ":" + string(tsecs);

	  g_free(thrs);
	  g_free(tmins);
	  g_free(tsecs);
     }
     
     // Update the status bar with the Fancy Style (like ketchup in packets) message
     _gwin->push_statusbar_msg(aamsg, 1000);
}

void AutoAway::set_away(jabberoo::Presence::Show show)
{
     Configurator& cf = G_App.getConfigurator();

     gchar* priority_gchr = g_strdup_printf("%d", cf.get_int(Keys::acct.priority));
     std::string priority = priority_gchr;
     g_free(priority_gchr);
     
     // If they want priority lowered when autoaway
     if (cf.get_bool(Keys::autoaway.zero_priority))
          priority = "0";

     G_App.getSession() << jabberoo::Presence("", jabberoo::Presence::ptAvailable,
                                    show, cf.get_string(Keys::autoaway.awaymessage),
                                    priority);

     _autoaway = true;
}

void AutoAway::set_back()
{
     _gwin->set_back();
     
     _autoaway = false;
}

void AutoAway::on_session_last(string& idletime)
{
     // If they don't want idle time to be reported, then bail out
     if (!G_App.getConfigurator().get_bool(Keys::autoaway.idletime_visible))
     {
          idletime = "0";
          return;
     }
     
     unsigned long idle;
     char idlestr[1024];

     idle = _idle.get_seconds();
     g_snprintf(idlestr, 1024, "%ld", idle);
//     cerr << "IIIII****III***III**II Idle Time is: " << idlestr << endl;
     idletime = idlestr;
}

}; // namespace Gabber
