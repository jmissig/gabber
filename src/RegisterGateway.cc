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

#include "RegisterGateway.hh"

#include "AddContactDlg.hh"
#include "ConfigPaths.hh"
#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "JabberConnection.hh"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/table.h>

#include "intl.h"

using namespace Gabber;

void RegisterGatewayDlg::display(AddContactDlg& parent)
{
     // NOTE: This doesn't take a JabberConnection because it's heavily tied
     // to GabberApp-specific usage. I doubt we'll use this with Rendezvous.
     RegisterGatewayDlg* e = new RegisterGatewayDlg(parent);
     e->show();
}

RegisterGatewayDlg::RegisterGatewayDlg(AddContactDlg& parent)
     : BaseGabberWindow("RegisterGateway_dlg"), _parent(parent)
{
     _thisWindow->hide();
     Gtk::Dialog* dlg = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(*(_parent.getGtkWindow()));
     dlg->signal_response().connect(SigC::slot(*this, &RegisterGatewayDlg::on_dlg_response));
     
     get_widget("Instructions_lbl", _lblInstructions);
     get_widget("GatewayList_rdo", _rdoGatewayList);
     get_widget("JID_rdo", _rdoJID);
     get_widget("GatewayList_optmenu", _optmenuGatewayList);
     get_widget("GatewayJID_ent", _entGatewayJID);
     
     Gtk::Menu* menu = manage(new Gtk::Menu());
     _optmenuGatewayList->set_menu(*menu);
     
     // set server name in the label
     Glib::ustring server = GabberApp::getSingleton().getConfigurator().get_string(Keys::acct.server);
     _rdoGatewayList->set_label(Util::substitute(_("From %s's list:"), server));
     
     Gtk::MenuItem* mi;
     
     try {
          // get our server
          jabberoo::DiscoDB::Item& server_item = G_App.getSession().discoDB()[server];
          
          // iterate over all children of the server
          for (jabberoo::DiscoDB::Item::const_iterator it = server_item.begin(); it != server_item.end(); ++it)
          {
               // and find children which implement iq:gateway
               const jabberoo::DiscoDB::Item::FeatureList& features = (*it)->getFeatureList();
               // XXX: for NOW this is iq:register. See GabberApp.cc for full note
               if (std::find(features.begin(), features.end(), "jabber:iq:register") != features.end())
               {
                    // add those children to the list of gateways and our pulldown
                    _listGateways.push_back((*it)->getJID());
                    mi = manage(new Gtk::MenuItem((*it)->getName(), false));
                    mi->show();
                    _optmenuGatewayList->get_menu()->append(*mi);
               }
          }
          
          if (_listGateways.empty())
          {
               Gtk::HBox* hb;
               get_widget("GatewayList_hbox", hb);
               hb->set_sensitive(false);
               _rdoGatewayList->set_active(false);
               mi = manage(new Gtk::MenuItem(_("<empty>"), false));
               mi->show();
               _optmenuGatewayList->get_menu()->append(*mi);
               _optmenuGatewayList->set_history(0);
               _rdoJID->set_active(true);
               _entGatewayJID->grab_focus();
          }
          
     } catch (jabberoo::DiscoDB::XCP_NotCached& e) {
          // We're not going to have much of a list anyway
          
          Gtk::HBox* hb;
          get_widget("GatewayList_hbox", hb);
          hb->set_sensitive(false);
          _rdoGatewayList->set_active(false);
          mi = manage(new Gtk::MenuItem(_("<empty>"), false));
          mi->show();
          _optmenuGatewayList->get_menu()->append(*mi);
          _optmenuGatewayList->set_history(0);
          _rdoJID->set_active(true);
          _entGatewayJID->grab_focus();
     }
     
     // if either of these get changed we need to select the appropriate radio button
     _optmenuGatewayList->signal_changed().connect(SigC::slot(*this, &RegisterGatewayDlg::on_optmenu_GatewayList_changed));
     _entGatewayJID->signal_changed().connect(SigC::slot(*this, &RegisterGatewayDlg::on_ent_GatewayJID_changed));
     
     show();
}

void RegisterGatewayDlg::on_dlg_response(int resp)
{
     if (resp == Gtk::RESPONSE_APPLY)
     {
          // Figure out which gateway to grab
          Glib::ustring server;
          
          if (_rdoGatewayList->get_active())
          {
               assert(_optmenuGatewayList->get_history() != -1);
               assert((guint)(_optmenuGatewayList->get_history()) < _listGateways.size());
               server = _listGateways[_optmenuGatewayList->get_history()];
          }
          else
          {
               server = _entGatewayJID->get_text();
          }
          
          Gtk::VBox* vb;
          get_widget("Select_vbox", vb);
          vb->hide();
          
          Gtk::Button* b;
          get_widget("Query_btn", b);
          b->hide();
          
          _lblInstructions->set_label(Util::substitute(_("<i>Please wait, fetching info from %s</i>"), server));
          
          _thisWindow->resize(1,1); // this should make it resize to a default size
          
          // disco cache this gateway
          try {
               G_App.getSession().discoDB()[server];
          } catch (jabberoo::DiscoDB::XCP_NotCached& e) {
               G_App.getSession().discoDB().cache(server, SigC::slot(*this, &RegisterGatewayDlg::on_gateway_disco));
          }
          
          // get registration info from this gateway
          G_App.getSession().queryNamespace("jabber:iq:register",
                                            SigC::slot(*this, &RegisterGatewayDlg::parse_register),
                                            server);
          
          return;
     }
     if (resp == Gtk::RESPONSE_OK)
     {
          // send what we have in the forms
          
          // Get next session ID
          std::string id = G_App.getSession().getNextID();
          
          jabberoo::Packet iq("iq");
          iq.setID(id);
          iq.setTo(_entGatewayJID->get_text());
          iq.getBaseElement().putAttrib("type", "set");
          
          judo::Element* query = iq.getBaseElement().addElement("query");
          query->putAttrib("xmlns", "jabber:iq:register");
          query->addElement("key", _key);
          
          for (EntryList::const_iterator eit = _entries.begin(); eit != _entries.end(); ++eit)
          {
               Gtk::Entry* ent = static_cast<Gtk::Entry*>(*eit);
               std::string& field = *static_cast<std::string *>(ent->get_data("fieldname"));
               
               if (!ent->get_text().empty())
               {
                    // previous versions we removed spaces here
                    // fuck it. gateways should be doing that.
                    query->addElement(field, ent->get_text());
               }
          }
          G_App.getSession() << iq;
          G_App.getSession().registerIQ(id, SigC::slot(*this, &RegisterGatewayDlg::parse_registered));
          
          Gtk::Button* b;
          get_widget("Register_btn", b);
          b->set_sensitive(false);
          
          return;
     }
     
     _parent.finish_gateway();
     
     close();
}

void RegisterGatewayDlg::on_optmenu_GatewayList_changed()
{
     _rdoGatewayList->set_active(true);
}

void RegisterGatewayDlg::on_ent_GatewayJID_changed()
{
     _rdoJID->set_active(true);
}

void RegisterGatewayDlg::on_gateway_disco(const jabberoo::DiscoDB::Item* item)
{
     // nada for now
}

void RegisterGatewayDlg::parse_register(const judo::Element& iq)
{
     // TODO: hook up to x:data once we have an x:data component
     
     if (iq.cmpAttrib("type", "result"))
     {
          const judo::Element* query = iq.findElement("query");
          
          // Create fields table
          Gtk::Table* tblFld = manage(new Gtk::Table(query->size() - 1, 2));
          tblFld->set_row_spacings(3);
          tblFld->set_col_spacings(3);
          tblFld->set_border_width(0);
          Gtk::VBox* vb;
          get_widget("Fields_vbox", vb);
          vb->pack_start(*tblFld, true, true, 6);
          
          Gtk::Label* lbl;
          Gtk::Entry* ent;
          std::string* fieldname;
          int row = 0;
          
          for (judo::Element::const_iterator it = query->begin(); it != query->end(); ++it)
          {
               // Cast the child element as a Tag
               if ((*it)->getType() != judo::Node::ntElement)
                    continue;
               judo::Element& t = *static_cast<judo::Element*>(*it);
               
               if (t.getName() == "instructions")
               {
                    _lblInstructions->set_label(t.getCDATA());
               }
               else if (t.getName() == "registered")
               {
                    // we are already registered with the agent.
                    // ok..
               }
               else if (t.getName() == "key")
               {
                    _key = t.getCDATA();
               }
               else
               {
                    // Create the label for this entry
                    lbl = manage(new Gtk::Label(t.getName() + ":", 0.0, 0.5));
                    tblFld->attach(*lbl, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL);
                    lbl->show();
                    
                    // Create the actual text entry
                    ent = manage(new Gtk::Entry());
                    // mask passwords
                    if (t.getName() == "password")
                         ent->set_visibility(false);
                    // set initial data
                    ent->set_text(t.getCDATA());
                    tblFld->attach(*ent, 1, 2, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::AttachOptions(0));
                    ent->show();
                    
                    _fieldnames.push_back(t.getName());
                    fieldname = &(_fieldnames.back());
                    ent->set_data("fieldname", fieldname);
                    _entries.push_back(ent);
                    
                    row++;
               }
          }
          
          tblFld->show();
          vb->show();

          Gtk::Button* b;
          get_widget("Register_btn", b);
          b->set_sensitive(true);
          b->show();
          
          return;
     }

     // error case
     _lblInstructions->set_label(Util::substitute(_("There was an error attempting to register with %s!"), _entGatewayJID->get_text()));
}

void RegisterGatewayDlg::parse_registered(const judo::Element& iq)
{
     // TODO: more error handling here with dialogs and shit
     _parent.finish_gateway(_entGatewayJID->get_text());
     
     close();
}
