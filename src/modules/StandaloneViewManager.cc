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

#include "StandaloneViewManager.hh"

#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "Menus.hh"
#include "StandaloneSendDlg.hh"

#include <sigc++/slot.h>
#include "intl.h"

using namespace Gabber;

static StandaloneViewManager* svm_instance;

extern "C" const gchar* g_module_check_init(GModule* module)
{
    svm_instance = new StandaloneViewManager();
    return NULL;
}

extern "C" void g_module_unload(GModule* module)
{ delete svm_instance; }

StandaloneViewManager::StandaloneViewManager() :
     _menu_item(_("Standalone Message..."), true),
     _action_menu_item(_("New _Standalone Message..."), true)
{
     // The query used to be "/message[not(@type) or @type='normal']"
     // XXX: registerXPath_unhandled or something like it should be used
     // instead of just handling them all
    _xp_query = GabberApp::getSingleton().getSession().registerXPath(
        "/message[not(@type='chat') and not(@type='groupchat') and not(@type='error')]", 
        SigC::slot(*this, &StandaloneViewManager::on_message_node));
    _menu_item.signal_activate().connect(SigC::slot(*this, 
        &StandaloneViewManager::on_menu_item_activate));
    Popups::User::getSingleton().addItem(&_menu_item);

    _action_menu_item.signal_activate().connect(SigC::slot(*this,
        &StandaloneViewManager::on_action_menu_item_activate));
    Popups::ActionMenu::getSingleton().addItem(&_action_menu_item);

    G_App.getPacketQueue().queue_flushing_event.connect(
        SigC::slot(*this, &StandaloneViewManager::on_queue_flushing));
}

StandaloneViewManager::~StandaloneViewManager()
{
    GabberApp::getSingleton().getSession().unregisterXPath(_xp_query);
    ViewMap::iterator it = _views.begin();
    while (it != _views.end())
    {
        ViewMap::iterator next = it;
        ++next;

        delete (*it).second;

        it = next;
    }
}

void StandaloneViewManager::releaseView(const std::string& jid)
{
    _views.erase(jabberoo::JID::getUserHost(jid));
}

void StandaloneViewManager::on_menu_item_activate()
{
     Glib::ustring to = Popups::User::getSingleton().getSelectedJID();
     StandaloneSendDlg::display(G_App, to);
}

void StandaloneViewManager::on_action_menu_item_activate()
{
     StandaloneSendDlg::display(G_App, Glib::ustring());
}

void StandaloneViewManager::on_message_node(const judo::Element& elem)
{
    // This isn't a valid judo::node
    if (elem.findElement("body") == NULL)
    {
        return;
    }

     
     std::string from = jabberoo::JID::getUserHost(elem.getAttrib("from"));
     ViewMap::iterator it = _views.find(from);
     
     // if it was queued, then we're either starting a new window or displaying in existing
     if (elem.cmpAttrib("gabber:queued", "true"))
     {
          if (it == _views.end())
          {
               _views.insert(ViewMap::value_type(from, new StandaloneView(*this, elem)));    
          }
          else
          {
               it->second->display_message(elem);
          }
     }
     // if it wasn't queued, and we want autodisplay, and we should autodisplay, and there
     // is not an existing window, then we display
     else if (G_App.getConfigurator().get_bool(Keys::intrface.messages_autodisplay)
              && (G_App.get_my_presence().getShow() == jabberoo::Presence::stChat
                  || G_App.get_my_presence().getShow() == jabberoo::Presence::stOnline)
              && (it == _views.end())
              )
     {
          _views.insert(ViewMap::value_type(from, 
                                            new StandaloneView(*this, elem)));  
     }
     // otherwise we just want to queue
     else
     {
          G_App.getPacketQueue().push(new judo::Element(elem), "message-standalone.png", "StandaloneView");
     }
}

void StandaloneViewManager::on_queue_flushing()
{
    PacketQueue& pq(G_App.getPacketQueue());
    PacketQueue::queue_iterator it = pq.begin();
    while (it != pq.end())
    {
        PacketQueue::queue_iterator next = it;
        ++next;

        // Only get the first message off the queue per jid
        if ( (it->type == "StandaloneView") && 
             (_views.count(it->jid) == 0) )
        {
            pq.pop(it);
        }

        it = next;
    }
}

