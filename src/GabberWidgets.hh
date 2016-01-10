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

#ifndef INCL_GABBER_WIDGETS_HH
#define INCL_GABBER_WIDGETS_HH

#include "fwd.h"
#include <jabberoo/presence.hh>

#include <gtkmm/eventbox.h>
#include <gtkmm/tooltips.h>

namespace Gabber
{

/**
 * A pretty display for a JabberID in Gabber.
 * @see Gtk::EventBox
 */
class PrettyJID
     : public Gtk::EventBox
{
public:
     /**
      * Display type.
      * This indicates what exactly should be displayed in the PrettyJID.
      */
     enum DisplayType {
	  dtNick,       /**< Only the nickname. */
	  dtNickRes,    /**< The nickname and the resource. */
	  dtJID,        /**< Only the JabberID, no resource. */
	  dtJIDRes,     /**< The JabberID and the resource. */
	  dtNickJID,    /**< The nickname and the JabberID, no resource. */
	  dtNickJIDRes  /**< Everything. */
     };

     /**
      * Create a PrettyJID.
      * The only required attribute is the JabberID, everything else
      * is optional.
      * @param jid The full JabberID. Whether or not it contains a resource is up to you.
      * @param nickname The nickname to display.
      * @param dt The DisplayType to use when initially displaying this PrettyJID.
      * @param select_resource If true, enable resource selection. The changed signal is emitted whenever the resource is changed.
      * @param select_jid If true, enable JabberID selection. The changed signal is emitted whenever the JabberID or resource are changed.
      */
     PrettyJID(const Glib::ustring& jid, const Glib::ustring& nickname = "", DisplayType dt = dtJID, bool select_resource = false, bool select_jid = false);

     /**
      * Set the display type.
      * This sets the DisplayType for this widget and changes it as 
      * appropriate.
      * @param dt The DisplayType to use to display this PrettyJID.
      */
     void set_display_type(DisplayType dt);

     /**
      * Hide the resource selection widget.
      * This uses whatever the current value of the resource 
      * selection widget is and redisplays the widget without 
      * the resource selection widget.
      */
     void hide_resource_select();

     /**
      * Whether or not the resource selection widget is being displayed.
      * @return true if resource selection is being displayed.
      */
     bool is_selecting_resource() const { return _select_resource; };

     /**
      * Get the displayed nickname.
      * @return string of the nickname.
      */
     const Glib::ustring get_nickname() const { return _nickname; }
     /**
      * Get the full JabberID plus resource.
      * @return string of the full JabberID, including resource.
      */
     const Glib::ustring get_full_jid() const { return _default_jid; }
     /**
      * Get the JabberID this PrettyJID points to.
      * This may or may not contain a resource, depending
      * on the cirumstances.
      * @retun the JabberID this PrettyJID points to.
      */
     const Glib::ustring get_jid() const { return _jid; }

     /**
      * Whether or not this JID appears to be on the Roster.
      * @return true if the JID is on the Roster.
      */
     bool is_on_roster() { return _on_roster; }

     /**
      * This signal is emitted whenever the JabberID or resource are changed.
      */
     SigC::Signal0<void> changed;

protected:
     void build_widget();
     bool is_displaying_jid() const;
     bool on_button_press_event(GdkEventButton* e);
     void on_presence(const jabberoo::Presence& p, const jabberoo::Presence::Type prev);
     void on_entJID_changed();
     void on_entResource_changed();

private:
     Glib::ustring _jid;
     Glib::ustring _nickname;
     Glib::ustring _resource;
     Glib::ustring _default_jid;
     bool _on_roster;
     DisplayType   _display_type;
     bool _select_resource;
     bool _select_jid;

     Gtk::HBox*    _hboxPJ;
     Gtk::Combo*   _cboResource;
     Gtk::Label*   _lblResource;
     Gtk::Entry*   _entJID;
     Gtk::Label*   _lblPJ;
     Gtk::Image*   _pixPJ;
     Gtk::Tooltips _tips;

}; // PrettyJID


/**
 * A pretty display for a URI in Gabber.
 * When clicked, it will be opened in appropriate browser and all.
 */
class PrettyURI
     : public Gtk::EventBox
{
public:
     /**
      * Create a PrettyURI.
      * @param uri The URI to link to.
      * @param desc The description of the link.
      */
     PrettyURI(const Glib::ustring& uri, const Glib::ustring& desc);
protected:
     bool on_button_press_event(GdkEventButton* e);
     
private:
     Gtk::Tooltips _tips;
     Glib::ustring _uri;
}; // PrettyURI

}; // namespace Gabber

#endif // INCL_GABBER_WIDGETS_HH
