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
 *  Based on Gabber, Copyright (c) 1999-2003 Dave Smith & Julian Missig
 *  Copyright (c) 2002-2003 Julian Missig
 */

#include "FTProfile.hh"

#include <jabberoo/judo.hpp>

using namespace Gabber;

FTProfile::FTProfile(const judo::Element& elem)
{
    _info.suggested_name = elem.getAttrib("name");
    std::string size_str = elem.getAttrib("size");
    if (size_str.empty())
    {
        // XXX error
        return;
    }
    _info.size = ::atol(size_str.c_str());
    _info.date = elem.getAttrib("date");
    _info.hash = elem.getAttrib("hash");
    const judo::Element* desc = elem.findElement("desc");
    if (desc)
    {
        _info.desc = desc->getCDATA();
    }
}

judo::Element* FTProfile::buildNode()
{
    judo::Element* file = new judo::Element("file");
    file->putAttrib("xmlns", 
        "http://jabber.org/protocol/si/profile/file-transfer");
    file->putAttrib("name", _info.suggested_name);
    char size_str[16];
    snprintf(size_str, 16, "%ld", _info.size);
    file->putAttrib("size", size_str);
    if(!_info.date.empty())
        file->putAttrib("date", _info.date);
    if(!_info.hash.empty())
        file->putAttrib("hash", _info.hash);
    if (!_info.desc.empty())
    {
        file->addElement("desc", _info.desc);
    }

    // XXX Range support

    return file;
}

judo::Element* FTProfile::buildResultNode()
{
    // XXX Range support
    return NULL;
}
