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

#ifndef INCL_ADDCONTACTDLG_HH
#define INCL_ADDCONTACTDLG_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

#include <jabberoo/judo.hpp>

namespace Gabber {
/**
 * Add Contact Dialog.
 * Add a contact to the Contact List (roster) based on JabberID.
 */
class AddContactDlg : public BaseGabberWindow
{
public:
     /**
      * Display an Add Contact Dialog.
      * @param conn The JabberConnection we're using.
      * @param parent The parent window this dialog is a child of.
      * @param jid If you want to pre-fill the JabberID, the JabberID.
      * @param imsys If you want an IM system to be selected, this is the JabberID of that gateway.
      */
     static void display(JabberConnection& conn, Gtk::Window& parent,
		   const Glib::ustring& jid = "");
     /**
      * Finish adding a gateway--cancelled.
      * This returns the Add Contact Dlg to normal after attempting to register with a gateway.
      */
     void finish_gateway();
     
     /**
      * Finish adding a gateway--successful.
      * This returns the Add Contact Dlg to normal after attempting to register with a gateway.
      * @param gateway The JID of the gateway to use now.
      */
      void finish_gateway(Glib::ustring gateway);
protected:
     AddContactDlg(JabberConnection& conn, Gtk::Window& parent,
		   const Glib::ustring& jid = "");
     void on_dlg_response(int resp);
     void on_DisplayName_optmenu_changed();
     void send_vcard_request();
     void parse_vcard(const judo::Element& t);
     void on_DisplayName_ent_changed();
     void on_IMID_ent_changed();
     void on_JID_ent_changed();
     void on_IMSystem_optmenu_changed();
     void parse_gateway_instructions(const judo::Element& t);
     void parse_gateway_id(const judo::Element& t);
private:
     JabberConnection& _conn;
     Gtk::Entry*       _entJID;
     Gtk::Entry*       _entIMID;
     Glib::ustring     _jid;
     Gtk::Button*      _btnAddContact;
     Gtk::OptionMenu*  _optmenuIMSystem;
     Gtk::OptionMenu*  _optmenuDisplayName;
     Gtk::Entry*       _entDisplayName;
     Gtk::Label*       _lblIMInstructions;
     
     std::vector<std::string> _listGateways;

     Glib::ustring     _Nickname;
     Glib::ustring     _Fullname;

     SigC::Connection  _conn_changed;

}; // class AddContactDlg

} // namespace Gabber

#endif // INCL_ADDCONTACTDLG_HH
