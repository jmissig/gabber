// Win32Configurator.cc --- Configuration Access
//
// Based on Win32Configurator.cc from WorkRave
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
// Modified for Gabber by Thomas Muldowney
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "Win32Configurator.hh"

using namespace Gabber;

Win32Configurator::Win32Configurator() : key_root("Software/gabber")
{
}


Win32Configurator::~Win32Configurator()
{
}

void Win32Configurator::save()
{
}


Glib::ustring Win32Configurator::get_string(const Glib::ustring& key) const
{
  HKEY handle;
  Glib::ustring k, p, p32, c;
  LONG err;
  Glib::ustring out;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      char buf[256]; // FIXME: yuck, should be dynamic.
      size = sizeof(buf);
      err = RegQueryValueEx(handle, c.c_str(), 0, &type, (LPBYTE) buf, &size);
      if (err == ERROR_SUCCESS)
        {
          out = buf;
        }
      RegCloseKey(handle);
    }
  return out;
}


bool Win32Configurator::get_bool(const Glib::ustring& key) const
{
  return get_int(key) > 0 ? true : false;
}


int Win32Configurator::get_int(const Glib::ustring& key) const
{
    long l = get_long(key);
    if (l >= 0)
    {
        return (int) l;
    }
    return l;
}

long Win32Configurator::get_long(const Glib::ustring& key) const
{
    long out;
    Glib::ustring s = get_string(key);
    if (!s.empty())
    {
        int f = sscanf(s.c_str(), "%ld", &out);
        if (f == 0)
            return -1;
    }
    return out;
}

double Win32Configurator::get_float(const Glib::ustring& key) const
{
    float out;
    Glib::ustring s = get_string(key);
    if (!s.empty())
    {
        int f = sscanf(s.c_str(), "%f", &out);
        if (f == 0)
            return -1;
    }
    return out;
}

std::list<Glib::ustring> Win32Configurator::get_string_list(const Glib::ustring& key) const
{
    Glib::ustring val = get_string(key);

    std::list<Glib::ustring> out;

    Glib::ustring::size_type pos = 0;
    Glib::ustring::size_type prev_pos = 0;
    while ( (pos = val.find(';', prev_pos)) != Glib::ustring::npos)
    {
        out.push_back(val.substr(prev_pos, (pos - prev_pos)));
        prev_pos = ++pos;
    }
    if (pos > 0 && prev_pos > 0)
        out.push_back(val.substr(prev_pos, (pos - prev_pos)));
    else if (pos == Glib::ustring::npos && prev_pos == 0)
    {
        // One key
        out.push_back(val);
    }

    return out;
}

void Win32Configurator::set(const Glib::ustring& key, const Glib::ustring& v)
{
  HKEY handle;
  bool rc = false;
  Glib::ustring k, p, p32, c;
  DWORD disp;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);
  err = RegCreateKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      err = RegSetValueEx(handle, c.c_str(), 0, REG_SZ, (BYTE *) v.c_str(), v.length()+1);
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
      if (rc)
	{
	  //fire_configurator_event(key);
	}
    }
}

void Win32Configurator::set(const Glib::ustring& key, int v)
{
  char buf[32];
  sprintf(buf, "%d", v);
  return set(key, Glib::ustring(buf));
}

void Win32Configurator::set(const Glib::ustring& key, bool v)
{
  char buf[32];
  sprintf(buf, "%d", v ? 1 : 0);
  return set(key, Glib::ustring(buf));
}


void Win32Configurator::set(const Glib::ustring& key, double v)
{
  char buf[32];
  sprintf(buf, "%f", v);
  return set(key, Glib::ustring(buf));
}

void Win32Configurator::set(const Glib::ustring& key,
        const std::list<Glib::ustring>& v)
{
    Glib::ustring val;
    for(std::list<Glib::ustring>::const_iterator it = v.begin(); it != v.end();
            ++it)
    {
        if (!val.empty())
            val += ":";
        val += *it;
    }

    return set(key, val);
}

bool Win32Configurator::dir_exists(const Glib::ustring& key) const
{
  HKEY handle;
  bool rc = false;
  std::string k, k32;
  LONG err;

  k = key_add_part(key_root, key);
  k32 = key_win32ify(k);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, k32.c_str(), 0, KEY_READ, &handle);
  if (err == ERROR_SUCCESS)
    {
      rc = true;
      RegCloseKey(handle);
    }
  return rc;
}

std::list<Glib::ustring> Win32Configurator::get_all_dirs(const Glib::ustring& key) const
{
	std::list<Glib::ustring> ret;
	LONG err;
	HKEY handle;

	Glib::ustring k = key_add_part(key_root, key);
	Glib::ustring k32 = key_win32ify(k);
	err = RegOpenKeyEx(HKEY_CURRENT_USER, k32.c_str(), 0,
			KEY_ALL_ACCESS, &handle);
	if (err == ERROR_SUCCESS)
	{
		for (int i = 0; ; i++)
		{
			// FIXME: static length, eek
			char buf[256];
			DWORD buf_size = sizeof(buf);
			FILETIME time;

			err = RegEnumKeyEx(handle, i, buf, &buf_size, NULL, NULL, NULL, &time);
			if (err == ERROR_SUCCESS)
			{
				std::string s = buf;
				ret.push_back(s);
			}
			else
			{
				break;
			}
		}
		RegCloseKey(handle);
	}
	return ret;  
}


Glib::ustring Win32Configurator::key_add_part(const Glib::ustring& s, 
                                              const Glib::ustring& t) const
{
  Glib::ustring ret = s;
  add_trailing_slash(ret);
  return ret + t;
}

void Win32Configurator::key_split(const Glib::ustring& key, 
                                  Glib::ustring &parent, 
                                  Glib::ustring &child) const
{
  const char *s = key.c_str();
  char *slash = strrchr(s, '/');
  if (slash)
    {
      parent = key.substr(0, slash-s);
      child = slash+1;
    }
  else
    {
      parent = "";
      child = "";
    }
}

Glib::ustring Win32Configurator::key_win32ify(const Glib::ustring& key) const
{
	Glib::ustring rc = key;
	strip_trailing_slash(rc);
	for (Glib::ustring::size_type i = 0; i < rc.length(); ++i)
	{
		if (rc[i] == '/')
		{
			rc.replace(i, 1, "\\");
		}
	}
	return rc;
}


