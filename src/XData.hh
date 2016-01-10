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

#ifndef INCL_XDATA_HH
#define INCL_XDATA_HH

#include "GabberUtility.hh"

#include <jabberoo/judo.hpp>

#include <string>
#include <list>
#include <functional>
#include <algorithm>

namespace Gabber {

/** Wrapper for Jabber Data Gathering and Reporting. */
class XData
{
public:
    class Field
    {
    public:
        struct Option
        {
            std::string label;
            std::string value; // templatetize? XXX
        };

        struct OptionNodeBuilder
        {
            OptionNodeBuilder(judo::Element* field_) : field(field_)
            { }
            void operator()(Field::Option* opt)
            { 
                judo::Element* opt_elem = field->addElement("option");
                if (!opt->label.empty())
                    opt_elem->putAttrib("label", opt->label);
                opt_elem->addElement("value", opt->value);
            }
            judo::Element* field;
        };

        // XXX Make this generic a bit more so we can pass it what value
        struct OptionValueFinder : 
            public std::unary_function<Field::Option*, bool>
        {
            OptionValueFinder(const std::string& value_) : value(value_) { }
            bool operator()(Field::Option* option) 
            { return option->value == value; }
            std::string value;
        };
        
        typedef std::list<Option*> OptionList;
    public:
        Field();
        Field(const judo::Element& elem);
        ~Field()
        { for_each( _options.begin(), _options.end(), Util::ListDeleter()); }

        std::string getLabel() const
        { return _label; }
        void setLabel(const std::string& label)
        { _label = label; }

        std::string getVar() const
        { return _var; }
        void setVar(const std::string& var)
        { _var = var; }

        std::string getType() const
        { return _type; }
        void setType(const std::string& type)
        { _type = type; }

        std::string  getDesc() const
        { return _desc; }
        void setDesc(const std::string& desc)
        { _desc = desc; }

        bool getRequired() const
        { return _required; }
        void setRequired(bool req)
        { _required = req; }

        OptionList& getOptions()
        { return _options; }

        void addOption(Option* opt)
        { _options.push_back(opt); }

        std::string getValue() const
        { 
            if (_cur_option)
                return _cur_option->value;
            else
                return std::string("");
        }
        void setCurOption(Option* opt)
        { _cur_option = opt; }

        judo::Element* buildInfoNode();
        judo::Element* buildValueNode();
        judo::Element* buildOptionNode();
    private:
        std::list<Option*> _options;
        Option* _cur_option;
        std::string _label;
        std::string _var;
        std::string _type;
        std::string _desc;
        bool _required;
    };

    struct FieldNodeBuilder
    {
        FieldNodeBuilder(judo::Element* paren_, 
                std::mem_fun_t<judo::Element*, XData::Field> func_) : 
            paren(paren_), func(func_)
        { }

        void operator()(Field* field)
        {
            paren->appendChild(func(field));
        }

        judo::Element* paren;
        std::mem_fun_t<judo::Element*, XData::Field> func;
    };

    struct FieldVarFinder : public std::unary_function<Field*, bool>
    {
        FieldVarFinder(const std::string& var_) : var(var_) { }
        bool operator()(Field* field)
        { return field->getVar() == var; }
        std::string var;
    };

    typedef std::list<XData::Field*> FieldList;
public:
    XData();
    XData(const judo::Element& elem);
    ~XData()
    { std::for_each( _fields.begin(), _fields.end(), Util::ListDeleter()); }

    FieldList& getFields()
    { return _fields; }
    void addField(Field* field)
    { _fields.push_back(field); }

    Field* getField(const std::string& var)
    {
        FieldList::iterator it = 
            std::find_if(_fields.begin(), _fields.end(), 
                    FieldVarFinder(var));
        if (it != _fields.end())
        {
            return *it;
        }
        return NULL;
    }

    std::string getTitle() const
    { return _title; }
    void setTitle(const std::string& title)
    { _title = title; }

    std::string getInstructions() const 
    { return _instructions; }
    void setInstructions(const std::string& instructions)
    { _instructions = instructions; }

    judo::Element* buildNode(const std::string& type);
private:
    FieldList _fields;
    std::string _title;
    std::string _instructions;
};

};

#endif // INCL_XDATA_HH
