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

#include "StreamInitiation.hh"
#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "FTProfile.hh"

#include <jabberoo/judo.hpp>
#include <algorithm>

using namespace Gabber;

SI::SI(const judo::Element& elem)
{
    _id = elem.getAttrib("id");
    printf("set SI id to %s\n", _id.c_str());
    _mime_type = elem.getAttrib("mime-type");
    std::string profile_ns = elem.getAttrib("profile");

    // XXX We need to handle other profiles in a clean way.  Factory function?
    judo::Element::const_iterator it = std::find_if(elem.begin(), elem.end(), 
        Util::MatchNamespace(profile_ns));

    if (it == elem.end())
    {
        _profile = NULL;
    }
    else
    {
        _profile = new FTProfile(*static_cast<const judo::Element*>(*it));
    }

    it = std::find_if(elem.begin(), elem.end(), 
        Util::MatchNamespace("http://jabber.org/protocol/feature-neg"));
    if (it == elem.end())
    {
        // XXX Is this appropiate?
        _fneg = new FeatureNegotiation;
    }
    else
    {
        _fneg = new FeatureNegotiation(*static_cast<const judo::Element*>(*it));
    }
}

void SI::setProfile(SIProfile* profile)
{
    if (_profile != NULL)
    {
        delete _profile;
    }
    _profile = profile;
}

judo::Element* SI::buildNode()
{
    assert(_profile);

    judo::Element* si = new judo::Element("si");
    si->putAttrib("xmlns", "http://jabber.org/protocol/si");
    if (_id.empty())
    {
        _id = G_App.getSession().getNextID();
    }
    si->putAttrib("id", _id);
    si->putAttrib("mime-type", _mime_type);
    si->putAttrib("profile", _profile->getNamespace());
    si->appendChild(_profile->buildNode());
    si->appendChild(_fneg->buildNode());

    return si;
}

judo::Element* SI::buildResultNode()
{
    judo::Element* si = new judo::Element("si");
    si->putAttrib("xmlns", "http://jabber.org/protocol/si");
    si->putAttrib("id", _id);
    judo::Element* profile = _profile->buildResultNode();
    if (profile)
    {
        si->appendChild(profile);
        si->putAttrib("profile", _profile->getNamespace());

    }
    si->appendChild(_fneg->buildResultNode());

    return si;
}

judo::Element* SI::buildInvalidStreamsNode()
{ 
    judo::Element* iq = new judo::Element("iq");
    iq->putAttrib("type", "error");
    judo::Element* err = iq->addElement("error");
    err->putAttrib("code", "400");
    err->putAttrib("type", "cancel");
    err->addCDATA("No Valid Streams", 16);
    judo::Element* tmp = err->addElement("bad-request");
    tmp->putAttrib("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
    tmp = err->addElement("no-valid-streams");
    tmp->putAttrib("xmlns", "http://jabber.org/protocol/si");

    return iq;
}
judo::Element* SI::buildInvalidProfileNode()
{ 
    judo::Element* iq = new judo::Element("iq");
    iq->putAttrib("type", "error");
    judo::Element* err = iq->addElement("error");
    err->putAttrib("code", "400");
    err->putAttrib("type", "cancel");
    err->addCDATA("Bad Profile", 11);
    judo::Element* tmp = err->addElement("bad-request");
    tmp->putAttrib("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
    tmp = err->addElement("bad-profile");
    tmp->putAttrib("xmlns", "http://jabber.org/protocol/si");

    return iq;
}

judo::Element* SI::buildRejectionNode()
{
    judo::Element* iq = new judo::Element("iq");
    iq->putAttrib("type", "error");
    judo::Element* err = iq->addElement("error");
    err->putAttrib("code", "403");
    err->putAttrib("type", "cancel");
    err->addCDATA("Offer Declined", 14);
    judo::Element* tmp = err->addElement("forbidden");
    tmp->putAttrib("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    return iq;
}
