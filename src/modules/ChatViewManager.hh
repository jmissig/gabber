#ifndef CHATVIEWMANAGER_HH
#define CHATVIEWMANAGER_HH

#include "ChatView.hh"
#include "BaseModule.hh"
#include <sigc++/object.h>
#include <gtkmm/menuitem.h>
#include <jabberoo/JID.hh>

namespace Gabber {

class ChatViewManager : public BaseModule, public SigC::Object
{
public:
    ChatViewManager();
    ~ChatViewManager();

     void register_chat(const std::string& jid, ChatView* chat);
     void releaseChat(const std::string& from);

protected:
     void on_menu_item_activate();
     void on_action_menu_item_activate();
     void on_chat_node(const judo::Element& elem);
     void on_queue_flushing();
    
private:
     judo::XPath::Query* _xp_query;
     std::map<std::string, ChatView*, jabberoo::JID::Compare> _chats;
     Gtk::MenuItem _menu_item;
     Gtk::MenuItem _action_menu_item;
}; // class ChatViewManager

} // namespace Gabber

#endif // CHATVIEWMANAGER_HH
