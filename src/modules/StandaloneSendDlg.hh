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

#ifndef INCL_STANDALONESENDDLG_HH
#define INCL_STANDALONESENDDLG_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

namespace Gabber {
/**
 * Standalone Message Send Dialog.
 */
class StandaloneSendDlg : public BaseGabberWindow
{
public:
     /**
      * Display a StandaloneSendDlg replying to the given Message.
      * @param conn The JabberConnection we're using.
      * @param parent The parent window of this dialog.
      * @param m The Message we're replying to.
      */
     static void display(JabberConnection& conn, Gtk::Window& parent,
			 const jabberoo::Message& m);

     /**
      * Display a StandaloneSendDlg to the given JabberID.
      * @param conn The JabberConnection we're using.
      * @param parent The parent window of this dialog.
      * @param jid The JabberID to send this message to.
      */
     static void display(JabberConnection& conn, const Glib::ustring& jid = "");

protected:
     StandaloneSendDlg(JabberConnection& conn, Gtk::Window& parent,
		       const jabberoo::Message& m);
     StandaloneSendDlg(JabberConnection& conn, const Glib::ustring& jid);
     ~StandaloneSendDlg();
     void close();
     void init();
     void on_Send_clicked();
     bool on_window_event(GdkEvent* ev);
     void on_txtMessage_changed();
     void on_PrettyJID_changed();

private:
     JabberConnection& _conn;
     Glib::ustring     _jid;
     PrettyJID*        _prettyjid;
     Glib::ustring     _thread;
     Gtk::Button*      _btnSend;
     Gtk::Entry*       _entSubject;
     Gtk::TextView*    _txtMessage;

     bool                   _sent_composing;
     std::string            _composing_id;
};

} // namespace Gabber

#endif // INCL_STANDALONESENDDLG_HH
