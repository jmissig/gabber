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

#include "AddContactDlg.hh"

#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "JabberConnection.hh"
#include "RegisterGateway.hh"

#include <jabberoo/discoDB.hh>
#include <jabberoo/vCard.h>

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/optionmenu.h>

#include "intl.h"

using namespace Gabber;

void AddContactDlg::display(JabberConnection& conn, Gtk::Window& parent,
			     const Glib::ustring& jid)
{
     AddContactDlg* e = new AddContactDlg(conn, parent, jid);
     e->show();
}

AddContactDlg::AddContactDlg(JabberConnection& conn, Gtk::Window& parent,
			     const Glib::ustring& jid)
     : BaseGabberWindow("AddContact_dlg"), _conn(conn)
{
     _thisWindow->hide();
     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(parent);
     dlg->signal_response().connect(SigC::slot(*this, &AddContactDlg::on_dlg_response));

     get_widget("IMSystem_opt", _optmenuIMSystem);
     
     // FIXME: two ctors for AddContactDlg, one which doesn't use GabberApp.
     // This way it can be safely used by other connections.. the IM Systems bit would just be hidden
     
     Glib::ustring server = GabberApp::getSingleton().getConfigurator().get_string(Keys::acct.server);
     _listGateways.push_back(server);
     
     Util::JIDList& agents = G_App.getAgentList();
     Gtk::MenuItem* mi;
     for (Util::JIDList::iterator it = agents.begin(); it != agents.end(); ++it)
     {
          try {
               _listGateways.push_back(_conn.getSession().discoDB()[*it].getJID());
               
               // TODO: grab roster entry name for *it rather than just putting in the address
               // if there are duplicates, append (jid)
               mi = manage(new Gtk::MenuItem(*it, false));
               mi->show();
               _optmenuIMSystem->get_menu()->append(*mi);
               
          } catch (jabberoo::DiscoDB::XCP_NotCached& e) {
               continue;
          }
     }
     
     _listGateways.push_back("---");
     mi = manage(new Gtk::SeparatorMenuItem());
     mi->show();
     _optmenuIMSystem->get_menu()->append(*mi);
     _listGateways.push_back("--Other--");
     mi = manage(new Gtk::MenuItem(_("Other..."), false));
     mi->show();
     _optmenuIMSystem->get_menu()->append(*mi);
          
     get_widget("JID_ent", _entJID);
     _entJID->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_JID_ent_changed));
     _entJID->set_text(jid);

     get_widget("IMID_ent", _entIMID);
     _entIMID->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_IMID_ent_changed));

     get_widget("IMInstructions_lbl", _lblIMInstructions);

     get_widget("DisplayName_optmenu", _optmenuDisplayName);
     _optmenuDisplayName->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_DisplayName_optmenu_changed));

     get_widget("DisplayName_ent", _entDisplayName);

     get_widget("AddContact_btn", _btnAddContact);

     _optmenuIMSystem->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_IMSystem_optmenu_changed));
     
     show();
}

void AddContactDlg::finish_gateway()
{
     _optmenuIMSystem->set_history(0);
     _thisWindow->set_sensitive(true);
}

void AddContactDlg::finish_gateway(Glib::ustring gateway)
{
     _thisWindow->set_sensitive(true);
     
     _listGateways.push_back(gateway);
     // TODO: get roster entry name or disco result name and display that instead
     Gtk::MenuItem* mi = manage(new Gtk::MenuItem(gateway, false));
     mi->show();
     _optmenuIMSystem->get_menu()->append(*mi);
     
     _optmenuIMSystem->set_history(_listGateways.size() - 1);
}

void AddContactDlg::on_dlg_response(int resp)
{
     if (resp == Gtk::RESPONSE_OK)
     {
          _conn.getSession().roster() << jabberoo::Roster::Item(_jid, _entDisplayName->get_text());
          
          _conn.getSession() << jabberoo::Presence(_jid, jabberoo::Presence::ptSubRequest);
     }

     close();
}

void AddContactDlg::on_DisplayName_optmenu_changed()
{
     if (_optmenuDisplayName->get_history() == 0) // they just clicked "Choose..."
          return;
     
     // This is the first time we've done this, so fetch the vCard
     if (!_entDisplayName->is_visible())
     {
          Gtk::MenuItem* mi;
          get_widget("Choose_menuitem", mi);
          mi->hide();
          
          // display fetching label
          Gtk::Label* l;
          get_widget("Fetching_lbl", l);
          l->show();
          
	  // if we're doing this for a Gateway,
	  // then here is where we do an iq:transport JID lookup
          if (!_entIMID->get_text().empty())
          {
               string id = _conn.getSession().getNextID();
               
               jabberoo::Packet iq("iq");
               iq.setID(id);
               iq.setTo(_listGateways[_optmenuIMSystem->get_history()]);
               iq.getBaseElement().putAttrib("type", "set");
               judo::Element* query = iq.getBaseElement().addElement("query");
               query->putAttrib("xmlns", "jabber:iq:gateway");
               query->addElement("prompt", _entIMID->get_text());
               
               _conn.getSession() << iq;
               _conn.getSession().registerIQ(id, SigC::slot(*this, &AddContactDlg::parse_gateway_id));
          }
          else
          {
               // otherwise, the JID is just what's in the entry
               _jid = _entJID->get_text();
               
               // fetch the vcard
               send_vcard_request();
          }
     }
     else
     {
          if (_optmenuDisplayName->get_history() == 1)
          {
               _conn_changed.disconnect();
               _entDisplayName->set_text(_Fullname);
               if (!_Fullname.empty())
                    _btnAddContact->set_sensitive(true);
               _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_DisplayName_ent_changed));
          }
          else if (_optmenuDisplayName->get_history() == 2)
          {
               _conn_changed.disconnect();
               _entDisplayName->set_text(_Nickname);
               if (!_Nickname.empty())
                    _btnAddContact->set_sensitive(true);
               _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_DisplayName_ent_changed));
          }
     }
}

void AddContactDlg::send_vcard_request()
{
     // Get next session ID
     string id = _conn.getSession().getNextID();

     // Construct vCard request
     jabberoo::Packet iq("iq");
     iq.setID(id);
     iq.setTo(_jid);
     iq.getBaseElement().putAttrib("type", "get");
     judo::Element* vCard = iq.getBaseElement().addElement("vCard");
     vCard->putAttrib("xmlns", "vcard-temp");
     vCard->putAttrib("version", "2.0");
     vCard->putAttrib("prodid", "-//HandGen//NONSGML vGen v1.0//EN");

     // Send the vCard request
     _conn.getSession() << iq;
     _conn.getSession().registerIQ(id, SigC::slot(*this, &AddContactDlg::parse_vcard));
}

void AddContactDlg::parse_vcard(const judo::Element& t)
{
     // We should check for errors and report them!
     
     if (!t.empty())
     {
          // make sure we still actually want this vCard
          if (t.getAttrib("from") != _jid)
               return;
          
          const judo::Element* vCardElem = t.findElement("vCard");
          if (vCardElem != NULL)
          {
               jabberoo::vCard vc(*vCardElem);
               
               _Nickname = vc[jabberoo::vCard::Nickname];
               _Fullname = vc[jabberoo::vCard::Fullname];
          }
     }
     
     if (_optmenuDisplayName->get_history() == 1)
     {
          _entDisplayName->set_text(_Fullname);
          if (!_Fullname.empty())
               _btnAddContact->set_sensitive(true);
     }
     else if (_optmenuDisplayName->get_history() == 2)
     {
          _entDisplayName->set_text(_Nickname);
          if (!_Nickname.empty())
               _btnAddContact->set_sensitive(true);
     }
     
     _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &AddContactDlg::on_DisplayName_ent_changed));
     
     
     Gtk::Label* l;
     get_widget("Fetching_lbl", l);
     l->hide();
     _entDisplayName->show();
}

void AddContactDlg::on_DisplayName_ent_changed()
{
     if (_optmenuDisplayName->get_history() != 3)
          _optmenuDisplayName->set_history(3);
     
     if (_entDisplayName->get_text_length() > 0)
          _btnAddContact->set_sensitive(true);
     else
          _btnAddContact->set_sensitive(false);
}

void AddContactDlg::on_IMID_ent_changed()
{
     _entJID->set_text(_entIMID->get_text());
     // on_JID_ent_changed() will be called, so we need not handle all that
}

void AddContactDlg::on_JID_ent_changed()
{
     // we currently have the name/nick display visible
     // and either the _jid is empty or it's not equal to what's in the JID entry
     if (_entDisplayName->is_visible() && 
         (_jid.empty() || _jid != _entJID->get_text()))
     {
          // basically reset the "select display name" part
          _btnAddContact->set_sensitive(false);

          if (_conn_changed.connected())
               _conn_changed.disconnect();
          _entDisplayName->hide();
          _entDisplayName->set_text(Glib::ustring());
          
          _Nickname = Glib::ustring();
          _Fullname = Glib::ustring();
          
          Gtk::MenuItem* mi;
          get_widget("Choose_menuitem", mi);
          mi->show();

          _optmenuDisplayName->set_history(0);
     }
     // this is the case if called from on_IMSystem_optmenu_changed()
     else if (_entJID->get_text_length() <= 0 && _entIMID->get_text_length() <= 0)
     {
          Gtk::MenuItem* mi;
          get_widget("Choose_menuitem", mi);
          mi->show();

          _optmenuDisplayName->set_history(0);
          
          Gtk::Label* l;
          get_widget("Fetching_lbl", l);
          l->hide();
          
          _optmenuDisplayName->set_sensitive(false);
     }
     else
     {
          // we're all good
          _optmenuDisplayName->set_sensitive(true);
     }
}

void AddContactDlg::on_IMSystem_optmenu_changed()
{
     // reset _jid -- it's constructed
     _jid = Glib::ustring();
     // setting the IMID to nothing will set the JID entry to the same
     // doing this will call on_IMID_ent_changed()
     // which will call on_JID_ent_changed()
     _entIMID->set_text(Glib::ustring());
 
     Gtk::VBox* vb;
     get_widget("IMID_vbox", vb);
     
     assert(_optmenuIMSystem->get_history() != -1);
     assert((guint)(_optmenuIMSystem->get_history()) < _listGateways.size());
     
     std::string server = _listGateways[_optmenuIMSystem->get_history()];
     
     // if they selected "Other..." then we start the add gateway dialog
     if (server == "---" || server == "--Other--")
     {
          _thisWindow->set_sensitive(false);
          RegisterGatewayDlg::display(*this);
          return;
     }
     // if they changed the IM system, we reset it (done by on_JID_ent_changed())
     else if (_entIMID->is_visible() && _optmenuIMSystem->get_history() != 0)
     {
          // make sure the IMID selection is visible
          vb->show();

          _entIMID->grab_focus();
     }
     else if (_optmenuIMSystem->get_history() == 0)
     {
          // they selected "Jabber"
          vb->hide();
          _entJID->set_sensitive(true);
          _entJID->grab_focus();
          return;
     }
     
     Gtk::HBox* hb;
     get_widget("IMID_hbox", hb);
     hb->hide();

     Gtk::Label* l;
     _lblIMInstructions->set_label(_("<i>Please wait, fetching info...</i>"));
     _lblIMInstructions->show();
     get_widget("IMID_lbl", l);
     l->set_text(_("IM ID:"));
     _entJID->set_sensitive(false);
     _optmenuDisplayName->set_sensitive(false);

     // well, we told them we were fetching info. Perhaps we should :)
     _conn.getSession().queryNamespace("jabber:iq:gateway",
                                       SigC::slot(*this, &AddContactDlg::parse_gateway_instructions),
                                       server);
}

void AddContactDlg::parse_gateway_instructions(const judo::Element& t)
{
     if (!t.empty())
     {
          const judo::Element* query = t.findElement("query");
          if (query != NULL)
          {
               _lblIMInstructions->set_text(query->getChildCData("desc"));
          }
     }
     
     Gtk::HBox* hb;
     get_widget("IMID_hbox", hb);
     hb->show();
     _entIMID->grab_focus();
}

void AddContactDlg::parse_gateway_id(const judo::Element& t)
{
     if (!t.empty())
     {
          const judo::Element* query = t.findElement("query");
          if (query != NULL)
          {
               if (!query->getChildCData("prompt").empty() 
                   && (query->findElement("error") == NULL))
               {
                    _jid = query->getChildCData("prompt");
                    _entJID->set_text(_jid);
                    send_vcard_request();
                    return;
               }
          }
     }

     // otherwise, we attempt to munge up the JID

     // This is such a hack. That's why the above is preferred.
     
     string username = _entIMID->get_text();
     // Remove spaces and @'s
     for (string::size_type i = 0; i < username.length(); i++)
     {
          if (username[i] == ' ') // If character at i is a space
               username.erase(i, 1); // Erase the character at i
          else if (username[i] == '@') // If character at i is @
               username[i] = '%'; // Replace the character at i
     }
     
     _jid = username + "@" + _listGateways[_optmenuIMSystem->get_history()];

     _entJID->set_text(_jid);
     
     // fetch the vcard. heaven help us.
     send_vcard_request();
}
