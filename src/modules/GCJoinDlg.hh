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

#ifndef INCL_GCJOINDLG_HH
#define INCL_GCJOINDLG_HH

#include "BaseGabberWindow.hh"

namespace Gabber {

class GCJoinDlg : public BaseGabberWindow
{
public:
     static void display(GCViewManager& mgr);
    GCJoinDlg(GCViewManager& mgr);
    ~GCJoinDlg();

protected:
    void loadconfig();
    void saveconfig();
    void on_response(int code);
    void on_entry_changed();

private:
    GCViewManager&      _mgr;
    Gtk::Combo*         _nickname_cbo;
    Gtk::Combo*         _room_cbo;
    Gtk::Button*        _join_btn;
}; // class GCJoinDlg

}; // namespace Gabber

#endif // INCL_GCJOINDLG_HH
