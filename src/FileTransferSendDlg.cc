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

#include "FileTransferSendDlg.hh"
#include "FileTransferDlg.hh"
#include "GabberWidgets.hh"
#include "GabberApp.hh"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/textview.h>
#include <gtkmm/dialog.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/table.h>
#include <gtkmm/arrow.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Gabber;

FileTransferSendDlg::FileTransferSendDlg(const std::string& jid) :
    BaseGabberWindow("FTSend_dlg"), _jid(jid), _show_details(false)
{
    Gtk::Button* btn;
    get_widget("FileBrowse_btn", btn);
    btn->signal_clicked().connect(
        SigC::slot(*this, &FileTransferSendDlg::on_browse_clicked));
    Gtk::Dialog* dlg = static_cast<Gtk::Dialog*>(getGtkWindow());
    dlg->signal_response().connect(
        SigC::slot(*this, &FileTransferSendDlg::on_response));
    Gtk::HBox* hbox;
    get_widget("JIDInfo_hbox", hbox);
    _prettyjid = Gtk::manage(new PrettyJID(jid, "", PrettyJID::dtNickRes, true));
    _prettyjid->show();
    hbox->pack_end(*_prettyjid);
    get_widget("Expand_arw", _expand_arw);
    get_widget("Details_lbl", _details_lbl);
    get_widget("Info_tbl", _details_tbl);
    get_widget("FileSelection_ent", _fileselection_ent);
    get_widget("Description_txtview", _description_tv);
    get_widget("IPAddress_ent", _host_ent);
    get_widget("Send_btn", _send_btn);

    Gtk::EventBox* evb;
    get_widget("Expand_evb", evb);
    evb->signal_button_release_event().connect(
        SigC::slot(*this, &FileTransferSendDlg::on_expand_clicked));
}

FileTransferSendDlg::~FileTransferSendDlg()
{
}

void FileTransferSendDlg::on_browse_clicked()
{
    Gtk::FileSelection filesel;
    int resp = filesel.run();
    if (resp != Gtk::RESPONSE_OK)
    {
        return;
    }

    std::string filename = filesel.get_filename();
    _send_btn->set_sensitive( (filename.size() > 0) );
    _fileselection_ent->set_text(filename);
    filesel.hide();
}

void FileTransferSendDlg::on_response(int response_id)
{
    if (response_id != Gtk::RESPONSE_OK)
    {
        close();
        return;
    }

    getGtkWindow()->hide();

    // Setup the info for the file we're sending
    FTProfile::FileInfo info;
    info.desc = _description_tv->get_buffer()->get_text();
    std::string filename = _fileselection_ent->get_text();
    info.suggested_name = Glib::path_get_basename(filename);
    struct stat stat_info;
    stat(filename.c_str(), &stat_info);
    info.size = stat_info.st_size;

    // Setup the stream and transfer dialog
    std::string proxy = _host_ent->get_text();
    std::string id = G_App.getFileTransferManager().initiate(_prettyjid->get_full_jid(), filename, info, G_App.getSession().getNextID(), proxy);
    FileTransferDlg* dlg = new FileTransferDlg(info, _prettyjid->get_full_jid(), filename, id, false);
    dlg->show();
    G_App.getFileTransferManager().add_listener(id, dlg);
    close();
}

bool FileTransferSendDlg::on_expand_clicked(GdkEventButton* event)
{
    if (_show_details)
    {
        _details_tbl->hide();
        _expand_arw->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_OUT);
        _details_lbl->set_text("Show details");
        _show_details = false;
    }
    else
    {
        _details_tbl->show();
        _expand_arw->set(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT);
        _details_lbl->set_text("Hide details");
        _show_details = true;
    }
    return true;
}
