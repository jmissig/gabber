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
 *  Copyright (c) 2002-2004 Julian Missig
 */

#include "LogViewer.hh"
#include "GabberApp.hh"
#include "LogManager.hh"
#include "GabberWidgets.hh"
#include "intl.h"

#include <jabberoo/filestream.hh>

#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>
#include <gtkmm/liststore.h>

#include <glibmm/fileutils.h>

using namespace Gabber;

LogViewer::LogViewer(const std::string& jid) : 
    BaseGabberWindow("LogViewer_win"), _jid(jid)
{
    // Init the defaults
    init();
}

LogViewer::~LogViewer()
{ 
    delete _logView;
}

void LogViewer::init()
{
    PrettyJID* pj = Gtk::manage(new PrettyJID(_jid, "", 
        PrettyJID::dtNickRes, false));
    pj->show();

    Gtk::HBox* box;
    get_widget("PrettyJID_hbox", box);
    box->pack_start(*pj, false, true, 0);

    Gtk::ScrolledWindow* scroll_win;
    get_widget("LogView_scroll", scroll_win);
    _logView = new PlainTextView(scroll_win, false);
    _logView->clear();
    scroll_win->show_all();

    get_widget("LogMonths_treeview", _logMonths);
    _month_store = Gtk::ListStore::create(_columns);
    _logMonths->set_model(_month_store);
    _logMonths->append_column("Logs", _columns.name);

    _nickname = pj->get_nickname();
    getGtkWindow()->set_title(Util::substitute(_("Logs for %s"), _nickname));

    _local_nickname = G_App.getSession().getUserName();

    _selection = _logMonths->get_selection();
    _selection->signal_changed().connect(
        SigC::slot(*this, &LogViewer::on_month_changed));

    std::string cur_log(G_App.getLogManager().get_log_path(_jid));
    Glib::ustring log_path = Glib::build_filename(
        Glib::build_filename(Glib::get_home_dir(), ".Gabber"), "Logs");
    Glib::Dir log_dir(log_path);
    std::string userhost(jabberoo::JID::getUserHost(_jid));
    for(Glib::Dir::iterator it = log_dir.begin(); it != log_dir.end(); ++it)
    {
        if (Glib::file_test(Glib::build_filename(log_path, *it),
            Glib::FILE_TEST_IS_DIR))
        {
            Glib::ustring path = Glib::build_filename(
                Glib::build_filename(log_path, *it), userhost);

            if (Glib::file_test(path, Glib::FILE_TEST_EXISTS))
            {
                Gtk::TreeModel::iterator sit = _month_store->append();
                Gtk::TreeModel::Row row = *sit;
                row[_columns.path] = path;
                if ( (*it) == "old" )
                {
                    row[_columns.name] = "Old Logs";
                }
                else
                {
                    int month, year;
                    sscanf( (*it).c_str(), "%4d-%2d", &year, &month);
                    Glib::Date date(1, Glib::Date::Month(month), 
                        Glib::Date::Year(year) );
                    row[_columns.name] = date.format_string("%B - %Y");
                }

                // If this is the log we'll be displaying, select it
                if (path == cur_log)
                {
                    _selection->select(sit);
                }
            }
        }
    }
}

void LogViewer::show_log(const judo::Element& root)
{
    // XXX Only handles messages so far
    std::string userhost(jabberoo::JID::getUserHost(_jid));

    for(judo::Element::const_iterator it = root.begin();
        it != root.end(); ++it)
    {
        if ( ((*it)->getType() == judo::Node::ntElement) &&
             ((*it)->getName() == "message") )
        {
            const judo::Element* elem = static_cast<const judo::Element*>(*it);
            const judo::Element* body = elem->findElement("body");
            // Only process if it has a body
            if (!body)
            {
                continue;
            }

            // Figure out if they sent to us or we sent to them
            std::string to(jabberoo::JID::getUserHost(elem->getAttrib("to")));
            if (to == userhost)
            {
                _logView->append(jabberoo::Message(*elem), _local_nickname, true);
            }
            else
            {
                _logView->append(jabberoo::Message(*elem), _nickname);
            }
        }
    }
}

void LogViewer::on_month_changed()
{
    Gtk::TreeRow row = *_selection->get_selected();
    _logView->clear();
    Glib::ustring path = row[_columns.path];
    jabberoo::FileStream fs(path.c_str());
    if (fs.ParseFile("logview"))
    {
        judo::Element* root = fs.getRoot();
        show_log(*root);
        delete root;
    }
    // XXX error if not parsed
}
