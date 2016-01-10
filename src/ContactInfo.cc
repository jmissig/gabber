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
 *  Copyright (c) 2002-2003 Julian Missig
 */

#include "ContactInfo.hh"

#include "JabberConnection.hh"

#include "ConfigPaths.hh" // for MyInfoDlg
#include "Environment.hh" // for MyInfoDlg
#include "GabberApp.hh" // for MyInfoDlg
#include "GabberUtility.hh"
#include "GabberWidgets.hh"
#include "TextParser.hh"

#include "intl.h"

#include <jabberoo/discoDB.hh>
#include <jabberoo/packet.hh>
#include <jabberoo/vCard.h>

#include <sigc++/bind.h>

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>

#ifndef WIN32
#include <sys/utsname.h> // for MyInfoDlg
#endif

using namespace jabberoo;

namespace Gabber {

// ---------------------------------------------------------
//
// Contact Information Dialog
//
// ---------------------------------------------------------

void ContactInfoDlg::display(JabberConnection& conn, Gtk::Window& parent,
			     const Glib::ustring& jid)
{
     ContactInfoDlg* e = new ContactInfoDlg(conn, parent, jid);
     e->show();
}

ContactInfoDlg::ContactInfoDlg(JabberConnection& conn, Gtk::Window& parent,
			       const Glib::ustring& jid)
     : BaseGabberWindow("ViewInfo_dlg"), _conn(conn), _jid(jid)
{
     // Set the dialog's parent
     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
     dlg->set_transient_for(parent);

     Gtk::Button* b;
     get_widget("Close_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &ContactInfoDlg::close));

     // Align the labels with the magic of SizeGroup
     Gtk::Label* l;
     Glib::RefPtr<Gtk::SizeGroup> client_grp = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
     get_widget("JabberClientName_lbl", l);
     client_grp->add_widget(*l);
     get_widget("JabberClientVersion_lbl", l);
     client_grp->add_widget(*l);
     get_widget("ComputerOS_lbl");
     client_grp->add_widget(*l);
     get_widget("ComputerTime_lbl");
     client_grp->add_widget(*l);

     _presence_query = _conn.getSession().registerXPath(
        "/presence[jid-equals(@from,'" + _jid + "')]",
        SigC::slot(*this, &ContactInfoDlg::on_presence_node));
     
     _prettyjid = manage(new PrettyJID(_jid, "", PrettyJID::dtJIDRes, true));
     _prettyjid->show();
     _prettyjid->changed.connect(SigC::slot(*this, &ContactInfoDlg::on_PrettyJID_changed));
     on_PrettyJID_changed(); // Fire once since this won't normally be fired when we init

     Gtk::HBox* hb;
     get_widget("JIDInfo_hbox", hb);
     hb->pack_start(*_prettyjid, true, true, 0);
     _nickname = _prettyjid->get_nickname();

     // Set the window title
     _thisWindow->set_title(Util::substitute(_("%s's Info"), _nickname));

     Gtk::Entry* nickent;
     get_widget("Nickname_ent", nickent);
     nickent->set_text(_nickname);
     nickent->set_editable(_prettyjid->is_on_roster());

     // Set the s10n info
     try {
          // grab the roster item
          jabberoo::Roster::Item item = _conn.getSession().roster()[jabberoo::JID::getUserHost(_jid)];

          // set the s10n labels
          get_widget("S10nType_lbl_lbl", l);
          l->set_label(Util::getS10nName(item.getSubsType()) + ":");
          get_widget("S10nType_lbl", l);
          l->set_label(Util::getS10nInfo(item.getSubsType()));

          // display "Pending" if applicable
          if (item.isPending())
          {
               get_widget("S10nPending_lbl_lbl", l);
               l->show();
               get_widget("S10nPending_lbl", l);
               l->show();
          }

          // display "Subscribe" button if applicable
          switch (item.getSubsType())
          {
               case Roster::rsFrom:
               case Roster::rsNone:
                    get_widget("Subscribe_btn", b);
                    b->show();
                    get_widget("Subscribe_lbl", l);
                    l->show();
                    break;
               default:
                    break;
          }
          
     } catch (jabberoo::Roster::XCP_InvalidJID& e) {
          get_widget("AddContact_btn", b);
          b->show();
          get_widget("AddContact_lbl", l);
          l->show();
     }

     send_vcard_request();
}

void ContactInfoDlg::close()
{
     // Figure out if the nickname has changed, and if so, update it
     Gtk::Entry* nickent;
     get_widget("Nickname_ent", nickent);
     Glib::ustring newnick = nickent->get_text();
     
     // no empty nicks, thank you
     if (newnick.empty())
     {
          Util::MessageDialog dlg(*_thisWindow,
                                 _("Empty Display Names are not allowed. The Display Name has been changed back to its previous value."),
                                 Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
          dlg.set_title(_("Display Name cannot be empty"));
          dlg.run();
          nickent->set_text(_prettyjid->get_nickname());
          return;
     }
     
     if (newnick != _prettyjid->get_nickname())
     {
          try {
               jabberoo::Roster::Item item = _conn.getSession().roster()[jabberoo::JID::getUserHost(_jid)];

               // Set the new nickname
               item.setNickname(newnick);
               _conn.getSession().roster() << item;
               
          } catch (jabberoo::Roster::XCP_InvalidJID& e) {
               Util::MessageDialog dlg(*_thisWindow,
                                      Util::substitute(_("The Display Name cannot be changed because this Contact (%s) was not found on the Contact List. This is probably a bug in Gabber."), _jid),
                                      Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
               dlg.set_title(_("Contact cannot be found"));
               dlg.run();
               nickent->set_text(_prettyjid->get_nickname());
               return;
          }
     }

     // No more presence xpath for us
     _conn.getSession().unregisterXPath(_presence_query);
     
     // destroy the Gtk::Window
     BaseGabberWindow::close();
}

void ContactInfoDlg::send_vcard_request()
{
     // Get next session ID
     string id = _conn.getSession().getNextID();

     // Construct vCard request
     Packet iq("iq");
     iq.setID(id);
     iq.setTo(_jid);
     iq.getBaseElement().putAttrib("type", "get");
     judo::Element* vCard = iq.getBaseElement().addElement("vCard");
     vCard->putAttrib("xmlns", "vcard-temp");
     vCard->putAttrib("version", "2.0");
     vCard->putAttrib("prodid", "-//HandGen//NONSGML vGen v1.0//EN");

     // Send the vCard request
     _conn.getSession() << iq;
     _conn.getSession().registerIQ(id, SigC::slot(*this, &ContactInfoDlg::parse_vcard));
}

void ContactInfoDlg::set_info_label(const Glib::ustring& label_name, const Glib::ustring& data)
{
     if (data.empty())
	  return;

     Gtk::Label* l;
     get_widget(label_name + "_lbl", l);
     l->show();
     get_widget(label_name, l);
     l->set_label(data);
     l->show();
}

void ContactInfoDlg::parse_vcard(const judo::Element& t)
{
     if (t.empty())
	  return;

     const judo::Element* vCardElem = t.findElement("vCard");
     if (vCardElem == NULL)
	  return;

     Glib::ustring curtext;
 
     vCard vc(*vCardElem);

     // General
     curtext = vc[vCard::Fullname];
     set_info_label("FullName_lbl", curtext);
     curtext = vc[vCard::EMail];
     set_info_label("Email_lbl", curtext);
     curtext = vc[vCard::URL];
     set_info_label("WebSite_lbl", curtext);
     curtext = vc[vCard::Telephone];
     set_info_label("Telephone_lbl", curtext);
     
     // Location
     curtext = vc[vCard::Street];
     set_info_label("Street_lbl", curtext);
     curtext = vc[vCard::Street2];
     set_info_label("Street2_lbl", curtext);
     curtext = vc[vCard::City];
     set_info_label("City_lbl", curtext);
     curtext = vc[vCard::State];
     set_info_label("State_lbl", curtext);
     curtext = vc[vCard::PCode];
     set_info_label("PCode_lbl", curtext);
     curtext = vc[vCard::Country];
     set_info_label("Country_lbl", curtext);
     
     // Organization
     curtext = vc[vCard::OrgName];
     set_info_label("OrgName_lbl", curtext);
     curtext = vc[vCard::OrgUnit];
     set_info_label("OrgUnit_lbl", curtext);
     curtext = vc[vCard::PersonalTitle];
     set_info_label("PersonalTitle_lbl", curtext);
     curtext = vc[vCard::PersonalRole];
     set_info_label("PersonalRole_lbl", curtext);
     
     // About
     curtext = vc[vCard::Birthday];
     if (!curtext.empty())
     {
	  Gtk::Widget* f;
	  get_widget("Birthday_frame", f);
	  f->show();
	  set_info_label("Birthday_lbl", curtext);
     }
     curtext = vc[vCard::Description];
     Gtk::TextView* tv;
     get_widget("About_txtview", tv);
     TextBufferParser tbp(tv->get_buffer());
     tv->get_buffer()->set_text(curtext);
     tbp.parse_buffer(tv->get_buffer()->create_mark("startmark", tv->get_buffer()->begin()),
                      tv->get_buffer()->create_mark("endmark", tv->get_buffer()->end()));
}

void ContactInfoDlg::send_disco_request()
{
     // clear previous capabilities
     Gtk::Label* lblcaps;
     get_widget("JabberClientCaps_lbl", lblcaps);
     lblcaps->set_label("");
     
     // See if we have disco cached
     try {
          parse_disco( &(_conn.getSession().discoDB()[_prettyjid->get_full_jid()]) );
     } catch (jabberoo::DiscoDB::XCP_NotCached& e) {
          // Send Disco request
          _conn.getSession().discoDB().cache(_prettyjid->get_full_jid(), SigC::slot(*this, &ContactInfoDlg::parse_disco), false);
     }
}

void ContactInfoDlg::parse_disco(const jabberoo::DiscoDB::Item* item)
{
     Glib::ustring strcaps;
     Gtk::Label* lblcaps;
     get_widget("JabberClientCaps_lbl", lblcaps);
     
     const jabberoo::DiscoDB::Item::FeatureList& features = item->getFeatureList();
     
     // I know this seems inefficient, but either way we need to do a bunch of compares
     // and this guarantees an order
     if (std::find(features.begin(), features.end(), "jabber:iq:last") != features.end())
          strcaps = _("Idle Time");
     if (std::find(features.begin(), features.end(), "jabber:x:event") != features.end())
          if (strcaps.empty())
               strcaps = _("Typing Notification");
          else
               strcaps += _(", Typing Notification");
     if (std::find(features.begin(), features.end(), "gc-1.0") != features.end())
          if (strcaps.empty())
               strcaps = _("Basic Group Chat");
          else
               strcaps += _(", Basic Group Chat");
     if (std::find(features.begin(), features.end(), "http://jabber.org/protocol/muc") != features.end())
          if (strcaps.empty())
               strcaps = _("Advanced Group Chat");
          else
               strcaps += _(", Advanced Group Chat");
     if (std::find(features.begin(), features.end(), "jabber:x:conference") != features.end())
          if (strcaps.empty())
               strcaps = _("Group Chat Invitation");
          else
               strcaps += _(", Group Chat Invitation");
     if (std::find(features.begin(), features.end(), "jabber:x:roster") != features.end())
          if (strcaps.empty())
               strcaps = _("Receive Contacts");
          else
               strcaps += _(", Receive Contacts");
     if (std::find(features.begin(), features.end(), "jabber:iq:oob") != features.end())
          if (strcaps.empty())
               strcaps = _("Basic File Transfer");
          else
               strcaps += _(", Basic File Transfer");
     if (std::find(features.begin(), features.end(), "http://jabber.org/protocol/si/profile/file-transfer") != features.end())
          if (strcaps.empty())
               strcaps = _("Advanced File Transfer");
          else
               strcaps += _(", Advanced File Transfer");
     if (std::find(features.begin(), features.end(), "http://jabber.org/protocol/pubsub") != features.end())
          if (strcaps.empty())
               strcaps = _("Publish/Subscribe");
          else
               strcaps += _(", Publish/Subscribe");
     
     lblcaps->set_label("<span size=\"small\">" + strcaps + "</span>");
}

void ContactInfoDlg::send_version_request()
{
     _conn.getSession().queryNamespace("jabber:iq:version",
                                       SigC::slot(*this, &ContactInfoDlg::parse_version),
                                       _prettyjid->get_full_jid());
}

void ContactInfoDlg::parse_version(const judo::Element& t)
{
     if (t.empty())
	  return;

     const judo::Element* query = t.findElement("query");
     if (query == NULL)
	  return;
	 
     set_info_label("JabberClientName_lbl", query->getChildCData("name"));
     set_info_label("JabberClientVersion_lbl", query->getChildCData("version"));
     set_info_label("ComputerOS_lbl", query->getChildCData("os"));
}

void ContactInfoDlg::send_time_request()
{
     _conn.getSession().queryNamespace("jabber:iq:time",
                                       SigC::slot(*this, &ContactInfoDlg::parse_time),
                                       _prettyjid->get_full_jid());
}

void ContactInfoDlg::parse_time(const judo::Element& t)
{
     if (t.empty())
	  return;

     const judo::Element* query = t.findElement("query");
     if (query == NULL)
	  return;

     Glib::ustring curtext;
     curtext = query->getChildCData("display");
     curtext += " "; // Don't duplicate the time zone:
     if(curtext.find(query->getChildCData("tz"))==string::npos)
	  curtext += query->getChildCData("tz");
     set_info_label("ComputerTime_lbl", curtext);
}

void ContactInfoDlg::get_status()
{
     // We want to catch a different presence this time
     _conn.getSession().unregisterXPath(_presence_query);
     _presence_query = _conn.getSession().registerXPath(
        "/presence[@from='" + _prettyjid->get_full_jid() + "']",
        SigC::slot(*this, &ContactInfoDlg::on_presence_node));
     
     // copy & paste from PrettyJID

     // Grab an existing presence for this particular resource (maybe)
     // and process it
     try {
          const jabberoo::Presence& p = _conn.getSession().presenceDB().findExact(_prettyjid->get_full_jid());
          on_presence_node(p.getBaseElement());
     } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
          // Construct an offline presence
          jabberoo::Presence p("", jabberoo::Presence::ptUnavailable, jabberoo::Presence::stOffline, _("No presence has been received."));
          p.setFrom(_prettyjid->get_full_jid());
          on_presence_node(p.getBaseElement());
     }

     // end copy & paste from PrettyJID
}

void ContactInfoDlg::on_presence_node(const judo::Element& e)
{
     const Presence p(e);

     Gtk::Label* l;
     get_widget("Status_lbl", l);
     if (p.getStatus().empty())
          l->set_label(Util::getShowName(p.getShow()));
     else
          l->set_label(Util::substitute(_("%s: %s"), Util::getShowName(p.getShow()), p.getStatus()));

     // Whether or not the last request should be considered "offline"
     bool offline = false;
     if (p.getShow() == Presence::stOffline ||
         p.getShow() == Presence::stInvalid)
          offline = true;
     
     send_last_request(offline);
}

void ContactInfoDlg::send_last_request(bool offline)
{
     _conn.getSession().queryNamespace("jabber:iq:last",
                                       SigC::bind(SigC::slot(*this, &ContactInfoDlg::parse_last), offline),
                                       _prettyjid->get_full_jid());
}

void ContactInfoDlg::parse_last(const judo::Element& t, bool offline)
{
     if (!t.empty())
     {
          int seconds;
          string lasttime;
          const judo::Element* query = t.findElement("query");
          if (query != NULL)
          {
               // Grab the idle or last logged out time
               seconds = atoi(query->getAttrib("seconds").c_str());
               if (seconds == 0)
                    return;
               
               // Conversion magic
               int mins, hrs, days, secs;
               days = (seconds / (3600 * 24));
               hrs  = ((seconds / 3600) - (days * 24));
               mins = ((seconds / 60) - (days * 24 * 60) - (hrs * 60));
               secs = (seconds - (days * 24 * 60 * 60) - (hrs * 60 * 60) - (mins * 60));
               char *tdays, *thrs, *tmins, *tsecs;
               tdays = g_strdup_printf("%d", days);
               thrs  = g_strdup_printf("%d", hrs);
               tmins = g_strdup_printf("%d", mins);
               tsecs = g_strdup_printf("%d", secs);

               // Figure out the exact text to display
               bool prevt;
               if (string(tdays) == "1")
               {
                    lasttime += " " + string(tdays) + _(" day");
                    prevt = true;
               }
               else if (string(tdays) != "0")
               {
                    lasttime += " " + string(tdays) + _(" days");
                    prevt = true;
               }
               else
                    prevt = false;
               g_free(tdays);

               if (string(thrs) == "1")
               {
                    if (prevt)
                         lasttime += ",";
                    lasttime += " " + string(thrs) + _(" hr");
                    prevt = true;
               }
               else if (string(thrs) != "0")
               {
                    if (prevt)
                         lasttime += ",";
                    lasttime += " " + string(thrs) + _(" hrs");
                    prevt = true;
               }
               else
                    prevt = false;
               g_free(thrs);

               if (string(tmins) == "1")
               {
                    if (prevt)
                         lasttime += ",";
                    lasttime += " " + string(tmins) + _(" min");
                    prevt = true;
               }
               else if (string(tmins) != "0")
               {
                    if (prevt)
                         lasttime += ",";
                    lasttime += " " + string(tmins) + _(" mins");
                    prevt = true;
               }
               else
                    prevt = false;
               g_free(tmins);

               if (!prevt)
               {
                    if (string(tsecs) == "1")
                         lasttime += " " + string(tsecs) + _(" sec");
                    else if (string(tsecs) != "0")
                         lasttime += " " + string(tsecs) + _(" secs");
               }
               g_free(tsecs);


               Gtk::Label* l;
               get_widget("Last_lbl", l);
               l->show();

               // last logged out or idle?
               if (offline)
               {
                    l->set_label(Util::substitute(_("Last logged out %s ago"), lasttime));

                    get_widget("Status_lbl", l);
                    l->set_label(Util::substitute(_("%s: %s"), _("Offline"), t.getChildCData("query"))); // Get the status from iq:last
               }
               else
               {
                    l->set_label(Util::substitute(_("Idle for %s"), lasttime));
               }
          }
     }     
}

void ContactInfoDlg::on_PrettyJID_changed()
{
     send_disco_request();
     send_version_request();
     send_time_request();
     get_status();

     // Hide the idle labels.
     // get_status() will call on_presence(), which will send_last_request() for us
     Gtk::Label* l;
     get_widget("Last_lbl", l);
     l->hide();
}

// ---------------------------------------------------------
//
// My Contact Information Dialog
//
// ---------------------------------------------------------

void MyInfoDlg::execute()
{
     MyInfoDlg* e = new MyInfoDlg();
     e->show();
}

MyInfoDlg::MyInfoDlg()
     : BaseGabberWindow("MyInfo_dlg")
{
     // Set the dialog's parent
//     Gtk::Dialog* dlg  = static_cast<Gtk::Dialog*>(getGtkWindow());
//     dlg->set_transient_for(parent);
          
     // Align the labels with the magic of SizeGroup
     Gtk::Label* l;
     Glib::RefPtr<Gtk::SizeGroup> client_grp = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
     get_widget("JabberClientName_lbl", l);
     client_grp->add_widget(*l);
     get_widget("JabberClientVersion_lbl", l);
     client_grp->add_widget(*l);
     get_widget("ComputerOS_lbl");
     client_grp->add_widget(*l);
     get_widget("ComputerTime_lbl");
     client_grp->add_widget(*l);
     
     Gtk::Button* b;
     get_widget("Cancel_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &MyInfoDlg::close));
     get_widget("Update_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &MyInfoDlg::on_Update_clicked));
     b->set_sensitive(false);
     
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     get_widget("JID_lbl", l);
     // XXX: Hack. We need to have a better way to get our own JID.
     // Probably put something in JabberConnection
     l->set_text(config.get_string(Keys::acct.username) + "@"
                 + config.get_string(Keys::acct.server) + "/"
                 + config.get_string(Keys::acct.resource));
     
     get_widget("JabberClientName_lbl", l);
     l->set_text(ENV_VARS.package);
     get_widget("JabberClientVersion_lbl", l);
     l->set_text(ENV_VARS.version);
     get_widget("ComputerOS_lbl", l);
#ifndef WIN32
     // Run uname
     struct utsname osinfo;
     uname(&osinfo);
     l->set_text(string(osinfo.sysname) + " " + string(osinfo.machine));
#else
    l->set_text("Unknown Windows Machine");
#endif
     
     send_vcard_request();
}

MyInfoDlg::~MyInfoDlg()
{
     free(_vcard);
}

void MyInfoDlg::on_Update_clicked()
{
     Gtk::Entry* e;
     // General
     get_widget("Nickname_ent", e);
     (*_vcard)[vCard::Nickname] = e->get_text();
     get_widget("FullName_ent", e);
     (*_vcard)[vCard::Fullname] = e->get_text();
     get_widget("Email_ent", e);
     (*_vcard)[vCard::EMail] = e->get_text();
     get_widget("WebSite_ent", e);
     (*_vcard)[vCard::URL] = e->get_text();
     get_widget("Telephone_ent", e);
     (*_vcard)[vCard::Telephone] = e->get_text();
     
     // Location
     get_widget("Street_ent", e);
     (*_vcard)[vCard::Street] = e->get_text();
     get_widget("Street2_ent", e);
     (*_vcard)[vCard::Street2] = e->get_text();
     get_widget("City_ent", e);
     (*_vcard)[vCard::City] = e->get_text();
     get_widget("State_ent", e);
     (*_vcard)[vCard::State] = e->get_text();
     get_widget("PCode_ent", e);
     (*_vcard)[vCard::PCode] = e->get_text();
     get_widget("Country_ent", e);
     (*_vcard)[vCard::Country] = e->get_text();
     
     // Organization
     get_widget("OrgName_ent", e);
     (*_vcard)[vCard::OrgName] = e->get_text();
     get_widget("OrgUnit_ent", e);
     (*_vcard)[vCard::OrgUnit] = e->get_text();
     get_widget("PersonalTitle_ent", e);
     (*_vcard)[vCard::PersonalTitle] = e->get_text();
     get_widget("PersonalRole_ent", e);
     (*_vcard)[vCard::PersonalRole] = e->get_text();
     
     // About
     get_widget("Birthday_ent", e);
     (*_vcard)[vCard::Birthday] = e->get_text();
     Gtk::TextView* tv;
     get_widget("About_txtview", tv);
     (*_vcard)[vCard::Description] = tv->get_buffer()->get_text();
     
     // Get next session ID
     string id = G_App.getSession().getNextID();
     
     // Construct vCard request
     Packet iq("iq");
     iq.setID(id);
     iq.getBaseElement().putAttrib("type", "set");
     iq.getBaseElement().appendChild(new judo::Element(_vcard->getBaseElement()));
     
     // Send the vCard request
     G_App.getSession() << iq;
     //G_App.getSession().registerIQ(id, slot(*this, &MyInfoDlg::parse_vcard));
     
     close();
}
     
void MyInfoDlg::send_vcard_request()
{
     // Get next session ID
     string id = G_App.getSession().getNextID();
     
     // Construct vCard request
     Packet iq("iq");
     iq.setID(id);
     iq.getBaseElement().putAttrib("type", "get");
     judo::Element* vCard = iq.getBaseElement().addElement("vCard");
     vCard->putAttrib("xmlns", "vcard-temp");
     vCard->putAttrib("version", "2.0");
     vCard->putAttrib("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
     
     // Send the vCard request
     G_App.getSession() << iq;
     G_App.getSession().registerIQ(id, SigC::slot(*this, &MyInfoDlg::parse_vcard));
}

void MyInfoDlg::parse_vcard(const judo::Element& t)
{
     if (t.empty())
          return;
     
     const judo::Element* vCardElem = t.findElement("vCard");
     if (vCardElem == NULL)
          return;
     
     Gtk::Button* b;
     get_widget("Update_btn", b);
     b->set_sensitive(true);

     Glib::ustring curtext;
     
     _vcard = new vCard(*vCardElem);
     
     Gtk::Entry* e;
     
     // General
     curtext = (*_vcard)[vCard::Nickname];
     get_widget("Nickname_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::Fullname];
     get_widget("FullName_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::EMail];
     get_widget("Email_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::URL];
     get_widget("WebSite_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::Telephone];
     get_widget("Telephone_ent", e);
     e->set_text(curtext);
     
     // Location
     curtext = (*_vcard)[vCard::Street];
     get_widget("Street_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::Street2];
     get_widget("Street2_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::City];
     get_widget("City_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::State];
     get_widget("State_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::PCode];
     get_widget("PCode_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::Country];
     get_widget("Country_ent", e);
     e->set_text(curtext);
     
     // Organization
     curtext = (*_vcard)[vCard::OrgName];
     get_widget("OrgName_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::OrgUnit];
     get_widget("OrgUnit_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::PersonalTitle];
     get_widget("PersonalTitle_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::PersonalRole];
     get_widget("PersonalRole_ent", e);
     e->set_text(curtext);
     
     // About
     curtext = (*_vcard)[vCard::Birthday];
     get_widget("Birthday_ent", e);
     e->set_text(curtext);
     curtext = (*_vcard)[vCard::Description];
     Gtk::TextView* tv;
     get_widget("About_txtview", tv);
     tv->get_buffer()->set_text(curtext);
}

} // namespace Gabber
