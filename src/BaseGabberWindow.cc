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

#include "BaseGabberWindow.hh"
#include "Environment.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"
#include <gtkmm/window.h>

namespace Gabber {

// ---------------------------------------------------------
//
// Base Gabber Window
//
// ---------------------------------------------------------
Glib::RefPtr<Gnome::Glade::Xml> BaseGabberWindow::getGladeWidget(const Glib::ustring& widgetname)
{
    Glib::ustring filename = Glib::build_filename(
        G_App.getConfigurator().get_string(Keys::paths.datadir), 
        widgetname + ".glade");
    return Gnome::Glade::Xml::create(filename, widgetname);
}

BaseGabberWindow::BaseGabberWindow(const Glib::ustring& widgetname)
     : _thisWindow(0), _thisXml(getGladeWidget(widgetname))
{ 
     // Get the Gtk::Window
     _thisXml->get_widget(widgetname, _thisWindow);

     // ref this Object
     reference();
          
     _thisWindow->signal_delete_event().connect(SigC::slot(*this, &BaseGabberWindow::on_delete_event));
}

void BaseGabberWindow::close()
{
     // unref this Object
     unreference();
}

BaseGabberWindow::~BaseGabberWindow()
{ 
     // destroy the Gtk::Window
     delete _thisWindow;
}

void BaseGabberWindow::show() 
{ _thisWindow->show(); }

void BaseGabberWindow::hide() 
{ _thisWindow->hide(); }

Gtk::Window* BaseGabberWindow::getGtkWindow() const 
{ return _thisWindow; }

void BaseGabberWindow::raise()
{ _thisWindow->raise(); }

bool BaseGabberWindow::on_delete_event(GdkEventAny* e)
{
     close();
     
     return false;
}

} // namespace Gabber
