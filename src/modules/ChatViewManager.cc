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

#include "ConfigPaths.hh"
#include "ChatViewManager.hh"
#include "GabberApp.hh"
#include "Menus.hh"

#include <sigc++/slot.h>
#include "intl.h"

#include <gtkmm/window.h>

namespace Gabber {

static ChatViewManager* cvm_instance;

extern "C"   const gchar* g_module_check_init (GModule *module) {
    cvm_instance = new ChatViewManager();
    return NULL;
}

extern "C" void g_module_unload (GModule* module) {
    delete cvm_instance;
}

ChatViewManager::ChatViewManager() :
    BaseModule("ChatView", "Provides support for chat messages."), 
    _menu_item(_("_One-on-one Chat..."), true),
    _action_menu_item(_("New _One-on-one Chat..."), true)
{
    _xp_query = G_App.getSession().registerXPath("/message[@type='chat']", 
            SigC::slot(*this, &ChatViewManager::on_chat_node));
    _menu_item.signal_activate().connect(SigC::slot(*this, 
            &ChatViewManager::on_menu_item_activate));
    Popups::User::getSingleton().addItem(&_menu_item);

    _action_menu_item.signal_activate().connect(SigC::slot(*this,
            &ChatViewManager::on_action_menu_item_activate));
    G_App.getPacketQueue().queue_flushing_event.connect(
        SigC::slot(*this, &ChatViewManager::on_queue_flushing));
//    Popups::ActionMenu::getSingleton().addItem(&_action_menu_item);
}

ChatViewManager::~ChatViewManager()
{
    G_App.getSession().unregisterXPath(_xp_query);
    typedef std::map<std::string, ChatView*>::iterator IT;
    IT it = _chats.begin();
    while (it != _chats.end())
    {
        IT next = it;
        ++next;
        
        delete (*it).second;

        it = next;
    }
}

void ChatViewManager::register_chat(const std::string& jid, ChatView* chat)
{
     _chats.insert(make_pair(jid, chat));
}

void ChatViewManager::releaseChat(const std::string& from)
{
    _chats.erase(from);
}

void ChatViewManager::on_menu_item_activate()
{
    std::string to = Popups::User::getSingleton().getSelectedJID();
    typedef std::map<std::string, ChatView*>::iterator IT;
    bool foundchat = false;

    // find all chats with matching JID::getUserHost() (jid minus resource)
    // and present them too. this will highlight all open chats and open a new one...
    for (IT it = _chats.begin(); it != _chats.end(); ++it)
    {
	 if (jabberoo::JID::getUserHost(to) == jabberoo::JID::getUserHost(it->first))
         {
	      it->second->getGtkWindow()->present();
              foundchat = true;
         }
    }

    IT it = _chats.find(to);
    if (!foundchat && it == _chats.end())
	 _chats.insert(make_pair(to, new ChatView(*this, to)));
}

void ChatViewManager::on_action_menu_item_activate()
{
     new ChatView(*this, Glib::ustring());
}

void ChatViewManager::on_chat_node(const judo::Element& elem)
{
    if ( elem.findElement("body") == NULL )
    {
        return;
    }

    std::string from = elem.getAttrib("from");
    typedef std::map<std::string, ChatView*>::iterator IT;
    IT it = _chats.find(from);
    if (it == _chats.end())
    {
        // If this has been queued already
         // or if we want messages auto-displayed and are currently free for chat or available
        // we can view it now, else queue
        if (elem.cmpAttrib("gabber:queued", "true")
            || (G_App.getConfigurator().get_bool(Keys::intrface.messages_autodisplay)
                && (G_App.get_my_presence().getShow() == jabberoo::Presence::stChat
                    || G_App.get_my_presence().getShow() == jabberoo::Presence::stOnline)
                )
            )
        {
            _chats.insert(make_pair(from, new ChatView(*this, elem)));
        }
        else
        {
            G_App.getPacketQueue().push(new judo::Element(elem), "message-chat.png", "ChatView");
        }
    }
}

void ChatViewManager::on_queue_flushing()
{
    PacketQueue& pq(G_App.getPacketQueue());
    PacketQueue::queue_iterator it = std::find_if(pq.begin(), pq.end(), 
        PacketQueue::QueueInfoTypeCompare("ChatView"));
    while ( it != pq.end())
    {
        pq.pop(it);
        it = std::find_if(pq.begin(), pq.end(), 
            PacketQueue::QueueInfoTypeCompare("ChatView"));
    }
}

} // namespace Gabber
