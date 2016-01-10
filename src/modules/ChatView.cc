#include "GabberApp.hh"
#include "GabberWidgets.hh"
#include "GabberUtility.hh"
#include "ChatView.hh"
#include "ChatViewManager.hh"
#include "ConfigPaths.hh"
#include "LogManager.hh"
#include "XPathLog.hh"
#include "PlainTextView.hh"
#include "PrettyText.hh"

#include <jabberoo/filestream.hh>

#ifdef GTKSPELL
#include "gtkspell.h"
#endif // GTKSPELL

#include "intl.h"
#include <sigc++/slot.h>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/togglebutton.h>

namespace Gabber {

// -------------------------------------------------------------------
//
// Chat (One-on-One) Message View
//
// -------------------------------------------------------------------

ChatView::ChatView(ChatViewManager& mgr, const judo::Element& msg)
     :BaseGabberWindow("OOOChat_win"), _mgr(mgr),
     _jid(msg.getAttrib("from")), _sent_composing(false), _composing_id("")
{
    init(false);// Basic initialization

    _incoming_logger->check(msg);
    on_event_node(msg);
    on_chat_node(msg);

    PacketQueue& pq(G_App.getPacketQueue());
    // Process out any elements we want from the queue
    if (msg.cmpAttrib("gabber:queued", "true") && pq.isQueued(_jid))
    {
        
        PacketQueue::iterator it = pq.open(_jid); 
        while(it != pq.end(_jid))
        {
            const judo::Element& qelem(*it);
            if (_chat_query->check(qelem) || _event_query->check(qelem))
            {
                _incoming_logger->check(qelem);
                on_event_node(qelem);
                on_chat_node(qelem);
                it = pq.erase(it);
            }
            else
            {
                ++it;
            }
        }
        
        pq.close(_jid);
    }
}

ChatView::ChatView(ChatViewManager& mgr, const std::string& jid)
     :BaseGabberWindow("OOOChat_win"), _mgr(mgr),
     _jid(jid), _sent_composing(false), 
     _composing_id("")
{
     init(true);// Basic initialization
}


ChatView::~ChatView()
{
    delete _txtChatview;
    delete _incoming_logger;
    delete _outgoing_logger;
}

void ChatView::close()
{
    if(!_composing_id.empty() && _sent_composing)
    {
        // They deleted, stop the event
        Message m(_jid, "", Message::mtChat);
        judo::Element* x = m.addX("jabber:x:event");
        x->addElement("id", _composing_id);

        G_App.getSession() << m;
        
        _sent_composing = false;
    }

    // Don't get xpaths anymore, that'd be bad
     if (!_jid.empty())
     {
          G_App.getSession().unregisterXPath(_chat_query);
          G_App.getSession().unregisterXPath(_event_query);
          G_App.getSession().unregisterXPath(_presence_query);
          
          // unregister chat
          _mgr.releaseChat(_jid);
     }
     
    // destroy the Gtk::Window
    BaseGabberWindow::close();
}
    
void ChatView::init(bool is_blank)
{
    // Get widgets
    get_widget("AddContact_btn", _btnAddContact);
    get_widget("SendAsMsg_tglbtn", _tglbtnSendAsMsg);
    get_widget("Message_txt", _txtMessage);

    // Nickname and status display
    if (is_blank)
    {
        // We allow resource selection
        _prettyjid = manage(new PrettyJID(_jid, "", PrettyJID::dtNickRes, 
                                          true));
    }
    else if (_jid.empty())
    {
         _prettyjid = manage(new PrettyJID(_jid, "", PrettyJID::dtNickRes,
                                           true, true));
    }
    else
    {
        _prettyjid = manage(new PrettyJID(_jid, "", PrettyJID::dtNickRes, 
                                          false));
    }
    _prettyjid->show();
    _prettyjid->changed.connect(SigC::slot(*this, &ChatView::on_resource_changed));

    Gtk::HBox* hb;
    get_widget("JIDInfo_hbox", hb);
    hb->pack_start(*_prettyjid, true, true, 0);
    _nickname = _prettyjid->get_nickname();

    // Set the nicknames
    _local_nick = G_App.getSession().getUserName();


#ifdef GTKSPELL
    if (GabberApp::getSingleton().getConfigurator().get_bool(Keys::intrface.messages_spellcheck))
    {
	 // GtkSpell
	 GError *error = NULL;
	 char *errortext = NULL;
	 if (gtkspell_new_attach(GTK_TEXT_VIEW(_txtMessage->gobj()), NULL, &error) == NULL) {
	      g_print("gtkspell error: %s\n", error->message);
	      errortext = g_strdup_printf("GtkSpell was unable to initialize.\n"
					  "%s", error->message);
	      g_error_free(error);
	 }
    }
#endif // GTKSPELL

    _txtMessage->get_buffer()->signal_changed().connect(
        SigC::slot(*this, &ChatView::on_txtMessage_changed));

    get_widget("Message_scroll", _scrMessage);

    // Connect events
    _thisWindow->signal_event().connect(SigC::slot(*this, 
        &ChatView::on_window_event));
    _thisWindow->signal_focus_in_event().connect(SigC::slot(*this,
	&ChatView::on_focus_in_event));

    // XXX: should use a JabberConnection reference passed by manager
    G_App.evtConnected.connect(SigC::slot(*this, &ChatView::on_session_evtConnected));
    G_App.evtDisconnected.connect(SigC::slot(*this, &ChatView::on_session_evtDisconnected));
    
    // Set the window title
    _thisWindow->set_title(Util::substitute(_("Chat with %s"), _nickname));

    if (!_jid.empty())
    {
        // Register to recieve chat messages from _jid
        _chat_query = G_App.getSession().registerXPath(
            "/message[jid-equals(@from, '" + _jid + "')][@type='chat' or @type='error']",
            SigC::slot(*this, &ChatView::on_chat_node));
        // Register to recieve chat events addressed from _jid
        _event_query = G_App.getSession().registerXPath(
            "/message[jid-equals(@from, '" + _jid + "')]/x[@xmlns='jabber:x:event']", 
            SigC::slot(*this, &ChatView::on_event_node));

        // Register for presence events
        _presence_query = G_App.getSession().registerXPath(
            "/presence[jid-equals(@from,'" + _jid + "')]", 
            SigC::slot(*this, &ChatView::on_presence_node));

        // Setup the loggers, only packets with bodies? XXX
        if (!is_blank)
        {
            _incoming_logger = G_App.getLogManager().create_xpath_log(
               "/message[@type='chat' and jid-equals(@from, '" + _jid + "')]/body", _jid);
        }
        else
        {
            _incoming_logger = NULL;
        }
        _outgoing_logger = G_App.getLogManager().create_xpath_log(
            "/message[@type='chat' and jid-equals(@to, '" + _jid + "')]/body", _jid, false);
    }
    
    // Grab any existing presence
    try 
    {
        const Presence& p = G_App.getSession().presenceDB().findExact(
            _prettyjid->get_full_jid());
        on_presence_node(p.getBaseElement());
    } 
    catch (PresenceDB::XCP_InvalidJID& e) 
    {
        Presence p("", Presence::ptUnavailable, Presence::stOffline, 
            _("No presence has been received."));
        on_presence_node(p.getBaseElement());
    }

    Gtk::ScrolledWindow* scroll_win;
    get_widget("Chatview_scroll", scroll_win);
    std::string path = G_App.getLogManager().get_log_path(_jid);
    jabberoo::FileStream fs(path.c_str());
    if (fs.ParseFile("chatview"))
    {
        judo::Element* root = fs.getRoot();

        typedef std::list<jabberoo::Message> MsgListT;
        MsgListT msgs;

        // Build a list of the last 10 msgs
        int count = 0;
        for(judo::Element::reverse_iterator it = root->rbegin(); 
            it != root->rend(); ++it)
        {
            if ((*it)->getType() == judo::Node::ntElement)
            {
                judo::Element* e = static_cast<judo::Element*>(*it);

                msgs.push_front(jabberoo::Message( *e ));
                if (++count == 10)
                    break;
            }
        }

        if (msgs.empty())
        {
            _txtChatview = new PlainTextView(scroll_win);
        }
        else
        {
            _txtChatview = new PlainTextView(scroll_win, false);
            std::string userhost = jabberoo::JID::getUserHost(_jid);
            for(MsgListT::iterator it = msgs.begin(); it != msgs.end(); ++it)
            {
                if (jabberoo::JID::getUserHost( (*it).getFrom() ) == userhost)
                {
                    _txtChatview->append(*it, _nickname, false, true);
                }
                else
                {
                    _txtChatview->append(*it, _local_nick, true, true);
                }
            }
        }

        delete root;
    }
    scroll_win->show_all();

    // Display the window
    show();
//    _thisWindow->raise();
}

void ChatView::display(const Message& m)
{
     // bail on useless nodes
     if (m.getBody().empty()) return;
     
     // Display it
     _txtChatview->append(m, _nickname);
     
     // Save the thread!
     _thread = m.getThread();
     
     // Say we were displayed if they want that
     judo::Element* x = m.findX("jabber:x:event");
     if (x && x->findElement("displayed"))
          G_App.getSession() << m.displayed();
     
     // if the user doesn't currently see this window, change the title
     if (!_thisWindow->is_focus())
          _thisWindow->set_title("*" + Util::substitute(_("Chat with %s"), _nickname));
}

void ChatView::on_Send_clicked()
{
    // if we're selecting a resource
    if (_prettyjid->is_selecting_resource())
    {
	    // Then lock down the resource
	    on_resource_locked();
    }

    Glib::RefPtr<Gtk::TextBuffer> tb = _txtMessage->get_buffer();
    Glib::ustring body = tb->get_text(tb->begin(), tb->end());
    
    // Bogus
    if (body.empty())
        return;

    // Construct message
    Message m(_jid, body, Message::mtChat);

    // XXX Need disco or something here to check this a better way
    m.requestComposing();
    m.setID(G_App.getSession().getNextID());

    if (!_thread.empty())
	 m.setThread(_thread);

    G_App.getSession() << m; // Send the message

    _txtChatview->cancel_composing("");
    _txtChatview->append(m, _local_nick, true);

    _txtMessage->get_buffer()->set_text(Glib::ustring()); // reset the message area
    _sent_composing = false; // don't send a cancel composing message, because we haven't sent composing
}

bool ChatView::check_close()
{
    time_t now = time(NULL);
    // XXX configurable time somehow?
    if ( (now - _last_received_time) <= 3 )
    {
        Util::MessageDialog dlg(*_thisWindow, 
            Util::substitute(_("A message was just received from %s, are you sure you want to close this chat?"), _nickname),
            Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
	dlg.set_title(Util::substitute(_("Close chat with %s"), _nickname));
	dlg.add_button(_("_Don't Close"), Gtk::RESPONSE_NO);
	dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_YES);
	dlg.set_default_response(Gtk::RESPONSE_YES);

        int ret = dlg.run();
        dlg.hide();
        if (ret == Gtk::RESPONSE_NO)
            return false;
    }

    return true;
}

bool ChatView::on_delete_event(GdkEventAny* e)
{
    if(!check_close())
        return true;

    BaseGabberWindow::on_delete_event(e);

    return false;
}

bool ChatView::on_window_event(GdkEvent* ev) 
{
    if (ev->type != GDK_KEY_PRESS)
    return false;

    GdkEventKey* e = (GdkEventKey*)ev;

    // escape closes our window
    if (e->keyval == GDK_Escape)
    {
        if (check_close())
            close();
        return false;
    }

    // If they pressed the Keypad enter, make it act like a normal enter
    if (e->keyval == GDK_KP_Enter)
        e->keyval = GDK_Return;

    if (e->keyval == GDK_Return)
    {
        //enter a newline if shift-return is used
        if (e->state & GDK_SHIFT_MASK)
        {
            //unset the shift bit. shift-return seems to have a special meaning for the widget
            e->state ^= GDK_SHIFT_MASK;
            return false;
        }

        // Ctrl-Return sends the message so we don't need to check for it  
        on_Send_clicked();
        return true;
    }

    return false;
}

bool ChatView::on_focus_in_event(GdkEventFocus* e)
{
     // set the title back to normal, they've seen this window
     _thisWindow->set_title(Util::substitute(_("Chat with %s"), _nickname));
     
     return true;
}

void ChatView::on_session_evtConnected()
{
     _txtMessage->set_sensitive(true);
}

void ChatView::on_session_evtDisconnected()
{
     _txtMessage->set_sensitive(false);
}

void ChatView::on_txtMessage_changed()
{
    static bool composing = false;

    // Normal typing, send the appropiate composing msg
    int chars = _txtMessage->get_buffer()->get_char_count();
    if (chars == 0)
    {
        if(!_composing_id.empty() && _sent_composing)
        {
            // They deleted, stop the event
            Message m(_jid, "", Message::mtChat);
            judo::Element* x = m.addX("jabber:x:event");
            x->addElement("id", _composing_id);

            G_App.getSession() << m;
            
            _sent_composing = false;
        }

        _txtChatview->cancel_composing("");
        composing = false;
    }
    else if (chars > 0 && !composing)
    {
        if (!_composing_id.empty() && !_sent_composing)
        {
            // Send a fresh composing
            Message m(_jid, "", Message::mtChat);
            judo::Element* x = m.addX("jabber:x:event");
            x->addElement("composing");
            x->addElement("id", _composing_id);

            G_App.getSession() << m;

            _sent_composing = true;
        }
        _txtChatview->cancel_composing("");
        _txtChatview->composing("", _local_nick);
        composing = true;
    }
}

void ChatView::on_chat_node(const judo::Element& e)
{
    // XXX If this window is not visible we copy into _pending_msgs
    if (e.getAttrib("type") == "error")
    {
        // XXX Do more?!
        printf("ERROR: %s\n", e.toString().c_str());
        const judo::Element* err = e.findElement("error");
        _txtChatview->append_error(err->getCDATA());
    }
    else
    {
        display(Message(e));
        _last_received_time = time(NULL);
    }
}

void ChatView::on_event_node(const judo::Element& e)
{
    // Avoid errors
    if (e.getAttrib("type") == "error")
        return;

    Message m(e);

    const judo::Element* x = m.findX("jabber:x:event");
    if (!x)
        return;
    const judo::Element* delivered = x->findElement("delivered");
    const judo::Element* composing = x->findElement("composing");
    const judo::Element* id = x->findElement("id");

    if (composing && id)
    {
        _last_received_time = time(NULL);
        // If we have both of these they are typing
        _txtChatview->composing(e.getAttrib("from"), _nickname);
    }
    else if (!composing && id)
    {
        // cancel the events
        _txtChatview->cancel_composing(e.getAttrib("from"));
    }
    else if (composing && !id)
    {
        _composing_id = m.getID();
    }
    else if (delivered)
    {
        // Always handle these, if they are in there
        G_App.getSession() << m.delivered();
    }
}

void ChatView::on_presence_node(const judo::Element& e)
{
     Presence p(e);
     
     Gtk::HBox* hb;
     get_widget("Show_hbox", hb);

     Gtk::Label* l;
     get_widget("Status_lbl", l);

     // Show the extra bar if they are not available or chatty
     if (p.getShow() == Presence::stOnline || p.getShow() == Presence::stChat)
     {
	  hb->hide();
     }
     else
     {
	  hb->show();
	  // If there is an explanation to go along with the short presence, display it properly
	  if (!p.getStatus().empty())
	  {
	       l->set_label("<b>" + Util::getShowName(p.getShow()) + "</b>: " + p.getStatus());
	  }
	  else
	  {
	       l->set_label("<b>" + Util::getShowName(p.getShow()) + "</b>");
	  }
     }

     // Prevent sending composing events anymore if user goes offline
     if (p.getShow() == Presence::stOffline)
     {
          _composing_id = Glib::ustring();
     }
}

void ChatView::on_resource_changed()
{
     G_App.getSession().unregisterXPath(_presence_query);

     // Register for presence events
     _presence_query = G_App.getSession().registerXPath(
	  "/presence[@from='" + _jid + "']", 
	  SigC::slot(*this, &ChatView::on_presence_node));
		
     // Grab any existing presence
     try {
	  const Presence& p = G_App.getSession().presenceDB().findExact(_prettyjid->get_full_jid());
	  on_presence_node(p.getBaseElement());
     } catch (PresenceDB::XCP_InvalidJID& e) {
	  Presence p("", Presence::ptUnavailable, Presence::stOffline, _("No presence has been received."));
	  on_presence_node(p.getBaseElement());
     }
}

void ChatView::on_resource_locked()
{
     // hide the resource selection now
     _prettyjid->hide_resource_select();

     // unregister the general jid
     if (!_jid.empty())
     {
          G_App.getSession().unregisterXPath(_chat_query);
          G_App.getSession().unregisterXPath(_event_query);
          G_App.getSession().unregisterXPath(_presence_query);
          _mgr.releaseChat(_jid);
     }
     
     // get the new jid
     _jid = _prettyjid->get_full_jid();

     // if there's no resource, we probably want to do something
     // like catch whatever message is sent back.. ? XXX
     if (JID::getResource(_jid).empty())
     {
	  cout << "empty resource selected" << endl;
     }
     
     // register the specific jid
     _mgr.register_chat(_jid, this);
     // Register to recieve chat messages from _jid
     _chat_query = G_App.getSession().registerXPath(
	  "/message[@type='chat' and jid-equals(@from,'" + _jid + "')]",
	  SigC::slot(*this, &ChatView::on_chat_node));
     // Register to recieve chat events addressed from _jid
     _event_query = G_App.getSession().registerXPath(
	  "/message[jid-equals(@from,'" + _jid + "')]/x[@xmlns='jabber:x:event']", SigC::slot(*this, 
						&ChatView::on_event_node));
     // Register for presence events
    _presence_query = G_App.getSession().registerXPath(
	    "/presence[jid-equals(@from,'" + _jid + "')]", 
        SigC::slot(*this, &ChatView::on_presence_node));
     // Setup the loggers, only packets with bodies? XXX
    delete _incoming_logger;
    _incoming_logger = G_App.getLogManager().create_xpath_log(
        "/message[@type='chat' and jid-equals(@from,'" + _jid + "')]/body", _jid);
    delete _outgoing_logger;
    _outgoing_logger = G_App.getLogManager().create_xpath_log(
        "/message[@type='chat' and jid-equals(@to,'" + _jid + "')]/body", _jid, false);
}


} // namespace Gabber
