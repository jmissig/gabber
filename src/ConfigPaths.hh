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

#ifndef INCL_CONFIG_PATHS_HH
#define INCL_CONFIG_PATHS_HH

#include <glibmm/ustring.h>

// A word on public const data members:
// Some people feel it's a mortal sin of OOP to make any data member public
// under any circumstances. My feeling is that it's a safe thing to do when
// we want the data to be globally read-only throughout the program's 
// execution and throughout the duration of the program's development 
// life-cycle. We ONLY want to assign values to these strings at 
// initialization time, we want the initialization itself to be independent 
// of client code, and we NEVER want the case to be otherwise during the 
// duration of Gabber2 development. So having an accessor for each data 
// member of each class in this file would be superfluous. 
//
// But hey, I could be wrong -- we're experimenting a bit with Gabber2, and
// I'll be the guy to blame later if this turns out to be a Bad Idea(TM). ;)
//              -- James

namespace Gabber {

namespace Keys {

class Account {
public:
    const Glib::ustring main_dir;     // Where Account stuff is stored
    const Glib::ustring savepassword; // Holds a bool
    const Glib::ustring password;     // Holds a Glib::ustring
    const Glib::ustring port;         // Holds a gint
    const Glib::ustring priority;     // Holds a gint
    const Glib::ustring resource;     // Holds a Glib::ustring
    const Glib::ustring server;       // Holds a Glib::ustring
    const Glib::ustring username;     // Holds a Glib::ustring
    const Glib::ustring ssl;
    const Glib::ustring autoconnect;
    const Glib::ustring reconnect;
    const Glib::ustring closed_groups;

    Account() :
    main_dir     ("/apps/gabber/account"),
    savepassword ( main_dir + "/savepassword"),
    password     ( main_dir + "/password"),
    port         ( main_dir + "/port"),
    priority     ( main_dir + "/priority"),
    resource     ( main_dir + "/resource"),
    server       ( main_dir + "/server"),
    username     ( main_dir + "/username"), 
    ssl          ( main_dir + "/ssl"),
    autoconnect  ( main_dir + "/autoconnect"),
    reconnect    ( main_dir + "/reconnect"),
    closed_groups  ( main_dir + "/closed_groups")
    {}
};
static const Account acct;

class GroupChat {
public:
    const Glib::ustring main_dir;
    const Glib::ustring nickname;

    GroupChat() :
    main_dir     ("/apps/gabber/groupchat"),
    nickname     ( main_dir + "/nickname")
    {}
};
static const GroupChat groupchat;

class History {
public:
     const Glib::ustring main_dir;
     const Glib::ustring last_rotate_month; // int
     const Glib::ustring last_rotate_year;  // int
     const Glib::ustring moved_old_logs;    // bool
     
     History() :
     main_dir ("/apps/gabber/history"),
     last_rotate_month (main_dir + "/last_rotate_month"),
     last_rotate_year (main_dir + "/last_rotate_year"),
     moved_old_logs (main_dir + "/moved_old_logs")
     {}
};
static const History history;

class Interface {
public:
     const Glib::ustring main_dir;
     const Glib::ustring contactlist_hideoffline; // bool
     const Glib::ustring messages_autodisplay;    // bool
     const Glib::ustring messages_spellcheck;     // bool
     const Glib::ustring contactlist_pos_x;       // int
     const Glib::ustring contactlist_pos_y;       // int
     const Glib::ustring contactlist_size_width;  // int
     const Glib::ustring contactlist_size_height; // int

     Interface() :
	  main_dir                ("/apps/gabber/interface"),
       contactlist_hideoffline ( main_dir + "/contactlist_hideoffline"),
	  messages_autodisplay    ( main_dir + "/messages_autodisplay"),
	  messages_spellcheck     ( main_dir + "/messages_spellcheck"),
       contactlist_pos_x       ( main_dir + "/contactlist_pos_x"),
       contactlist_pos_y       ( main_dir + "/contactlist_pos_y"),
       contactlist_size_width  ( main_dir + "/contactlist_size_width"),
       contactlist_size_height ( main_dir + "/contactlist_size_height")
	  {}
};
static const Interface intrface;

class Paths {
public:
    const Glib::ustring main_dir;
    const Glib::ustring libdir;     // string
    const Glib::ustring pixmapdir;  // string
    const Glib::ustring datadir;    // string
    const Glib::ustring plugindirs; // string
    
    Paths() :
    main_dir ("/apps/gabber/paths"),
    libdir ( main_dir + "/libdir"),
    pixmapdir (main_dir + "/pixmapdir"),
    datadir (main_dir + "/datadir"),
    plugindirs (main_dir + "/plugindirs")
    {}
};
static const Paths paths;

class Plugin {
public:
    const Glib::ustring main_dir;
    const Glib::ustring pluginlist;

    Plugin() : 
    main_dir     ("/apps/gabber/plugins"),
    pluginlist   ( main_dir + "/pluginlist")
    {}
};
static const Plugin plug;

class AutoAway {
public:
    const Glib::ustring main_dir;         // Where Account stuff is stored
    const Glib::ustring away_after;       // Holds an int
    const Glib::ustring idletime_visible; // Holds a bool
    const Glib::ustring zero_priority;    // Holds a bool
    const Glib::ustring awaymessage;      // Holds a Glib::ustring
    const Glib::ustring xa_after;         // Holds an int
    const Glib::ustring enable;           // Holds a bool

    AutoAway() : 
    main_dir         ("/apps/gabber/autoaway"),
    away_after	      ( main_dir + "/away_after"),
    idletime_visible ( main_dir + "/idletime_visible"),
    zero_priority    ( main_dir + "/zero_priority"),
    awaymessage      ( main_dir + "/awaymessage"),
    xa_after         ( main_dir + "/xa_after"),
    enable           ( main_dir + "/enable")
    {}
};
static const AutoAway autoaway;

class Avatar {
public:
    const Glib::ustring main_dir;         // Where Account stuff is stored
    const Glib::ustring jid;
    const Glib::ustring node;
    const Glib::ustring hash;
    const Glib::ustring filename;

    Avatar():
    main_dir("/apps/gabber/avatar"),
    jid (main_dir + "/jid"),
    node (main_dir + "/node"),
    hash (main_dir + "/hash"),
    filename (main_dir + "/filename")
    {}
};

static const Avatar avatar;

}// ...More config path categories to come....

}; // namespace Gabber
#endif // INCL_CONFIG_PATHS_HH
