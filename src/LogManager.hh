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

#ifndef INCL_LOGMANAGER_HH
#define INCL_LOGMANAGER_HH

#include "fwd.h"

#include <sigc++/object.h>

#include <string>
#include <map>
#include <fstream>

namespace Gabber {

class XPathLog;

/**
* Manages log streams
*/
class LogManager : public SigC::Object
{
public:
    LogManager();
    ~LogManager();

    /**
    * Get a stream to the debug log
    * This will log to the correct location only if debugging is enabled.
    */
    std::ostream& debug();
    /**
    * Get a stream to the log
    * @param jid The jid to log to, only user@host is used.
    */
    std::ostream& log(const std::string& jid);
    
    /**
     * Creates a XPathLog object.
     *
     * @param xpath The XPath Query string to log for.
     * @param jid The jid the XPath should be stored under.
     * @param in Whether it's on the incoming or outgoing stream, defaults to true.
     * @return The XPathLog object that was created.
     */
    XPathLog* create_xpath_log(const std::string& xpath, const std::string& jid,
                               bool in=true);

    /**
    * Get the path to the specified users log.
    * @param jid The jid of the desired log.
    * @return The log path
    */
    std::string get_log_path(const std::string& jid);
private:
    typedef std::map<std::string, std::pair<std::ofstream*, int> > LogMap;
    LogMap _logs;
    std::ofstream _debug_log;
    std::string _log_dir;

    void rotate_logs();
    void move_old_logs();
    void timestamp_element(judo::Element& elem);
    void build_log_dir();
}; // class LogManager

}; // namespace Gabber
#endif // INCL_LOGMANAGER_HH
