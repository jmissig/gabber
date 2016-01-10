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

#ifndef INCL_GABBER_DIALOG_HH
#define INCL_GABBER_DIALOG_HH

#include <sigc++/object.h>
#include "fwd.h"
#include <libglademm/xml.h>

namespace Gabber
{

/**
 * Class for simple Glade-based dialogs.
 * A typical run for this dialog involves constructing it,
 * then calling show(), then run(), then hide(), then close().
 * show() and hide() are optional, depending on what you want to do.
 */
class GabberDialog
     : public SigC::Object
{
public:
     /**
      * Construct a simple dialog.
      * This class makes it easy to deal with dialogs in glade.
      * @param widgetname The name of the dialog.
      * @param windowname The name of the glade file the dialog is in.
      * @param parent The parent window of this dialog. NULL if no parent.
      */
     GabberDialog(const Glib::ustring& widgetname,
                  const Glib::ustring& windowname,
                  Gtk::Window* parent);
      
     /**
      * Accessor for the Gtk::Dialog.
      */
     Gtk::Dialog* getDialog() const;
     
     /**
      * Convenience method for showing the dialog.
      */
     void show();
     
     /**
      * Run the dialog.
      * @return int value for the Gtk Response.
      */
     int run();
     
     /**
      * Convenience method for hiding the dialog.
      */
     void hide();
     
     /**
      * Convenience method for getting a widget.
      */
     template<class T_Widget> void get_widget(const Glib::ustring & name, T_Widget *& widget )
	  { _thisXml->get_widget(name, widget); }
       
     /**
      * Convenience method for getting a widget.
      */
     Gtk::Widget* get_widget(const Glib::ustring & name)
	  { return _thisXml->get_widget(name); }
     
     /**
      * Close the dialog
      */
     void close();

protected:
     GabberDialog() : _thisDialog(NULL) { }
     /**
      * Pointer to the actual dialog widget.
      */
     Gtk::Dialog* _thisDialog;

private:
     Glib::RefPtr<Gnome::Glade::Xml> _thisXml;
     
}; // class GabberDialog

}; // namespace Gabber

#endif // INCL_GABBER_DIALOG_HH
