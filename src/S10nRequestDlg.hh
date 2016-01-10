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
 *  Copyright (c) 2002 Julian Missig
 */

#ifndef INCL_S10NREQUESTDLG_HH
#define INCL_S10NREQUESTDLG_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

namespace Gabber {

/**
 * Handles a subscription request dialog
 */
class S10nRequestDlg : public BaseGabberWindow
{
public:
    /**
     * Display a subscription confirmation dialog
     * @param app The JabberConnection this subscription is taking place on
     * @param parent The window that owns the dialog
     * @param jid The jid of the person requesting a subscription
     * @param reason The reason supplied by the requester
     */
    static void display(JabberConnection& conn, Gtk::Window& parent, 
                        const Glib::ustring& jid, const Glib::ustring& reason);
    S10nRequestDlg(JabberConnection& conn, Gtk::Window& parent, 
                   const Glib::ustring& jid, const Glib::ustring& reason);
    ~S10nRequestDlg();
protected:
    // Dialog Callbacks
    void on_dlg_response(int resp);
    void on_ViewInfo_clicked();
    void send_vcard_request();
    void parse_vcard(const judo::Element& t);
    void on_DisplayName_optmenu_changed();
    void on_DisplayName_ent_changed();
private:
    JabberConnection& _conn;
    Glib::ustring _jid; // The jid of who's asking for the request
    Gtk::Button*      _btnAllow;
    Gtk::OptionMenu*  _optmenuDisplayName;
    Gtk::Entry*       _entDisplayName;
    
    Glib::ustring     _Nickname;
    Glib::ustring     _Fullname;

    SigC::Connection  _conn_changed;
}; // class S10nRequestDlg

} // namespace Gabber
#endif // INCL_S10NREQUESTDLG_HH
