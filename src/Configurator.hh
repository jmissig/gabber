// Configurator.hh 
//
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
// $Id: Configurator.hh,v 1.2 2003/10/30 03:42:00 temas Exp $
//

#ifndef CONFIGURATOR_HH
#define CONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <glibmm/ustring.h>

namespace Gabber {

class Configurator
{
public:
  Configurator();
  virtual ~Configurator();

  virtual void save() = 0;

  virtual Glib::ustring get_string(const Glib::ustring& key) const = 0;
  virtual bool get_bool(const Glib::ustring& key) const = 0;
  virtual int get_int(const Glib::ustring& key) const = 0;
  virtual double get_float(const Glib::ustring& key) const = 0;
  virtual std::list<Glib::ustring> get_string_list(const Glib::ustring& key) const = 0;
  virtual void set(const Glib::ustring& key, const Glib::ustring& v) = 0;
  virtual void set(const Glib::ustring& key, int v) = 0;
  virtual void set(const Glib::ustring& key, bool v) = 0;
  virtual void set(const Glib::ustring& key, double v) = 0;
  virtual void set(const Glib::ustring& key, const std::list<Glib::ustring>& v) = 0;

  virtual std::list<Glib::ustring> get_all_dirs(const Glib::ustring& key) const = 0;
  virtual bool dir_exists(const Glib::ustring& key) const = 0;

  /**
  * Signals a change on a key.
  */
  //Signal2<void, const Glib::ustring&, const Glib::Value&> signal_key_changed();

protected:
  //void fire_configurator_event(const Glib::ustring& key);
  void strip_leading_slash(Glib::ustring& key) const;
  void strip_trailing_slash(Glib::ustring& key) const;
  void add_trailing_slash(Glib::ustring& key) const;
}; // class Configurator

}; // namespace Gabber

#endif // CONFIGURATOR_HH
