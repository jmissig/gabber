// mmfwd.h
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//  Gabber 2
//  Based on Gabber, Copyright (c) 1999-2002 Dave Smith & Julian Missig
//  Copyright (c) 2002-2003 Julian Missig
// =========================================================================== 

#ifndef FWD_H
#define FWD_H

#include <iosfwd>
#include <jabberoo/jabberoofwd.h>

namespace Gtk {
    class Window;
    class OptionMenu;
    class ScrolledWindow;
    class TextView;
    class Label;
    class Dialog;
    class ProgressBar;
    class ToggleButton;
    class Widget;
    class HBox;
    class Combo;
    class Entry;
    class Image;
    class Tooltips;
    class Button;
    class ListStore;
    class Module;
    class CheckButton;
    class RadioButton;
    class SpinButton;
    class TreeModel;
    class TreeSelection;
    class TreeView;
    class TreeViewColumn;
    class TextMark;
    class TextBuffer;
    class MenuItem;
    class Frame;
    class EventBox;
    class Table;
}

namespace Gdk {
    class Pixbuf;
}
namespace Glib {
     template <typename> class RefPtr;
     class ustring;
}

namespace Gabber {
    class ChatViewManager;
    class FileTransferManager;
    class GabberApp;
    class GabberWin;
    class GCViewManager;
    class JabberConnection;
    class LogManager;
    class PrettyJID;
    class PrettyText;
    class ResourceManager;
    class RosterModel;
    class RosterModel_Class;
    class RosterView;
    class PacketQueueView;
}


#endif // #define FWD_H
