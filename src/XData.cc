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

#include "XData.hh"

#include <jabberoo/judo.hpp>

using namespace Gabber;

XData::Field::Field() : _cur_option(NULL), _required(false)
{
}

XData::Field::Field(const judo::Element& elem) : 
    _cur_option(NULL), _required(false)
{
    _var = elem.getAttrib("var");
    _type = elem.getAttrib("type");
    _label = elem.getAttrib("label");
    _desc = elem.getChildCData("desc");
    _required = elem.findElement("required") == NULL ? true : false;

    std::string cur_val = elem.getChildCData("value");

    for(judo::Element::const_iterator it = elem.begin(); it != elem.end(); ++it)
    {
        if ( ((*it)->getType() == judo::Node::ntElement) && 
             ((*it)->getName() == "option") )
        {
            const judo::Element* opt_elem = 
                static_cast<const judo::Element*>(*it);
            Option* opt = new Option;
            opt->value = opt_elem->getChildCData("value");
            opt->label = opt_elem->getAttrib("label");
            if (opt->value == cur_val)
            {
                _cur_option = opt;
            }
            _options.push_back(opt);
        }
    }
}

judo::Element* XData::Field::buildInfoNode()
{
    judo::Element* field = new judo::Element("field");
    if (!_var.empty())
        field->putAttrib("var", _var);
    if (!_label.empty())
        field->putAttrib("label", _label);
    if (!_type.empty())
        field->putAttrib("type", _type);

    return field;
}

judo::Element* XData::Field::buildValueNode()
{
    judo::Element* field = new judo::Element("field");
    field->putAttrib("var", _var);
    field->addElement("value", _cur_option->value);

    return field;
}

judo::Element* XData::Field::buildOptionNode()
{
    judo::Element* field = new judo::Element("field");
    if (!_var.empty())
        field->putAttrib("var", _var);
    if (!_type.empty())
        field->putAttrib("type", _type);
    if (!_label.empty())
        field->putAttrib("label", _label);

    if (!_desc.empty())
        field->addElement("desc", _desc);

    if (_cur_option)
        field->addElement("value", _cur_option->value);

    if (_required)
        field->addElement("required");

    for_each(_options.begin(), _options.end(), OptionNodeBuilder(field));

    return field;
}

XData::XData()
{
}

XData::XData(const judo::Element& elem)
{
    _title = elem.getChildCData("title");
    _instructions = elem.getChildCData("instructions");

    for ( judo::Element::const_iterator it = elem.begin(); 
            it != elem.end(); ++it)
    {
        if ((*it)->getType() == judo::Node::ntElement && 
            (*it)->getName() == "field")
        {
            _fields.push_back(
                new Field(*static_cast<const judo::Element*>(*it)));
        }
    }
}  

judo::Element* XData::buildNode(const std::string& type)
{
    judo::Element* x = new judo::Element("x");
    x->putAttrib("xmlns", "jabber:x:data");
    x->putAttrib("type", type);

    if (!_title.empty())
        x->addElement("title", _title);
    if (!_instructions.empty())
        x->addElement("instructions", _instructions);
    
    if( type == "form" )
    {
        for_each(_fields.begin(), _fields.end(), 
            FieldNodeBuilder(x, std::mem_fun(&Field::buildOptionNode)));
    }
    else if (type == "submit")
    {
        for_each(_fields.begin(), _fields.end(),
            FieldNodeBuilder(x, std::mem_fun(&Field::buildValueNode)));
    }
    // XXX Other types
    //
    
    return x;
}
