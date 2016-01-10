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
 *  Copyright (c) 2003 Julian Missig
 */

#include "StandaloneSendDlg.hh"

#include "ConfigPaths.hh"
#include "GabberApp.hh" // for config
#include "GabberUtility.hh"
#include "GabberWidgets.hh"
#include "JabberConnection.hh"
#include "LogManager.hh"

#ifdef GTKSPELL
#include "gtkspell.h"
#endif // GTKSPELL

#include <jabberoo/message.hh>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/textview.h>
#include "intl.h"

namespace Gabber {

// ---------------------------------------------------------
//
// Standalone Message Send Dialog
//
// ---------------------------------------------------------

void StandaloneSendDlg::display(JabberConnection& conn, Gtk::Window& parent,
				const jabberoo::Message& m)
{
     StandaloneSendDlg* e = new StandaloneSendDlg(conn, parent, m);
     e->show();
}

void StandaloneSendDlg::display(JabberConnection& conn, const Glib::ustring& jid)
{
     StandaloneSendDlg* e = new StandaloneSendDlg(conn, jid);
     e->show();
}

StandaloneSendDlg::StandaloneSendDlg(JabberConnection& conn, Gtk::Window& parent,
				     const jabberoo::Message& m)
     : BaseGabberWindow("StandaloneMsgSend_dlg"), _conn(conn), _jid(m.getFrom()),
       _thread(m.getThread()), _sent_composing(false), _composing_id("")
{
     // Set the dialog's parent
     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(parent);

     init();

     Glib::ustring subject = m.getSubject();
     Glib::ustring subpre = subject.substr(0,4);
     // prepend Re: if necessary
     if (!subject.empty() &&
	 (subpre != "Re: " && subpre != "re: " && subpre != "RE: "))
	  subject = "Re: " + subject;

     _entSubject->set_text(subject);

     // Composing?
     const judo::Element* x = m.findX("jabber:x:event");
     if (x)
     {
	  const judo::Element* composing = x->findElement("composing");
	  if (composing)
	  {
	       _composing_id = m.getID();
	  }
     }

     // Display
     show();
}

StandaloneSendDlg::StandaloneSendDlg(JabberConnection& conn, const Glib::ustring& jid)
     : BaseGabberWindow("StandaloneMsgSend_dlg"), _conn(conn), _jid(jid),
       _thread(Glib::ustring()), _sent_composing(false), _composing_id("")
{
     init();

     // Display
     show();
}

StandaloneSendDlg::~StandaloneSendDlg()
{
}

void StandaloneSendDlg::close()
{
    if(!_composing_id.empty() && _sent_composing)
    {
        // They deleted, stop the event
        jabberoo::Message m(_jid, "", jabberoo::Message::mtNormal);
        judo::Element* x = m.addX("jabber:x:event");
        x->addElement("id", _composing_id);

        _conn.getSession() << m;
        
        _sent_composing = false;
    }

    // destroy the Gtk::Window
    BaseGabberWindow::close();
}

void StandaloneSendDlg::init()
{
     // Get widgets
     Gtk::Button* b;
     get_widget("Cancel_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &StandaloneSendDlg::close));
     get_widget("Send_btn", _btnSend);
     _btnSend->set_sensitive(false);
     _btnSend->signal_clicked().connect(SigC::slot(*this, &StandaloneSendDlg::on_Send_clicked));

     get_widget("Subject_ent", _entSubject);
     get_widget("Message_txtview", _txtMessage);

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
#endif

     _txtMessage->get_buffer()->signal_changed().connect(SigC::slot(*this, &StandaloneSendDlg::on_txtMessage_changed));

     _thisWindow->signal_event().connect(SigC::slot(*this, &StandaloneSendDlg::on_window_event));

     // Nickname and status display
     _prettyjid = manage(new PrettyJID(_jid, "", PrettyJID::dtNickRes, true, _jid.empty()));
     _prettyjid ->changed.connect(SigC::slot(*this, &StandaloneSendDlg::on_PrettyJID_changed));
     _prettyjid ->show();

     Gtk::HBox* hb;
     get_widget("JIDInfo_hbox", hb);
     hb->pack_start(*_prettyjid, true, true, 0);

     // call this to set window title and such initially
     on_PrettyJID_changed();
}

void StandaloneSendDlg::on_Send_clicked()
{
     Glib::RefPtr<Gtk::TextBuffer> tb = _txtMessage->get_buffer();
     Glib::ustring body = tb->get_text(tb->begin(), tb->end());

     // Bogus
     if (body.empty())
	  return;

     // Construct message
     jabberoo::Message m(_jid, body, jabberoo::Message::mtNormal);

     m.setID(_conn.getSession().getNextID());

     if (!_thread.empty())
	  m.setThread(_thread);

     _conn.getSession() << m; // Send the message
     // XXX Move to xpath logs
     //LogManager::getSingleton().log(_jid) << m.toString() << std::endl;

     close();
}

bool StandaloneSendDlg::on_window_event(GdkEvent* ev)
{
    if (ev->type != GDK_KEY_PRESS)
    return false;

    GdkEventKey* e = (GdkEventKey*)ev;

    // escape closes our window
    if (e->keyval == GDK_Escape)
    {
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
        }

        // Ctrl-Return sends the message
        if (_btnSend->is_sensitive() && (e->state & GDK_CONTROL_MASK))
        {
	     on_Send_clicked();
             return true;
        }
    }

    return false;
}

void StandaloneSendDlg::on_txtMessage_changed()
{
     if (_jid.empty())
          return;
     
    // Normal typing, send the appropiate composing msg
    int chars = _txtMessage->get_buffer()->get_char_count();
    if (chars == 0)
    {
        if(!_composing_id.empty() && _sent_composing)
        {
            // They deleted, stop the event
            jabberoo::Message m(_jid, "", jabberoo::Message::mtNormal);
            judo::Element* x = m.addX("jabber:x:event");
            x->addElement("id", _composing_id);

            _conn.getSession() << m;
            
            _sent_composing = false;
        }

	_btnSend->set_sensitive(false);

    }
    else if (chars > 0)
    {
        if (!_composing_id.empty() && !_sent_composing)
        {
            // Send a fresh composing
            jabberoo::Message m(_jid, "", jabberoo::Message::mtNormal);
            judo::Element* x = m.addX("jabber:x:event");
            x->addElement("composing");
            x->addElement("id", _composing_id);

            _conn.getSession() << m;

            _sent_composing = true;
        }

	_btnSend->set_sensitive(true);

    }
}

void StandaloneSendDlg::on_PrettyJID_changed()
{
     // Update the JabberID
     _jid = _prettyjid->get_full_jid();

     // Set the title
     if (_jid.empty())
     {
	  _thisWindow->set_title(_("New Standalone Message"));
	  _btnSend->set_sensitive(false);
     }
     else
     {
	  _thisWindow->set_title(Util::substitute(_("Message to %s"), _prettyjid->get_nickname()));
	  if (_txtMessage->get_buffer()->get_char_count() > 0)
	       _btnSend->set_sensitive(true);
     }
     
}

} // namespace Gabber
