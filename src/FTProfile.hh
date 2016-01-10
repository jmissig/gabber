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

#ifndef INCL_FTPROFILE_HH
#define INCL_FTPROFILE_HH

#include "SIProfile.hh"

namespace Gabber {

class FTProfile : public SIProfile
{
public:
    struct FileInfo
    {
        long size;
        std::string suggested_name;
        std::string date;
        std::string hash;
        std::string desc;
    };

public:
    FTProfile()
    { }
    FTProfile(const judo::Element& elem);

    virtual ~FTProfile() { }

    const std::string getNamespace()
    { return "http://jabber.org/protocol/si/profile/file-transfer"; }

    FileInfo& getFileInfo()
    { return _info; }

    judo::Element* buildNode();
    judo::Element* buildResultNode();
private:
    FileInfo _info;
};

};

#endif // INCL_FTPROFILE_HH
