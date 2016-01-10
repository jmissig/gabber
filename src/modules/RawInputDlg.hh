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
 *  Copyright (c) 2004 Julian Missig
 */
 
 
#ifndef INCL_RAWINPUTDLG_HH
#define INCL_RAWINPUTDLG_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"

namespace Gabber {
/**
 * Raw Input Dialog.
 */
class RawInputDlg : public BaseGabberWindow
{
public:
     /**
      * Display a RawInputDlg to the given JabberConnection.
      * @param conn The JabberConnection we're using.
      */
     static void display(JabberConnection& conn);

protected:
     RawInputDlg(JabberConnection& conn);
     //~RawInputDlg();
     void on_Send_clicked();
     bool on_window_event(GdkEvent* ev);

private:
     JabberConnection& _conn;
     Gtk::TextView*    _txtMessage;
};

} // namespace Gabber

#endif // INCL_RAWINPUTDLG_HH
