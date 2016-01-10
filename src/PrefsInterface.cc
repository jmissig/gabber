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

#include "PrefsInterface.hh"

#include "GabberApp.hh"

#include "intl.h"

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>

namespace Gabber {

// ---------------------------------------------------------
//
// Login Dialog
//
// ---------------------------------------------------------

LoginDlg* LoginDlg::_Dialog = NULL;

void LoginDlg::execute()
{
     if (_Dialog == NULL)
     {
          // Create a login dialog
          _Dialog = new LoginDlg(false);
     }
}

void LoginDlg::execute_as_settings()
{
     if (_Dialog == NULL)
     {
          // Create a Connection Settings dialog
          _Dialog = new LoginDlg(true);
     }
}

LoginDlg::LoginDlg(bool is_settings)
     : BaseGabberWindow("Login_dlg"), _is_settings(is_settings)
{
     // Connect buttons to handlers
     Gtk::Button* b = 0;
     if (is_settings)
     {
	  _thisWindow->set_title(_("Gabber Connection Settings"));
          get_widget("Close_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_Close_clicked));
          b->show();
          get_widget("LogOut_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_LogOut_clicked));
          b->show();
          get_widget("LogIn_btn", _btnLogIn);
          _btnLogIn->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_LogIn_clicked));
          _btnLogIn->hide();
          get_widget("Cancel_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_Cancel_clicked));
          b->hide();
     }
     else
     {
          get_widget("Close_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_Close_clicked));
          b->hide();
          get_widget("LogOut_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_LogOut_clicked));
          b->hide();
          get_widget("LogIn_btn", _btnLogIn);
          _btnLogIn->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_LogIn_clicked));
          _btnLogIn->show();
          get_widget("Cancel_btn", b);
          b->signal_clicked().connect(SigC::slot(*this, &LoginDlg::on_Cancel_clicked));
          b->show();
     }
     
     // Initialize pointers
     get_widget("Username_ent", _entUsername);
     _entUsername ->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Server_ent", _entServer);
     _entServer   ->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Password_ent", _entPassword);
     _entPassword ->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Resource_ent", _entResource);
     _entResource ->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Port_spin", _spinPort);
     _spinPort    ->set_width_chars(4);
     _spinPort    ->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Priority_spin", _spinPriority);
     _spinPriority->signal_changed().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("SavePassword_chk", _ckPassword);
     _ckPassword->signal_toggled().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("SSL_chk", _ckUseSSL);
#ifndef WITH_SSL
    _ckUseSSL->set_active(false);
    _ckUseSSL->hide();
#else
    _ckUseSSL->show();
     _ckUseSSL->signal_toggled().connect(SigC::slot(*this, &LoginDlg::on_changed));
#endif
     get_widget("Autoconnect_chk", _ckAutoConnect);
     _ckAutoConnect->signal_toggled().connect(SigC::slot(*this, &LoginDlg::on_changed));
     get_widget("Reconnect_chk", _ckReconnect);
     _ckReconnect->signal_toggled().connect(SigC::slot(*this, &LoginDlg::on_changed));

     // Load configuration
     loadconfig();
    
     // display
     show();
}

LoginDlg::~LoginDlg()
{
     // Initialize the main window
     G_App.init_win();
     
     _Dialog = NULL;
}

void LoginDlg::loadconfig()
{
    Configurator& config = GabberApp::getSingleton().getConfigurator();
    _entUsername-> set_text  (config.get_string(Keys::acct.username));
    _entServer->   set_text  (config.get_string(Keys::acct.server));
    _entResource-> set_text  (config.get_string(Keys::acct.resource));
    _entPassword-> set_text  (config.get_string(Keys::acct.password));
    _spinPort->    set_value (config.get_int   (Keys::acct.port));
    _spinPriority->set_value (config.get_int   (Keys::acct.priority));
    _ckPassword->  set_active(config.get_bool  (Keys::acct.savepassword));
    _ckUseSSL->set_active(config.get_bool(Keys::acct.ssl));
    _ckReconnect->set_active(config.get_bool(Keys::acct.reconnect));
    _ckAutoConnect->set_active(config.get_bool(Keys::acct.autoconnect));
}

void LoginDlg::saveconfig()
{
    Configurator& config = GabberApp::getSingleton().getConfigurator();
    config.set(Keys::acct.username    ,_entUsername-> get_text());
    config.set(Keys::acct.server      ,_entServer->   get_text());
    config.set(Keys::acct.resource    ,_entResource-> get_text());
    if (_ckPassword->get_active())
        config.set(Keys::acct.password    ,_entPassword-> get_text());
    else
        config.set(Keys::acct.password, Glib::ustring());
    config.set(Keys::acct.port        ,_spinPort->    get_value_as_int());
    config.set(Keys::acct.priority    ,_spinPriority->get_value_as_int());
    config.set(Keys::acct.savepassword,_ckPassword->  get_active());
    config.set(Keys::acct.ssl, _ckUseSSL->get_active());
    config.set(Keys::acct.reconnect, _ckReconnect->get_active());
    config.set(Keys::acct.autoconnect, _ckAutoConnect->get_active());
}

void LoginDlg::on_LogIn_clicked()
{
     // Save configuration
     saveconfig();

     G_App.set_password(_entPassword->get_text());

     // Initialize the main window
     G_App.init_win();

     // Start the login process
     G_App.connect();

     // And now close it all up, we're done
     close();
}

void LoginDlg::on_Cancel_clicked()
{
     close();
}

void LoginDlg::on_LogOut_clicked()
{
     G_App.clean_disconnect();
     
     Gtk::Button* b;
     get_widget("Close_btn", b);
     b->hide();
     get_widget("LogOut_btn", b);
     b->hide();
     get_widget("LogIn_btn", _btnLogIn);
     _btnLogIn->show();
     get_widget("Cancel_btn", b);
     b->show();
}

void LoginDlg::on_Close_clicked()
{
     saveconfig();
     close();
}

void LoginDlg::on_changed()
{
     // If they pass our test, let them click Log In
     if (jabberoo::JID::isValidUser(_entUsername->get_text()) &&
	 jabberoo::JID::isValidHost(_entServer->get_text()) &&
	 !_entPassword->get_text().empty() &&
	 !_entResource->get_text().empty())
     {
	  _btnLogIn->set_sensitive(true);
     }
     else
     {
	  _btnLogIn->set_sensitive(false);
     }
}

// gint LoginDlg::on_Username_key_press(GdkEventKey* e)
// {
//      if (e->keyval == GDK_at)
//      {
// 	  // HACK --  to stop the signal from continuing, and the @ from being put into the Username field
// 	  gtk_signal_emit_stop_by_name(GTK_OBJECT(_entUsername->gtkobj()), "key_press_event");
// 	  _entServer->grab_focus();
//      }
//      return 0;
// }


// ---------------------------------------------------------
//
// Preferences Window
//
// ---------------------------------------------------------
PrefsWin* PrefsWin::_Dialog = NULL;

void PrefsWin::execute()
{
     // Create the preferences window
     _Dialog = new PrefsWin();
}

PrefsWin::PrefsWin()
     : BaseGabberWindow("Prefs_win")
{
     Gtk::Button* b;
     get_widget("Close_btn", b);
     b->signal_clicked().connect(SigC::slot(*this, &PrefsWin::close));

     Gtk::CheckButton* cb;
     get_widget("Autoaway_Enable_chk", cb);
     cb->signal_toggled().connect(SigC::slot(*this, &PrefsWin::on_AutoAway_toggled));
     
     loadconfig();
}

PrefsWin::~PrefsWin()
{
}

void PrefsWin::close()
{
     saveconfig();

     BaseGabberWindow::close();
}

void PrefsWin::loadconfig()
{
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     Gtk::CheckButton* cb;
     
     get_widget("Messages_AutoDisplay_chk", cb);
     cb->set_active(config.get_bool(Keys::intrface.messages_autodisplay));

     get_widget("Messages_SpellCheck_chk", cb);
     cb->set_active(config.get_bool(Keys::intrface.messages_spellcheck));
#ifndef GTKSPELL
     cb->hide();
#endif // GTKSPELL
     
     get_widget("Autoaway_Enable_chk", cb);
     cb->set_active(config.get_bool(Keys::autoaway.enable));
     
     Gtk::SpinButton* sb;
     get_widget("Autoaway_AwayAfter_spin", sb);
     sb->set_value(config.get_int(Keys::autoaway.away_after));
     get_widget("Autoaway_XAAfter_spin", sb);
     sb->set_value(config.get_int(Keys::autoaway.xa_after));
     
     Gtk::TextView* tv;
     get_widget("Autoaway_Status_txtview", tv);
     tv->get_buffer()->set_text(config.get_string(Keys::autoaway.awaymessage));
     
     get_widget("Autoaway_ZeroPriority_chk", cb);
     cb->set_active(config.get_bool(Keys::autoaway.zero_priority));

     get_widget("Autoaway_IdleVisible_chk", cb);
     cb->set_active(config.get_bool(Keys::autoaway.idletime_visible));
}

void PrefsWin::saveconfig()
{
     Configurator& config = GabberApp::getSingleton().getConfigurator();
     Gtk::CheckButton* cb;
     get_widget("Messages_AutoDisplay_chk", cb);
     config.set(Keys::intrface.messages_autodisplay, cb->get_active());
     
     get_widget("Messages_SpellCheck_chk", cb);
     config.set(Keys::intrface.messages_spellcheck, cb->get_active());
     
     get_widget("Autoaway_Enable_chk", cb);
     config.set(Keys::autoaway.enable, cb->get_active());
     
     get_widget("Autoaway_Enable_chk", cb);
     config.set(Keys::autoaway.enable, cb->get_active());

     Gtk::SpinButton* sb;
     get_widget("Autoaway_AwayAfter_spin", sb);
     config.set(Keys::autoaway.away_after, sb->get_value_as_int());
     get_widget("Autoaway_XAAfter_spin", sb);
     config.set(Keys::autoaway.xa_after, sb->get_value_as_int());

     Gtk::TextView* tv;
     get_widget("Autoaway_Status_txtview", tv);
     config.set(Keys::autoaway.awaymessage, tv->get_buffer()->get_text());

     get_widget("Autoaway_ZeroPriority_chk", cb);
     config.set(Keys::autoaway.zero_priority, cb->get_active());

     get_widget("Autoaway_IdleVisible_chk", cb);
     config.set(Keys::autoaway.idletime_visible, cb->get_active());
}

void PrefsWin::on_AutoAway_toggled()
{
     Gtk::CheckButton* cb;
     Gtk::Frame* f;
     get_widget("Autoaway_Enable_chk", cb);
     if (cb->get_active())
     {
          get_widget("Autoaway_PresChange_frm", f);
          f->set_sensitive(true);
          get_widget("Autoaway_PresStatus_frm", f);
          f->set_sensitive(true);
     }
     else
     {
          get_widget("Autoaway_PresChange_frm", f);
          f->set_sensitive(false);
          get_widget("Autoaway_PresStatus_frm", f);
          f->set_sensitive(false);
     }
}

} // namespace Gabber
