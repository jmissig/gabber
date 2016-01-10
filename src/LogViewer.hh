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

#ifndef INCL_GABBER_LOGVIEWER_HH
#define INCL_GABBER_LOGVIEWER_HH

#include "BaseGabberWindow.hh"
#include "PlainTextView.hh"

#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

namespace Gabber {

class LogViewer : public BaseGabberWindow
{
public:
    LogViewer(const std::string& jid);
    ~LogViewer();
private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        { add(name); add(path); }

        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> path;
    };

    std::string _jid;
    std::string _nickname;
    std::string _local_nickname;
    PlainTextView* _logView;
    Gtk::TreeView* _logMonths;
    Glib::RefPtr<Gtk::ListStore> _month_store;
    Glib::RefPtr<Gtk::TreeSelection> _selection;
    ModelColumns _columns;

    void init();
    void show_log(const judo::Element& root);
    void on_month_changed();
};

};

#endif // INCL_GABBER_LOGVIEWER_HH
