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

#include "ResourceManager.hh"
#include "Environment.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"

#include <gdkmm/pixbufloader.h>

using namespace Gabber;

Glib::RefPtr<Gdk::Pixbuf>& ResourceManager::getPixbuf(const std::string& file)
{
    // Return it if we already have it
    PBMap::iterator it = _pixbufs.find(file);
    if (it != _pixbufs.end())
    {
        return it->second;
    }

    Glib::RefPtr<Gdk::Pixbuf> pb =
        Gdk::Pixbuf::create_from_file(Glib::build_filename(
            G_App.getConfigurator().get_string(Keys::paths.pixmapdir), file));
    if (!pb)
        throw InvalidResource();

    std::pair<PBMap::iterator, bool> ins = _pixbufs.insert(
        PBMap::value_type(file, pb));
    return ins.first->second;
}

void ResourceManager::releasePixbuf(const std::string& file)
{
    PBMap::iterator it = _pixbufs.find(file);
    if (it == _pixbufs.end())
        throw InvalidResource();
    
    _pixbufs.erase(it);
}

ResourceManager& ResourceManager::getSingleton(void)
{
    return Singleton<ResourceManager>::getSingleton();
}

