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

#ifndef INCL_FILETRANSFERDLG_HH
#define INCL_FILETRANSFERDLG_HH

#include "BaseGabberWindow.hh"
#include "Stream.hh"
#include "FTProfile.hh"
#include "FileTransferManager.hh"

#include <jabberoo/XPath.h>
#include <jabberoo/discoDB.hh>

#include <string>
#include <fstream>

namespace Gtk {
    class Arrow;
    class Label;
    class Table;
};

namespace Gabber {

/**
 * Dialog for receiving a file transfer.
 */
class FileTransferDlg : 
    public BaseGabberWindow, public FileTransferManager::FileTransferListener
{
public:
    /**
     * Creates a dialog for transferring a file.
     * @param info The FileInfo for the file.
     * @param recvr Whether this transfer is a sender or receiver, default true
     */
    FileTransferDlg(FTProfile::FileInfo& info, const std::string& jid, 
                    const std::string& filename, const std::string& sid,
                    bool recvr = true);
    ~FileTransferDlg();

    void transfer_update(int sz);
    void transfer_closed();
    void transfer_error(const std::string& msg);
private:
    FTProfile::FileInfo _info;
    std::string _jid;
    std::string _filename;
    bool _recvr; // true if we are receiving the file, false if sending
    Gtk::Label* _status_lbl;
    Gtk::Label* _size_remaining_lbl;
    Gtk::Label* _transfer_speed_lbl;
    Gtk::Label* _expand_lbl;
    Gtk::Arrow* _expand_arw;
    Gtk::ProgressBar* _progress_pbar;
    Gtk::Table* _info_tbl;
    Gtk::Button* _stop_btn;
    bool _info_is_hidden; // true when the info table is hidden
    time_t _start_time;
    long _total;
    std::string _sid;
    bool _running;
    bool _has_error;

    void init_gui();
    void on_error(const std::string& msg);
    bool on_timeout();
    // GUI Callbacks
    void on_stop_clicked();
    bool on_expand_clicked(GdkEventButton* event);
};

};

#endif // INCL_FILETRANSFERDLG_HH
