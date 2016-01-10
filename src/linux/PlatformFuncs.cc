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
 *  Copyright (c) 2003-2004 Julian Missig
 */
#ifndef INCL_PLATFORM_FUNCS_CC
#define INCL_PLATFORM_FUNCS_CC

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "GConfConfigurator.hh"
#include "PlatformFuncs.hh"

#include <gconfmm/init.h>

namespace Gabber {

Configurator* create_configurator(void)
{
    return new GConfConfigurator;
}

void StartupPlatform()
{ Gnome::Conf::init(); }

void ShutdownPlatform()
{ }

PlatformIdleTracker::PlatformIdleTracker()
{ }

PlatformIdleTracker::~PlatformIdleTracker()
{ }

bool PlatformIdleTracker::init()
{
#ifdef HAVE_XSS
     int event_base, error_base;
     if (XScreenSaverQueryExtension(GDK_DISPLAY(), &event_base, &error_base))
     {
          _scrnsaver_info = XScreenSaverAllocInfo();
          return true;
     }
     else
#endif // HAVE_XSS
          return false;
}

unsigned long PlatformIdleTracker::get_seconds()
{
#ifdef HAVE_XSS
     XScreenSaverQueryInfo(GDK_DISPLAY(), GDK_ROOT_WINDOW(), _scrnsaver_info);
     return (_scrnsaver_info->idle / 1000.0);
#else
     return 0; // too bad.
#endif /* HAVE_XSS */
}

} // namespace Gabber

#endif //INCL_PLATFORM_FUNCS_CC
