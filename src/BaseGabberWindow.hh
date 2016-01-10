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

#ifndef INCL_BASE_GABBER_WINDOW_H
#define INCL_BASE_GABBER_WINDOW_H

#include "fwd.h"
#include <glibmm/object.h>
#include <libglademm/xml.h>

namespace Gabber
{

/**
 * Provides base window loading and functionality
 */
class BaseGabberWindow
     : public Glib::Object
{
public:
     /**
      * Construct a Gabber window based on widgetname.
      * It is assumed that there exists a widgetname.glade
      * with the window widget named widgetname.
      */
     BaseGabberWindow(const Glib::ustring& widgetname);
     virtual ~BaseGabberWindow();
     /**
      * Convenience method for showing the window.
      */
     void show();
     /**
      * Convenience method for hiding the window.
      */
     void hide();
     /**
      * Accessor for the Gtk::Window.
      */
     Gtk::Window* getGtkWindow() const;
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
      * Close the window
      * Override this to change any behavior when the window closes.
      */
     virtual void close(); 

     /**
      * Raise the window
      */
      void raise();

      static Glib::RefPtr<Gnome::Glade::Xml> getGladeWidget(const Glib::ustring& widgetname);
protected:
     // Event handlers
     virtual bool on_delete_event(GdkEventAny* e);
protected:
     BaseGabberWindow() : _thisWindow(NULL) { }
     /**
      * Pointer to the actual window widget.
      */
     Gtk::Window* _thisWindow;
private:
     Glib::RefPtr<Gnome::Glade::Xml> _thisXml;
};

}; // namespace Gabber

#endif // INCL_BASE_GABBER_WINDOW_HH
