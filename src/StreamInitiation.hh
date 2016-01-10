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

#ifndef INCL_STREAMINITIATION_HH
#define INCL_STREAMINITIATION_HH

#include "FeatureNegotiation.hh"
#include "SIProfile.hh"

#include <string>

namespace judo { class Element; }

namespace Gabber {

/** Stream Initiation Node Wrapper */
class SI
{
public:
    SI() : _profile(NULL), _fneg(new FeatureNegotiation)
    { }
    SI(const judo::Element& elem);
    ~SI()
    {
        delete _fneg;
        delete _profile;
    }

    std::string getID() const
    { return _id; }
    void setID(const std::string& id)
    { _id = id; }

    std::string getMimeType() const
    { return _mime_type; }
    void setMimeType(const std::string& mime_type)
    { _mime_type = mime_type; }

    SIProfile* getProfile() const
    { return _profile; }
    void setProfile(SIProfile* profile);

    FeatureNegotiation* getFeatureNegotiation() const
    { return _fneg; }
    
    judo::Element* buildNode();
    judo::Element* buildResultNode();

    // Error node builders
    static judo::Element* buildInvalidStreamsNode();
    static judo::Element* buildInvalidProfileNode();
    static judo::Element* buildRejectionNode();
private:
    std::string _id;
    std::string _mime_type;
    SIProfile* _profile;
    FeatureNegotiation* _fneg;
};

};

#endif //INCL_STREAMINITIATION_HH
