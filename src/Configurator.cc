// Configurator.cc --- Configuration Access
//
// Based on Configurator.cc from Workrave
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// Modifications for Gabber by Thomas Muldowney
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sstream>

#include "Configurator.hh"

using namespace Gabber;

Configurator::Configurator()
{
}

Configurator::~Configurator()
{
}

#if 0
//! Fire a configuration changed event.
void Configurator::fire_configurator_event(const Glib::ustring& key)
{
    Glib::ustring stripkey = key;
    strip_leading_slash(stripkey);

    ListenerIter i = listeners.begin();
    while (i != listeners.end())
    {
        Glib::ustring prefix = i->first;

        if (stripkey.substr(0, prefix.length()) == prefix)
        {
            ConfiguratorListener *l = i->second;
            if (l != NULL)
            {
                l->config_changed_notify(stripkey);
            }
        }
        i++;
    }
}
#endif 


//! Removes the leading '/'
void Configurator::strip_leading_slash(Glib::ustring &key) const
{
    int len = key.length();
    if (len > 1)
    {
        if (key[0] == '/')
        {
            key = key.substr(1, len - 1);
        }
    }
}


//! Removes the trailing '/'
void Configurator::strip_trailing_slash(Glib::ustring &key) const
{
    int len = key.length();
    if (len > 0)
    {
        if (key[len - 1] == '/')
        {
            key = key.substr(0, len - 1);
        }
    }
}


//! Add add trailing '/' if it isn't there yet.
void Configurator::add_trailing_slash(Glib::ustring &key) const
{
    int len = key.length();
    if (len > 0)
    {
        if (key[len - 1] != '/')
        {
            key += '/';
        }
    }
}
