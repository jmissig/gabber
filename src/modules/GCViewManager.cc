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

#include "GCViewManager.hh"
#include "GabberApp.hh"

#include <sigc++/slot.h>

using namespace Gabber;

static GCViewManager* gcm_instance;

extern "C" const gchar* g_module_check_init(GModule* module)
{
    gcm_instance = new GCViewManager();
    return NULL;
}

extern "C" void g_module_unload(GModule* module)
{
    delete gcm_instance;
}

GCViewManager::GCViewManager()
{
    _signal_connection = GabberApp::getSingleton().sigJoinGCDlg.connect(
        SigC::slot(*this, &GCViewManager::on_join_gc_dlg));
}

GCViewManager::~GCViewManager()
{
    _signal_connection.disconnect();

    for(GCViewMap::iterator it = _views.begin(); it != _views.end(); ++it)
    {
        it->second->close();
    }
}

void GCViewManager::join_groupchat(const Glib::ustring& jid, 
                                   const Glib::ustring& nick)
{
    GCViewMap::iterator it = _views.find(jid);

    // If we already have it we raise it
    if (it != _views.end())
    {
        it->second->raise();
        return;
    }

    _views.insert(GCViewMap::value_type(jid, 
        new GCView(*this, jid, nick)));

}

void GCViewManager::end_groupchat(const Glib::ustring& jid)
{
    // XXX do more?  Rely on the self destruction?
    _views.erase(jid);
}
    
void GCViewManager::on_join_gc_dlg()
{
     GCJoinDlg::display(*this);
}

