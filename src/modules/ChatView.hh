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

#ifndef INCL_CHAT_VIEW_HH
#define INCL_CHAT_VIEW_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

#include <string>
#include <map>
#include <time.h>
#include <jabberoo/XPath.h>

using namespace jabberoo;

namespace Gabber {

class ChatViewManager;
class PrettyJID;
class XPathLog;

class ChatView
     : public BaseGabberWindow
{
public:
     ChatView(ChatViewManager& mgr, const judo::Element& msg);
     ChatView(ChatViewManager& mgr, const std::string& jid);
     ~ChatView();
     
     void close();
     
protected:
     void init(bool is_blank);
     void display(const Message& m);

     // Handlers
     void on_Send_clicked();
     bool on_delete_event(GdkEventAny* e);
     bool on_window_event(GdkEvent* e);
     bool on_focus_in_event(GdkEventFocus* e);
     void on_session_evtConnected();
     void on_session_evtDisconnected();
     void on_txtMessage_changed();
     
     /// XPath Callbacks
     void on_chat_node(const judo::Element& msg);
     void on_event_node(const judo::Element& msg);
     void on_presence_node(const judo::Element& e);

     void on_resource_changed();
     void on_resource_locked();

private:
     ChatViewManager&       _mgr;
     bool                   _onRoster;
     Glib::ustring          _jid;
     Glib::ustring          _thread;
     Glib::ustring          _local_nick;
     Glib::ustring          _nickname;
     time_t                 _last_received_time;

     Gtk::ToggleButton*     _tglbtnSendAsMsg;
     Gtk::Button*           _btnAddContact;
     PrettyText*            _txtChatview;
     Gtk::TextView*         _txtMessage;
     Gtk::ScrolledWindow*   _scrMessage;
     PrettyJID*             _prettyjid;

     judo::XPath::Query*    _chat_query;
     judo::XPath::Query*    _event_query;
     judo::XPath::Query*    _presence_query;

     bool                   _sent_composing;
     std::string            _composing_id;

     std::list<Message*>    _pending_msgs;

     XPathLog*              _outgoing_logger;
     XPathLog*              _incoming_logger;

     bool check_close();
};

}; // namespace Gabber
#endif // INCL_CHAT_VIEW_HH
