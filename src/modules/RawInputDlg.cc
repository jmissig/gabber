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

#include "RawInputDlg.hh"

#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "JabberConnection.hh"

#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>

namespace Gabber {

void RawInputDlg::display(JabberConnection& conn)
{
     RawInputDlg* e = new RawInputDlg(conn);
     e->show();
}

RawInputDlg::RawInputDlg(JabberConnection& conn) :
     BaseGabberWindow("RawInput_dlg"), _conn(conn)
{
     // Get widgets
     Gtk::Button* b;
     get_widget("Close_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &RawInputDlg::close));
     get_widget("Send_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &RawInputDlg::on_Send_clicked));
     
     get_widget("Message_txtview", _txtMessage);
     
     Gtk::Label* l;
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     get_widget("Server_lbl", l);
     // XXX: Hack. We need to have a better way to get our own JID.
     // Probably put something in JabberConnection
     l->set_text(config.get_string(Keys::acct.server));

     _thisWindow->signal_event().connect(SigC::slot(*this, &RawInputDlg::on_window_event));
     
     show();
}

void RawInputDlg::on_Send_clicked()
{
     Glib::RefPtr<Gtk::TextBuffer> tb = _txtMessage->get_buffer();
     Glib::ustring body = tb->get_text(tb->begin(), tb->end());
     
     _conn.getSession() << body.c_str();
     
     tb->set_text("");
}

bool RawInputDlg::on_window_event(GdkEvent* ev)
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
          if (e->state & GDK_CONTROL_MASK)
          {
               on_Send_clicked();
               return true;
          }
     }
     
     return false;
}

} // namespace Gabber
