// GConfConfigurator.hh
//
// Based on GConfConfigurator.hh from Workrave
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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
// $Id: GConfConfigurator.hh,v 1.1 2004/05/19 01:00:31 temas Exp $
//
// Modifications for Gabber by Thomas Muldowney
//

#ifndef GCONFCONFIGURATOR_HH
#define GCONFCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <glibmm/ustring.h>
#include <gconfmm/client.h>

#include "Configurator.hh"

namespace Gabber {

class GConfConfigurator : public Configurator
{
public:
  GConfConfigurator();
  virtual ~GConfConfigurator();

  virtual void save();

  virtual Glib::ustring get_string(const Glib::ustring& key) const;
  virtual bool get_bool(const Glib::ustring& key) const;
  virtual int get_int(const Glib::ustring& key) const;
  virtual double get_float(const Glib::ustring& key) const;
  virtual std::list<Glib::ustring> get_string_list(const Glib::ustring& key) const;
  virtual void set(const Glib::ustring& key, const Glib::ustring& v);
  virtual void set(const Glib::ustring& key, int v);
  virtual void set(const Glib::ustring& key, bool v);
  virtual void set(const Glib::ustring& key, double v);
  virtual void set(const Glib::ustring& key, const std::list<Glib::ustring>& v);

  virtual std::list<Glib::ustring> get_all_dirs(const Glib::ustring& key) const;
  virtual bool dir_exists(const Glib::ustring& key) const;
  
private:
  // void key_changed(guint id, Gnome::Conf::Entry entry);
  
private:
    Glib::ustring _gconf_root;
    Glib::RefPtr<Gnome::Conf::Client> _client;
}; // class Gabber

}; // namespace Gabber

#endif // GCONFCONFIGURATOR_HH
