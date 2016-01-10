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

#ifndef __GABBER_INCL_TEXTVIEW_HH__
#define __GABBER_INCL_TEXTVIEW_HH__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fwd.h"
#include "PrettyText.hh"
#include "TextParser.hh"
#include <sigc++/object.h>
#include <gtkmm/textbuffer.h>
#include <set>

namespace Gabber
{

/**
 * A PrettyText implementation using regular Gtk::TextView
 */
class PlainTextView : public PrettyText, public SigC::Object
{
public:
    /**
     * Create a new PlainTextView
     * @param scr_win The scrolled window to interact with.
     */
    PlainTextView(Gtk::ScrolledWindow* scr_win, bool stamp = true);
    ~PlainTextView();

    void append(const jabberoo::Message& m, const Glib::ustring& nick, 
                bool local_msg = false, bool seen=false);
    void append(const Glib::ustring& str);
    void append_timestamp(time_t t, bool temp=false);
    void append_error(const Glib::ustring& msg);
    void composing(const std::string& jid, const Glib::ustring& nick);
    void cancel_composing(const std::string& jid);
    void clear();

    Gtk::Widget* get_widget();

private:
    Glib::RefPtr<Gtk::TextBuffer> _buffer;
    Gtk::TextView* _view;
    Gtk::ScrolledWindow* _scr_win;
    TextBufferParser* _bufferparser;

    typedef Glib::RefPtr<Gtk::TextMark> txt_mark_ref;
    typedef std::set<std::string> mark_map;
    mark_map _composers;

    txt_mark_ref _startmark; // start mark for URI/smiley parsing
    txt_mark_ref _bottom;// The bottom of the TextView
    txt_mark_ref _start_of_last_msg; // The name says it all.

    int _view_height;    // We need to store the size of the TextView
    int _view_width;     // so that we'll know when it changes, and therefore
                         // when to auto-scroll down.
    int _font_lineheight; // the height of one line of text, for scrolling
    bool _goto_bottom;
    time_t _last_timestamp;

    bool is_at_bottom(const Gtk::Adjustment* vadj);
    void on_scroll_changed(void);
#ifdef OLD_GTKMM
    void on_size_allocate(GtkAllocation* alloc);
#else
    void on_size_allocate(Gtk::Allocation& alloc);
#endif
    void remove_timestamp();
     // Updates the timestamp if it's been long enough
     void update_timestamp(time_t t, bool temp=false);
};

}; //namespace Gabber

#endif // __GABBER_INCL_TEXTVIEW_HH__
