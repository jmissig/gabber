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
 *  Based on Gabber, Copyright (c) 1999-2003 Dave Smith & Julian Missig
 *  Copyright (c) 2002-2003 Julian Missig
 */

#ifndef INCL_FILETRANSFERRECVDL_HH
#define INCL_FILETRANSFERRECVDL_HH

#include "BaseGabberWindow.hh"
#include "StreamInitiation.hh"
#include "FTProfile.hh"

#include <gtkmm/fileselection.h>

namespace Gabber {

class FileTransferRecvDlg : public BaseGabberWindow
{
public:
    FileTransferRecvDlg(const Glib::ustring& from, const Glib::ustring& id,
                        SI* si);
    ~FileTransferRecvDlg();
protected:
    void on_browse();
    void on_accept();
    void on_decline();
private:
    Gtk::Entry* _fileselection_ent;
    Glib::ustring _from;
    Glib::ustring _id;
    SI* _si;
    FTProfile::FileInfo& _info;
};

};

#endif // INCL_FILETRANSFERRECVDL_HH
