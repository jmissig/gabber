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

#ifndef INCL_STREAM_HH
#define INCL_STREAM_HH

#include <string>
#include <sigc++/signal.h>

namespace Gabber {

/**
 * An abstract base class for a stream.
 */
class Stream
{
public:
    enum State
    {
        Disconnected,
        Connecting,
        Open,
        Authenticated,
        Activated,
        Error
    };

    virtual ~Stream() { }

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void write(const std::string& out) = 0;
    virtual void write(const char* out, int len) = 0;
    virtual void need_write(bool need = true) = 0;
    
    SigC::Signal1<void, State> signal_state_change;
    SigC::Signal2<void, const char*, int> signal_data_available;
    SigC::Signal1<void, int> signal_data_sent;
    SigC::Signal0<void> signal_can_send_more;
    SigC::Signal1<void, const std::string&> signal_error;
    SigC::Signal0<void> signal_closed;
};

};

#endif // INCL_STREAM_HH
