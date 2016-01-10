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

#include "FileTransferDlg.hh"
#include "GabberApp.hh"
#include "GabberWidgets.hh"
#include "GabberUtility.hh"
#include "StreamInitiation.hh"
#include "FeatureNegotiation.hh"
#include "XData.hh"
#include "S5B.hh"
#include "intl.h"

#include <gtkmm/label.h>
#include <gtkmm/arrow.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>

#include <string>

using namespace Gabber;

static std::string printBytes(float bytes)
{
    char string_buffer[32];
    static const char* size_name[] = {"B", "kB", "MB", "GB", "TB"};
    int i = 0;
    float final_size = bytes;

    while ( final_size > 1024 )
    {
        final_size = final_size / 1024.0;
        i++;
    }

    snprintf(string_buffer, 32, "%.1f %s", final_size, size_name[i]);
    return std::string(string_buffer);
}

static std::string float2string(float i)
{
     char buf[16];
     snprintf(buf, 16, "%.0f", i);
     return std::string(buf);
}

FileTransferDlg::FileTransferDlg(FTProfile::FileInfo& info, 
                                 const std::string& jid, 
                                 const std::string& filename,
                                 const std::string& sid, bool recvr) : 
    BaseGabberWindow("FTTransfer_dlg"), _info(info), _jid(jid),
    _filename(filename), _recvr(recvr), _info_is_hidden(true), 
    _start_time(0), _total(0), _sid(sid), _running(true), _has_error(false)
{
    init_gui();
    Glib::signal_timeout().connect( SigC::slot(*this,
                &FileTransferDlg::on_timeout), 60000);
}

FileTransferDlg::~FileTransferDlg()
{
    if (_running)
    {
        G_App.getFileTransferManager().stop(_sid);
    }
}

void FileTransferDlg::init_gui()
{
    Gtk::Label* lbl;
    
    // If we're the receiver flip a few labels and names
    if (_recvr)
    {
        getGtkWindow()->set_title(_("Receiving File"));
        get_widget("lblJIDInfo", lbl);
        lbl->set_text(_("Receiving from:"));
    }

    Gtk::HBox* hbox;
    get_widget("JIDInfo_hbox", hbox);
    PrettyJID* pj = Gtk::manage(
        new PrettyJID(_jid, "", PrettyJID::dtNickJID));
    pj->show();
    hbox->pack_end(*pj);
    
    get_widget("Filename_lbl", lbl);
    lbl->set_text(Glib::path_get_basename(_info.suggested_name));

    get_widget("Location_lbl", lbl);
    lbl->set_text(_filename);

    get_widget("Size_lbl", lbl);
    std::string file_size = printBytes(_info.size);
    lbl->set_text(file_size);

    get_widget("SizeRemaining_lbl", _size_remaining_lbl);
    _size_remaining_lbl->set_text(file_size);
    
    get_widget("TransferSpeed_lbl", _transfer_speed_lbl);
    get_widget("Status_lbl", _status_lbl);
    get_widget("Progress_pbar", _progress_pbar);
    get_widget("Expand_arw", _expand_arw);
    get_widget("Expand_lbl", _expand_lbl);
    get_widget("Info_tbl", _info_tbl);
    _info_is_hidden = true;
    Gtk::EventBox* evb;
    get_widget("Expand_evb", evb);
    evb->signal_button_release_event().connect(
        SigC::slot(*this, &FileTransferDlg::on_expand_clicked));
    get_widget("Stop_btn", _stop_btn);
    _stop_btn->signal_clicked().connect(
        SigC::slot(*this, &FileTransferDlg::on_stop_clicked));
}
    
void FileTransferDlg::transfer_update(int sz)
{
    // Update the size and labels representing it
    _total += sz;
    //printf("Updated: %d total: %ld size: %ld percent: %f\n", sz, _total, _info.size, static_cast<float>(_total) / static_cast<float>(_info.size));
    _progress_pbar->set_fraction(static_cast<float>(_total) / static_cast<float>(_info.size));

    // See if we're done
    if (_total == _info.size)
    {
        _status_lbl->set_text(_("Complete."));
        return;
    } 
    else if (_total > _info.size) 
    {
        on_error(_("Error, more data was transferred than expected."));
        return;
    }

    _size_remaining_lbl->set_text(printBytes(_info.size - _total));

    // How long have we been working on this?
    if (_start_time == 0)
    {
        Gtk::HBox* hbox;
        get_widget("Progress_hbox", hbox);
        hbox->show();
        get_widget("Expand_hbox", hbox);
        hbox->show();
        time(&_start_time);
    }

    time_t now, elapsed;
    time(&now);
    elapsed = now - _start_time;

    // Damn you divide by zero
    if (elapsed <= 0)
    {
        return;
    }

    float speed = (float)_total / (float)elapsed;
    float time_left = (float)(_info.size - _total)/ speed;

    //This is an icky hack to redraw better
    _transfer_speed_lbl->set_text("");
    _transfer_speed_lbl->set_text(printBytes(speed) + "/s");

    // How much longer is left
    std::string status;
    if (time_left < 60) 
    {
        status = _("Under one minute remaining.");
    }
    else if (time_left > 3600)
    {
        float hours = (time_left / 3600);
        time_left -= (hours * 3600);
        status = Util::substitute(_("%s hours, %s minutes remaining."), float2string(hours), float2string(time_left / 60.0));
    }
    else
    {
        status = Util::substitute(_("%s minutes remaining."), float2string(time_left / 60.0));
    }
    _status_lbl->set_text("");
    _status_lbl->set_text(status);
}

void FileTransferDlg::transfer_closed()
{
    if (!_running || _total == _info.size)
    {
        close();
        return;
    }

    if (!_has_error)
        _status_lbl->set_text(_("Transfer Stopped"));
    _stop_btn->set_label(_("Close"));
    _running = false;
}

void FileTransferDlg::transfer_error(const std::string& msg)
{
    _has_error = true;
    on_error(msg);
}

void FileTransferDlg::on_stop_clicked()
{
    if (!_running)
    {
        close();
        return;
    }

    _running = false;
    _status_lbl->set_text(_("Disconnecting..."));
    G_App.getFileTransferManager().stop(_sid);
}

bool FileTransferDlg::on_expand_clicked(GdkEventButton* event)
{
    if (_info_is_hidden)
    {
        _info_tbl->show();
        _expand_lbl->set_text(_("Hide details"));
        _expand_arw->set(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT);
        _info_is_hidden = false;
    }
    else
    {
        _info_tbl->hide();
        _expand_lbl->set_text(_("Show details"));
        _expand_arw->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_OUT);
        _info_is_hidden = true;
    }

    return true;
}

void FileTransferDlg::on_error(const std::string& msg)
{
    _stop_btn->set_label(_("Close"));
    _status_lbl->set_text(msg);
}

bool FileTransferDlg::on_timeout()
{
    if (_start_time == 0)
    {
        on_error("Connection timed out.");
    }

    return false;
}

