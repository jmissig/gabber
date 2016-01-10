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
 *  Copyright (c) 2002-2004 Julian Missig
 */
#ifndef _INCL_GABBER_ROSTERMODEL_HH_
#define _INCL_GABBER_ROSTERMODEL_HH_

#include "GabberUtility.hh"

#include <jabberoo/roster.hh>
#include <jabberoo/session.hh>
#include <jabberoo/JID.hh>
#include <jabberoo/presence.hh>

#include <sigc++/object.h>
#include <sigc++/signal.h>

#include <list>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <functional>

namespace Gabber {

class RosterNode;
typedef std::list<RosterNode*> RosterNodeList;

/**
* Represents a single node in the displayable roster.
*
* This is derived by the actual nodes to provide more specific
* information.
*/
class RosterNode
{
public:
    RosterNode() : mParent(NULL)
    { }
    virtual ~RosterNode() 
    {
        // TODO: XXX NB cleanup the mChildren
    }
    /**
    * Returns whether this node is a group.
    */
    virtual std::string get_type(void) = 0;
    /**
    * Returns whether this node has children.
    */
    virtual bool has_children(void)
    {
        if(mChildren.empty())
            return false;
        return true;
    }
    /**
    * Gets a list of the child nodes.
    */
    virtual RosterNodeList& get_children(void)
    { return mChildren; }
    /**
    * Gets a pointer to the parent node of the current node.
    */
    virtual RosterNode* get_parent(void)
    { return mParent; }
protected:
    RosterNodeList mChildren;
    RosterNode* mParent;
};

/**
* Represents a group of RosterItemNodes
*/
class GroupNode : public RosterNode
{
public:
    GroupNode(const std::string& name) : mName(name)
    { }
    
    ~GroupNode()
    { }

    /**
    * Get the name of the group
    */
    std::string get_name(void) const
    { return mName; }

    // RosterNode impl
    std::string get_type(void)
    { return "group"; }
private:
    std::string mName;
};

/**
* Encapsulates a jabberoo::Roster::Item
*/
class RosterItemNode : public RosterNode
{
public:
    RosterItemNode(jabberoo::Roster::Item& item, GroupNode* parent) : 
        mItem(item)
    { mParent = parent; }
    ~RosterItemNode()
    { }

    /**
    * Access to the underlying jabberoo::Roster::Item
    */
    jabberoo::Roster::Item& get_item(void)
    { return mItem; }

    const jabberoo::Roster::Item& get_item(void) const
    { return mItem; }

    // RosterNode impl
    std::string get_type(void)
    { return "item"; }
private:
    jabberoo::Roster::Item& mItem;
};

/**
 * Base class for a strategy of RosterModel organization.
 */
class RosterStrategy
{
public:
    virtual ~RosterStrategy() { }
    virtual void add_roster_item(jabberoo::Roster::Item& item) = 0;
    virtual void del_roster_item(jabberoo::Roster::Item& item) = 0;
    virtual void clear(void) = 0;
};

/**
* Controls the access to organized RosterNode instances.
*/
class RosterModel : public SigC::Object
{
public:
    typedef std::deque<int> Path;
    typedef std::multimap<jabberoo::Roster::Item*, RosterNode*> ItemNodeMap;

    struct InvalidNode { };
    struct InvalidPath { };
public:
    RosterModel(jabberoo::Session& sess);
    virtual ~RosterModel();

    /**
    * Sets the strategy for ordering and controller model nodes.
    *
    * The RosterModel will assume control of the specified RosterStrategy and
    * clean it up when the strategy is switched or the model is destroyed.
    */
    void set_strategy(RosterStrategy* strat)
    {
        if(mRosterStrategy)
            delete mRosterStrategy;
        mRosterStrategy = strat;
    }
    /**
    * Retrieves the RosterNodeList of the model.
    */
    RosterNodeList& get_roster(void)
    { return mRosterNodes; }
    /**
    * Retrieves a map of JIDs to Path points they exist at.
    */
    ItemNodeMap& get_item_map(void)
    { return mItemNodes; }
    /**
    * Get the next sibling RosterNode of the specified RosterNode
    */
    RosterNode& get_next_node(RosterNode& node);
    /**
    * Get the RosterNode at the specified path.
    */
    RosterNode& get_node(Path& path);
    /**
    * Get the Path to the speicified RosterNode
    */
    Path get_path(RosterNode& node);
    /**
    * Refresh the model from the underlying data.
    */
    void refresh(void);
    /**
    * Clear the model of all current data.
    */
    void clear(void);
    /**
    * Set whether offline users should be hidden or not
    */
    void set_hide_offline(bool hide)
    { mHideOffline = hide; }

    SigC::Signal2<void, RosterNode&, Path&> signal_node_inserted;
    SigC::Signal2<void, RosterNode&, Path&> signal_node_updated;
    SigC::Signal2<void, RosterNode&, Path&> signal_node_removed;
private:
    jabberoo::Session& mSession;
    jabberoo::Roster& mRoster;
    RosterNodeList mRosterNodes;
    ItemNodeMap mItemNodes;
    RosterStrategy* mRosterStrategy;
    bool mHideOffline;

    // Gabber connected callback
    void on_connected(void);
    void on_roster_pres(const std::string& jid, bool available, 
            jabberoo::Presence::Type prev_type);
    void on_removing_item(jabberoo::Roster::Item& item);
    void on_updating_item(jabberoo::Roster::Item& item);
    void on_update_done(jabberoo::Roster::Item& item);
};

struct AddStrategyAdapter : 
    public std::unary_function<jabberoo::Roster::Item&, void>
{
    AddStrategyAdapter(RosterStrategy* _strat, bool _hide_offline) : 
        strat(_strat), hide_offline(_hide_offline)
    { }
    void operator()(jabberoo::Roster::Item& item)
    { 
        if(hide_offline && !item.isAvailable())
            return;
        strat->add_roster_item(item); 
    }
    RosterStrategy* strat;
    bool hide_offline;
};

struct DelStrategyAdapter : 
    public std::unary_function<jabberoo::Roster::Item&, void>
{
    DelStrategyAdapter(RosterStrategy* _strat) : strat(_strat) { }
    void operator()(jabberoo::Roster::Item& item)
    { strat->del_roster_item(item); }
    RosterStrategy* strat;
};

/**
* A RosterStrategy that is two deep with ordered groups and then users.
*/
class GroupedRosterStrategy : public RosterStrategy
{
public:
    GroupedRosterStrategy(RosterModel& model);
    ~GroupedRosterStrategy();

    void add_roster_item(jabberoo::Roster::Item& item);
    void del_roster_item(jabberoo::Roster::Item& item);
    void clear(void);
private:
    struct sortNicknameJID :
        std::binary_function<const RosterNode*, const RosterNode*, bool>
    {
        bool operator()(const RosterNode* lhs, const RosterNode* rhs) const
        {
            const jabberoo::Roster::Item& lhsi(
                static_cast<const RosterItemNode*>(lhs)->get_item());
            const jabberoo::Roster::Item& rhsi(
                static_cast<const RosterItemNode*>(rhs)->get_item());
            if (strcasecmp(lhsi.getNickname().c_str(), 
                           rhsi.getNickname().c_str()) == 0)
            {
                return jabberoo::JID::compare(lhsi.getJID().c_str(),
                                              rhsi.getJID().c_str());
            }
            return Util::sortCaseInsensitive()(lhsi.getNickname().c_str(),
                                               rhsi.getNickname().c_str());
        }
    };

    RosterModel& mModel;
    RosterNodeList& mRoster;
    RosterModel::ItemNodeMap& mNodeMap;
};

}; // namespace Gabber

#endif // _INCL_GABBER_ROSTERMODEL_HH_
