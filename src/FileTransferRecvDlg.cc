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

#include "FileTransferRecvDlg.hh"
#include "GabberWidgets.hh"
#include "FileTransferDlg.hh"
#include "S5B.hh"
#include "GabberApp.hh"
#include "intl.h"

#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

using namespace Gabber;

FileTransferRecvDlg::FileTransferRecvDlg(const Glib::ustring& from, 
                                         const Glib::ustring& id, SI* si) : 
    BaseGabberWindow("FTRecv_dlg"), _from(from), _id(id), _si(si), 
    _info(static_cast<FTProfile*>(_si->getProfile())->getFileInfo())
{
    printf("Namespace: %s\n", _si->getProfile()->getNamespace().c_str());
    printf("Suggested name: %s\n", static_cast<FTProfile*>(_si->getProfile())->getFileInfo().suggested_name.c_str());
    Gtk::HBox* hbox;
    get_widget("JIDInfo_hbox", hbox);
    PrettyJID* pj = Gtk::manage(
        new PrettyJID(_from, "", PrettyJID::dtNickJID));
    pj->show();
    hbox->pack_end(*pj);

    getGtkWindow()->set_title(_("Offered file from ") + pj->get_nickname());
    
    Gtk::Label* lbl;
    get_widget("FileName_lbl", lbl);
    lbl->set_text(_info.suggested_name);

    if (!_info.desc.empty())
    {
        Gtk::TextView* tv;
        get_widget("Description_txtview", tv);
        tv->get_buffer()->set_text(_info.desc);
        Gtk::ScrolledWindow* sw;
        get_widget("Description_scroll", sw);
        sw->show();
        get_widget("Description_lbl", lbl);
        lbl->show();
    }

    Gtk::Button* btn;
    get_widget("Decline_btn", btn);
    btn->signal_clicked().connect(
            SigC::slot(*this, &FileTransferRecvDlg::on_decline));
    get_widget("Accept_btn", btn);
    btn->signal_clicked().connect(
            SigC::slot(*this, &FileTransferRecvDlg::on_accept));
    get_widget("FileBrowse_btn", btn);
    btn->signal_clicked().connect(
            SigC::slot(*this, &FileTransferRecvDlg::on_browse));

    get_widget("FileSelection_ent", _fileselection_ent);
    _fileselection_ent->set_text(_info.suggested_name);
}

FileTransferRecvDlg::~FileTransferRecvDlg()
{
    delete _si;
}

void FileTransferRecvDlg::on_browse()
{
    Gtk::FileSelection filesel;
    filesel.set_filename(_info.suggested_name);
    filesel.run();
    _fileselection_ent->set_text(filesel.get_filename());
}

void FileTransferRecvDlg::on_accept()
{
    Glib::ustring fil(_fileselection_ent->get_text());

    // Create the view dialog and hook it in
    FileTransferDlg* dlg = new FileTransferDlg(_info, _from, fil, _si->getID(), true);

    // XXX For now we assume S5B
    Stream* stream = new S5B(_si->getID());
    dlg->show();
    G_App.getFileTransferManager().receive(_from, fil, stream, _si->getID());
    G_App.getFileTransferManager().add_listener(_si->getID(), dlg);
    
    FeatureNegotiation* fneg = _si->getFeatureNegotiation();
    fneg->choose("stream-method", "http://jabber.org/protocol/bytestreams");

    // Send the reply
    judo::Element iq("iq");
    iq.putAttrib("to", _from);
    iq.putAttrib("id", _id);
    iq.putAttrib("type", "result");
    judo::Element* si_res = _si->buildResultNode();
    iq.appendChild(si_res);
    G_App.getSession() << iq.toString().c_str();
    close();
}

void FileTransferRecvDlg::on_decline()
{
    judo::Element* err = SI::buildRejectionNode();
    err->putAttrib("to", _from);
    err->putAttrib("id", _id);
    err->putAttrib("type", "error");
    G_App.getSession() << err->toString().c_str();
    delete err;
    close();
}
