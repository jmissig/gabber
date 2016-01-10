#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "RosterController.hh"
#include "RosterController.hh"
#include "ResourceManager.hh"
#include "GabberApp.hh"

#include <session.hh>

#include <gtkmm/treepath.h>
#include <glibmm/class.h>

#include <sigc++/bind.h>

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

RosterController_Class RosterController::rostercontroller_class_;

RosterController::RosterController(Session& sess, bool hide_offline, 
                         bool sort_with_groups):
    Glib::ObjectBase("Gabber_RosterController"), 
    Glib::Object(Glib::ConstructParams(rostercontroller_class_.init(), (char*) 0)),
    TreeModel(), 
    _sess(sess), _roster(_sess.roster()), _model(new RosterModel(sess)),
    _avatar_manager(new AvatarManager(sess)),
    _hide_offline(hide_offline), _sort_with_groups(sort_with_groups),
    _has_first_refresh(false), _update_running(false)
{
    _model->set_hide_offline(_hide_offline);
    _model->signal_node_inserted.connect(
        SigC::slot(*this, &RosterController::on_rostermodel_inserted));
    _model->signal_node_updated.connect(
        SigC::slot(*this, &RosterController::on_rostermodel_updated));
    _model->signal_node_removed.connect(
        SigC::slot(*this, &RosterController::on_rostermodel_removed));
    if(sort_with_groups)
        _model->set_strategy(new GroupedRosterStrategy(*_model));
    G_App.evtConnected.connect(SigC::slot(*this, &RosterController::on_connected));
#if 0
    _roster.evtPresence.connect(SigC::slot(*this, &RosterController::on_roster_pres));
    _roster.evtRemovingItem.connect(SigC::slot(*this, &RosterController::on_removing_item));
    _roster.evtUpdating.connect(SigC::slot(*this, &RosterController::on_updating_item));
    _roster.evtUpdateDone.connect(SigC::slot(*this, &RosterController::on_update_done));
#endif
    G_App.getPacketQueue().packet_queued_event.connect(
        SigC::slot(*this, &RosterController::on_packet_queued));
    G_App.getPacketQueue().queue_emptied_event.connect(
        SigC::slot(*this, &RosterController::on_queue_emptied));
    G_App.getPacketQueue().queue_changed_event.connect(
        SigC::slot(*this, &RosterController::on_queue_changed));
}

RosterController::~RosterController()
{ 
    delete _model;
}

void RosterController::setSortType(SortType type)
{
}

void RosterController::setHideOffline(bool hide)
{ 
    _hide_offline = hide;
    _model->set_hide_offline(hide);
    refresh();
}

Glib::RefPtr<RosterController> RosterController::create(jabberoo::Session& sess, bool hide_offline, bool sort_with_groups)
{
    return Glib::RefPtr<RosterController>( new RosterController(sess, hide_offline, sort_with_groups) );
}

// Inheritted from Gtk::TreeModel
void RosterController::set_value_impl(const iterator& row, int column, const Glib::ValueBase& value)
{
}


void RosterController::get_value_impl(const iterator& row, int column, Glib::ValueBase& value)
{
    const GtkTreeIter* it = row.get_gobject_if_not_end();

    if (!it || it->user_data == NULL)
    {
        // XXX return better?
        return;
    }

    RosterNode* node = static_cast<RosterNode*>(it->user_data);
    gpointer icon = NULL;

    switch(column)
    {
    // JID
    case 0:
        value.init(G_TYPE_STRING);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            g_value_set_string(value.gobj(), item.getJID().c_str());
        }
        else
        {
            g_value_set_string(value.gobj(), NULL);
        }
        break;
    // Nick
    case 1:
        value.init(G_TYPE_STRING);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            g_value_set_string(value.gobj(), item.getNickname().c_str());
        }
        else if (node->get_type() == "group")
        {
            GroupNode* gnode = static_cast<GroupNode*>(node);
            Glib::ustring name = "<markup><b>" + gnode->get_name() + "</b></markup>";
            g_value_set_string(value.gobj(), name.c_str());
        }
        else
        {
            g_value_set_string(value.gobj(), NULL);
        }
        break;
    // Icon
    case 2:
        value.init(G_TYPE_OBJECT);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            NodeInfoMap::iterator it = _node_info.find(node);
            if (it != _node_info.end() && (!it->second.icon.empty()))
            {
                Glib::RefPtr<Gdk::Pixbuf> pb_icon = ResourceManager::getSingleton().getPixbuf(it->second.icon);

                if (pb_icon && it->second.fade_percent < 65535)
                {
                    bool is_selected = (item.getJID() == _selected_jid);
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
        g_value_set_object(value.gobj(), icon);
        break;
    // Use_Icon (bool)
    case 3:
        value.init(G_TYPE_BOOLEAN);
        if (node->get_type() == "item")
        {
            g_value_set_boolean(value.gobj(), TRUE);
        }
        else
        {
            g_value_set_boolean(value.gobj(), FALSE);
        }
        break;
    // Is group
    case 4:
        value.init(G_TYPE_BOOLEAN);
        if (node->get_type() == "item")
        {
            g_value_set_boolean(value.gobj(), FALSE);
        }
        else
        {
            g_value_set_boolean(value.gobj(), TRUE);
        }
        break;
    // fore color control
    case 5:
        value.init(GDK_TYPE_COLOR);
        if(node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            NodeInfoMap::const_iterator it = _node_info.find(node);
            if (it != _node_info.end() && it->second.fade_percent < 65535)
            {
                int fade(it->second.fade_percent);
                bool is_selected = (item.getJID() == _selected_jid);
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

TreeModelFlags RosterController::get_flags_vfunc()
{ 
    return TreeModelFlags();
}

TreeModelFlags RosterController::get_flags_vfunc() const
{
    return TreeModelFlags();
}

int RosterController::get_n_columns_vfunc()
{ 
    return 6;
}

int RosterController::get_n_columns_vfunc() const
{
    return 6;
}

GType RosterController::get_column_type_vfunc(int index)
{ 
    switch(index)
    {
    case 0:
        return G_TYPE_STRING;
    case 1:
        return G_TYPE_STRING;
    case 2:
        return G_TYPE_OBJECT;
    case 3:
        return G_TYPE_BOOLEAN;
    case 4:
        return G_TYPE_BOOLEAN;
    case 5:
        return GDK_TYPE_COLOR;
    };

    // XXX Is this an OK fallback?
    return G_TYPE_OBJECT;
}

GType RosterController::get_column_type_vfunc(int index) const
{ 
    switch(index)
    {
    case 0:
        return G_TYPE_STRING;
    case 1:
        return G_TYPE_STRING;
    case 2:
        return G_TYPE_OBJECT;
    case 3:
        return G_TYPE_BOOLEAN;
    case 4:
        return G_TYPE_BOOLEAN;
    case 5:
        return GDK_TYPE_COLOR;
    };

    // XXX Is this an OK fallback?
    return G_TYPE_OBJECT;
}

bool RosterController::iter_next_vfunc(GtkTreeIter* iter)
{
    const TreeModel::iterator cur(gobj(), iter);
    TreeModel::iterator it(gobj(), iter);
    bool ret = iter_next_vfunc(cur, it);
    if (ret)
        iter->user_data = it.gobj()->user_data;
    return ret;
}

bool RosterController::iter_next_vfunc(const TreeModel::iterator& iter, TreeModel::iterator& iter_next) const
{
    const GtkTreeIter* cur = iter.gobj();
    GtkTreeIter* next = iter_next.gobj();
    
    // If this hits we're not going to be able to get a next iter
    if (cur->user_data == NULL)
    {
        return false;
    }

    RosterNode* node = static_cast<RosterNode*>(cur->user_data);
    next->stamp = 51;

    try
    {
        RosterNode& next_node(_model->get_next_node(*node));
        next->user_data = &next_node;
        return true;
    }
    catch(RosterModel::InvalidNode& e)
    {
        return false;
    }
}

bool RosterController::iter_children_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent)
{
    const TreeModel::iterator pit(gobj(), parent);
    TreeModel::iterator it(gobj(), iter);
    bool ret = iter_children_vfunc(pit, it);
    if (ret)
        iter->user_data = it.gobj()->user_data;
    return ret;
}

bool RosterController::iter_children_vfunc(const TreeModel::iterator& parent, TreeModel::iterator& iter) const
{
    const GtkTreeIter* pit = parent.gobj();
    GtkTreeIter* it = iter.gobj();
    
    // Handle the case where we just want to reset to root
    if (pit == NULL)
    {
        it->stamp = 40;
        it->user_data = _model->get_roster().front();
        it->user_data2 = NULL;
        it->user_data3 = NULL;

        return true;
    }

    if (pit->user_data == NULL)
    {
        return false;
    }

    // Handle the normal case where have children
    RosterNode* node = static_cast<RosterNode*>(pit->user_data);
    RosterNodeList& children(node->get_children());

    if(children.empty())
    {
        return false;
    }

    it->stamp = 41; // This stamp thing is odd
    it->user_data = children.front();
    it->user_data2 = NULL;
    it->user_data3 = NULL;

    return true;
}

bool RosterController::iter_has_child_vfunc(const TreeModel::iterator& iter) const
{
    const GtkTreeIter* it = iter.gobj();
    RosterNode* node = static_cast<RosterNode*>(it->user_data);
    return !node->get_children().empty();
}

bool RosterController::iter_has_child_vfunc(const GtkTreeIter* iter)
{
    const TreeModel::iterator it(gobj(), iter);
    return iter_has_child_vfunc(it);
}

int RosterController::iter_n_children_vfunc(const GtkTreeIter* iter)
{
    const TreeModel::iterator it(gobj(), iter);
    return iter_n_children_vfunc(it);
}

int RosterController::iter_n_children_vfunc(const TreeModel::iterator& iter) const
{
    const GtkTreeIter* it = iter.gobj();
    RosterNode* node = static_cast<RosterNode*>(it->user_data);
    return node->get_children().size();
}

int RosterController::iter_n_root_children_vfunc(void) const
{
    return _model->get_roster().size();
}

bool RosterController::iter_nth_child_vfunc(GtkTreeIter* iter, 
    const GtkTreeIter* parent, int n)
{
    TreeModel::iterator it(gobj(), iter);
    
    bool ret = false;
    if (parent == NULL)
    {
        ret = iter_nth_root_child_vfunc(n, it);
        if(ret)
            iter->user_data = it.gobj()->user_data;
        return ret;
    }
    
    const TreeModel::iterator pit(gobj(), parent);
    ret = iter_nth_child_vfunc(pit, n, it);
    if(ret)
        iter->user_data = it.gobj()->user_data;
    return ret;
}

bool RosterController::iter_nth_child_vfunc(const TreeModel::iterator& parent, 
    int n, TreeModel::iterator& iter) const
{
    const GtkTreeIter* pit = parent.gobj();
    GtkTreeIter* iit = iter.gobj();
    
    RosterNodeList& nodes(static_cast<RosterNode*>(pit->user_data)->get_children());

    if(nodes.size() < (guint)n)
    {
        return false;
    }

    RosterNodeList::iterator it = nodes.begin();
    std::advance(it, n);

    iit->stamp = 42;
    iit->user_data = *it;
    iit->user_data2 = NULL;
    iit->user_data3 = NULL;

    return true;
}

bool RosterController::iter_nth_root_child_vfunc(int n, TreeModel::iterator& iter) const
{
    GtkTreeIter* iit = iter.gobj();
    
    RosterNodeList& nodes(_model->get_roster());

    if(nodes.size() < (guint)n)
    {
        return false;
    }

    RosterNodeList::iterator it = nodes.begin();
    std::advance(it, n);

    iit->stamp = 422;
    iit->user_data = *it;
    iit->user_data2 = NULL;
    iit->user_data3 = NULL;

    return true;
}

bool RosterController::iter_parent_vfunc(const TreeModel::iterator& child, TreeModel::iterator& iter) const
{
    const GtkTreeIter* cit = child.gobj();
    GtkTreeIter* it = iter.gobj();
    
    if(cit && cit->user_data != NULL)
    {
        RosterNode* node = static_cast<RosterNode*>(cit->user_data);
        RosterNode* parent = node->get_parent();
        
        if(parent)
        {
            it->stamp = 43;
            it->user_data = parent;
            it->user_data2 = NULL;
            it->user_data3 = NULL;

            return true;
        }
    }
    
    return false;
}

bool RosterController::iter_parent_vfunc(GtkTreeIter* iter, const GtkTreeIter* child)
{
    const TreeModel::iterator cit(gobj(), child);
    TreeModel::iterator it(gobj(), iter);
    bool ret = iter_parent_vfunc(cit, it);
    if(ret)
        iter->user_data = it.gobj()->user_data;
    return ret;
}

void RosterController::ref_node_vfunc(GtkTreeIter* iter)
{
    // XXX ignored for now
}

void RosterController::unref_node_vfunc(GtkTreeIter* iter)
{
    // XXX ignored for now
}

#ifdef OLD_GTKMM
TreeModel::Path RosterController::get_path_vfunc(const TreeModel::iterator& iter)
#else
TreeModel::Path RosterController::get_path_vfunc(const TreeModel::iterator& iter) const
#endif
{
    const GtkTreeIter* it = iter.get_gobject_if_not_end();

    if (!it || it->user_data == NULL)
    {
        return TreeModel::Path();
    }

    RosterNode* node = static_cast<RosterNode*>(it->user_data);
    RosterModel::Path path(_model->get_path(*node));

    TreeModel::Path res;
    assign_path(path, res);

    return res;
}

bool RosterController::get_iter_vfunc(GtkTreeIter* iter, 
    const TreeModel::Path& path)
{
    TreeModel::iterator it(gobj(), iter);
    bool ret = get_iter_vfunc(path, it);
    if(ret)
        iter->user_data = it.gobj()->user_data;
    return  ret;
}

bool RosterController::get_iter_vfunc(const TreeModel::Path& path, TreeModel::iterator& iter) const
{
    GtkTreeIter* it = iter.gobj();
    
    RosterModel::Path roster_path;
    roster_path.assign(path.begin(), path.end());
    
    try
    {
        RosterNode& node(_model->get_node(roster_path));

        it->stamp = 44;
        it->user_data = &node;
        it->user_data2 = NULL;
        it->user_data3 = NULL;

        return true;
    }
    catch(RosterModel::InvalidPath& e)
    {
        return false;
    }
}

void RosterController::get_value_vfunc(const TreeModel::iterator& iter,
    int column, GValue* value)
{
    do_get_value_vfunc(iter, column, value);
}

void RosterController::get_value_vfunc(const TreeModel::iterator& iter, 
    int column, Glib::ValueBase& value) const
{
    do_get_value_vfunc(iter, column, value.gobj());
    value.init(G_VALUE_TYPE(value.gobj()));
}

void RosterController::do_get_value_vfunc(const TreeModel::iterator& iter, 
    int column, GValue* value) const
{ 
    // XXX Do something useful here
    const GtkTreeIter* it = iter.get_gobject_if_not_end();
    //printf("Get value for column: %d\n", column);

    if (!it || it->user_data == NULL)
    {
        // XXX return better?
        return;
    }

    RosterNode* node = static_cast<RosterNode*>(it->user_data);
    gpointer icon = NULL;

    switch (column)
    {
    // JID
    case 0:
        g_value_init(value, G_TYPE_STRING);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            g_value_set_string(value, item.getJID().c_str());
        }
        else
        {
            g_value_set_string(value, NULL);
        }
        break;
    // Nickname
    case 1:
        g_value_init(value, G_TYPE_STRING);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            g_value_set_string(value, item.getNickname().c_str());
        }
        else if (node->get_type() == "group")
        {
            GroupNode* gnode = static_cast<GroupNode*>(node);
            Glib::ustring name = "<markup><b>" + gnode->get_name() + "</b></markup>";
            g_value_set_string(value, name.c_str());
        }
        else
        {
            g_value_set_string(value, NULL);
        }
        break;
    // Icon
    case 2:
        g_value_init(value, G_TYPE_OBJECT);
        if (node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            NodeInfoMap::const_iterator it = _node_info.find(node);
            if (it != _node_info.end() && (!it->second.icon.empty()))
            {
                Glib::RefPtr<Gdk::Pixbuf> pb_icon = ResourceManager::getSingleton().getPixbuf(it->second.icon);

                if (pb_icon && it->second.fade_percent < 65535)
                {
                    bool is_selected = (item.getJID() == _selected_jid);
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
        if (node->get_type() == "item")
        {
            g_value_set_boolean(value, TRUE);
        }
        else
        {
            g_value_set_boolean(value, FALSE);
        }
        break;
    // Is group
    case 4:
        g_value_init(value, G_TYPE_BOOLEAN);
        if (node->get_type() == "item")
        {
            g_value_set_boolean(value, FALSE);
        }
        else
        {
            g_value_set_boolean(value, TRUE);
        }
        break;
    // Fore color
    case 5:
        g_value_init(value, GDK_TYPE_COLOR);
        if(node->get_type() == "item")
        {
            Roster::Item& item(static_cast<RosterItemNode*>(node)->get_item());
            NodeInfoMap::const_iterator it = _node_info.find(node);
            if (it != _node_info.end() && it->second.fade_percent < 65535)
            {
                int fade(it->second.fade_percent);
                bool is_selected = (item.getJID() == _selected_jid);
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

void RosterController::clear()
{
    _has_first_refresh = false;
    _model->clear();
    _node_info.clear();
    _need_update.clear();
}

void RosterController::clearIter(GtkTreeIter* iter)
{
    iter->user_data = iter->user_data2 = iter->user_data3 = NULL;
}

void RosterController::refresh()
{
    _model->refresh();
}

static void avatarResult(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::ustring err)
{
    std::cout << "avatar result: " << err << std::endl;
}

void RosterController::on_connected()
{
#if 0
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    _avatar_manager->publish("/Users/temas/temas-ihop-small.jpg");
    if (G_App.getSession().getUserName() != "temas-test")
    {
        std::cout << "retrieving temas-test" << std::endl;
        _avatar_manager->retrieve("temas-test@jabber.org/Gabber2",
            SigC::slot(avatarResult));
    }
#endif
}

#if 0 /* Old RosterModel ways */
void RosterController::on_roster_refresh(void)
{
    // We only want one startup!
    if (_has_first_refresh)
        return;

    refresh();

    // Don't do this again -- jabberoo workaround
    _has_first_refresh = true;
}

void RosterController::on_roster_pres(const std::string& jid, bool available, 
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
                SigC::slot(*this, &RosterController::fadeIn), &item), 200);
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
                SigC::slot(*this, &RosterController::fadeOut), &item), 200);
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

void RosterController::delRosterItem(jabberoo::Roster::Item& item)
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
        SigC::slot(*this, &RosterController::row_deleted));
}

Gtk::TreeModel::Path RosterController::getPathFromIter(const GtkTreeIter* iter)
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


void RosterController::clearRosterItems(ModelItems::iterator mit)
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

void RosterController::on_removing_item(jabberoo::Roster::Item& item)
{
    delRosterItem(item);
}

void RosterController::on_updating_item(jabberoo::Roster::Item& item)
{
    delRosterItem(item);
}

void RosterController::on_update_done(jabberoo::Roster::Item& item)
{
    if (_hide_offline && !item.isAvailable())
        return;
    addRosterItem(item);
}
#endif

std::string RosterController::getJIDIcon(const std::string& jid)
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

void RosterController::on_packet_queued(const std::string& jid, const std::string& icon)
{
    set_queue_icon(jid, icon, true);
}

void RosterController::on_queue_emptied(const std::string& jid)
{
    if (!_roster.containsJID(jid))
        return;

    Roster::Item& item(_roster[jid]);
    RosterModel::ItemNodeMap& node_map(_model->get_item_map());

    for(RosterModel::ItemNodeMap::iterator it = node_map.lower_bound(&item); 
        it != node_map.upper_bound(&item); ++it)
    {
        RosterNode* node = it->second;
        NodeInfo& info(_node_info[node]);

        info.icon = getJIDIcon(jid);
        info.queued = false;
        info.blink = false;

        TreeModel::Path gtk_path;
        assign_path(_model->get_path(*node), gtk_path);
        
        GtkTreeIter iter;
        iter.stamp = 61; // I hate you
        iter.user_data = node;
        iter.user_data2 = NULL;
        iter.user_data3 = NULL;

        row_changed(gtk_path, TreeModel::iterator(gobj(), &iter));
        _need_update.erase(node);
    }
}

void RosterController::on_queue_changed(const PacketQueue::QueueInfo& jid_next, 
        const PacketQueue::QueueInfo& first)
{
    set_queue_icon(jid_next.jid, jid_next.icon);
}

void RosterController::set_queue_icon(const std::string& jid, 
                                 const std::string& icon, bool first)
{
    if (!_roster.containsJID(jid))
    {
        // Skip em
        return;
    }

    Roster::Item& item(_roster[jid]);
    RosterModel::ItemNodeMap& node_map(_model->get_item_map());
    for(RosterModel::ItemNodeMap::iterator it = node_map.lower_bound(&item); 
        it != node_map.upper_bound(&item); ++it)
    {
        RosterNode* node = it->second;
        NodeInfo& info(_node_info[node]);

        if (first && info.queued)
        {
            // We only want the icon/info of the first packet.
            continue;
        }

        info.icon = icon;
        info.prev_icon = getJIDIcon(jid);
        info.queued = true;
        info.blink = true;

        if (_need_update.count(node) == 0)
        {
            _need_update.insert(node);
        }

        if (!_update_running)
        {
            Glib::signal_timeout().connect(
                SigC::slot(*this, &RosterController::do_roster_updates), 500);
            _update_running = true;
        }

        TreeModel::Path gtk_path;
        assign_path(_model->get_path(*node), gtk_path);
        
        GtkTreeIter iter;
        iter.stamp = 60; // I hate you
        iter.user_data = node;
        iter.user_data2 = NULL;
        iter.user_data3 = NULL;

        row_changed(gtk_path, TreeModel::iterator(gobj(), &iter));
    }
}

void RosterController::assign_path(const RosterModel::Path& rpath, TreeModel::Path& tpath) const
{
    for(RosterModel::Path::const_iterator it = rpath.begin(); it != rpath.end(); ++it)
    {
        tpath.append_index(*it);
    }
}

bool RosterController::fadeIn(RosterNode* node, int max_fade)
{
    int& fade(_node_info[node].fade_percent);

    fade += 5000;

    if (fade > max_fade)
    {
        fade = max_fade;
    }

    // Tell the world we changed so it updates the view
    TreeModel::Path gtk_path;
    assign_path(_model->get_path(*node), gtk_path);

    GtkTreeIter iter;
    iter.stamp = 50; // I hate you
    iter.user_data = node;
    iter.user_data2 = iter.user_data3 = NULL;

    row_changed(gtk_path, TreeModel::iterator(gobj(), &iter));

    if ( fade == max_fade )
    {
        return false;
    }

    return true;
}

bool RosterController::fadeOut(RosterNode* node)
{
    int& fade(_node_info[node].fade_percent);

    fade -= 5000;

    if (fade < 20000)
    {
        fade = 20000;
    }

    // Tell the world we changed so it updates the view
    TreeModel::Path gtk_path;
    assign_path(_model->get_path(*node), gtk_path);

    GtkTreeIter iter;
    iter.stamp = 50; // I hate you
    iter.user_data = node;
    iter.user_data2 = iter.user_data3 = NULL;

    if ( fade == 20000 )
    {
        return false;
    }

    return true;
}

bool RosterController::do_roster_updates()
{
    if (!_has_first_refresh)
    {
        return false;
    }

    for (NodeSet::iterator it = _need_update.begin(); 
         it != _need_update.end(); ++it)
    {
        RosterNode* node = *it;
        NodeInfo& info(_node_info[node]);
        // Pretty blinken
        if (info.blink)
        {
            std::string prev_icon = info.prev_icon;
            info.prev_icon = info.icon;
            info.icon = prev_icon;
        }
            
        TreeModel::Path gtk_path;
        assign_path(_model->get_path(*node), gtk_path);

        GtkTreeIter iter;
        iter.stamp = 62; // I hate you
        iter.user_data = node;
        iter.user_data2 = NULL;
        iter.user_data3 = NULL;

        row_changed(gtk_path, TreeModel::iterator(gobj(), &iter));
    }

    if (_need_update.empty())
    {
        _update_running = false;
        return false;
    }

    return true;
}

void RosterController::on_rostermodel_inserted(RosterNode& node,
                                               RosterModel::Path& path)
{
    if(node.get_type() == "item")
    {
        Roster::Item& item(static_cast<RosterItemNode&>(node).get_item());
        NodeInfo& info(_node_info[&node]);
        // Setup the info we'll always need
        if (G_App.getPacketQueue().isQueued(item.getJID()))
        {
            info.icon = G_App.getPacketQueue().begin()->icon;
            info.prev_icon = getJIDIcon(item.getJID());
            info.blink = true;
            info.queued = true;
        }
        else
        {
            info.icon = getJIDIcon(item.getJID());
            info.prev_icon = "";
            info.blink = false;
            info.queued = false;
        }

        info.fade_percent = 0;
        int max_fade = 65535;
        if (!_hide_offline && !item.isAvailable())
        {
            max_fade = 20000;
        }

        info.fade_connection.disconnect();
        info.fade_connection = Glib::signal_timeout().connect(SigC::bind(
            SigC::slot(*this, &RosterController::fadeIn), 
            &node, max_fade), 200);
    }

    // Let's do the gtk fun
    TreeModel::Path gtk_path;
    assign_path(path, gtk_path);

    GtkTreeIter iter;
    iter.stamp = 99;
    iter.user_data = &node;
    iter.user_data2 = NULL;
    iter.user_data3 = NULL;
    
    row_inserted(gtk_path, TreeModel::iterator(gobj(), &iter));
}

void RosterController::on_rostermodel_updated(RosterNode& node,
                                              RosterModel::Path& path)
{
    if(node.get_type() == "item")
    {
        Roster::Item& item(static_cast<RosterItemNode&>(node).get_item());
        NodeInfo& info(_node_info[&node]);

        if(info.queued)
            info.prev_icon = getJIDIcon(item.getJID());
        else
            info.icon = getJIDIcon(item.getJID());

        if (!_hide_offline && (info.fade_percent > 20000) && 
            !item.isAvailable())
        {
            info.prev_icon = "";
            info.fade_connection.disconnect();
            info.fade_connection = Glib::signal_timeout().connect(SigC::bind(
                SigC::slot(*this, &RosterController::fadeOut), &node), 200);
        }
        else if(!_hide_offline && item.isAvailable() && 
                (info.fade_percent < 65535))
        {
            info.fade_connection.disconnect();
            info.fade_connection = Glib::signal_timeout().connect(SigC::bind(
                SigC::slot(*this, &RosterController::fadeIn), &node, 65535),
                200);
        }
    }

    TreeModel::Path gtk_path;
    assign_path(path, gtk_path);

    GtkTreeIter iter;
    iter.stamp = 100;
    iter.user_data = &node;
    iter.user_data2 = NULL;
    iter.user_data3 = NULL;

    row_changed(gtk_path, TreeModel::iterator(gobj(), &iter));
}

void RosterController::on_rostermodel_removed(RosterNode& node, 
    RosterModel::Path& path)
{
    TreeModel::Path gtk_path;
    assign_path(path, gtk_path);

    _node_info[&node].fade_connection.disconnect();
    row_deleted(gtk_path);
    _node_info.erase(&node);
}

//*********************** RosterController_Class Stuff ********************//
const Glib::Class& RosterController_Class::init()
{
    if (!gtype_)
    {
        class_init_func_ = &RosterController_Class::class_init_function;

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
        
        gtype_ = g_type_register_static(G_TYPE_OBJECT, "Gabber_RosterController",
            &derived_info, GTypeFlags(0));

        TreeModel::add_interface(get_type());
    }

    return *this;
}

void RosterController_Class::class_init_function(void* g_class, void* class_data)
{
    // XXX Do something here?
}

} // namespace Gabber
