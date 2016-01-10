/*  $Id: GabberDialog.cc,v 1.4 2004/06/17 05:29:04 temas Exp $
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

#include "GabberDialog.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"
#include <gtkmm/dialog.h>
#include <glibmm/miscutils.h>

namespace Gabber {

// ---------------------------------------------------------
//
// Gabber Dialog
//
// ---------------------------------------------------------
GabberDialog::GabberDialog(const Glib::ustring& widgetname,
                           const Glib::ustring& windowname,
                           Gtk::Window* parent)
     : _thisDialog(0), 
       _thisXml(Gnome::Glade::Xml::create(
           Glib::build_filename(
               G_App.getConfigurator().get_string(Keys::paths.datadir),
               windowname + ".glade"), widgetname))
{
     // Get the Gtk::Dialog
     _thisXml->get_widget(widgetname, _thisDialog);
     
     if (parent)
          _thisDialog->set_transient_for(*parent);
}

Gtk::Dialog* GabberDialog::getDialog() const
{
     return _thisDialog;
}

void GabberDialog::show()
{
     _thisDialog->show();
}

int GabberDialog::run()
{
     return _thisDialog->run();
}

void GabberDialog::hide()
{
     _thisDialog->hide();
}

void GabberDialog::close()
{
    delete this;
}

} // namespace Gabber

