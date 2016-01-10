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

#ifndef INCL_XPATHLOG_HH
#define INCL_XPATHLOG_HH

#include <sigc++/object.h>

#include <string>

namespace judo {
    class Element;

    namespace XPath {
        class Query;
    }
}

namespace Gabber {

class LogManager;

/**
 * Logs based on a XPath.
 *
 * This class will log to the specified jid when the XPath query hits.
 */
class XPathLog : public SigC::Object
{
public:
    XPathLog(LogManager& lm, const std::string& query, 
             const std::string& jid, bool in);
    ~XPathLog();

    /**
     * Force the logger to check a packet, and log it if it matches.
     * @param elem The element to check
     */
    void check(const judo::Element& elem);
private:
    LogManager& m_LogManager;
    std::string m_jid;
    bool m_in;
    judo::XPath::Query* m_query;

    void on_node(const judo::Element& elem);
}; // class XPathLog

}; // namespace Gabber

#endif // INCL_XPATHLOG_HH
