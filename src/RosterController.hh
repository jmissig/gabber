#ifndef ROSTERCONTROLLER_HH
#define ROSTERCONTROLLER_HH

#include "RosterModel.hh"
#include "JabberConnection.hh"
#include "GabberUtility.hh"
#include "AvatarManager.hh"

#include <vector>

#include <gtkmm/treemodel.h>
#include <gdkmm/color.h>
#include <gdkmm/pixbuf.h>

#include <presence.hh>
#include <roster.hh>

#include <jabberoo/JID.hh>

#include "fwd.h"

namespace Gabber
{
/**
 * Model for a Gtk::TreeView that is based on the roster state
 */
class RosterController : public Glib::Object, public Gtk::TreeModel
{
public:

    struct ModelColumns : public Gtk::TreeModelColumnRecord 
    {
        Gtk::TreeModelColumn<Glib::ustring> jid;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
        Gtk::TreeModelColumn<bool> use_icon;
        Gtk::TreeModelColumn<bool> is_group;

        ModelColumns()
        { add(jid); add(name); add(icon); add(use_icon); add(is_group); }
    };
    /**
    * The list of available columns.
    */
    const ModelColumns columns;

    RosterController(jabberoo::Session& sess, bool hide_offline = true, 
                bool sort_with_groups = true);
    ~RosterController();

    static Glib::RefPtr<RosterController> create(jabberoo::Session& sess, bool hide_offline = true, bool sort_with_groups = true);
     /**
      * Clear the RosterModel.
      */
     void clear();

    enum SortType { rmSortAvailability, rmSortFullName };

    // XXX Not used yet
    void setSortType(SortType type);
    SortType getSortType() const
    { return _sort_type; }

    /// use groups in the sorting methods
    void setSortWithGroups(bool use_groups)
    { _sort_with_groups = use_groups; refresh(); }
    bool getSortWithGroups() const
    { return _sort_with_groups; }

    /// Hide users that are offline
    void setHideOffline(bool hide);
    bool getHideOffline() const
    { return _hide_offline; }

    /// Set the default foreground color for items
    void set_default_colors(const Gdk::Color& fg_col, const Gdk::Color& bg_col,
                            const Gdk::Color& sel_fg_col, 
                            const Gdk::Color& sel_bg_col)
    { 
        _default_colors.fg = fg_col;
        _default_colors.bg = bg_col;
        _default_colors.sel_fg = sel_fg_col;
        _default_colors.sel_bg = sel_bg_col;
    }

    void set_selected_jid(const std::string& jid)
    { _selected_jid = jid; }

protected:
    // Inheritted from Gtk::TreeModel
    void set_value_impl(const iterator& row, int column, const Glib::ValueBase& value);
    void get_value_impl(const iterator& row, int column, Glib::ValueBase& value);
    Gtk::TreeModelFlags get_flags_vfunc();
    Gtk::TreeModelFlags get_flags_vfunc() const;
    int get_n_columns_vfunc();
    int get_n_columns_vfunc() const;
    GType get_column_type_vfunc(int index);
    GType get_column_type_vfunc(int index) const;
    bool iter_next_vfunc(GtkTreeIter* iter);
    bool iter_next_vfunc(const TreeModel::iterator& iter, TreeModel::iterator& iter_next) const;
    bool iter_children_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent);
    bool iter_children_vfunc(const TreeModel::iterator& parent, TreeModel::iterator& iter) const;
    bool iter_has_child_vfunc(const GtkTreeIter* iter);
    bool iter_has_child_vfunc(const TreeModel::iterator& iter) const;
    int iter_n_children_vfunc(const GtkTreeIter* iter);
    int iter_n_children_vfunc(const TreeModel::iterator& iter) const;
    int iter_n_root_children_vfunc(void) const;
    bool iter_nth_child_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent, int n);
    bool iter_nth_child_vfunc(const TreeModel::iterator& parent, int n, TreeModel::iterator& iter) const;
    bool iter_nth_root_child_vfunc(int n, TreeModel::iterator& iter) const;
    bool iter_parent_vfunc(GtkTreeIter* iter, const GtkTreeIter* child);
    bool iter_parent_vfunc(const TreeModel::iterator& child, TreeModel::iterator& iter) const;
    void ref_node_vfunc(GtkTreeIter* iter);
    void unref_node_vfunc(GtkTreeIter* iter);
#ifdef OLD_GTKMM
    TreeModel::Path get_path_vfunc(const TreeModel::iterator& iter);
#else
    TreeModel::Path get_path_vfunc(const TreeModel::iterator& iter) const;
#endif
    bool get_iter_vfunc(GtkTreeIter* iter, const TreeModel::Path& path);
    bool get_iter_vfunc(const TreeModel::Path& path, TreeModel::iterator& iter) const;
    void get_value_vfunc(const TreeModel::iterator& iter, int column, GValue* value);
    void do_get_value_vfunc(const TreeModel::iterator& iter, int column, GValue* value) const;
    void get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const;

    // Packet Queue
    void on_packet_queued(const std::string& jid, const std::string& icon);
    void on_queue_emptied(const std::string& jid);
    void on_queue_changed(const PacketQueue::QueueInfo& jid_next,
                          const PacketQueue::QueueInfo& first);
private:
    friend class RosterController_Class;
    static RosterController_Class rostercontroller_class_;

    /**
    * Houses extra information for a path.
    * This is primarily used to cache things that might be slow to lookup a lot.
    */
    struct NodeInfo
    {
        std::string icon;
        std::string prev_icon;
        SigC::Connection fade_connection;
        int fade_percent;
        bool blink;
        bool queued;
        mutable Glib::RefPtr<Gdk::Pixbuf> icon_fade;
    };

    struct DefaultColors
    {
        Gdk::Color fg;
        Gdk::Color bg;
        Gdk::Color sel_fg;
        Gdk::Color sel_bg;
    };

    typedef std::set<RosterNode*> NodeSet;
    typedef std::map<RosterNode*, NodeInfo> NodeInfoMap;

    jabberoo::Session&                          _sess;
    jabberoo::Roster&                           _roster;
    RosterModel*                                _model;
    AvatarManager*                              _avatar_manager;
    NodeInfoMap                                 _node_info;
    bool                                        _hide_offline;
    SortType                                    _sort_type;
    bool                                        _sort_with_groups;
    bool                                        _has_first_refresh;
    DefaultColors                               _default_colors;
    NodeSet                                     _need_update;
    bool                                        _update_running;
    std::string                                 _selected_jid;

    void on_connected(void);
#if 0
    void addRosterItem(jabberoo::Roster::Item& item);
    void delRosterItem(jabberoo::Roster::Item& item);
    Gtk::TreeModel::Path getPathFromIter(const GtkTreeIter* iter);
    void clearRosterItems(ModelItems::iterator mit);

    // Roster signals
    void on_roster_refresh(void);
    void on_roster_pres(const std::string&, bool available, 
            jabberoo::Presence::Type prev_type);
    void on_removing_item(jabberoo::Roster::Item& item);
    void on_updating_item(jabberoo::Roster::Item& item);
    void on_update_done(jabberoo::Roster::Item& item);
#endif

    void refresh();
    void clearIter(GtkTreeIter* iter);
    std::string getJIDIcon(const std::string& jid);
    bool fadeIn(RosterNode* node, int max_fade);
    bool fadeOut(RosterNode* node);
    bool do_roster_updates();
    void set_queue_icon(const std::string& jid, const std::string& icon, bool first = false);
    inline void assign_path(const RosterModel::Path& rpath, TreeModel::Path& tpath) const;
    // RosterModel callbacks
    void on_rostermodel_inserted(RosterNode& node, RosterModel::Path& path);
    void on_rostermodel_updated(RosterNode& node, RosterModel::Path& path);
    void on_rostermodel_removed(RosterNode& node, RosterModel::Path& path);
    
}; // class RosterController

class RosterController_Class : public Glib::Class
{
public:
    struct RosterControllerClass
    {
        GObjectClass parent_class;
    };

    friend class RosterController;

    const Glib::Class& init();

    static void class_init_function(void* g_class, void* class_data);
}; // class RosterController_Class

}; // namespace Gabber

#endif // ROSTERCONTROLLER_HH
