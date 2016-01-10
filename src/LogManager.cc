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

#include "LogManager.hh"
#include "XPathLog.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"

#include <jabberoo/JID.hh>

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#  include <io.h>
#  define mkdir(a, b) ::mkdir(a)
#endif

#include <sstream>

using namespace Gabber;

LogManager::LogManager() :
    _log_dir(Glib::build_filename(Glib::get_home_dir(),".Gabber"))
{
    // Make the gabber dir if it doesn't exist
    if (!Glib::file_test(_log_dir, 
        Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_log_dir.c_str(), 0700);
    }

    // We actually drop the debug log in the main dir, rest of the logs
    // in a subdir
    std::string debug_path = Glib::build_filename(_log_dir, "debug.log");
    _debug_log.open(debug_path.c_str());

    _log_dir = Glib::build_filename(_log_dir, "Logs");
    if (!Glib::file_test(_log_dir, 
        Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_log_dir.c_str(), 0700);
    }

    // Move the logs to the new log dir if people don't have them there
    move_old_logs();

    build_log_dir();

    // Hook up the packet timestampper
    G_App.getSession().evtOnRecvElement.connect(
        SigC::slot(*this, &LogManager::timestamp_element));
}

LogManager::~LogManager()
{
    for(LogMap::iterator i = _logs.begin(); i != _logs.end(); ++i)
    {
        i->second.first->close();
        delete i->second.first;
    }
    _debug_log.close();
}

// XXX Time stamp option for debug logs?
std::ostream& LogManager::debug()
{ return _debug_log; }

std::ostream& LogManager::log(const std::string& jid)
{
    rotate_logs();

    LogMap::iterator it = _logs.find(jid);
    // If we don't have this log yet we need to remove one and add this one
    if (it != _logs.end())
    {
        return *it->second.first;
    }

    // XXX make this not hardcoded?
    if (_logs.size() > 5)
    {
        LogMap::iterator max = _logs.begin();
        for(LogMap::iterator i = _logs.begin(); i != _logs.end(); ++i)
        {
            if (i->second.second > max->second.second)
            {
                max = i;
            }
        }
        max->second.first->close();
        delete max->second.first;
        _logs.erase(max);
    }

    // setup and insert the new one
    std::string log_path = Glib::build_filename(_log_dir, 
        jabberoo::JID::getUserHost(jid));
    std::ofstream* log = new std::ofstream;
    log->open(log_path.c_str(), std::ios::app|std::ios::out);
    if (!log->is_open())
        std::cout << "Unable to open log: " << log_path << std::endl;
    _logs.insert(LogMap::value_type(jid, std::make_pair(log, time(NULL))));

    return *log;
}

XPathLog* LogManager::create_xpath_log(const std::string& xpath, 
                                       const std::string& jid, bool in)
{
    return new XPathLog(*this, xpath, jid, in);
}

std::string LogManager::get_log_path(const std::string& jid)
{
    return Glib::build_filename(_log_dir, jabberoo::JID::getUserHost(jid));
}

void LogManager::rotate_logs()
{
    int last_month =
        G_App.getConfigurator().get_int(Keys::history.last_rotate_month);
    int last_year =
        G_App.getConfigurator().get_int(Keys::history.last_rotate_year);
    time_t t = time(NULL);
    struct tm* curtm = localtime(&t);
    if ( (curtm->tm_year == last_year) && (curtm->tm_mon <= last_month) )
    {
        return;
    }

    if (!_logs.empty())
    {
        for(LogMap::iterator i = _logs.begin(); i != _logs.end(); ++i)
        {
            i->second.first->close();
            delete i->second.first;
        }
        _logs.clear();
    }

    build_log_dir();

    G_App.getConfigurator().set(Keys::history.last_rotate_month, curtm->tm_mon);
    G_App.getConfigurator().set(Keys::history.last_rotate_year, curtm->tm_year);
}

void LogManager::move_old_logs()
{
    bool moved = G_App.getConfigurator().get_bool(Keys::history.moved_old_logs);
    
    if (moved)
        return;

    std::string save_to_dir(Glib::build_filename(_log_dir, "old"));
    mkdir(save_to_dir.c_str(), 0700);
    std::string old_log_dir(Glib::build_filename(Glib::get_home_dir(),".Gabber"));
    Glib::Dir oldLogDir(old_log_dir);
    for (Glib::Dir::iterator it = oldLogDir.begin(); it != oldLogDir.end(); ++it)
    {
        std::string cur_file = Glib::build_filename(old_log_dir, *it);
        if (Glib::file_test(cur_file, Glib::FILE_TEST_IS_DIR) || 
            (*it) == "debug.log")
        {
            continue;
        }

        ::rename(cur_file.c_str(),
            Glib::build_filename(save_to_dir, *it).c_str());
    }

    G_App.getConfigurator().set(Keys::history.moved_old_logs, true);
}

void LogManager::timestamp_element(judo::Element& elem)
{
    // We add a gabber:timestamp so we know when we received it
    elem.putAttrib("gabber:timestamp", Util::build_timestamp(time(NULL)) );
}

void LogManager::build_log_dir()
{
    _log_dir = Glib::build_filename(Glib::get_home_dir(), ".Gabber");

    // Make the gabber dir if it doesn't exist
    if (!Glib::file_test(_log_dir, 
        Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_log_dir.c_str(), 0700);
    }

    _log_dir = Glib::build_filename(_log_dir, "Logs");
    if (!Glib::file_test(_log_dir, 
        Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_log_dir.c_str(), 0700);
    }

    // Now make sure we're pointing at the correct dir
    time_t t = time(NULL);
    struct tm* curtm = localtime(&t);
    std::ostringstream dirstr;
    dirstr << (1900 + curtm->tm_year) << "-" << (curtm->tm_mon + 1);

    std::string dirname = dirstr.str();
    _log_dir = Glib::build_filename(_log_dir, dirname);

    if (!Glib::file_test(_log_dir, 
        Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_log_dir.c_str(), 0700);
    }
}
