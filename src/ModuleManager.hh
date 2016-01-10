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

#ifndef INCL_MODULE_MANAGER_HH
#define INCL_MODULE_MANAGER_HH

#include <glibmm/ustring.h>
#include <glibmm/module.h>
#include <string>
#include <list>
#include <map>

namespace Gabber
{

/**
* Managees the loading and unloading of Gabber plugins.
*/
class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager(){};

    /**
    * Load the specified plugin
    * @param plugin The name (not path) of the plugin to load
    * @returns bool Whether the plugin was loaded or not
    */
    bool loadPlugin(const std::string& plugin);
    /**
    * Load all the plugins listed in the configuration
    * This should really only be called once at start, other plugins should be
    * loaded individually.
    */
    void loadAll();
private:
    typedef std::string filename;
    std::map< filename, Glib::Module* > _plugin;
    std::list<Glib::ustring> _dirs;
    Glib::ustring _mainDir;
};

}; // namespace Gabber

#endif // INCL_MODULE_MANAGER_HH
