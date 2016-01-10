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

#ifndef INCL_CONTACTINFO_HH
#define INCL_CONTACTINFO_HH

#include "BaseGabberWindow.hh"
#include <judo.hpp>
#include <jabberoo/discoDB.hh>
#include <jabberoo/XPath.h>

namespace jabberoo {
class vCard;
}

namespace Gabber {

class JabberConnection;
class PrettyJID;

/**
 * Contact Information Dialog.
 * Displays various information about a JID, based on vCard.
 */
class ContactInfoDlg : public BaseGabberWindow
{
public:
     /**
      * Display a ContactInfoDlg for the given JabberID.
      * @param conn The JabberConnection (GabberApp) we're using.
      * @param parent The parent Window.
      * @param jid The JabberID to get info for.
      */
     static void display(JabberConnection& conn, Gtk::Window& parent,
		    const Glib::ustring& jid);

protected:
     ContactInfoDlg(JabberConnection& conn, Gtk::Window& parent,
		    const Glib::ustring& jid);

     void close();
     void send_vcard_request();
     void set_info_label(const Glib::ustring& label_name, const Glib::ustring& data);
     void parse_vcard(const judo::Element& t);
     void send_disco_request();
     void parse_disco(const jabberoo::DiscoDB::Item* item);
     void send_version_request();
     void parse_version(const judo::Element& t);
     void send_time_request();
     void parse_time(const judo::Element& t);
     void get_status();
     void on_presence_node(const judo::Element& e);
     void send_last_request(bool offline);
     void parse_last(const judo::Element& t, bool offline);
     void on_PrettyJID_changed();
private:
     JabberConnection& _conn;
     Glib::ustring     _jid;
     Glib::ustring     _nickname;
     PrettyJID*        _prettyjid;
     judo::XPath::Query*    _presence_query;
}; // class ContactInfoDlg


/**
 * Edit My Contact Information.
 */
class MyInfoDlg : public BaseGabberWindow
{
public:
     /**
      * Display a My Info dialog.
      */
     static void execute();
     
protected:
     MyInfoDlg();
     ~MyInfoDlg();
     void on_Update_clicked();
     void send_vcard_request();
     void parse_vcard(const judo::Element& t);
     
private:
     jabberoo::vCard* _vcard;
}; // class MyInfoDlg

} // namespace Gabber

#endif // INCL_CONTACTINFO_HH
