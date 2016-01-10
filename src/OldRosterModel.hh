#ifndef ROSTERMODEL_HH
#define ROSTERMODEL_HH

#include "JabberConnection.hh"
#include "GabberUtility.hh"

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
class RosterModel : public Glib::Object, public Gtk::TreeModel
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

    RosterModel(jabberoo::Session& sess, bool hide_offline = true, 
                bool sort_with_groups = true);
    ~RosterModel();

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
    void setHideOffline(bool hide)
    { _hide_offline = hide; refresh(); }
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
    void get_value_impl(const iterator& row, int column, Glib::ValueBase& value) const;

    Gtk::TreeModelFlags get_flags_vfunc();
    int get_n_columns_vfunc();
    GType get_column_type_vfunc(int index);
    bool iter_next_vfunc(GtkTreeIter* iter);
    bool iter_children_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent);
    bool iter_has_child_vfunc(const GtkTreeIter* iter);
    int iter_n_children_vfunc(const GtkTreeIter* iter);
    bool iter_nth_child_vfunc(GtkTreeIter* iter, const GtkTreeIter* parent, int n);
    bool iter_parent_vfunc(GtkTreeIter* iter, const GtkTreeIter* child);
    void ref_node_vfunc(GtkTreeIter* iter);
    void unref_node_vfunc(GtkTreeIter* iter);
    TreeModel::Path get_path_vfunc(const TreeModel::iterator& iter);
    bool get_iter_vfunc(GtkTreeIter* iter, const TreeModel::Path& path);
    void get_value_vfunc(const TreeModel::iterator& iter, int column, GValue* value);

    // Roster signals
    void on_roster_refresh(void);
    void on_roster_pres(const std::string&, bool available, 
            jabberoo::Presence::Type prev_type);
    void on_removing_item(jabberoo::Roster::Item& item);
    void on_updating_item(jabberoo::Roster::Item& item);
    void on_update_done(jabberoo::Roster::Item& item);

    // Roster XPath
    void on_roster_set(const judo::Element& elem);

    // Packet Queue
    void on_packet_queued(const std::string& jid, const std::string& icon);
    void on_queue_emptied(const std::string& jid);
    void on_queue_changed(const PacketQueue::QueueInfo& jid_next,
                          const PacketQueue::QueueInfo& first);
private:
    friend class RosterModel_Class;
    static RosterModel_Class rostermodel_class_;

    struct sortGroups :
        std::binary_function<const Glib::ustring&, const Glib::ustring&, bool>
    {
        bool operator()(const Glib::ustring& lhs, const Glib::ustring& rhs)
        {
            return (lhs.casefold().compare(rhs.casefold())) < 0;
        }
    };

    struct sortNicknameJID :
        std::binary_function<const jabberoo::Roster::Item*, 
                             const jabberoo::Roster::Item*, bool>
    {
        bool operator()(const jabberoo::Roster::Item* lhs, 
                        const jabberoo::Roster::Item* rhs) const
        {
            if (strcasecmp(lhs->getNickname().c_str(), 
                           rhs->getNickname().c_str()) == 0)
            {
                return jabberoo::JID::compare(lhs->getJID().c_str(),
                                              rhs->getJID().c_str());
            }
            return Util::sortCaseInsensitive()(lhs->getNickname().c_str(),
                                               rhs->getNickname().c_str());
        }
    };

    /**
    * Houses extra information for a roster item.
    * This is primarily used to cache things that might be slow to lookup a lot.
    */
    struct ItemInfo
    {
        std::string icon;
        std::string prev_icon;
        int fade_percent;
        bool blink;
        bool queued;
        Glib::RefPtr<Gdk::Pixbuf> icon_fade;
    };

    struct DefaultColors
    {
        Gdk::Color fg;
        Gdk::Color bg;
        Gdk::Color sel_fg;
        Gdk::Color sel_bg;
    };

    typedef std::set<jabberoo::Roster::Item*, sortNicknameJID> RosterItems;
    typedef std::map<Glib::ustring, RosterItems, sortGroups> ModelItems;
    typedef std::map<jabberoo::Roster::Item*, ItemInfo> RosterItemInfos;

    jabberoo::Session&                          _sess;
    jabberoo::Roster&                           _roster;
    ModelItems                                  _items;
    RosterItemInfos                             _item_info;
    bool                                        _hide_offline;
    SortType                                    _sort_type;
    bool                                        _sort_with_groups;
    bool                                        _has_first_refresh;
    judo::XPath::Query*                         _roster_xpath;
    DefaultColors                               _default_colors;
    RosterItems                                 _need_update;
    bool                                        _update_running;
    std::string                                 _selected_jid;

    void refresh();
    void addRosterItem(jabberoo::Roster::Item& item);
    void delRosterItem(jabberoo::Roster::Item& item);
    Gtk::TreeModel::Path getPathFromIter(const GtkTreeIter* iter);
    void clearIter(GtkTreeIter* iter);
    void clearRosterItems(ModelItems::iterator mit);
    std::string getJIDIcon(const std::string& jid);
    bool fadeIn(jabberoo::Roster::Item* item);
    bool fadeOut(jabberoo::Roster::Item* item);
    bool do_roster_updates();
    void set_queue_icon(const std::string& jid, const std::string& icon);
    
}; // class RosterModel

class RosterModel_Class : public Glib::Class
{
public:
    struct RosterModelClass
    {
        GObjectClass parent_class;
    };

    friend class RosterModel;

    const Glib::Class& init();

    static void class_init_function(void* g_class, void* class_data);
}; // class RosterModel_Class

}; // namespace Gabber

#endif // ROSTERMODEL_HH
