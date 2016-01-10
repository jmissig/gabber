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

#ifndef INCL_GCMANAGER_HH
#define INCL_GCMANAGER_HH

#include "GCView.hh"
#include "GCJoinDlg.hh"

#include <sigc++/object.h>
#include <sigc++/signal.h>
#include <jabberoo/JID.hh>

namespace Gabber {

/**
* Manage groupchat view instances
*/
class GCViewManager : public SigC::Object
{
public:
    GCViewManager();
    ~GCViewManager();

    /**
     * This tells the manager to join a groupchat
     * If a view is already open to the specified jid we'll raise it, otherwise
     * a new view is created and stored.
     * @param jid The userhost jid of the groupchat to join.
     * @param nick THe nickname to use in the grupochat.
     */
    void join_groupchat(const Glib::ustring& jid, const Glib::ustring& nick);

    /**
     * Removes a jid from the list of currently active groupchats.
     * @param jid The userhost jid to remove.
     */
    void end_groupchat(const Glib::ustring& jid);
protected:
    // GabberApp callbacks
    void on_join_gc_dlg();

private:
    typedef std::map<std::string, GCView*, jabberoo::JID::Compare> GCViewMap;
    GCViewMap           _views;
    SigC::Connection    _signal_connection;
}; // class GCViewManager

}; // namespace Gabber

#endif // INCL_GCMANAGER_HH
