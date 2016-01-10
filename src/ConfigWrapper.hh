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

// A convenience wrapper around Gnome::Conf. 

#ifndef INCL_CONFIG_WRAPPER_HH
#define INCL_CONFIG_WRAPPER_HH

#include <glibmm/ustring.h>
#include <gconfmm/client.h>

namespace Gabber
{

/**
 * Base class for interacting with the configuration system.
 * This is likely to change in the near future.  You've been warned.
 */
class ConfigWrapper
{
public: 
    ConfigWrapper(const Glib::ustring& main_dir,
            Gnome::Conf::ClientPreloadType preload = 
            Gnome::Conf::CLIENT_PRELOAD_ONELEVEL): _main_dir(main_dir) {
        client = Gnome::Conf::Client::get_default_client();
        client->add_dir(_main_dir, preload); 
    }

    ~ConfigWrapper() {
        client->suggest_sync();
        client->remove_dir(_main_dir); 
    }
    Glib::RefPtr<Gnome::Conf::Client> client;
private:
    const Glib::ustring _main_dir;
};

}; // namespace Gabber
#endif // INCL_CONFIG_WRAPPER_HH
