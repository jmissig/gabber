// GConfConfigurator.cc --- Configuration Access
//
// Based on GConfConfigurator.cc from Workrave
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

#include "GConfConfigurator.hh"
#include <gconfmm/client.h>

using namespace Gabber;

GConfConfigurator::GConfConfigurator() :
    _gconf_root("/apps/gabber/")
{
    _client = Gnome::Conf::Client::get_default_client();
}


GConfConfigurator::~GConfConfigurator()
{
}


void GConfConfigurator::save()
{
    _client->suggest_sync();
}


Glib::ustring GConfConfigurator::get_string(const Glib::ustring& key) const
{
    return _client->get_string(key);
}


bool GConfConfigurator::get_bool(const Glib::ustring& key) const
{
    return _client->get_bool(key);
}

int GConfConfigurator::get_int(const Glib::ustring& key) const
{
    return _client->get_int(key);
}

double GConfConfigurator::get_float(const Glib::ustring& key) const
{
    return _client->get_float(key);
}

std::list<Glib::ustring> GConfConfigurator::get_string_list(const Glib::ustring& key) const
{
    return _client->get_string_list(key);
}

void GConfConfigurator::set(const Glib::ustring& key, const Glib::ustring& v)
{
    _client->set(key, v);
}


void GConfConfigurator::set(const Glib::ustring& key, int v)
{
    _client->set(key, v);
}

void GConfConfigurator::set(const Glib::ustring& key, double v)
{
    _client->set(key, v);
}

void GConfConfigurator::set(const Glib::ustring& key, bool v)
{
    _client->set(key, v);
}


void GConfConfigurator::set(const Glib::ustring& key, const std::list<Glib::ustring>& v)
{
    Gnome::Conf::Value val(Gnome::Conf::VALUE_LIST);
    val.set_list_type(Gnome::Conf::VALUE_STRING);
    val.set_string_list(v);
    _client->set(key, val);
}


bool GConfConfigurator::dir_exists(const Glib::ustring& key) const
{
    Glib::ustring full_key = _gconf_root + key;
    strip_trailing_slash(full_key);

    return _client->dir_exists(full_key);
}

std::list<Glib::ustring> GConfConfigurator::get_all_dirs(const Glib::ustring& key) const
{
    Glib::ustring full_key = _gconf_root + key;
    strip_trailing_slash(full_key);

    return _client->all_dirs(full_key);
}


#if 0
void GConfConfigurator::static_key_changed(GConfClient *client, guint cnxn_id, 
    GConfEntry *entry, gpointer user_data)
{
    GConfConfigurator *c = (GConfConfigurator *) user_data;
    c->key_changed(cnxn_id, entry);
}


void GConfConfigurator::key_changed(guint id, GConfEntry *entry)
{
    Glib::ustring dir = id2key_map[id];

    Glib::ustring full_key = entry->key;

    if (full_key.substr(0, gconf_root.length()) == gconf_root)
    {
        full_key = full_key.substr(gconf_root.length());
    }

    fire_configurator_event(full_key);
}
#endif
