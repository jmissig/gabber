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
 *  Copyright (c) 2002-2004 Julian Missig
 */

#ifndef INCL_FILETRANSFERSENDDLG_HH
#define INCL_FILETRANSFERSENDDLG_HH

#include "BaseGabberWindow.hh"

namespace Gtk {
    class Entry;
    class TextView;
    class Button;
    class Arrow;
    class EventBox;
    class Table;
};

namespace Gabber {

class PrettyJID;

class FileTransferSendDlg : public BaseGabberWindow
{
public:
    FileTransferSendDlg(const std::string& jid);
    ~FileTransferSendDlg();
private:
    std::string _jid;
    PrettyJID* _prettyjid;
    bool _show_details;
    Gtk::EventBox* _expand_evb;
    Gtk::Arrow* _expand_arw;
    Gtk::Label* _details_lbl;
    Gtk::Entry* _fileselection_ent;
    Gtk::TextView* _description_tv;
    Gtk::Entry* _host_ent;
    Gtk::Table* _details_tbl;
    Gtk::Button* _send_btn;
    void on_browse_clicked();
    void on_response(int response_id);
    bool on_expand_clicked(GdkEventButton* event);
};

};

#endif // INCL_FILETRANSFERSENDDLG_HH
