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

#include "RawViewManager.hh"

#include "GabberApp.hh"
#include "Menus.hh"
#include "RawInputDlg.hh"

#include <sigc++/slot.h>
#include "intl.h"

namespace Gabber
{
     
static RawViewManager* rvm_instance;
     
extern "C" const gchar* g_module_check_init(GModule* module)
{
     rvm_instance = new RawViewManager();
     return NULL;
}
     
extern "C" void g_module_unload(GModule* module)
{
     delete rvm_instance;
}
     
RawViewManager::RawViewManager() :
     _action_menu_item(_("_Raw XML Input..."), true)
{
     _action_menu_item.signal_activate().connect(SigC::slot(*this,
          &RawViewManager::on_action_menu_item_activate));
     Popups::ActionMenu::getSingleton().addItem(&_action_menu_item);
}

void RawViewManager::on_action_menu_item_activate()
{
     RawInputDlg::display(G_App);
}

};
