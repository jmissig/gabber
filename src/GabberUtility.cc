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
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "GabberUtility.hh"

#include <fstream>
#include <string>
#include <iostream>

#include "intl.h"

#include <JID.hh>

#include <gtk/gtkclist.h>
#include <gtk/gtkselection.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/imagemenuitem.h>

#include <time.h>

// XXX: so sue me. I'm ifdef'ing Mac OS X.
// TODO: temas, ifdef windows here ;)
// what we really should do is search for this stuff.. and ah, whatever
#ifdef __MACH__
#ifdef __APPLE__
#define OPENCMD "open "
#else
#define OPENCMD "gnome-open "
#endif
#else
#define OPENCMD "gnome-open "
#endif

using namespace jabberoo;
using namespace Gabber::Util;

namespace Gabber {

#ifdef OLD_GTKMM
#  define MDBaseInit(a,b,c,d) Gtk::MessageDialog(a,b,c,d,false)
#  define MDBaseInitParent(a,b,c,d,e) Gtk::MessageDialog(a,b,c,d,e,false)
#else
#  define MDBaseInit(a,b,c,d) Gtk::MessageDialog(a,true,b,c,d)
#  define MDBaseInitParent(a,b,c,d,e) Gtk::MessageDialog(a,b,true,c,d,e)
#endif

Util::MessageDialog::MessageDialog(const Glib::ustring& msg, 
    Gtk::MessageType type, Gtk::ButtonsType buttons, bool modal) :
    MDBaseInit(msg, type, buttons, modal)
{ }

Util::MessageDialog::MessageDialog(Gtk::Window& parent, 
    const Glib::ustring& msg, Gtk::MessageType type, Gtk::ButtonsType buttons,
    bool modal) :
    MDBaseInitParent(parent, msg, type, buttons, modal)
{ }

void Util::main_dialog(Gtk::Dialog* d)
{
     // If the main window is hidden,
     // child dialogs will appear off screen,
     // so we have to have all possible parents here
// XXX
//     if (G_Win && G_Win->getBaseWindow()->is_visible())
//	  d->set_parent(* (G_Win->getBaseWindow()) );
//     else if (StatusIndicator::get_dock_window())
//	  d->set_parent(* (StatusIndicator::get_dock_window()) );
}

void Util::openuri(const Glib::ustring& theuri)
{
     Glib::spawn_command_line_async(OPENCMD + theuri);
}

gint Util::update_progress_bar(Gtk::ProgressBar* bar)
{
// XXX What is the point of this?
//     // Grab the current %, increment
//     float pvalue = bar->get_current_percentage();
//     pvalue += 0.01;
//
//     // If it's going to hit an end, reverse it, so it goes back and forth
//     if (pvalue <= 0.01)
//     {
//	  bar->set_orientation(Gtk::PROGRESS_LEFT_TO_RIGHT);
//	  pvalue = 0.01;
//     }
//     else if (pvalue >= 0.99)
//     {
//	  bar->set_orientation(Gtk::PROGRESS_RIGHT_TO_LEFT);
//	  pvalue = 0.01;
//     }
//
//     // Actually set the percentage
//     bar->set_percentage(pvalue);
//
     return TRUE;
}

void Util::toggle_visible(Gtk::ToggleButton* t, Gtk::Widget* w)
{
     if (t->get_active())
	  w->show();
     else
	  w->hide();
}

// XXX
//gint Util::strcasecmp_clist_items(GtkCList* c, const void* lhs, const void* rhs)
//{
//     const char* lhs_txt = GTK_CELL_PIXTEXT(((GtkCListRow*)lhs)->cell[c->sort_column])->text;
//     const char* rhs_txt = GTK_CELL_PIXTEXT(((GtkCListRow*)rhs)->cell[c->sort_column])->text;
//     return  strcasecmp(lhs_txt, rhs_txt);
//}
//
//gint Util::strcmp_clist_items(GtkCList* c, const void* lhs, const void* rhs)
//{
//     string lhs_txt = GTK_CELL_PIXTEXT(((GtkCListRow*)lhs)->cell[c->sort_column])->text;
//     string rhs_txt = GTK_CELL_PIXTEXT(((GtkCListRow*)rhs)->cell[c->sort_column])->text;
//     return lhs_txt.compare(rhs_txt);
//}

Glib::ustring Util::substitute(const Glib::ustring& full_string, const Glib::ustring& var1)
{
     gchar* subs_gchr = g_strdup_printf(full_string.c_str(), var1.c_str());
     Glib::ustring subs_str = subs_gchr;
     g_free(subs_gchr);
     return subs_str;
}

Glib::ustring Util::substitute(const Glib::ustring& full_string, const Glib::ustring& var1, 
			       const Glib::ustring& var2)
{
     gchar* subs_gchr = g_strdup_printf(full_string.c_str(), var1.c_str(), var2.c_str());
     Glib::ustring subs_str = subs_gchr;
     g_free(subs_gchr);
     return subs_str;
}

Glib::ustring Util::substitute(const Glib::ustring& full_string, const Glib::ustring& var1, 
			       const Glib::ustring& var2, const Glib::ustring& var3)
{
     gchar* subs_gchr = g_strdup_printf(full_string.c_str(), var1.c_str(), var2.c_str(), var3.c_str());
     Glib::ustring subs_str = subs_gchr;
     g_free(subs_gchr);
     return subs_str;
}

// XXX
//void Util::change_pixmap(Gtk::PixmapMenuItem* pitem, const string& filename)
//{
//     // The number of incomplete widgets in gnome-libs is starting to scare me
//     // This function claws inside the guts of the widget
//     // I found something like this inside bonobo
//     Gtk::Pixmap* pitem_pixmap = manage(new Gtk::Pixmap(filename));
//     GtkPixmapMenuItem* gack = pitem->gtkobj();
//     if (gack->pixmap)
//     {
//	  gtk_widget_destroy(gack->pixmap);
//	  gack->pixmap = NULL;
//     }
//     pitem_pixmap->show();
//     pitem->set_pixmap(*pitem_pixmap);
//}

Glib::ustring Util::lookup_nickname(const Roster& r, const Glib::ustring& jid)
{
     // attempt to look it up in the given roster
     try {
	  return r[JID::getUserHost(jid)].getNickname();
     } catch (Roster::XCP_InvalidJID& e) {
	  // the default nickname is the username
	  return JID::getUser(jid);
     }
     return JID::getUser(jid);
}

Glib::ustring Util::getShowName(Presence::Show type)
{
     switch (type)
     {
     case Presence::stInvalid:
	  return _("Invalid");
     case Presence::stOffline:
	  return _("Offline");
     case Presence::stOnline:
	  return _("Available");
     case Presence::stChat:
	  return _("Free for Chat");
     case Presence::stAway:
	  return _("Away");
     case Presence::stXA:
	  return _("Extended Away");
     case Presence::stDND:
	  return _("Busy");
     }
     return "";
}

std::string Util::getShowFilename(Presence::Show type)
{
     switch(type)
     {
     case Presence::stInvalid:
	  return std::string("invalid");
     case Presence::stOnline:
	  return std::string("online");
     case Presence::stOffline:
	  return std::string("offline");
     case Presence::stChat:
	  return std::string("chat");
     case Presence::stAway:
	  return std::string("away");
     case Presence::stXA:
	  return std::string("xa");
     case Presence::stDND:
	  return std::string("dnd");
     }
     return std::string("");
}

Glib::ustring Util::getS10nName(Roster::Subscription type)
{

     switch (type)
     {
     case Roster::rsBoth:
	  return _("both");
     case Roster::rsFrom:
	  return _("from");
     case Roster::rsTo:
	  return _("to");
     case Roster::rsNone:
     default:
	  return _("none");
     }
}

Glib::ustring Util::getS10nInfo(Roster::Subscription type)
{
     switch (type)
     {
     case Roster::rsBoth:
	  return _("Both of you can see each other's presence.");
     case Roster::rsFrom:
	  return _("You cannot see their presence, but they can see yours.");
     case Roster::rsTo:
	  return _("You can see their presence, but they cannot see yours.");
	  break;
     case Roster::rsNone:
     default:
	  return _("Neither of you can see the other's presence.");
     }
}

// We have these indexShow functions in case the enum changes
Presence::Show Util::indexShow(int show_index)
{
     switch (show_index)
     {
     case 0:                 // Invalid
	  return Presence::stInvalid;
     case 1:                 // Offline
	  return Presence::stOffline;
     case 2:                 // Online
	  return Presence::stOnline;
     case 3:		     // Chat
	  return Presence::stChat;
     case 4:		     // Away
	  return Presence::stAway;
     case 5:		     // Not available
	  return Presence::stXA;
     case 6:		     // Do not disturb
	  return Presence::stDND;
     default:
	  return Presence::stInvalid;
     }
}

int Util::indexShow(Presence::Show show_index)
{
     switch (show_index)
     {
     case Presence::stInvalid:
	  return 0;                 // Invalid
     case Presence::stOffline:
	  return 1;                 // Offline
     case Presence::stOnline:
	  return 2;                 // Online
     case Presence::stChat:
	  return 3;		     // Chat
     case Presence::stAway:
	  return 4;		     // Away
     case Presence::stXA:
	  return 5;		     // Not available
     case Presence::stDND:
	  return 6;		     // Do not disturb
     default:
	  return 0;
     }
}

Presence::Type Util::indexType(int type_index)
{
     switch (type_index)
     {
     case 0:                 // Error
	  return Presence::ptError;
     case 1:                 // Unavailable
	  return Presence::ptUnavailable;
     case 2:                 // Available
	  return Presence::ptAvailable;
     case 3:                 // Invisible
	  return Presence::ptInvisible;
     default:
	  return Presence::ptError;
     }
}

int Util::indexType(Presence::Type type_index)
{
     switch (type_index)
     {
     case Presence::ptError:
	  return 0;                 // Error
     case Presence::ptUnavailable:
	  return 1;                 // Unavailable
     case Presence::ptAvailable:
	  return 2;                 // Available
     case Presence::ptInvisible:
	  return 3;                 // Invisible
     default:
	  return 0;
     }
}

std::string Util::int2string(int i)
{
     char buf[16];
     snprintf(buf, 16, "%d", i);
     return std::string(buf);
}

std::string Util::build_timestamp(time_t t, TimeFormat format)
{
    std::string format_str;

    switch(format)
    {
    case Date:
        format_str = "%C%g-%m-%d";
        break;
    case DateTime:
        format_str = "%C%g-%m-%dT%H:%M:%S%z";
        break;
    case TimeOnly:
        format_str = "%H:%M:%S%z";
        break;
    case OldISO:
        format_str = "%C%g%MdT%H:%M:%S";
        break;
    };

    struct tm* t_tm = localtime(&t);
    char out_str[32];
    strftime(out_str, 32, format_str.c_str(), t_tm);

    return std::string(out_str);
}

time_t Util::parse_timestamp(const std::string& time_str, TimeFormat format)
{
    struct tm t_tm;
    std::string format_str;

    switch(format)
    {
    case Date:
        sscanf(time_str.c_str(), "%4d-%2d-%2d", &t_tm.tm_year, &t_tm.tm_mon, 
            &t_tm.tm_mday);
        break;
    case DateTime:
        sscanf(time_str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &t_tm.tm_year, 
            &t_tm.tm_mon, &t_tm.tm_mday, &t_tm.tm_hour, &t_tm.tm_min, 
            &t_tm.tm_sec);
        break;
    case TimeOnly:
        sscanf(time_str.c_str(), "%2d:%2d:%2d", &t_tm.tm_hour, &t_tm.tm_min, 
            &t_tm.tm_sec);
        break;
    case OldISO:
        sscanf(time_str.c_str(), "%4d%2d%2dT%2d:%2d:%2d", &t_tm.tm_year, 
            &t_tm.tm_mon, &t_tm.tm_mday, &t_tm.tm_hour, &t_tm.tm_min, 
            &t_tm.tm_sec);
        break;
    };

    // Fix up the year
    if (t_tm.tm_year < 1900)
        return 0;

    t_tm.tm_year -= 1900;
    --t_tm.tm_mon;

    // XXX timezone support?

    return mktime(&t_tm);
    
}

} // namespace Gabber
