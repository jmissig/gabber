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

#ifndef __GABBER_INCL_PRETTYTEXT_H__
#define __GABBER_INCL_PRETTYTEXT_H__

#include <string>
#include "fwd.h"

namespace Gabber
{

/**
 * Abstract base class for the text views.
 */
class PrettyText
{
public:
    PrettyText() 
    { }
    virtual ~PrettyText() 
    { }

    /**
    * Append a complete message to the view.  
    * This will cancel a pending composing event.
    *
    * @param m A reference to the message to add
    * @param nick The nickname to add it under
    * @param local_msg If this is from the local user or not
    * @param seen If this msg has already been seen by the user before
    */
    virtual void append(const jabberoo::Message& m, const Glib::ustring& nick, 
                        bool local_msg = false, bool seen=false) = 0;
    /**
    * Append a string to the view.  
    * This does not cacnel any pending composing events because it is not bound
    * to a particular user.
    *
    * @param str The string to add
    * 
    */
    virtual void append(const Glib::ustring& str) = 0;

    /**
    * Append a timestamp to the view
    */
    virtual void append_timestamp(time_t t, bool temp = false) = 0;

    /**
    * Append an error message to the view.
    * @param msg The message to display.
    */
    virtual void append_error(const Glib::ustring& msg) = 0;

    /**
    * Show a composing note.  This could be in the form of a thought bubble or
    * something else.
    *
    * @param jid Who to add a composing event for
    */
    virtual void composing(const std::string& jid, 
                           const Glib::ustring& nick) = 0;

    /**
    * Cancel a pending composing event
    *
    * @param jid Who to cancel the composing event for
    */
    virtual void cancel_composing(const std::string& jid) = 0;

    /**
    * Completely clears the view.
    */
    virtual void clear() = 0;

    /**
    * Retrieve the widget
    * @returns A pointer to the displayable widget
    */
    virtual Gtk::Widget* get_widget() = 0;
};

}; // namespace Gabber

#endif // __GABBER_INCL_PRETTYTEXT_H__
