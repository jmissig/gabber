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

#ifndef INCL_BASE_MODULE_HH
#define INCL_BASE_MODULE_HH

namespace Gabber {

/**
* Provides information and base functionality for a module.
*
* Usually this class is derived by the class that is going to register as a
* module.  It is also possible to just have an instance of this and pass it to
* the module registration.
*/
class BaseModule
{
public:
    BaseModule(const Glib::ustring& name, const Glib::ustring& desc) :
        _name(name), _desc(desc)
    {}
    virtual ~BaseModule() {}

    virtual Glib::ustring getName() const
    { return _name; }

    virtual Glib::ustring getDesc() const
    { return _desc; }
private:
    Glib::ustring _name;
    Glib::ustring _desc;
};

}; // namespace Gabber

#endif // INCL_PLUGIN_BASE_HH
