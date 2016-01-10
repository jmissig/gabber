// Win32Configurator.hh
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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
// $Id: Win32Configurator.hh,v 1.2 2004/06/16 16:37:03 temas Exp $
//

#ifndef WIN32CONFIGURATOR_HH
#define WIN32CONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <windows.h>
#include "Configurator.hh"

#include <glibmm/ustring.h>

namespace Gabber {

class Win32Configurator :
  public Configurator
{
public:
  Win32Configurator();
  virtual ~Win32Configurator();

  virtual void save();

  Glib::ustring get_string(const Glib::ustring& key) const;
  bool get_bool(const Glib::ustring& key) const;
  int get_int(const Glib::ustring& key) const;
  long get_long(const Glib::ustring& key) const;
  double get_float(const Glib::ustring& key) const;
  std::list<Glib::ustring> get_string_list(const Glib::ustring& key) const;
  void set(const Glib::ustring& key, const Glib::ustring& v);
  void set(const Glib::ustring& key, int v);
  void set(const Glib::ustring& key, long v);
  void set(const Glib::ustring& key, bool v);
  void set(const Glib::ustring& key, double v);
  void set(const Glib::ustring& key, const std::list<Glib::ustring>& v);

  virtual std::list<Glib::ustring> get_all_dirs(const Glib::ustring& key) const;
  virtual bool dir_exists(const Glib::ustring& key) const;

private:
  Glib::ustring key_win32ify(const Glib::ustring& key) const;
  Glib::ustring key_add_part(const Glib::ustring& s, const Glib::ustring& t) const;
  void key_split(const Glib::ustring& key, Glib::ustring &parent, Glib::ustring &child) const;
  
  Glib::ustring key_root;
  PHKEY key_root_handle;
};

};

#endif // WIN32CONFIGURATOR_HH
