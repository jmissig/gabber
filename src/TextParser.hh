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

#ifndef __GABBER_INCL_TEXTPARSER_HH__
#define __GABBER_INCL_TEXTPARSER_HH__

#include "fwd.h"
#include <gtkmm/textbuffer.h>

#include <sigc++/object.h>

namespace Gabber
{

/**
 * A URI and smiley parser for a TextBuffer.
 */
class TextBufferParser : public SigC::Object
{
public:
     /**
      * Feed me a Gtk::TextBuffer and I shall do wonderful things.
      * Or just parse URIs and smileys.
      */
     TextBufferParser(Glib::RefPtr<Gtk::TextBuffer> buffer);
     ~TextBufferParser();
     
     /**
      * Parse between the two marks (they will be converted to iters).
      */
     void parse_buffer(const Glib::RefPtr<Gtk::TextMark>& startmark,
                       const Glib::RefPtr<Gtk::TextMark>& endmark);

protected:
     bool render_uri_to_buffer(const Gtk::TextIter& startchar,
                               const Gtk::TextIter& endchar);
     bool on_tag_uri_event(const Glib::RefPtr<Glib::Object>& o, 
			   GdkEvent* e, 
			   const Gtk::TextIter& i);

private:
     Glib::RefPtr<Gtk::TextBuffer> _buffer;
     Glib::RefPtr<Gtk::TextBuffer::Tag> _uritag;
     Glib::RefPtr<Gtk::TextBuffer::Tag> _clickedtag;
};

}; //namespace Gabber

#endif // __GABBER_INCL_TEXTPARSER_HH__
