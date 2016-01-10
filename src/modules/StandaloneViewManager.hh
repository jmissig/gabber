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

#ifndef INCL_STANDALONEVIEWMANAGER_HH
#define INCL_STANDALONEVIEWMANAGER_HH

#include "StandaloneView.hh"
#include <jabberoo/XPath.h>
#include <jabberoo/JID.hh>
#include <gtkmm/menuitem.h>

namespace Gabber {

/**
* Manage the views of standalone messages
*/
class StandaloneViewManager : public SigC::Object
{
public:
    StandaloneViewManager();
    ~StandaloneViewManager();
    /**
    * This signals that a view is no longer active.
    * This _does not_ delete the view.  The view should self destruct when it
    * is fully complete.
    * @param jid The full jid to remove
    */
    void releaseView(const std::string& jid);
protected:
    void on_menu_item_activate();
    void on_action_menu_item_activate();
    void on_message_node(const judo::Element& elem);
    void on_queue_flushing();
private:
    typedef std::map<std::string, StandaloneView*, jabberoo::JID::Compare> ViewMap;
    judo::XPath::Query*  _xp_query;
    ViewMap              _views;
    Gtk::MenuItem        _menu_item;
    Gtk::MenuItem        _action_menu_item;
}; // class StandaloneViewManager

} // namespace Gabber

#endif // INCL_STANDALONEVIEWMANAGER_HH
