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
 *  Copyright (c) 2004 Julian Missig
 */

#ifndef INCL_RAWVIEWMANAGER_HH
#define INCL_RAWVIEWMANAGER_HH

#include <sigc++/object.h>
#include <gtkmm/menuitem.h>

namespace Gabber {

/**
* Manage the views of standalone messages
*/
class RawViewManager : public SigC::Object
{
public:
    RawViewManager();
    //~RawViewManager();
    /**
    * This signals that a view is no longer active.
    * This _does not_ delete the view.  The view should self destruct when it
    * is fully complete.
    * @param jid The full jid to remove
    */
    //void releaseView(const std::string& jid);
protected:
    void on_action_menu_item_activate();
private:
    Gtk::MenuItem _action_menu_item;
}; // class RawViewManager

} // namespace Gabber

#endif // INCL_RAWVIEWMANAGER_HH

