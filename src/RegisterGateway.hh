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

#ifndef INCL_REGISTERGATEWAY_HH
#define INCL_REGISTERGATEWAY_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

#include <jabberoo/judo.hpp>
#include <jabberoo/discoDB.hh>

namespace Gabber {

class AddContactDlg;

/**
 * Register Gateway Dialog.
 * Allow the user to pick a gateway and register with the sucker,
 * and return to the Add Contact Dialog nicely.
 */
class RegisterGatewayDlg : public BaseGabberWindow
{
public:
     /**
      * Create a Register Gateway Dialog.
      * @param parent The Add Contact Dialog this was called from.
      */
     static void display(AddContactDlg& parent);
protected:
     RegisterGatewayDlg(AddContactDlg& parent);
     void on_dlg_response(int resp);
     void on_optmenu_GatewayList_changed();
     void on_ent_GatewayJID_changed();
     void on_gateway_disco(const jabberoo::DiscoDB::Item* item);
     void parse_register(const judo::Element& iq);
     void parse_registered(const judo::Element& iq);
private:
     typedef std::list<Gtk::Entry *> EntryList;
     typedef std::list<std::string>  StringList;

     AddContactDlg&    _parent;
     Gtk::Label*       _lblInstructions;
     Gtk::RadioButton* _rdoGatewayList;
     Gtk::RadioButton* _rdoJID;
     Gtk::OptionMenu*  _optmenuGatewayList;
     Gtk::Entry*       _entGatewayJID;
     
     std::vector<std::string> _listGateways;
     EntryList         _entries;
     StringList        _fieldnames;
     std::string       _key;
     
}; // class RegisterGatewayDlg

} // namespace Gabber

#endif // INCL_REGISTERGATEWAY_HH
