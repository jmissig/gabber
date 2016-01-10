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

#include "PlainTextView.hh"
#include "GabberUtility.hh"

#include <message.hh>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/textview.h>
#include <gtkmm/textmark.h>

#include <gtk/gtk.h>

#ifdef OLD_SIGC
#    include <sigc++/slot.h>
#endif

#include <iostream>

namespace Gabber {

PlainTextView::PlainTextView(Gtk::ScrolledWindow* scroll_win, bool stamp) : 
_scr_win(scroll_win), _view_height(0), _view_width(0), _goto_bottom(true),
_last_timestamp(0)
{
    _view = new Gtk::TextView();
    _buffer = _view->get_buffer();
    
    // initialize buffer uri/smiley parser
    _bufferparser = new TextBufferParser(_buffer);
    
    // initialize marks:
    _startmark         = _buffer->create_mark("startmark", _buffer->end());
    _bottom            = _buffer->create_mark("bottom", _buffer->end());
    _start_of_last_msg = _buffer->create_mark("start_of_last", _buffer->end());
    _buffer->create_mark("timestamp_start", _buffer->end());
    _buffer->create_mark("timestamp_end", _buffer->end());
    
    // Set TextView attributes:
    _view->set_wrap_mode(Gtk::WRAP_WORD);
    _view->set_editable(false);
    _view->set_cursor_visible(false);
    _view->set_left_margin(3); // this way letters won't touch the side edge
    _view->set_right_margin(3);
    _view->property_can_focus().set_value(false);

    // Figure out the line height
    // THEME/FONT SPECIFIC
    Glib::RefPtr<Pango::Layout> playout = Pango::Layout::create(_view->get_pango_context());
    Pango::Rectangle inkrect, logicalrect;
    playout->get_pixel_extents(inkrect, logicalrect);
    
    // save the line height for later use
    _font_lineheight = logicalrect.get_height();
    
    // Set the pixels_below_lines to be half of the line height.
    _view->set_pixels_below_lines(_font_lineheight/4);
    

    // the Gtk::ScrolledWindow contains our TextView widget:
    _scr_win->add(*_view);

    // Remote tag
    Glib::RefPtr<Gtk::TextBuffer::Tag> tag =  _buffer->create_tag("remote");
    tag->property_foreground().set_value("red");

    tag =  _buffer->create_tag("remote_seen");
    tag->property_foreground().set_value("burlywood");

    // Local tag
    tag =  _buffer->create_tag("local");
    tag->property_foreground().set_value("blue");

    tag =  _buffer->create_tag("local_seen");
    tag->property_foreground().set_value("LightSlateGrey");

    // Note tag
    tag = _buffer->create_tag("note");
    tag->property_foreground().set_value("dark green");

    tag = _buffer->create_tag("note_seen");
    tag->property_foreground().set_value("dark sea green");

    // /me tag
    tag = _buffer->create_tag("action");  
    tag = _buffer->create_tag("action_seen");  
    tag->property_foreground().set_value("grey40");

    // Composing tag
    tag = _buffer->create_tag("composing");
    tag->property_style().set_value(Pango::STYLE_ITALIC);

    // Seen tag
    tag = _buffer->create_tag("seen");
    tag->property_foreground().set_value("grey40");

    // Timestamp tag
    tag = _buffer->create_tag("timestamp");
    tag->property_foreground().set_value("dark grey");
    tag->property_justification().set_value(Gtk::JUSTIFY_CENTER);
    int sz = _view->get_pango_context()->get_font_description().get_size();
    tag->property_size().set_value(sz - (sz/4));

#ifdef OLD_SIGC
    _scr_win->get_vscrollbar()->signal_value_changed().connect(
        SigC::slot(*this, &PlainTextView::on_scroll_changed));
    _view->signal_size_allocate().connect(
        SigC::slot(*this, &PlainTextView::on_size_allocate));
#else
    _scr_win->get_vscrollbar()->signal_value_changed().connect(
        sigc::mem_fun(*this, &PlainTextView::on_scroll_changed));
    _view->signal_size_allocate().connect(
        sigc::mem_fun(*this, &PlainTextView::on_size_allocate));
#endif

    if (stamp)
        update_timestamp(time(NULL));
}

PlainTextView::~PlainTextView()
{
    delete _bufferparser;
    delete _view;
}

void PlainTextView::append(const jabberoo::Message& m, 
                           const Glib::ustring& nick, bool local_msg, 
                           bool seen)
{
    if (m.findX("jabber:x:delay") != NULL &&
        m.getBaseElement().hasAttrib("gabber:timestamp"))
    {
        update_timestamp(Util::parse_timestamp(
            m.getBaseElement().getAttrib("gabber:timestamp"), Util::DateTime));
    }
    else
    {
        update_timestamp(m.get_timestamp());
    }

    Glib::ustring from = m.getFrom();
    Glib::ustring tag = "local";

    if (!local_msg) 
    {
        if (_composers.find(from) != _composers.end())
            cancel_composing(from);

        tag = "remote";
    }

    _buffer->move_mark(_start_of_last_msg , _buffer->end());

    if (seen)
        tag += "_seen";

    Gtk::TextBuffer::iterator sit, eit;
    const Glib::ustring& body = m.getBody();
    // Check for /me
    if (body.substr(0,3) == "/me" && (body.substr(0,4) == "/me " || body.substr(0,6) == "/me's "))
    {
        std::string action_tag = seen ? "action_seen" : "action";
	    // set the tags for /me
	    std::vector<Glib::ustring> metags(2);
	    metags[0] = action_tag;
	    metags[1] = tag;

	    // insert the /me text
	    _buffer->insert_with_tags_by_name(_buffer->end(), "* ", metags);
            _buffer->insert_with_tag(_buffer->end(), nick, action_tag);
        _buffer->end();
	    _buffer->insert_with_tag(_buffer->end(), 
                    body.substr(3) + "\n", action_tag);
    }
    else
    {
        // no /me, insert normally
        _buffer->insert_with_tag(_buffer->end(), "<", tag);
        if (seen)
            _buffer->insert_with_tag(_buffer->end(), nick, "seen");
        else
            _buffer->insert(_buffer->end(), nick);
        _buffer->move_mark(_startmark, 
                           _buffer->insert_with_tag(_buffer->end(), "> ", tag));
        if (seen)
            _buffer->insert_with_tag(_buffer->end(), body + "\n", "seen");
        else
            _buffer->insert(_buffer->end(), body + "\n");
        _buffer->move_mark(_bottom, _buffer->end());
    
        // URIs and smileys ahoy
        _bufferparser->parse_buffer(_startmark, _bottom);
    }

    _buffer->move_mark(_bottom, _buffer->end());

    if (local_msg || _goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
         _view->scroll_to(_bottom, 0);
#endif
    }
}

void PlainTextView::append(const Glib::ustring& str)
{
    update_timestamp(time(NULL), true);

    // set the tags for /me
    std::vector<Glib::ustring> tags(2);
    tags[0] = "action";
    tags[1] = "note";

    // insert the /me text
    _buffer->move_mark(_startmark, 
                       _buffer->insert_with_tags_by_name(_buffer->end(), "* ", tags));
    _buffer->insert_with_tag(_buffer->end(), str + "\n", "action");
    _buffer->move_mark(_bottom, _buffer->end());
    _bufferparser->parse_buffer(_startmark, _bottom);
    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }
}

void PlainTextView::append_timestamp(time_t t, bool temp)
{
    char time_str[24];

    std::string format("%H:%M");
    if ( (t - _last_timestamp) > 86400 )
    {
        format = "%A %d - %H:%M";
    }

#ifndef WIN32

    strftime(time_str, 8, "%H:%M", localtime(&t));
#else
    // XXX Figure this out for win32
#endif

    txt_mark_ref start = _buffer->get_mark("timestamp_start");
    txt_mark_ref end = _buffer->get_mark("timestamp_end");

    if (temp)
        _buffer->move_mark(start, _buffer->end());
    _buffer->insert_with_tag(_buffer->end(), time_str, "timestamp");
    _buffer->insert(_buffer->end(), "\n");
    if (temp)
        _buffer->move_mark(end, _buffer->end());
    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }
}

void PlainTextView::remove_timestamp()
{
    txt_mark_ref start = _buffer->get_mark("timestamp_start");
    txt_mark_ref end = _buffer->get_mark("timestamp_end");
    
    Gtk::TextBuffer::iterator it_start = _buffer->get_iter_at_mark(start);
    Gtk::TextBuffer::iterator it_end = _buffer->get_iter_at_mark(end);
    if (it_start == it_end)
    {
        return;
    }

    _buffer->erase(_buffer->get_iter_at_mark(start), 
            _buffer->get_iter_at_mark(end));
    _buffer->move_mark(start, _buffer->end());
    _buffer->move_mark(end, _buffer->end());

    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }

    _last_timestamp = time(NULL) - 300;
}

void PlainTextView::append_error(const Glib::ustring& msg)
{
    _buffer->insert(_buffer->end(), "ERROR: " + msg + "\n");
    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }
}

void PlainTextView::composing(const std::string& jid, const Glib::ustring& nick)
{
    update_timestamp(time(NULL), true);

    // Alread have it, do something?
    if (_composers.find(jid) != _composers.end())
        return;

    // XXX Use something other than the jid as the key?  That's what the set is
    // for
    _buffer->create_mark(jid + "_start", _buffer->end());
    _buffer->insert_with_tag(_buffer->end(), "(", "composing");
    _buffer->insert_with_tag(_buffer->end(), nick, "composing");
    _buffer->insert_with_tag(_buffer->end(), ") ...", "composing");
    _buffer->insert(_buffer->end(), "\n");
    _buffer->create_mark(jid + "_end", _buffer->end());

    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }

    _composers.insert(jid);
}

void PlainTextView::cancel_composing(const std::string& jid)
{
    mark_map::iterator it = _composers.find(jid);

    remove_timestamp();

    // Not in the map, do we error?
    if (it == _composers.end())
        return;

    txt_mark_ref start = _buffer->get_mark(jid + "_start");
    txt_mark_ref end = _buffer->get_mark(jid + "_end");
    if (!start || !end)
    {
        std::cerr << "No marks found for " << jid << std::endl;
        return;
    }
    _buffer->erase(_buffer->get_iter_at_mark(start), 
            _buffer->get_iter_at_mark(end));

    _buffer->delete_mark(start);
    _buffer->delete_mark(end);

    _buffer->move_mark(_bottom, _buffer->end());

    if (_goto_bottom) {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
         _view->scroll_to(_bottom, 0);
#endif
    }

    _composers.erase(it);
}

void PlainTextView::clear()
{
    _last_timestamp = 0;
    _buffer->erase(_buffer->begin(), _buffer->end());
}

Gtk::Widget* PlainTextView::get_widget()
{
    return _view;
}

bool PlainTextView::is_at_bottom(const Gtk::Adjustment* vadj)
{
     return vadj->get_value() + _font_lineheight >= vadj->get_upper() - vadj->get_page_size();
}

void PlainTextView::on_scroll_changed()
{
    if ( is_at_bottom(_scr_win->get_vadjustment()) )
        _goto_bottom = true;
    else
        _goto_bottom = false;
}

#ifdef OLD_GTKMM
void PlainTextView::on_size_allocate(GtkAllocation* alloc)
#else
void PlainTextView::on_size_allocate(Gtk::Allocation& alloc)
#endif
{
    _view_height = _view->get_height();
    _view_width = _view->get_width();
    if( _goto_bottom )
    {
#ifdef OLD_GTKMM
	_view->scroll_to_mark(_bottom, 0, 0, 0);
#else
        _view->scroll_to(_bottom, 0);
#endif
    }
}

void PlainTextView::update_timestamp(time_t t, bool temp)
{
    // Make sure we don't have a temp in there.
    remove_timestamp();

    if ( (t - _last_timestamp) >= 300)
    {
        time_t val = t - (t % 300);
        if ( (t - _last_timestamp) >= 3600 )
        {
            append_timestamp(t, temp);
        }
        else
        {
            append_timestamp(val, temp);
        }
        _last_timestamp = val;
    }
}


} // namespace Gabber
