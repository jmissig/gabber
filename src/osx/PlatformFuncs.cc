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

#ifndef INCL_PLATFORM_FUNCS_CC
#define INCL_PLATFORM_FUNCS_CC

#include "GConfConfigurator.hh"

#include "PlatformFuncs.hh"

#include <ApplicationServices/ApplicationServices.h>
#include <gconfmm/init.h>

// wow this is lame
extern "C" {
extern double CGSSecondsSinceLastInputEvent(unsigned long evType);
}

namespace Gabber {
     
Configurator* create_configurator(void)
{
          return new GConfConfigurator;
}

// Startup/Shutdown are in Startup.cc

PlatformIdleTracker::PlatformIdleTracker()
{ }

PlatformIdleTracker::~PlatformIdleTracker()
{ }

bool PlatformIdleTracker::init()
{
     return true;
}     

unsigned long PlatformIdleTracker::get_seconds()
{
     double idleTime = CGSSecondsSinceLastInputEvent(-1);
     
     // from Adium:
     //On MDD Powermacs, the above function will return a large value when the machine is active (-1?).
     //Here we check for that value and correctly return a 0 idle time.
     if (idleTime >= 18446744000.0) idleTime = 0.0; //18446744073.0
     
     return (long)idleTime;
}

}; // namespace Gabber

#endif //INCL_PLATFORM_FUNCS_CC
