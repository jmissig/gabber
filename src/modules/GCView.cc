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

#include "GCView.hh"
#include "GCViewManager.hh"
#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "PlainTextView.hh"
#include "GabberUtility.hh"
#include "ResourceManager.hh"
#include "PrettyText.hh"
#include "GabberWidgets.hh"

#ifdef GTKSPELL
#include "gtkspell.h"
#endif // GTKSPELL

#include "intl.h"
#include <jabberoo/session.hh>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>

using namespace Gabber;
using namespace jabberoo;

GCView::GCView(GCViewManager& mgr, const Glib::ustring& jid, const
Glib::ustring& nickname) : 
    BaseGabberWindow("GChat_win"), _mgr(mgr), _room_jid(jid), 
    _nickname(nickname), _session(GabberApp::getSingleton().getSession())
{
    ResourceManager& resman = ResourceManager::getSingleton();
    _icons.reserve(6);
    _icons.push_back(Glib::RefPtr<Gdk::Pixbuf>());
    _icons.push_back(resman.getPixbuf("online.png"));
    _icons.push_back(resman.getPixbuf("chat.png"));
    _icons.push_back(resman.getPixbuf("away.png"));
    _icons.push_back(resman.getPixbuf("dnd.png"));
    _icons.push_back(resman.getPixbuf("xa.png"));

    _pres_xpath = _session.registerXPath(
        "/presence[starts-with(@from,'" + jid + "')]", 
        SigC::slot(*this, &GCView::on_presence_node));
    _msg_xpath = _session.registerXPath(
        "/message[starts-with(@from,'" + jid +"')][@type='groupchat' or @type='error']", 
        SigC::slot(*this, &GCView::on_message_node));

    Gtk::ScrolledWindow* scr_win;
    get_widget("Chatview_scroll", scr_win);
    _txtChatView = new PlainTextView(scr_win);
    scr_win->show_all();

    // Set room name and window title
    get_widget("Room_lbl", _lblRoom);
    _lblRoom->set_label(JID::getUserHost(jid));
    _thisWindow->set_title(Util::substitute(_("Group Chat in %s"), JID::getUser(jid)));

    // Catch key press events. Fun fun
    _thisWindow->signal_event().connect(
        SigC::slot(*this, &GCView::on_window_event));

    // We have to separately catch the subject and message key press events
    get_widget("Message_txt", _txtMessage);

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

    _txtMessage->signal_key_release_event().connect(
        SigC::slot(*this, &GCView::on_message_key_released));
    get_widget("Subject_ent", _subject_ent);
    _subject_ent->signal_key_release_event().connect(
        SigC::slot(*this, &GCView::on_subject_key_released));

    get_widget("Participants_treeview", _users_treeview);
    _users_store = Gtk::ListStore::create(_columns);
    _users_treeview->set_model(_users_store);

    Gtk::TreeView::Column* column = 
        Gtk::manage( new Gtk::TreeView::Column(_("Participants")) ); 
    column->pack_start(_columns.icon, false);
    column->pack_start(_columns.nick);
    _users_treeview->append_column(*column);

    // Attempt to join
    _session << Presence(jid + "/" + nickname, Presence::ptAvailable);
}

GCView::~GCView()
{ 
     _session.unregisterXPath(_pres_xpath);
     _session.unregisterXPath(_msg_xpath);
     _mgr.end_groupchat(_room_jid);
}

void GCView::close()
{
    _session << Presence(_room_jid + "/" + _nickname, Presence::ptUnavailable);

    BaseGabberWindow::close();
}

void GCView::on_presence_node(const judo::Element& node)
{
    Presence p(node);
    Glib::ustring nick = JID::getResource(node.getAttrib("from"));
    Glib::ustring msg;
    if (p.getType() == Presence::ptAvailable)
    {
         Presence::Show show(p.getShow());
//         if (show != Presence::stOnline)
//         {
//             msg = _(" has gone ") + p.getShow_str();
//             Glib::ustring status = p.getStatus();
//             if (!status.empty())
//             {
//                 msg += " (" + status + ")";
//             }
//             _txtChatView->append(nick + msg);
//         }

        _users[nick] = show;
    }
    else
    {
        _users.erase(nick);
    }

    update_userlist();
}

void GCView::on_message_node(const judo::Element& node)
{
    const judo::Element* body = node.findElement("body");
    if (!body)
        return;

    Message m(node);

    bool is_local = false;
    Glib::ustring nick = JID::getResource(m.getFrom());
    if (nick == _nickname)
        is_local = true;

    if (!m.getSubject().empty())
    {
        _subject_ent->set_text(m.getSubject());
    }

    if (nick.empty())
        _txtChatView->append(m.getBody());
    else
        _txtChatView->append(m, nick, is_local);
}

bool GCView::on_window_event(GdkEvent* ev)
{
     if (ev->type != GDK_KEY_PRESS)
	  return false;

     GdkEventKey* e = (GdkEventKey*)ev;

     // escape closes our window                                                 
     if (e->keyval == GDK_Escape)
     {
          // Confirmation Dialog
          Util::MessageDialog dlg(*_thisWindow,
                                 Util::substitute(_("Would you like to close the group chat in room %s?"), _room_jid),
                                 Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
          dlg.set_title(Util::substitute(_("Close group chat %s"), _room_jid));
          dlg.add_button(_("_Don't Close"), Gtk::RESPONSE_NO);
          dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_YES);

          int ret = dlg.run();
          dlg.hide();
          if (ret == Gtk::RESPONSE_YES)
              close();
	  return false;
     }

     return false;
}

bool GCView::on_subject_key_released(GdkEventKey* ev)
{
    if (ev->keyval == GDK_KP_Enter)
	  ev->keyval = GDK_Return;

    if (ev->keyval == GDK_Return)
    {
        Glib::ustring subject = _subject_ent->get_text();
        Message m(_room_jid, 
            Util::substitute(_("%s has set the subject to: %s"), "/me", 
                subject));
        m.setSubject(subject);

        _session << m;
    }

    return false;
}

bool GCView::on_message_key_released(GdkEventKey* ev)
{
    if (ev->keyval == GDK_KP_Enter)
        ev->keyval = GDK_Return;

    if (ev->keyval == GDK_Return)
    {
        if (ev->state & GDK_SHIFT_MASK)
        {
            ev->state ^= GDK_SHIFT_MASK;
            return false;
        }

        send_message();
        return true;
    }
    
    return false;
}

void GCView::send_message()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _txtMessage->get_buffer();
    if (buffer->get_char_count() == 0)
    {
        return;
    }

    Gtk::TextBuffer::iterator start, end;
    buffer->get_bounds(start, end);
    _session << Message(_room_jid, buffer->get_text(start, --end), Message::mtGroupchat);
    buffer->set_text("");
}

void GCView::update_userlist()
{
    _users_store->clear();
    for (GCUserMap::iterator it = _users.begin(); it != _users.end(); ++it)
    {
        Gtk::TreeModel::Row row = *_users_store->append();
        row[_columns.nick] = it->first;
        int icon = 0;
        switch (it->second)
        {
        case Presence::stOnline:
            icon = 1;
            break;
        case Presence::stChat:
            icon = 2;
            break;
        case Presence::stAway:
            icon = 3;
            break;
        case Presence::stDND:
            icon = 4;
            break;
        case Presence::stXA:
            icon = 5;
            break;
        default:
            icon = 0;
            break;
        };
        //printf("Setting to %d\n", _icons[icon]->get_width());
        if (!_icons[icon])
        {
            printf("NO ICON!\n");
        }
        row[_columns.icon] = _icons[icon];
    }
}

