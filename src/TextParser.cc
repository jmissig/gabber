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
 *  Copyright (c) 2004 Julian Missig
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TextParser.hh"

#include "GabberUtility.hh"

#include <gtkmm/textbuffer.h>
#include <gtkmm/textmark.h>

#include <sigc++/slot.h>

namespace Gabber {

TextBufferParser::TextBufferParser(Glib::RefPtr<Gtk::TextBuffer> buffer)
     : _buffer(buffer)
{
     _uritag = _buffer->create_tag("uri");
     _uritag->property_foreground().set_value("blue");
     _uritag->property_underline().set_value(Pango::UNDERLINE_SINGLE);
#ifdef OLD_SIGC
     _uritag->signal_event().connect(SigC::slot(*this, &TextBufferParser::on_tag_uri_event));     
#else
    _uritag->signal_event().connect(sigc::mem_fun(*this, &TextBufferParser::on_tag_uri_event));
#endif
     _clickedtag = _buffer->create_tag("uri-clicked");
     _clickedtag->property_foreground().set_value("purple");
}

TextBufferParser::~TextBufferParser()
{
}

void TextBufferParser::parse_buffer(const Glib::RefPtr<Gtk::TextMark>& startmark,
                                    const Glib::RefPtr<Gtk::TextMark>& endmark)
{
     Gtk::TextIter startchar = _buffer->get_iter_at_mark(startmark);
     Gtk::TextIter enditer = _buffer->get_iter_at_mark(endmark);
     
     // iterate over each character.
     // yeah, it sucks, but you gotta find those URLs (URIs) and smileys :) somehow
     for(Gtk::TextIter endchar = startchar; startchar != enditer; ++startchar)
     {
          // we could also build up a startchar to endchar string here somehow...
          // would that be more efficient to insert?
          while (startchar != enditer && Glib::Unicode::isspace(*startchar))
               ++startchar;
          
          if (startchar == enditer)
               break;
          
          endchar = startchar;
          
          while (endchar != enditer && !Glib::Unicode::isspace(*endchar))
               ++endchar;
          
          render_uri_to_buffer(startchar, endchar);
          // Note to self: need to set mark, then render smiley, then get iter from mark
          // render smiley will modify the contents of the text, thus invalidating iters
          // && !(show_smileys && render_smiley_to_buffer(startchar, endchar))
     }
}

bool TextBufferParser::render_uri_to_buffer(const Gtk::TextIter& startchar,
                                            const Gtk::TextIter& endchar)
{
     // Pretty blatantly copied from GnomeChat
     // but converted to C++
     // Thanks James Cape
     Gtk::TextIter it = startchar;
     
     /* Parse the first character of the scheme, which must be a-z */
     if (*it < 'a' || *it > 'z')
          return false;
     
     ++it;
     
     /* Parse the rest of the scheme, which can be a-z, 0-9, +, -, or . */
     
     do
     {
          if (it == endchar)
               return false;
          
          if (*it != ':')
               ++it;
     }
     while ((*it >= 'a' && *it <= 'z') || (*it >= '0' && *it <= '9') || *it == '+' || *it == '-' || *it == '.');
     
     /* Parse the seperator */
     if (it == endchar || *it != ':')
          return false;
     
     ++it;
     
     /* Parse the rest of the URI.  I decided to be lazy here, and just assume everything is valid */
     if (it == endchar)
          return false;
     
     _buffer->apply_tag(_uritag, startchar, endchar);
     
     return true;
}

bool TextBufferParser::on_tag_uri_event(const Glib::RefPtr<Glib::Object>& o, 
                                        GdkEvent* e, 
                                        const Gtk::TextIter& i)
{
     if (e->type == GDK_BUTTON_PRESS && e->button.button == 1)
     {
          Gtk::TextIter startchar = i;
          Gtk::TextIter endchar = i;
          
          while(!startchar.begins_tag(_uritag))
               --startchar;
          
          while(!endchar.ends_tag(_uritag))
               ++endchar;
          
          Glib::ustring theuri = _buffer->get_text(startchar, endchar);
          
          Util::openuri(theuri);

	  _buffer->apply_tag(_clickedtag, startchar, endchar);
     }
     return false;
}

} // namespace Gabber
