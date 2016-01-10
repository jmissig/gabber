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

#ifndef INCL_FEATURENEGOTIATION_HH
#define INCL_FEATURENEGOTIATION_HH

#include "XData.hh"

namespace Gabber {

/** Handles the http://jabber.org/protocol/feature-neg namespace.
 */
class FeatureNegotiation
{
public:
    /** Initializes from the passed in feature negotiation element.
     * @param elem The &lt;query&gt; element containing the negotiation.
     */
    FeatureNegotiation(const judo::Element& elem)
    {
        const judo::Element* x = elem.findElement("x");
        _xdata = new XData(*x);
    }

    /** Creates an empty negotiation.
     */
    FeatureNegotiation() : _xdata(new XData)
    { }

    ~FeatureNegotiation()
    { delete _xdata; }

    XData& getXData() const
    { return *_xdata; }

    bool check(const std::string& field_name, const std::string& value)
    {
        XData::Field* field = _xdata->getField(field_name);
        if (field)
        {
            XData::Field::OptionList& options = field->getOptions();
            return std::find_if(options.begin(), options.end(), 
                XData::Field::OptionValueFinder(value)) != options.end();
        }
        return false;
    }

    void choose(const std::string& field_name, const std::string& value)
    {
        XData::Field* field = _xdata->getField(field_name);
        if (field)
        {
            XData::Field::OptionList& options = field->getOptions();
            XData::Field::OptionList::iterator it = 
                std::find_if(options.begin(), options.end(), 
                        XData::Field::OptionValueFinder(value));
            if ( it == options.end() )
                return;
            field->setCurOption(*it);
        }
    }

    judo::Element* buildNode()
    {
        judo::Element* fneg = new judo::Element("query");
        fneg->putAttrib("xmlns", "http://jabber.org/protocol/feature-neg");
        fneg->appendChild(_xdata->buildNode("form"));
        return fneg;
    }
    
    judo::Element* buildResultNode()
    {
        judo::Element* fneg = new judo::Element("query");
        fneg->putAttrib("xmlns", "http://jabber.org/protocol/feature-neg");
        fneg->appendChild(_xdata->buildNode("submit"));
        return fneg;
    }
private:
    XData* _xdata;
};

};

#endif // INCL_FEATURENEGOTIATION_HH
