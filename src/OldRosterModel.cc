#include "OldRosterModel.hh"
#include "ResourceManager.hh"
#include "GabberApp.hh"

#include <session.hh>

#include <gtkmm/treepath.h>
#include <glibmm/class.h>

#include <algorithm>
#include <fstream>

using namespace Gtk;
using namespace jabberoo;


/**
*  For those who dare troll in here.  Here is the layout of a GtkTreeIter:
*  iter->user_data:
*       The current value depending on the path depth.
*  iter->user_data2:
*       The parent to the current value.
*  iter->user_data3:
*       The first child of the current value.
*/

namespace Gabber {

#ifdef DEBUG_TREEMODEL
static void dump_iter(const GtkTreeIter* iter, const char* label = NULL)
{
    if (!iter)
    {
        printf("Iterator:  NULL\n");
        return;
    }

    printf("Iterator:");
    if (label)
        printf(" %s", label);
    printf("\n");
    printf("\tstamp:\t%d\n\tuser1:\t%p\n\tuser2:\t%p\n\tuser3:\t%p\n\n",
        iter->stamp, iter->user_data, iter->user_data2, iter->user_data3);
}
#else
#define dump_iter(__args__)
#endif

RosterModel_Class RosterModel::rostermodel_class_;

RosterModel::RosterModel(Session& sess, bool hide_offline, 
                         bool sort_with_groups):
    Glib::ObjectBase("Gabber_RosterModel"), 
    Glib::Object(Glib::ConstructParams(rostermodel_class_.init(), (char*) 0)),
    TreeModel(), 
    _sess(sess), _roster(_sess.roster()), 
    _hide_offline(hide_offline), _sort_with_groups(sort_with_groups),
    _has_first_refresh(false), _update_running(false)
{
    _roster_xpath = _sess.registerXPath(
        "/iq[@type='set']/query[@xmlns='jabber:iq:roster']/item", 
        slot(*this, &RosterModel::on_roster_set));
    G_App.evtConnected.connect(slot(*this, &RosterModel::on_roster_refresh));
    _roster.evtPresence.connect(slot(*this, &RosterModel::on_roster_pres));
    _roster.evtRemovingItem.connect(slot(*this, &RosterModel::on_removing_item));
    _roster.evtUpdating.connect(slot(*this, &RosterModel::on_updating_item));
    _roster.evtUpdateDone.connect(slot(*this, &RosterModel::on_update_done));
    G_App.getPacketQueue().packet_queued_event.connect(
        slot(*this, &RosterModel::on_packet_queued));
    G_App.getPacketQueue().queue_emptied_event.connect(
        slot(*this, &RosterModel::on_queue_emptied));
    G_App.getPacketQueue().queue_changed_event.connect(
        slot(*this, &RosterModel::on_queue_changed));
}

RosterModel::~RosterModel()
{ 
    // XXX Anything to do in here?
}

void RosterModel::setSortType(SortType type)
{
}

// Inheritted from Gtk::TreeModel
void RosterModel::set_value_impl(const iterator& row, int column, const Glib::ValueBase& value)
{
}

void RosterModel::get_value_impl(const iterator& row, int column, Glib::ValueBase& value) const
{
    const GtkTreeIter* it = row.get_gobject_if_not_end();
    //dump_iter(it);

    if (!it || it->user_data == NULL)
    {
        // XXX return better?
        return;
    }
    
    gpointer icon = NULL;

    switch(column)
    {
    // JID
    case 0:
        value.init(G_TYPE_STRING);
        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            g_value_set_string(value.gobj(), item->getJID().c_str());
        }
        else
        {
            g_value_set_string(value.gobj(), NULL);
        }
        break;
    // Nick
    case 1:
        value.init(G_TYPE_STRING);
        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            g_value_set_string(value.gobj(), item->getNickname().c_str());
        }
        else
        {
            ModelItems::value_type* val = 
                static_cast<ModelItems::value_type*>(it->user_data);
            Glib::ustring name = "<markup><b>" + val->first + "</b></markup>";
            g_value_set_string(value.gobj(), name.c_str());
        }
        break;
    // Icon
    case 2:
        value.init(G_TYPE_OBJECT);
        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            RosterItemInfos::const_iterator it = _item_info.find(item);
            if (it != _item_info.end() && (!it->second.icon.empty()))
            {
                icon = ResourceManager::getSingleton().getPixbuf(it->second.icon)->gobj();
            }
        }
        g_value_set_object(value.gobj(), icon);
        break;
    // Use_Icon (bool)
    case 3:
        value.init(G_TYPE_BOOLEAN);
        if (!_sort_with_groups || it->user_data2)
        {
            g_value_set_boolean(value.gobj(), TRUE);
        }
        else
        {
            g_value_set_boolean(value.gobj(), FALSE);
        }
        break;
    case 4:
        value.init(G_TYPE_BOOLEAN);
        if (!_sort_with_groups || it->user_data2)
        {
            g_value_set_boolean(value.gobj(), FALSE);
        }
        else
        {
            g_value_set_boolean(value.gobj(), TRUE);
        }
        break;
    case 5:
        value.init(GDK_TYPE_COLOR);
        if(!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            RosterItemInfos::const_iterator it = _item_info.find(item);
            if (it != _item_info.end() && it->second.fade_percent < 65535)
            {
                int fade(it->second.fade_percent);
                bool is_selected = (item->getJID() == _selected_jid);
                const Gdk::Color& fg_col(is_selected ? _default_colors.sel_fg : _default_colors.fg);
                const Gdk::Color& bg_col(is_selected ? _default_colors.sel_bg : _default_colors.bg);
                Gdk::Color color;
                int red = (fade * (fg_col.get_red() - bg_col.get_red())) / 65535 + bg_col.get_red();
                color.set_red(red);
                int green = (fade * (fg_col.get_green() - bg_col.get_green())) / 65535 + bg_col.get_green();
                color.set_green(green);
                int blue = (fade * (fg_col.get_blue() - bg_col.get_blue())) / 65535 + bg_col.get_blue();
                color.set_blue(blue); 

                g_value_set_boxed(value.gobj(), color.gobj());
            }
            else
                g_value_set_boxed(value.gobj(), NULL);

        }
        break;
    };

}

TreeModelFlags RosterModel::get_flags_vfunc()
{ 
    // XXX No flags for now
    return TreeModelFlags();
}

int RosterModel::get_n_columns_vfunc()
{ 
    // XXX how many columns do we have?
    return 6;
}

GType RosterModel::get_column_type_vfunc(int index)
{ 
    // XXX Just a string JID for now
    return G_TYPE_STRING;
}

bool RosterModel::iter_next_vfunc(GtkTreeIter* iter)
{
    //dump_iter(iter, "Getting next for");

    // If this hits we're not going to be able to get a next iter
    if (iter->user_data == NULL)
    {
        clearIter(iter);
        return false;
    }

    iter->stamp = 51;
    RosterItems* ris;
    if (_sort_with_groups)
    {

        // Short circuit the only group case
        if (iter->user_data2 == NULL)
        {
            ModelItems::value_type* val = 
                static_cast<ModelItems::value_type*>(iter->user_data);

            //printf("Getting roster items from key: %s\n", val->first.c_str());
            ModelItems::iterator it = _items.find(val->first);
            if ( (it == _items.end()) || ((++it) == _items.end()) )
            {
                printf("No next, clearing.\n");
                clearIter(iter);
                return false;
            }

            iter->user_data = &*it;
            iter->user_data2 = NULL;
            iter->user_data3 = *(it->second.begin());
            //printf ("Returning next group: %s\n", it->first.c_str());
            //dump_iter(iter, "Group Next Return Value");
            return true;
        }
        else
        {
            ModelItems::value_type* val = 
                static_cast<ModelItems::value_type*>(iter->user_data2);
            //printf("Val pointer: %p\n", val);
            //printf("Getting roster items from key2: %s\n", val->first.c_str());
            ris = &(val->second);
        }
    }
    else
    {
        ris = &(_items[""]);
    }

    Roster::Item* item = static_cast<Roster::Item*>(iter->user_data);
    RosterItems::iterator it = ris->find(item);

    if ( it == ris->end() || (*it) != item || (++it) == ris->end() )
    {
        clearIter(iter);
        //printf("Returning no next for %s.\n", item->getJID().c_str());
        return false;
    }
    
    iter->user_data = (*it);
    iter->user_data3 = NULL;
    //printf("Previous was: %s\n", item->getNickname().c_str());
    //printf("Returning next: %s\n", (*it)->getNickname().c_str());
    //dump_iter(iter, "Next Return Value");

    return true;
}

bool RosterModel::iter_children_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent)
{
    //dump_iter(parent, "Parent");

    // Handle the case where we just want to reset to root
    if (parent == NULL)
    {
        iter->stamp = 40;
        iter->user_data2 = NULL;

        if (_sort_with_groups)
        {
            ModelItems::iterator it = _items.begin();
            iter->user_data = &*it;
            if(it->second.size() > 0)
            {
                iter->user_data3 = *(it->second.begin());
            }
        }
        else
        {
            iter->user_data = (*_items[""].begin());
            iter->user_data3 = NULL;
        }
        
        return true;
    }

    // Handle the normal case where have children
    if (parent->user_data3)
    {
        iter->stamp = 41; // XXX This stamp thing is odd
        iter->user_data = parent->user_data3;
        iter->user_data2 = parent->user_data;
        iter->user_data3 = NULL;

        return true;
    }

    clearIter(iter);
    return false;
}

bool RosterModel::iter_has_child_vfunc(const GtkTreeIter* iter)
{
    //dump_iter(iter);
    
    return (iter->user_data3 != NULL);
}

int RosterModel::iter_n_children_vfunc(const GtkTreeIter* iter)
{
    //dump_iter(iter);
    
    if (iter != NULL && iter->user_data3 != NULL)
    {
        ModelItems::value_type* it = 
            static_cast<ModelItems::value_type*>(iter->user_data);
        //printf("returning %s has %d children\n", it->first.c_str(), it->second.size());
        return it->second.size();
    }
    else
    {
        //printf("returning _items has %d children\n", _items.size());
        return _items.size();
    }
}

bool RosterModel::iter_nth_child_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent, int n)
{
    //dump_iter(parent, "Parent");
    
    // Handle the special root case
    if ((parent == NULL) && (static_cast<unsigned long>(n) < _items.size()))
    {
        ModelItems::iterator it = _items.begin();
        for (int j = 0; j < n; ++j)
        {
            if (++it == _items.end())
            {
                clearIter(iter);
                return false;
            }
        }

        iter->stamp = 42;
        iter->user_data = &*it;
        iter->user_data2 = NULL;
        iter->user_data3 = (*it->second.begin());
        return true;
    }

    // Handle the standard case if applicable
    if (parent->user_data3 != NULL)
    {
        ModelItems::value_type* val = 
            static_cast<ModelItems::value_type*>(parent->user_data);
        
        RosterItems::iterator it = val->second.begin();
        // XXX Something more STLish for this?
        for (int j = 0; j < n; ++j)
        {
            if (++it == val->second.end())
            {
                clearIter(iter);
                return false;
            }
        }

        if (static_cast<unsigned long>(n) < _items.size())
        {
            iter->stamp = 42;
            iter->user_data = (*it);
            iter->user_data2 = parent->user_data;
            iter->user_data3 = NULL;

            return true;
        }
    }
    
    // Everything else is invalid
    clearIter(iter);
    return false;

}

bool RosterModel::iter_parent_vfunc(GtkTreeIter* iter, const GtkTreeIter* child)
{
    //dump_iter(child, "Child");

    // return our parent if valid
    if (child && child->user_data2 != NULL)
    {
        ModelItems::value_type* val =
            static_cast<ModelItems::value_type*>(child->user_data2);
        
        iter->stamp = 43;
        iter->user_data = child->user_data2;
        iter->user_data2 = NULL;
        iter->user_data3 = *(val->second.begin());

        return true;
    }
    
    clearIter(iter);
    return false;
}

void RosterModel::ref_node_vfunc(GtkTreeIter* iter)
{
    // XXX ignored for now
}

void RosterModel::unref_node_vfunc(GtkTreeIter* iter)
{
    // XXX ignored for now
}

TreeModel::Path RosterModel::get_path_vfunc(const TreeModel::iterator& iter)
{

    const GtkTreeIter* it = iter.get_gobject_if_not_end();
    //dump_iter(it);

    if (!it || it->user_data == NULL)
    {
        return TreeModel::Path();
    }

    return getPathFromIter(it);
}

bool RosterModel::get_iter_vfunc(GtkTreeIter* iter, const TreeModel::Path& path)
{
    int pos = path.front();

    //printf("Looking up iter for path(%s) pos(%d)\n", path.to_string().c_str(), pos);
    // Bad juju
    if (_items.empty() || static_cast<unsigned long>(pos) > _items.max_size())
    {
        clearIter(iter);
        return false;
    }

    iter->stamp = 44; // XXX Make this more meaningful?

    if (path.size() == 1)
    {

        if (_sort_with_groups)
        {
            ModelItems::iterator it = _items.begin();
            for (int j = 0; j < pos; j++) it++;
            
            iter->user_data = &*it;
            iter->user_data2 = iter->user_data3 = NULL;
            if (!it->second.empty())
            {
                iter->user_data3 = (*it->second.begin());
            }
        }
        else
        {
            RosterItems::iterator it = _items[""].begin();
            for (int j = 0; j < pos; j++) it++;
            iter->user_data = *it;
            iter->user_data2 = iter->user_data3 = NULL;
        }
    }
    else
    {
        // XXX Only two deep right now
        ModelItems::iterator it = _items.begin();
        for (int j = 0; j < pos; ++j) 
        {
            ++it;
            if (it == _items.end())
            {
                return false;
            }
        }
        iter->user_data2 = &*it;
        pos = path.back();
        //printf("Getting second pos: %d\n", pos);
        RosterItems::iterator rit = it->second.begin();
        //printf("Total rit size: %d\n", it->second.size());
        for (int j = 0; j < pos; ++j) 
        {
            ++rit;
            if (rit == it->second.end())
            {
                return false;
            }
        }
        iter->user_data = *rit;
        iter->user_data3 = NULL;
    }

    //dump_iter(iter, "Get Iter Return Value");

    return true;
}

void RosterModel::get_value_vfunc(const TreeModel::iterator& iter, int column, GValue* value)
{ 
    // XXX Do something useful here
    const GtkTreeIter* it = iter.get_gobject_if_not_end();
    //printf("Get value for column: %d\n", column);
    //dump_iter(it);

    if (!it || it->user_data == NULL)
    {
        // XXX return better?
        return;
    }

    gpointer icon = NULL;

    switch (column)
    {
    // JID
    case 0:
        g_value_init(value, G_TYPE_STRING);
        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            g_value_set_string(value, item->getJID().c_str());
        }
        else
        {
            g_value_set_string(value, NULL);
        }
        break;
    // Nickname
    case 1:
        g_value_init(value, G_TYPE_STRING);
        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            //printf("Getting nickname for %p\n", item);
            const std::string& nick(item->getNickname());
            //printf("JID: %s\n", item->getJID().c_str());
            if (nick.empty())
            {
                printf("Empty nick item: %p\n", item);
            }
            //printf("\tnickname: %p\n", item->getNickname());
            g_value_set_string(value, item->getNickname().c_str());
        }
        else
        {
            ModelItems::value_type* val = 
                static_cast<ModelItems::value_type*>(it->user_data);
            Glib::ustring name = "<markup><b>" + val->first + "</b></markup>";
            g_value_set_string(value, name.c_str());
        }
        break;
    // Icon
    case 2:
        g_value_init(value, G_TYPE_OBJECT);

        if (!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            RosterItemInfos::iterator it = _item_info.find(item);
            if (it != _item_info.end() && (!it->second.icon.empty()))
            {
                Glib::RefPtr<Gdk::Pixbuf> pb_icon = ResourceManager::getSingleton().getPixbuf(it->second.icon);

                if (pb_icon && it->second.fade_percent < 65535)
                {
                    bool is_selected = (item->getJID() == _selected_jid);
                    const Gdk::Color& bg_col(is_selected ? _default_colors.sel_bg : _default_colors.bg);
                    it->second.icon_fade = pb_icon->composite_color_simple( 
                        pb_icon->get_width(), pb_icon->get_height(), 
                        Gdk::INTERP_NEAREST, (it->second.fade_percent / 255), 
                        4, bg_col.get_pixel(), bg_col.get_pixel());
                    icon = it->second.icon_fade->gobj();
                }
                else
                {
                    it->second.icon_fade.clear();
                    icon = pb_icon->gobj();
                }
            }
        }
        g_value_set_object(value, icon);

        break;
    // Use Icon
    case 3:
        g_value_init(value, G_TYPE_BOOLEAN);
        if (!_sort_with_groups || it->user_data2)
        {
            g_value_set_boolean(value, TRUE);
        }
        else
        {
            g_value_set_boolean(value, FALSE);
        }
        break;
    case 4:
        g_value_init(value, G_TYPE_BOOLEAN);
        if (!_sort_with_groups || it->user_data2)
        {
            g_value_set_boolean(value, FALSE);
        }
        else
        {
            g_value_set_boolean(value, TRUE);
        }
        break;
    case 5:
        g_value_init(value, GDK_TYPE_COLOR);
        if(!_sort_with_groups || it->user_data2)
        {
            Roster::Item* item = static_cast<Roster::Item*>(it->user_data);
            RosterItemInfos::const_iterator it = _item_info.find(item);
            if (it != _item_info.end() && it->second.fade_percent < 65535)
            {
                int fade(it->second.fade_percent);
                bool is_selected = (item->getJID() == _selected_jid);
                const Gdk::Color& fg_col(is_selected ? _default_colors.sel_fg : _default_colors.fg);
                const Gdk::Color& bg_col(is_selected ? _default_colors.sel_bg : _default_colors.bg);

                Gdk::Color color;
                int red = (fade * (fg_col.get_red() - bg_col.get_red())) / 65535 + bg_col.get_red();
                color.set_red(red);
                int green = (fade * (fg_col.get_green() - bg_col.get_green())) / 65535 + bg_col.get_green();
                color.set_green(green);
                int blue = (fade * (fg_col.get_blue() - bg_col.get_blue())) / 65535 + bg_col.get_blue();
                color.set_blue(blue); 
                g_value_set_boxed(value, color.gobj());
            }
            else
                g_value_set_boxed(value, NULL);

        }
        break;
    };
}

void RosterModel::clearIter(GtkTreeIter* iter)
{
    iter->stamp = 0;
    iter->user_data = iter->user_data2 = iter->user_data3 = NULL;
}

void RosterModel::on_roster_refresh(void)
{
    // We only want one startup!
    if (_has_first_refresh)
        return;

    refresh();

    // Don't do this again -- jabberoo workaround
    _has_first_refresh = true;
}

void RosterModel::on_roster_pres(const std::string& jid, bool available, 
    Presence::Type prev_type)
{
    Roster::Item& item(_roster[jid]);

    // See if they need to be added or removed
    if (available && (prev_type != Presence::ptAvailable))
    {
        Roster::Subscription stype = item.getSubsType();
        if (stype == Roster::rsTo || stype == Roster::rsBoth)
        {
            if (_hide_offline)
            {
                addRosterItem(item);
                _item_info[&item].fade_percent = 0;
            }
            Glib::signal_timeout().connect(SigC::bind(
                SigC::slot(*this, &RosterModel::fadeIn), &item), 200);
        }
    }
    else if (!available && (prev_type == Presence::ptAvailable))
    {
        if (_hide_offline)
        {
            delRosterItem(item);
        }
        else
        {
            _item_info[&item].icon = getJIDIcon(jid);
            _item_info[&item].prev_icon = "";
            Glib::signal_timeout().connect(SigC::bind(
                SigC::slot(*this, &RosterModel::fadeOut), &item), 200);
        }
        return;
    }

    // We don't care about these freaks
    if (_hide_offline && !available)
        return;

    // Update our internal info and tell the world it changed
    if (_item_info[&item].queued)
    {
        _item_info[&item].prev_icon = getJIDIcon(jid);
    }
    else
    {
        _item_info[&item].icon = getJIDIcon(jid);
    }
    GtkTreeIter iter;
    iter.stamp = 45; // I hate you
    iter.user_data = &item;
    iter.user_data3 = NULL;

    for (Roster::Item::iterator it = item.begin(); it != item.end(); ++it)
    {
        ModelItems::iterator i = _items.find(*it);
        if (i == _items.end())
        {
            // XXX Add it?
            continue;
        }
        iter.user_data2 = &*i;
        row_changed(getPathFromIter(&iter), TreeModel::iterator(gobj(),&iter));
    }
}

void RosterModel::refresh()
{
    // Clear it up
    clear();

    // Clean up the remnants
    _items.clear();

    // add everyone back in
    for(Roster::iterator it = _roster.begin(); 
        it != _roster.end(); it++)
    {
        addRosterItem(*it);
    }
}

void RosterModel::addRosterItem(Roster::Item& item)
{
    // XXX Should this function check if it is already in?
    GtkTreeIter iter;
    iter.stamp = 46; // I rule you stamp

    // Skip it, we're hiding offline and they aren't available
    if (_hide_offline && !item.isAvailable())
        return;

    // Setup the info we'll always need
    if (G_App.getPacketQueue().isQueued(item.getJID()))
    {
        // XXX Needs a front call
        _item_info[&item].icon = G_App.getPacketQueue().begin()->icon;
        _item_info[&item].prev_icon = getJIDIcon(item.getJID());
        _item_info[&item].blink = true;
        _item_info[&item].queued = true;
    }
    else
    {
        _item_info[&item].icon = getJIDIcon(item.getJID());
        _item_info[&item].prev_icon = "";
        _item_info[&item].blink = false;
        _item_info[&item].queued = false;
    }

    if (item.isAvailable())
        _item_info[&item].fade_percent = 65535;
    else
        _item_info[&item].fade_percent = 25000;

    if (_sort_with_groups)
    {
        for (Roster::Item::iterator it = item.begin(); it != item.end(); ++it)
        {
            ModelItems::iterator mit = _items.find(*it);
            if (mit == _items.end())
            {
                std::pair<ModelItems::iterator, bool> ret = _items.insert(
                    make_pair(*it, RosterItems()));
                mit = ret.first;
                iter.user_data = &*mit;
                iter.user_data2 = NULL;
                iter.user_data3 = NULL;
                row_inserted(getPathFromIter(&iter), 
                             TreeModel::iterator(gobj(), &iter));
            }

            mit->second.insert(&item);
            //printf("%p is %s\n", &item, item.getNickname().c_str());
            iter.user_data = &item;
            iter.user_data2 = &*mit;
            iter.user_data3 = NULL;
            row_inserted(getPathFromIter(&iter), 
                         TreeModel::iterator(gobj(), &iter));

        }
    }
    else
    {
        _items[""].insert(&item);
        iter.user_data = &item;
        iter.user_data2 = iter.user_data3 = NULL;
        row_inserted(getPathFromIter(&iter), 
                     TreeModel::iterator(gobj(), &iter));
    }
}

void RosterModel::delRosterItem(jabberoo::Roster::Item& item)
{
    if ( jabberoo::JID::compare(item.getJID(), _selected_jid) == 0 )
    {
        _selected_jid = "";
    }

    typedef std::list<Gtk::TreeModel::Path> PathList;
    PathList delPaths;

    GtkTreeIter iter;
    iter.stamp = 47; // XXX I rule you stamp
    if (_sort_with_groups)
    {
        for (Roster::Item::iterator it = item.begin(); it != item.end(); ++it)
        {
            ModelItems::iterator mit = _items.find(*it);
            if (mit == _items.end())
            {
                // This is probably a new group, just skip it
                continue;
            }
            iter.user_data = &item;
            iter.user_data2 = &*mit;
            iter.user_data3 = NULL;

            delPaths.push_back(getPathFromIter(&iter));

            RosterItems::iterator rit = mit->second.find(&item);
            mit->second.erase(rit);
            _item_info.erase(&item);

            // If we cleared them, then we need to tell the world so
            if (mit->second.empty())
            {
                iter.user_data = &*mit;
                iter.user_data2 = iter.user_data3 = NULL;
                delPaths.push_back(getPathFromIter(&iter));

                _items.erase(mit);
            }
        }
    }
    else
    {
        iter.user_data = &item;
        iter.user_data2 = iter.user_data3 = NULL;

        delPaths.push_back(getPathFromIter(&iter));

        _items[""].erase(&item);
    }

    std::for_each(delPaths.begin(), delPaths.end(), 
        SigC::slot(*this, &RosterModel::row_deleted));
}

Gtk::TreeModel::Path RosterModel::getPathFromIter(const GtkTreeIter* iter)
{
    //dump_iter(iter);
    TreeModel::Path res;

    // Just the group path
    if (iter->user_data3 || 
        ( (!iter->user_data3 && !iter->user_data2) && _sort_with_groups) )
    {
        ModelItems::value_type* val = 
            static_cast<ModelItems::value_type*>(iter->user_data);
        res.push_back(distance(_items.begin(), _items.find(val->first)));
        //printf("Returning group path(%s): %s\n", val->first.c_str(), res.to_string().c_str());
        return res;
    }

    // If we have a parent add it in
    RosterItems* ri = NULL;
    if (iter->user_data2)
    {
        ModelItems::value_type* val = 
            static_cast<ModelItems::value_type*>(iter->user_data2);
        if (_items.find(val->first) == _items.end())
        {
            // XXX This really shouldn't happen, but we'll be nice
            return res;
        }
        res.push_back(distance(_items.begin(), _items.find(val->first)));
        ri = &(val->second);

    }
    else
    {
        ri = &_items[""];
    }

    // Add the actual node
    Roster::Item* rkey = static_cast<Roster::Item*>(iter->user_data);
    res.push_back(distance(ri->begin(), ri->find(rkey))); 

    //printf("Returning complete path: %s\n", res.to_string().c_str());
    return res;
}


void RosterModel::clear()
{
    _has_first_refresh = false;

    if (_sort_with_groups)
    {
        ModelItems::iterator it = _items.begin();
        while (it != _items.end())
        {
            // store our next stop
            ModelItems::iterator next = it;
            ++next;

            // Pull out the actual group items and clear them out
            clearRosterItems(it);

            // Get the path, erase it, then signal so there is no confusion
            GtkTreeIter iter;
            iter.stamp = 48;
            iter.user_data = &*it;
            iter.user_data2 = iter.user_data3 = NULL;
            Gtk::TreeModel::Path res;
            res.push_back(0);

            _items.erase(it);

            row_deleted(res);
            
            it = next;
        }
    }
    else
    {
        if (!_items.empty())
            clearRosterItems(_items.begin());
    }

    _items.clear();
    _need_update.clear();
}

void RosterModel::clearRosterItems(ModelItems::iterator mit)
{
    RosterItems& ritems(mit->second);

    GtkTreeIter iter;
    iter.stamp = 49;
    
    RosterItems::iterator it = ritems.begin();
    while (it != ritems.end())
    {
        RosterItems::iterator next = it;
        next++;

        // Get the path, erase it, then signal so there is no confusion
        iter.user_data = *it;
        iter.user_data2 = &*mit;
        iter.user_data3 = NULL;

        Gtk::TreeModel::Path path;
        path.push_back(0);
        path.push_back(0);
        
        _item_info.erase(*it);
        ritems.erase(it);

        row_deleted(path);

        it = next;
    }
}

std::string RosterModel::getJIDIcon(const std::string& jid)
{
    PresenceDB::const_iterator it;
    try
    {
        it = _sess.presenceDB().find(jid);
    }
    catch(PresenceDB::XCP_InvalidJID& e)
    {
        return "";
    }

    switch ((*it).getShow())
    {
    case Presence::stOnline:
        return "online.png";
    case Presence::stChat:
        return "chat.png";
    case Presence::stAway:
        return "away.png";
    case Presence::stDND:
        return "dnd.png";
    case Presence::stXA:
        return "xa.png";
    default:
        return "";
    };

    return "";
}

void RosterModel::on_roster_set(const judo::Element& elem)
{
    // The XPath ensures that we can blindly assume these are valid
    const judo::Element* query = elem.findElement("query");
    const judo::Element* item = query->findElement("item");
    std::string s10n = item->getAttrib("subscription");
    std::string jid = item->getAttrib("jid");

    /*
    if (s10n == "none")
    {
        // XXX This might not belong here?
        //_roster.deleteUser(jid);
    }
    else if (s10n == "remove")
    {
        Roster::Item& ritem(_roster[jid]);
        delRosterItem(ritem);
    }
    if (s10n != "remove")
    {
        Roster::Item& ritem(_roster[jid]);
        printf("Resetting %s(%s)\n", jid.c_str(), ritem.getNickname().c_str());
        delRosterItem(ritem);
        printf("Delete done for %s\n", jid.c_str());
        addRosterItem(ritem);
        printf("Add done for %s\n", jid.c_str());
    }
    */
}

void RosterModel::on_packet_queued(const std::string& jid, const std::string& icon)
{
    if (!_roster.containsJID(jid))
    {
        // Skip em
        return;
    }

    Roster::Item& item(_roster[jid]);
    ItemInfo& info(_item_info[&item]);

    if (info.queued)
    {
        // We only want the icon/info of the first packet.
        return;
    }

    set_queue_icon(jid, icon);
}

void RosterModel::on_queue_emptied(const std::string& jid)
{
    if (!_roster.containsJID(jid))
        return;

    Roster::Item& item(_roster[jid]);
    ItemInfo& info(_item_info[&item]);
    info.icon = getJIDIcon(jid);
    info.queued = false;
    info.blink = false;

    _need_update.erase(&item);

    GtkTreeIter iter;
    iter.stamp = 61; // I hate you
    iter.user_data = &item;
    iter.user_data3 = NULL;

    for (Roster::Item::iterator it = item.begin(); it != item.end(); ++it)
    {
        ModelItems::iterator i = _items.find(*it);
        if (i == _items.end())
        {
            continue;
        }
        iter.user_data2 = &*i;
        row_changed(getPathFromIter(&iter), TreeModel::iterator(gobj(),&iter));
    }
}

void RosterModel::on_queue_changed(const PacketQueue::QueueInfo& jid_next, 
        const PacketQueue::QueueInfo& first)
{
    set_queue_icon(jid_next.jid, jid_next.icon);
}

void RosterModel::set_queue_icon(const std::string& jid, 
                                 const std::string& icon)
{
    if (!_roster.containsJID(jid))
    {
        // Skip em
        return;
    }

    Roster::Item& item(_roster[jid]);
    ItemInfo& info(_item_info[&item]);

    info.icon = icon;
    info.prev_icon = getJIDIcon(jid);
    info.queued = true;
    info.blink = true;

    if (_need_update.count(&item) == 0)
    {
        _need_update.insert(&item);
    }

    if (!_update_running)
    {
        Glib::signal_timeout().connect(
            slot(*this, &RosterModel::do_roster_updates), 500);
        _update_running = true;
    }

    GtkTreeIter iter;
    iter.stamp = 60; // I hate you
    iter.user_data = &item;
    iter.user_data3 = NULL;

    for (Roster::Item::iterator it = item.begin(); it != item.end(); ++it)
    {
        ModelItems::iterator i = _items.find(*it);
        if (i == _items.end())
        {
            continue;
        }
        iter.user_data2 = &*i;
        row_changed(getPathFromIter(&iter), TreeModel::iterator(gobj(),&iter));
    }
}

void RosterModel::on_removing_item(jabberoo::Roster::Item& item)
{
    delRosterItem(item);
}

void RosterModel::on_updating_item(jabberoo::Roster::Item& item)
{
    delRosterItem(item);
}

void RosterModel::on_update_done(jabberoo::Roster::Item& item)
{
    if (_hide_offline && !item.isAvailable())
        return;
    addRosterItem(item);
}

bool RosterModel::fadeIn(jabberoo::Roster::Item* item)
{
    int& fade(_item_info[item].fade_percent);

    fade += 5000;

    if (fade > 65535)
    {
        fade = 65535;
    }

    // Tell the world we changed so it updates the view
    GtkTreeIter iter;
    iter.stamp = 50; // I hate you
    iter.user_data = item;
    iter.user_data2 = iter.user_data3 = NULL;

    if (_sort_with_groups)
    {
        for (Roster::Item::iterator it = item->begin(); it != item->end(); ++it)
        {
            ModelItems::iterator i = _items.find(*it);
            if (i == _items.end())
            {
                continue;
            }
            iter.user_data2 = &*i;
            row_changed(getPathFromIter(&iter), 
                TreeModel::iterator(gobj(),&iter));
        }
    }
    else
    {
        row_changed(getPathFromIter(&iter), 
            TreeModel::iterator(gobj(), &iter));
    }

    if ( fade == 65535 )
    {
        return false;
    }

    return true;
}

bool RosterModel::fadeOut(jabberoo::Roster::Item* item)
{
    int& fade(_item_info[item].fade_percent);

    fade -= 5000;

    if (fade < 20000)
    {
        fade = 20000;
    }

    // Tell the world we changed so it updates the view
    GtkTreeIter iter;
    iter.stamp = 79; // I hate you
    iter.user_data = item;
    iter.user_data3 = NULL;
    for (Roster::Item::iterator it = item->begin(); it != item->end(); ++it)
    {
        ModelItems::iterator i = _items.find(*it);
        if (i == _items.end())
        {
            continue;
        }
        iter.user_data2 = &*i;
        row_changed(getPathFromIter(&iter), TreeModel::iterator(gobj(),&iter));
    }

    if ( fade == 20000 )
    {
        return false;
    }

    return true;
}

bool RosterModel::do_roster_updates()
{
    if (!_has_first_refresh)
    {
        return false;
    }

    for (RosterItems::iterator it = _need_update.begin(); 
         it != _need_update.end(); ++it)
    {
        Roster::Item* item = *it;
        ItemInfo& info(_item_info[item]);
        // Pretty blinken
        if (info.blink)
        {
            std::string prev_icon = info.prev_icon;
            info.prev_icon = info.icon;
            info.icon = prev_icon;
        }
            
        GtkTreeIter iter;
        iter.stamp = 62; // I hate you
        iter.user_data = item;
        iter.user_data3 = NULL;

        for (Roster::Item::iterator it = item->begin(); it != item->end(); ++it)
        {
            ModelItems::iterator i = _items.find(*it);
            if (i == _items.end())
            {
                continue;
            }
            iter.user_data2 = &*i;
            row_changed(getPathFromIter(&iter), 
                        TreeModel::iterator(gobj(),&iter));
        }
    }

    if (_need_update.empty())
    {
        _update_running = false;
        return false;
    }

    return true;
}

//*********************** RosterModel_Class Stuff ********************//
const Glib::Class& RosterModel_Class::init()
{
    if (!gtype_)
    {
        class_init_func_ = &RosterModel_Class::class_init_function;

        const GTypeInfo derived_info = 
        {
            sizeof(GObjectClass),
            NULL,
            NULL,
            class_init_func_,
            NULL,
            NULL,
            sizeof(GObject),
            0,
            0,
            NULL,
        };
        
        gtype_ = g_type_register_static(G_TYPE_OBJECT, "Gabber_RosterModel",
            &derived_info, GTypeFlags(0));

        TreeModel::add_interface(get_type());
    }

    return *this;
}

void RosterModel_Class::class_init_function(void* g_class, void* class_data)
{
    // XXX Do something here?
}

} // namespace Gabber
