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

#ifndef INCL_STATUSICON_HH
#define INCL_STATUSICON_HH

#include "fwd.h"

#include "PacketQueue.hh"

namespace Gabber {

class GabberWin;

/**
 * A tray icon indicating the current status.
 * This is for the main GabberApp and GabberWin.
 * Additional connections will have to implement 
 * their own.
 */
class StatusIcon
     : public SigC::Object
{
public:
     StatusIcon(GabberWin* gwin);
     ~StatusIcon();
protected:
private:

}; // class StatusIcon

}; // namespace Gabber

#endif // INCL_STATUSICON_HH
