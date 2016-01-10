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
 *  Copyright (C) 1999-2001 Dave Smith & Julian Missig
 */

#ifndef INCL_GABBER_UTILITY_HH
#define INCL_GABBER_UTILITY_HH

#include "fwd.h"
#include <jabberoo/judo.hpp>
#include <jabberoo/presence.hh>
#include <jabberoo/roster.hh>

#include <gtkmm/messagedialog.h>
#include <glibmm/ustring.h>

namespace Gabber
{

/**
 * Utility Functions and types
 */
namespace Util
{

    class MessageDialog : public Gtk::MessageDialog
    {
    public:
        MessageDialog(const Glib::ustring& msg, 
            Gtk::MessageType type=Gtk::MESSAGE_INFO,
            Gtk::ButtonsType buttons=Gtk::BUTTONS_OK, bool modal=false);
        MessageDialog(Gtk::Window& parent, const Glib::ustring& msg, 
            Gtk::MessageType type=Gtk::MESSAGE_INFO, 
            Gtk::ButtonsType buttons=Gtk::BUTTONS_OK, 
            bool modal=false);
    };

     void main_dialog(Gtk::Dialog* d);
     void openuri(const Glib::ustring& theuri);
     gint update_progress_bar(Gtk::ProgressBar* bar);
     void toggle_visible(Gtk::ToggleButton* t, Gtk::Widget* w);
//     gint strcasecmp_clist_items(GtkCList* c, const void* lhs, const void* rhs);
//     gint strcmp_clist_items(GtkCList* c, const void* lhs, const void* rhs);
     Glib::ustring substitute(const Glib::ustring& full_string, const Glib::ustring& var1);
     Glib::ustring substitute(const Glib::ustring& full_string, const Glib::ustring& var1, 
			      const Glib::ustring& var2);
     Glib::ustring substitute(const Glib::ustring& full_string, const Glib::ustring& var1, 
			      const Glib::ustring& var2, const Glib::ustring& var3);
//     void change_pixmap(Gtk::PixmapMenuItem* pitem, const string& filename);
     // We still have the widget parameter because we may move back to needing that at some point
     /**
      * Lookup a nickname given a JabberID and a Roster.
      * XXX: Maybe this should be in JabberOO?
      * @param r The Roster to use to find the nickname.
      * @param jid The JabberID to lookup.
      * @return Glib::ustring containing the nickname.
      */
     Glib::ustring lookup_nickname(const jabberoo::Roster& r, const Glib::ustring& jid);
     Glib::ustring getShowName(jabberoo::Presence::Show type);
     std::string   getShowFilename(jabberoo::Presence::Show type);
     Glib::ustring getS10nName(jabberoo::Roster::Subscription type);
     Glib::ustring getS10nInfo(jabberoo::Roster::Subscription type);
     jabberoo::Presence::Show indexShow(int show_index);
     int    indexShow(jabberoo::Presence::Show show_index);
     jabberoo::Presence::Type indexType(int type_index);
     int    indexType(jabberoo::Presence::Type type_index);
    std::string int2string(int i);
//     ConfigManager::Proxy indexProxy(int proxy_index);
//     int    indexProxy(ConfigManager::Proxy proxy_index);

    /**
    * Allowable time formats.
    * These are specifed in JEP-82.  Fractions of seconds are not currently
    * supported.
    */
    enum TimeFormat
    {
        Date, /**< CCYY-MM-DD (2004-01-30) */
        DateTime, /**< CCYY-MM-DDThh:mm:ss[.sss]TZD (2004-01-30T13:52:17Z) */
        TimeOnly, /**< hh:mm:ss[.sss]TZD (13:52:17Z) */
        OldISO /**< CCYYMMDDThh:mm:ss (20040130T13:52:17) */
    };

    /**
    * Build a timestamp string using the specified time and format.
    * @param t The time to build for
    * @param format The format to use, defautls to DateTime
    * @returns string The formatted timestamp.
    */
    std::string build_timestamp(time_t t, TimeFormat format = DateTime);

    /**
    * Parses a timestamp string into a time_t.
    * @param time_str The timeestamp string to parse.
    * @param format The format to use during parsing.
    * @returns time_t The time_t equivalent of the timestamp string.
    */
    time_t parse_timestamp(const std::string& time_str, TimeFormat format);

    // Some common templates
    struct ListDeleter
    {
        template <typename T>
        void operator()(const T* ptr) { delete ptr; }
    };

    /** Predicate to test for a node in the specified namespace */
    struct MatchNamespace : public std::unary_function<judo::Node*, bool>
    {
        MatchNamespace(const std::string& ns_) : ns(ns_)
        { }
        bool operator()(const judo::Node* node)
        {
            return static_cast<const judo::Element*>(node)->cmpAttrib("xmlns", ns);
        }
        std::string ns;
    };

    struct sortCaseInsensitive :
        std::binary_function<const char*, const char*, bool>
    {
        bool operator()(const char* lhs, const char* rhs)
        { return strcasecmp(lhs, rhs) < 0; }
    };

    typedef std::list<Glib::ustring> JIDList;
}; // namespace Util

}; // namespace Gabber

#endif
