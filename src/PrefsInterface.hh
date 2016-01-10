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

#ifndef INCL_PREFS_INTERFACE_HH
#define INCL_PREFS_INTERFACE_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"
#include "ConfigPaths.hh"

namespace Gabber
{

/**
 * Displays the login information
 */
class LoginDlg : 
     public BaseGabberWindow
{
public:
     /**
      * Executes the LoginDlg.
      * There can be only one at a time, and this ensures that.
      */
     static void execute();
     /**
      * Execute Connection Settings Dialog.
      * The LoginDlg cannot log in this way.
      */
     static void execute_as_settings();
     ~LoginDlg();
protected:
     /**
      * Constructor is internalized.
      * @see LoginDlg::execute()
      */
     LoginDlg(bool is_settings);
     void loadconfig();
     void saveconfig();
     // Event handlers
     void on_LogIn_clicked();
     void on_Cancel_clicked();
     void on_LogOut_clicked();
     void on_Close_clicked();
     void on_changed();
private:
     const bool _is_settings;
     Gtk::Button* _btnLogIn;
     Gtk::Entry*  _entUsername;
     Gtk::Entry*  _entServer;
     Gtk::Entry*  _entPassword;
     Gtk::Entry*  _entResource;
     Gtk::CheckButton*  _ckPassword;
     Gtk::CheckButton*  _ckUseSSL;
     Gtk::CheckButton*  _ckAutoConnect;
     Gtk::CheckButton*  _ckReconnect;
     Gtk::SpinButton* _spinPort;
     Gtk::SpinButton* _spinPriority;

     static LoginDlg* _Dialog;
};


/**
 * Gabber Preferences Window
 */
class PrefsWin :
     public BaseGabberWindow
{
public:
     /**
      * Executes the Preferences Window.
      * There can be only one at a time, and this ensures that.
      */
      static void execute();
      ~PrefsWin();
protected:
     /**
      * Constructor is internalized.
      * @see PrefsWin::execute()
      */
      PrefsWin();
      void close();
      void loadconfig();
      void saveconfig();
      // Event handlers
      void on_AutoAway_toggled();
private:

     static PrefsWin* _Dialog;
};

}; // namespace Gabber

#endif // INCL_PREFS_INTERFACE_HH
