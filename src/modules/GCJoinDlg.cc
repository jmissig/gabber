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

#include "GCJoinDlg.hh"
#include "GCViewManager.hh"

#include "GabberApp.hh"
#include "ConfigPaths.hh"

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/combo.h>

#include <sigc++/slot.h>

using namespace Gabber;

void GCJoinDlg::display(GCViewManager& mgr)
{
     GCJoinDlg* e = new GCJoinDlg(mgr);
     e->show();
}

GCJoinDlg::GCJoinDlg(GCViewManager& mgr) : 
    BaseGabberWindow("GCJoin_dlg"), _mgr(mgr)
{ 
    Gtk::Dialog* dlg = static_cast<Gtk::Dialog*>(getGtkWindow());
    dlg->signal_response().connect(SigC::slot(*this, &GCJoinDlg::on_response));

    get_widget("Nickname_cbo", _nickname_cbo);
    _nickname_cbo->get_entry()->signal_changed().connect(SigC::slot(*this,
        &GCJoinDlg::on_entry_changed));
    get_widget("Room_cbo", _room_cbo);
    _room_cbo->get_entry()->signal_changed().connect(SigC::slot(*this,
        &GCJoinDlg::on_entry_changed));

    get_widget("JoinRoom_btn", _join_btn);
    _join_btn->set_sensitive(false);

    loadconfig();
    show();
}

GCJoinDlg::~GCJoinDlg()
{ }

void GCJoinDlg::loadconfig()
{
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     _nickname_cbo->get_entry()->set_text(config.get_string(Keys::groupchat.nickname));
}

void GCJoinDlg::saveconfig()
{
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     config.set(Keys::groupchat.nickname, _nickname_cbo->get_entry()->get_text());
}

void GCJoinDlg::on_response(int resp)
{
    hide();

    // This is the join case
    if (resp == Gtk::RESPONSE_OK)
    {
	 Glib::ustring jid = _room_cbo->get_entry()->get_text();
	 Glib::ustring nickname = _nickname_cbo->get_entry()->get_text();
	 _mgr.join_groupchat(jid, nickname);
	 saveconfig();
    }

    close();
}

void GCJoinDlg::on_entry_changed()
{
    if (_nickname_cbo->get_entry()->get_text_length() > 0 &&
        _room_cbo->get_entry()->get_text_length() > 0)
    {
        _join_btn->set_sensitive(true);
    }
    else
    {
        _join_btn->set_sensitive(false);
    }
}
