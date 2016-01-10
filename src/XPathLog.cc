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

#include "XPathLog.hh"
#include "GabberApp.hh"
#include "LogManager.hh"

#include <jabberoo/judo.hpp>

using namespace Gabber;

XPathLog::XPathLog(LogManager& lm, const std::string& query, 
                   const std::string& jid, bool in) : 
    m_LogManager(lm), m_jid(jid), m_in(in)
{ 
    m_query = GabberApp::getSingleton().getSession().registerXPath(query,
        SigC::slot(*this, &XPathLog::on_node), in);
}

XPathLog::~XPathLog()
{
    GabberApp::getSingleton().getSession().unregisterXPath(m_query, m_in);
}

void XPathLog::check(const judo::Element& elem)
{
    if(m_query->check(elem))
    {
        on_node(elem);
    }
}

void XPathLog::on_node(const judo::Element& elem)
{
    if ( !elem.hasAttrib("gabber:timestamp") )
    {
        // XXX I hate having to make a copy here.  JabberOO 2, jsomm,
        // whatever we make as the next underlayer should allow for
        // outgoing packet changes more easiliy.
        judo::Element e(elem);
        e.putAttrib("gabber:timestamp", Util::build_timestamp(time(NULL)));
        m_LogManager.log(m_jid) << e.toString() << std::endl;
    }
    else
    {
        m_LogManager.log(m_jid) << elem.toString() << std::endl;
    }
}
