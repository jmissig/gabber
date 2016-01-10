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
 *  Based on Gabber, Copyright (C) 1999-2002 Dave Smith & Julian Missig
 *  Copyright (c) 2003 Julian Missig
 */

#include "GabberWidgets.hh"

#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "ResourceManager.hh"
#include "Menus.hh"

#include "intl.h"
#include <jabberoo/JID.hh>
#include <jabberoo/presenceDB.hh>
#include <jabberoo/roster.hh>
#include <gtkmm/box.h>
#include <gtkmm/combo.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

namespace Gabber {

// PrettyJID - nice display of JabberID - draggable and all
PrettyJID::PrettyJID(const Glib::ustring& jid, const Glib::ustring& nickname, 
		     DisplayType dt, bool select_resource, bool select_jid)
     : _jid(jid), _nickname(nickname), _resource(jabberoo::JID::getResource(_jid)), 
       _default_jid(_jid), _display_type(dt), 
       _select_resource(select_resource), _select_jid(select_jid)
{
     // If we're selecting a resource
     if (_select_resource)
     {
	  // _jid should only contain user@host
	  _jid = jabberoo::JID::getUserHost(_jid);
     }

     // If we need to lookup the nickname
     if (_nickname.empty())
     {
	  // attempt to look it up
	  try {
	       _nickname = G_App.getSession().roster()[jabberoo::JID::getUserHost(_jid)].getNickname();
            _on_roster = true;
	  } catch (jabberoo::Roster::XCP_InvalidJID& e) {
	       // the default nickname is the username
	       _nickname = jabberoo::JID::getUser(_jid);
            _on_roster = false;
	  }
     }
     
    // Hook up the presence event
    G_App.getSession().evtPresence.connect(SigC::slot(*this, &PrettyJID::on_presence));

     build_widget();

     // Now that the widget is all created
     // Start setting up drag-n-drop

     // We're a source
     std::list<Gtk::TargetEntry> listSourceTargets;
     listSourceTargets.push_back(Gtk::TargetEntry("text/x-jabber-roster-item"));
     listSourceTargets.push_back(Gtk::TargetEntry("text/x-jabber-id"));

     drag_source_set(listSourceTargets);
     

     // We're a destination
     std::list<Gtk::TargetEntry> listDestTargets;
     listDestTargets.push_back(Gtk::TargetEntry("text/x-jabber-id"));
     drag_dest_set(listDestTargets);
}

void PrettyJID::build_widget()
{
     // Create the HBox
     _hboxPJ = manage(new Gtk::HBox(false, 3));
     add(*_hboxPJ);

     
     // Create the resource selector if needed
     if (_select_resource) {
	  // The resource selector combo
	  _cboResource = manage(new Gtk::Combo());
//	  _cboResource ->set_size_request(100, 0);
	  _hboxPJ      ->pack_end(*_cboResource, true, true, 0);
	  _cboResource ->show();
	  
	  // The label between the jid and the resource selector
	  _lblResource  = manage(new Gtk::Label("/", 0.0, 0.5));
	  _hboxPJ       ->pack_end(*_lblResource, false, true, 0);
	  _lblResource  ->show();
	  
	  // The list of resources
	  list<Glib::ustring> resources;
	  // If we have a resource, add it - it may not be among the list in presencedb
	  if (!_resource.empty()) {
	       resources.push_back(_resource);
	       _cboResource->get_entry()->set_text(_resource);
	  }
	  try {
	       // Walk the list of resources and add them to the combo box
	       jabberoo::PresenceDB::range r = G_App.getSession().presenceDB().equal_range(_jid);
	       for (jabberoo::PresenceDB::const_iterator it = r.first; it != r.second; it++) {
		    const jabberoo::Presence& p = *it;
		    
		    // If this presence is a NA presence, then skip it
		    if (p.getType() == jabberoo::Presence::ptUnavailable)
			 continue;
		    
		    // Extract the resource
		    const Glib::ustring& res = jabberoo::JID::getResource(p.getFrom());
		    // Avoid resource duplication
		    if (res != _resource)
			 resources.push_back(res);
	       }
	  } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	       // No presences from any resources
	  }

	  // Attach the list of resources to the combo
	  if (!resources.empty())
	       _cboResource->set_popdown_strings(resources);
	  
          if (_resource.empty())
          {
               _resource = _cboResource->get_entry()->get_text();
	       if (_resource.empty())
		    _default_jid = _jid;
	       else
		    _default_jid = _jid + "/" + _resource;
          }
          
	  // Hook up the changed event for the resource entry
	  // We do this *after* setting the resource because otherwise
	  // it would try to grab a presence and call on_presence etc
	  // before we've finished setting up the other widgets - 
	  // including the pixmap for status
	  // Plus, we're going to grab the presence at the end this ctor anyway
	  _cboResource->get_entry()->signal_changed().connect(SigC::slot(*this, &PrettyJID::on_entResource_changed));
     }
     
     // If we're selecting a jid
     if (_select_jid) {
	  // Create the entry for the jid
	  _entJID = manage(new Gtk::Entry());
	  _hboxPJ ->pack_end(*_entJID, true, true, 0);
	  _entJID ->set_text(_jid);
	  _entJID ->signal_changed().connect(SigC::slot(*this, &PrettyJID::on_entJID_changed));
	  _entJID ->show();
     } else {
	  // Create the label
	  _lblPJ  = manage(new Gtk::Label("", 0.0, 0.5));
	  _hboxPJ ->pack_end(*_lblPJ, !_select_resource, true, 0);
	  _lblPJ  ->show();
     }

     // Create the presence image
     std::string filename;
     try {
	  jabberoo::Presence presence = G_App.getSession().presenceDB().findExact(_default_jid);
	  if((presence.getShow() == jabberoo::Presence::stInvalid) ||
	     (presence.getShow() == jabberoo::Presence::stOffline))
	       filename = "offline.png";
	  else
	       filename = presence.getShow_str() + ".png";
     } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	  filename = "offline.png";
     }
     _pixPJ = manage(new Gtk::Image(ResourceManager::getSingleton().getPixbuf(filename)));
     _hboxPJ->pack_end(*_pixPJ, false, true, 0);
     _pixPJ  ->show();
     
     _hboxPJ ->show();
     
     // Set the default display
     set_display_type(_display_type);
     
     // And grab any existing presence...
     try {
	  const jabberoo::Presence& p = G_App.getSession().presenceDB().findExact(_default_jid);
	  on_presence(p, p.getType());
     } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	  _tips.set_tip(*this, _jid);
     }
}

void PrettyJID::set_display_type(DisplayType dt)
{
     // save the display type
     _display_type = dt;

     Glib::ustring display_text;

     switch (dt)
     {
     case dtNickRes:
	  if (!_select_resource && !_resource.empty())
	  {
	       display_text = _nickname + "/" + _resource;
	       break;
	  }
	  // else it'll continue on and display just the nickname
     case dtNick:
	  display_text = _nickname;
	  break;
     case dtJID:
	  if (!_select_resource && !_resource.empty())
	  {
	       display_text = jabberoo::JID::getUserHost(_jid);
	       break;
	  }
	  // else it'll just display the contents of _jid
     case dtJIDRes:
	  display_text = _jid;
	  break;
     case dtNickJIDRes:
	  if (!_resource.empty())
	  {
	       display_text = _nickname + "/" + _resource;
	       display_text += " (" + jabberoo::JID::getUserHost(_jid) + ")";
	       break;
	  }
	  // else it'll continue on and display just the nickname
     case dtNickJID:
	  display_text = _nickname;
	  display_text += " (" + jabberoo::JID::getUserHost(_jid) + ")";
	  break;
     }

     // Finally, set the text
     if (!_select_jid)
	  _lblPJ->set_text(display_text);
}

void PrettyJID::hide_resource_select()
{
     g_assert( _cboResource != NULL );

     // no-op short circuit
     if (_select_resource == false)
	  return;

     // No longer selecting resource
     _select_resource = false;
     // Allow the JabberID label to expand
//FIXME: Why is this an undefined reference to Gtk::Box_Helpers::Child::set_options(bool, bool, unsigned) ?
//FIXME: Even gtk--/box.h is included, it compiles fine but doesn't link (?)
//     (*_hboxPJ->children().find(*_lblPJ))->set_options(true, true, 0);
     // Remove the resource selection
//     _hboxPJ->remove(*_cboResource);
//     _hboxPJ->remove(*_lblResource);

//     _hboxPJ->remove(*_lblPJ);


     // rebuild widget
     hide();
     remove();
     build_widget();
     show();
}

bool PrettyJID::is_displaying_jid() const
{
     switch (_display_type)
     {
     case dtNickRes:
     case dtNick:
	  return false;
     case dtJID:
     case dtJIDRes:
     case dtNickJIDRes:
     case dtNickJID:
	  return true;
     }
     return true;
}

void PrettyJID::on_presence(const jabberoo::Presence& p, const jabberoo::Presence::Type prev)
{
    // Display a notification message if this presence packet
    // is from the jabberoo::JID associated with this widget
    if (jabberoo::JID::compare(p.getFrom(), _default_jid) != 0)
        return;

    // Set the labels
    std::string filename;
    if((p.getShow() == jabberoo::Presence::stInvalid) ||
       (p.getShow() == jabberoo::Presence::stOffline) ||
       p.getShow_str().empty())
        filename = "offline.png";
    else
        filename = p.getShow_str() + ".png";
    _pixPJ->set(ResourceManager::getSingleton().getPixbuf(filename));

    // Set the tooltip
    Glib::ustring tooltip;
    // if they can't see the jabberoo::JID, show it in the tooltip
    if (!is_displaying_jid())
    tooltip += _default_jid + "\n";
    tooltip += Gabber::Util::getShowName(p.getShow());
    if (!p.getStatus().empty())
         tooltip += ": " + p.getStatus();
    _tips.unset_tip(*this);
    _tips.set_tip(*this, tooltip);

// XXX Move to an extension
//     Element* x = p.findX("gabber:x:music:info");
//     if (!x)
//     {
//	  x = p.findX("jabber:x:music:info");
//     }
//     if (x)
//     {
//	  string song_title = x->getChildCData("title");
//	  string state = x->getChildCData("state");
//	  if (state == "stopped")
//	  {
//	       song_title += _(" [stopped]");
//	       _pixMusic->load(ENV_VARS.pixmapdir + "xmms_stopped.xpm");
//	  }
//	  else if (state == "paused")
//	  {
//	       song_title += _(" [paused]");
//	       _pixMusic->load(ENV_VARS.pixmapdir + "xmms_paused.xpm");
//	  }
//	  else
//	  {
//	       _pixMusic->load(ENV_VARS.pixmapdir + "xmms.xpm");
//	  }
//	  _tips.set_tip(*_evtMusic, fromUTF8(this, song_title));
//	  _evtMusic->show();
//     }
//     else
//     {
//	  _evtMusic->hide();
//     }

     // Presence can fundamentally change
     // some things (gpg), so say so
//     changed();
}

bool PrettyJID::on_button_press_event(GdkEventButton* e)
{
     
     if (e->type == GDK_BUTTON_PRESS && e->button == 3)
     {
	  Popups::User::getSingleton().jid_popup(_default_jid, e->button, e->time);
     }
     return false;
}

void PrettyJID::on_entJID_changed()
{
     g_assert( _entJID != NULL );

     // I don't really like the duplication of code in this function
     // but this is such a rare case that I didn't want to redesign
     // PrettyJID to accomplish this without code duplication...

     _jid = _entJID->get_text();
     if (!_jid.empty())
     {
	  // Set the new full jabberid
	  if (_resource.empty())
	  {
	       _default_jid = _jid;
	  }
	  else
	  {
	       _default_jid = _jid + "/" + _resource;
	  }

	  // Grab an existing presence for this particular resource (maybe)
	  // and process it
	  try {
	       const jabberoo::Presence& p = G_App.getSession().presenceDB().findExact(_default_jid);
	       on_presence(p, p.getType());
	  } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	       // Construct an offline presence
	       jabberoo::Presence p("", jabberoo::Presence::ptUnavailable, jabberoo::Presence::stOffline, _("No presence has been received."));
	       p.setFrom(_default_jid);
	       on_presence(p, p.getType());
	  }
     }

     // Lookup nickname - default is the username
     try {
	  _nickname = G_App.getSession().roster()[jabberoo::JID::getUserHost(_jid)].getNickname();
     } catch (jabberoo::Roster::XCP_InvalidJID& e) {
	  _nickname = jabberoo::JID::getUser(_jid);
     }

     // The list of resources
     list<Glib::ustring> resources;
     // If we have a resource, add it - it may not be among the list in presencedb
     try {
	  // Walk the list of resources and add them to the combo box
	  jabberoo::PresenceDB::range r = G_App.getSession().presenceDB().equal_range(_jid);
	  for (jabberoo::PresenceDB::const_iterator it = r.first; it != r.second; it++)
	  {
	       const jabberoo::Presence& p = *it;
	       
	       // If this presence is a NA presence, then skip it
	       if (p.getType() == jabberoo::Presence::ptUnavailable || p.getType() == jabberoo::Presence::ptError)
		    continue;
	       
	       // Extract the resource
	       const Glib::ustring& res = jabberoo::JID::getResource(p.getFrom());
	       resources.push_back(res);
	  }
     } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	  // No presences from any resources
     }
     
     // Attach the list of resources to the combo
     if (!resources.empty())
	  _cboResource->set_popdown_strings(resources);

     // Fire the signal
     changed();
}

void PrettyJID::on_entResource_changed()
{
     g_assert( _cboResource != NULL );
     g_assert( _cboResource->get_entry() != NULL);

     _resource = _cboResource->get_entry()->get_text();
     if (!_resource.empty())
     {
	  // Set the new full jabberid
	  _default_jid = _jid + "/" + _resource;

	  // Grab an existing presence for this particular resource (maybe)
	  // and process it
	  try {
	       const jabberoo::Presence& p = G_App.getSession().presenceDB().findExact(_default_jid);
	       on_presence(p, p.getType());
	  } catch (jabberoo::PresenceDB::XCP_InvalidJID& e) {
	       // Construct an offline presence
	       jabberoo::Presence p("", jabberoo::Presence::ptUnavailable, jabberoo::Presence::stOffline, _("No presence has been received."));
	       p.setFrom(_default_jid);
	       on_presence(p, p.getType());
	  }
     }
     else
     {
	  _default_jid = _jid;
     }

     // Fire the signal
     changed();
}


// PrettyURI - nice display of a URI with description. Clickable.
PrettyURI::PrettyURI(const Glib::ustring& uri, const Glib::ustring& desc)
     : _uri(uri)
{
     // TODO: do fancy stuff based on desc length
     Gtk::Label* lbl = manage(new Gtk::Label("<span color='blue' underline='single'>" + desc + "</span>", 0.0, 0.0));
     lbl->set_use_markup(true);
     lbl->set_line_wrap(true);
     lbl->set_selectable(false);
     //lbl->add_events(Gdk::BUTTON_PRESS_MASK);
     //lbl->signal_button_press_event().connect(SigC::slot(*this, &PrettyURI::on_lbl_button_press_event));
     
     // TODO: set the cursor to whatever the "hand" link cursor is
     //get_parent_window()->set_cursor(Gdk::Cursor
     
     _tips.set_tip(*this, _uri);
     
     add(*lbl);
     lbl->show();
}

bool PrettyURI::on_button_press_event(GdkEventButton* e)
{
     if (e->type == GDK_BUTTON_PRESS)
     {
          if (e->button == 1)
          {
               Util::openuri(_uri);
          }
     }
     
     return false;
}


}; // namespace Gabber
