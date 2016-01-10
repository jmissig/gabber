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

#include "S10nRequestDlg.hh"

#include "ContactInfo.hh"
#include "GabberUtility.hh"
#include "JabberConnection.hh"

#include <presence.hh>
#include <jabberoo/vCard.h>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/table.h>
#include <gtkmm/textview.h>

namespace Gabber {
     
void S10nRequestDlg::display(JabberConnection& conn, Gtk::Window& parent, 
                             const Glib::ustring& jid,
                             const Glib::ustring& reason)
{
     S10nRequestDlg* dlg = new S10nRequestDlg(conn, parent, jid, reason);
     dlg->show();
}

S10nRequestDlg::S10nRequestDlg(JabberConnection& conn, Gtk::Window& parent, 
                               const Glib::ustring& jid, 
                               const Glib::ustring& reason) : 
    BaseGabberWindow("S10nRequest_dlg"), _conn(conn), _jid(jid)
{
     Glib::ustring nickname;
     bool on_roster = false;

     try {
          nickname = _conn.getSession().roster()[jabberoo::JID::getUserHost(_jid)].getNickname();
          on_roster = true;
     } catch (jabberoo::Roster::XCP_InvalidJID& e) {
          on_roster = false;
     }
     
     Gtk::Button* b;
     get_widget("ViewInfo_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &S10nRequestDlg::on_ViewInfo_clicked));

     Gtk::Label* lbl;
     get_widget("JID_lbl", lbl);
     lbl->set_text(jid);

     Gtk::Table* tbl;

     get_widget("Allow_btn", _btnAllow);

     get_widget("S10n_lbl", lbl);
     if (on_roster)
     {
          lbl->set_markup(Util::substitute("<b>%s</b> would like to add you to their Contact List. This contact is already on your Contact List as <b>%s</b>.", _jid, nickname));

          get_widget("DisplayName_tbl", tbl);
          tbl->hide();
          
          Gtk::CheckButton* cb;
          get_widget("AddToList_chk", cb);
          cb->set_active(false);
          cb->hide();

          _btnAllow->set_sensitive(true);
     }
     else
     {
          lbl->set_markup(Util::substitute("<b>%s</b> would like to add you to their Contact List.", _jid));
          _btnAllow->set_sensitive(false);
     }

     get_widget("DisplayName_optmenu", _optmenuDisplayName);
     _optmenuDisplayName->signal_changed().connect(SigC::slot(*this, &S10nRequestDlg::on_DisplayName_optmenu_changed));

     get_widget("DisplayName_ent", _entDisplayName);

     if (!reason.empty())
     {
          Gtk::TextView* tv;
          get_widget("RequestReason_txtview", tv);
          tv->get_buffer()->set_text(reason);

          Gtk::Frame* f;
          get_widget("Reason_frame", f);
          f->show();
     }

     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(parent);
     dlg->signal_response().connect(SigC::slot(*this,
                                               &S10nRequestDlg::on_dlg_response));

     show();
}

S10nRequestDlg::~S10nRequestDlg()
{ }

void S10nRequestDlg::on_dlg_response(int resp)
{
     hide();
    
     jabberoo::Presence p(_jid, jabberoo::Presence::ptSubscribed);

     // Reject if asked to do so
     if (resp == Gtk::RESPONSE_REJECT)
          p.setType(jabberoo::Presence::ptUnsubscribed);

     // Neither Reject nor Accept were pressed. Nevermind.
     if (resp != Gtk::RESPONSE_ACCEPT && resp != Gtk::RESPONSE_REJECT)
     {
	  close();
	  return;
     }

     _conn.getSession() << p;

     // See if we should subscribe back
     Gtk::ToggleButton* btn;
     get_widget("AddToList_chk", btn);
     if (btn->get_active() && resp == Gtk::RESPONSE_ACCEPT)
     {
          // Add to roster and request subscription
          _conn.getSession().roster() << jabberoo::Roster::Item(_jid, _entDisplayName->get_text());
          p.setType(jabberoo::Presence::ptSubRequest);
          _conn.getSession() << p;
     }

     close();
}

void S10nRequestDlg::on_ViewInfo_clicked()
{
     ContactInfoDlg::display(_conn, *_thisWindow, _jid);
}

void S10nRequestDlg::on_DisplayName_optmenu_changed()
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

          // fetch the vcard
          send_vcard_request();
     }
     else
     {
          if (_optmenuDisplayName->get_history() == 1)
          {
               _conn_changed.disconnect();
               _entDisplayName->set_text(_Fullname);
               if (!_Fullname.empty())
                    _btnAllow->set_sensitive(true);
               _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &S10nRequestDlg::on_DisplayName_ent_changed));
          }
          else if (_optmenuDisplayName->get_history() == 2)
          {
               _conn_changed.disconnect();
               _entDisplayName->set_text(_Nickname);
               if (!_Nickname.empty())
                    _btnAllow->set_sensitive(true);
               _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &S10nRequestDlg::on_DisplayName_ent_changed));
          }
     }
}

void S10nRequestDlg::send_vcard_request()
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
     _conn.getSession().registerIQ(id, SigC::slot(*this, &S10nRequestDlg::parse_vcard));
}

void S10nRequestDlg::parse_vcard(const judo::Element& t)
{
     // We should check for errors and report them!

     if (!t.empty())
     {
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
               _btnAllow->set_sensitive(true);
     }
     else if (_optmenuDisplayName->get_history() == 2)
     {
          _entDisplayName->set_text(_Nickname);
          if (!_Nickname.empty())
               _btnAllow->set_sensitive(true);
     }

     _conn_changed = _entDisplayName->signal_changed().connect(SigC::slot(*this, &S10nRequestDlg::on_DisplayName_ent_changed));


     Gtk::Label* l;
     get_widget("Fetching_lbl", l);
     l->hide();
     _entDisplayName->show();
}

void S10nRequestDlg::on_DisplayName_ent_changed()
{
     if (_optmenuDisplayName->get_history() != 3)
          _optmenuDisplayName->set_history(3);

     if (_entDisplayName->get_text_length() > 0)
          _btnAllow->set_sensitive(true);
     else
          _btnAllow->set_sensitive(false);
}

}; // namespace Gabber
