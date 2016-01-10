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

#ifndef GABBER_RERSOURCEMANAGER_HH
#define GABBER_RERSOURCEMANAGER_HH

#include "Singleton.hh"
#include <map>
#include <gdkmm/pixbuf.h>

namespace Gabber
{
    
/**
 * Controls resource usage
 * This is primarily used to ensure large binary data is only loaded once and
 * then that instance is used repeatedly.
 */
class ResourceManager : public Singleton<ResourceManager>
{
public:
    // XXX Fill this out?
    struct InvalidResource { };

    ResourceManager()
    { }
    ~ResourceManager()
    { }

    /**
    * get a pixbuf reference from the common store
    */
    Glib::RefPtr<Gdk::Pixbuf>& getPixbuf(const std::string& file);
    /**
    * tell the manager it can release the resource fully
    */
    void releasePixbuf(const std::string& name);

    // XXX Sounds?

    static ResourceManager& getSingleton(void);
private:
    typedef std::map<std::string, Glib::RefPtr<Gdk::Pixbuf> > PBMap;
    PBMap _pixbufs;
}; // class ResourceManager

}; // namespace Gabber

#endif // GABBER_RERSOURCEMANAGER_HH
