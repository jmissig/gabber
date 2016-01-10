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

#include "ModuleManager.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <glibmm/module.h>
#include <iostream>

namespace Gabber {

ModuleManager::ModuleManager()
{
    _dirs = G_App.getConfigurator().get_string_list(Keys::paths.plugindirs);
    // The user should not have to think about dynamically loadable module
    // files; If we can't find the module, say so to the user and give the 
    // option to go into a plugin import dialog, which will ask for the 
    // location of a plugin package (which should contain the .so/.dll, 
    // plus, perhaps, a description file along with supporting files like 
    // pixmaps).  The user should also get the oportunity to install the 
    // plugin system-wide if he/she provides a root password, but that's
    // not very portable....
}

bool ModuleManager::loadPlugin(const std::string& plugin_name) {
    Glib::Module* m = NULL;

    for (std::list<Glib::ustring>::iterator it = _dirs.begin();
            it != _dirs.end(); ++it) {
        Glib::ustring file = Glib::build_filename(*it, plugin_name + "." + G_MODULE_SUFFIX);
        if (Glib::file_test(file, Glib::FILE_TEST_EXISTS)) {                
            try
            {
                    m = new Glib::Module(file);
            }
            catch (Glib::Exception& e)
            {
                    std::cout << "Exception while loading: " << e.what() << std::endl;
                    continue;
            }
            catch (std::exception& e)
            {
                    std::cout << "Standard exception: " << e.what() << std::endl;
                    continue;
            }
            std::cout << "Loaded: " << file << std::endl;
            if (m->get_last_error() == "") { // no errors; it loaded properly. 
                _plugin[plugin_name] = m;
                return true;
            }
            else
            {
                std::cout << "Error loading " << m->get_name() << ": " <<
                    m->get_last_error() << std::endl;
                delete m;
                return false;
            }
        }
        else
        {
                std::cout << "No module at:" << file << std::endl;
        }
    }
    return false;
}

void ModuleManager::loadAll()
{
    std::list<Glib::ustring> v = GabberApp::getSingleton().getConfigurator().get_string_list(Keys::plug.pluginlist);

    for (std::list<Glib::ustring>::iterator it = v.begin();
            it != v.end(); ++it) {
        if(!loadPlugin(*it))
            std::cerr << "Plugin \"" << *it << "\" did not load!" << std::endl;
        // FIXME: Give the user a sensible error dialog.
    }
}

} // namespace Gabber
