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

#include "RosterModel.hh"
#include "GabberApp.hh"

#include <sigc++/slot.h>

#include <functional>
#include <iterator>

using namespace Gabber;
using namespace jabberoo;

RosterModel::RosterModel(Session& sess) : 
    mSession(sess), mRoster(sess.roster()), mRosterStrategy(NULL)
{
    G_App.evtConnected.connect(SigC::slot(*this, &RosterModel::on_connected));
    mRoster.evtPresence.connect(SigC::slot(*this, &RosterModel::on_roster_pres));
    mRoster.evtRemovingItem.connect(SigC::slot(*this, &RosterModel::on_removing_item));
    mRoster.evtUpdating.connect(SigC::slot(*this, &RosterModel::on_updating_item));
    mRoster.evtUpdateDone.connect(SigC::slot(*this, &RosterModel::on_update_done));
}

RosterModel::~RosterModel()
{
}

RosterNode& RosterModel::get_next_node(RosterNode& node)
{
    RosterNode* parent = node.get_parent();
    RosterNodeList& children(parent == NULL ? mRosterNodes : parent->get_children());
    RosterNodeList::iterator it = std::find(children.begin(), 
        children.end(), &node);
    if (it == children.end() || ++it == children.end())
        throw InvalidNode();

    return *(*it);
}

RosterNode& RosterModel::get_node(Path& path)
{
    RosterNodeList* nl = &mRosterNodes;
    RosterNode* node = NULL;

    for(Path::iterator it = path.begin(); it != path.end(); ++it)
    {
        if(nl == NULL || (guint)(*it + 1) > nl->size())
            throw InvalidPath();
        RosterNodeList::iterator nlit = nl->begin();
        std::advance(nlit, *it);

        if(nlit == nl->end())
            throw InvalidPath();

        node = (*nlit);
        if (!node)
            throw InvalidPath();
        nl = &(node->get_children());
    }

    return *node;
}

RosterModel::Path RosterModel::get_path(RosterNode& node)
{
    RosterModel::Path res;
    RosterNode* cur_node = &node;
    RosterNode* parent = node.get_parent();
    
    while(parent != NULL)
    {
        RosterNodeList& nl(parent->get_children());
        RosterNodeList::iterator it = std::find(nl.begin(), nl.end(),
            cur_node);
        if(it == nl.end())
            throw InvalidNode();
        res.push_front(std::distance(nl.begin(), it));
        cur_node = parent;
        parent = parent->get_parent();
    }
    RosterNodeList::iterator it = std::find(mRosterNodes.begin(), 
        mRosterNodes.end(), cur_node);
    if(it == mRosterNodes.end())
        throw InvalidNode();
    res.push_front(std::distance(mRosterNodes.begin(), it));

    return res;
}

void RosterModel::refresh()
{
    clear();

    on_connected();
}

void RosterModel::clear()
{
    mRosterStrategy->clear();
}

void RosterModel::on_connected()
{
    // When we're not hiding the offline users pump them all in
    std::for_each(mRoster.begin(), mRoster.end(),
        AddStrategyAdapter(mRosterStrategy, mHideOffline));

#if 0 // Debug the roster model
    FILE* fd = fopen("new-roster.txt", "w");
    for(RosterNodeList::iterator it = mRosterNodes.begin();
            it != mRosterNodes.end(); ++it)
    {
        fprintf(fd, "%s\n", static_cast<GroupNode*>(*it)->get_name().c_str());
        if((*it)->has_children())
        {
            RosterNodeList& nl((*it)->get_children());
            for(RosterNodeList::iterator git = nl.begin(); git != nl.end();
                    ++git)
            {
                Roster::Item& item(static_cast<RosterItemNode*>(*git)->get_item());
                fprintf(fd, "\t%s\t[%s]\n", item.getNickname().c_str(), item.getJID().c_str());
            }
        }
    }
    fclose(fd);
#endif
}

void RosterModel::on_roster_pres(const std::string& jid, bool available, 
    Presence::Type prev_type)
{
    Roster::Item& item(mRoster[jid]);

    // do the addition or removal
    if(mHideOffline)
    {
        if(available && (prev_type != Presence::ptAvailable))
        {
            mRosterStrategy->add_roster_item(item);
            return;
        }
        else if(!available && (prev_type == Presence::ptAvailable))
        {
            mRosterStrategy->del_roster_item(item);
            return;
        }
        else if(!available)
        {
            return;
        }
    }

    // Just tell the world that the nodes updated
    for(ItemNodeMap::iterator it = mItemNodes.lower_bound(&item);
        it != mItemNodes.upper_bound(&item); ++it)
    {
        RosterModel::Path path(get_path(*(it->second)));
        signal_node_updated(*(it->second), path);
    }
}

/* XXX TODO: Is there a better way to handle these? */
void RosterModel::on_removing_item(jabberoo::Roster::Item& item)
{
    mRosterStrategy->del_roster_item(item);
}

void RosterModel::on_updating_item(jabberoo::Roster::Item& item)
{
    if(mHideOffline && !item.isAvailable())
        return;

    mRosterStrategy->del_roster_item(item);
}

void RosterModel::on_update_done(jabberoo::Roster::Item& item)
{
    if(mHideOffline && !item.isAvailable())
        return;

    mRosterStrategy->add_roster_item(item);
}

GroupedRosterStrategy::GroupedRosterStrategy(RosterModel& model) :
    mModel(model), mRoster(model.get_roster()), mNodeMap(model.get_item_map())
{
}

GroupedRosterStrategy::~GroupedRosterStrategy()
{
}

static bool GroupLowerBound(const RosterNode* lh, const RosterNode* rh)
{
    return Gabber::Util::sortCaseInsensitive()(
            static_cast<const GroupNode*>(lh)->get_name().c_str(), 
            static_cast<const GroupNode*>(rh)->get_name().c_str());
}

struct FindGroup : public unary_function<const RosterNode*, bool>
{
    FindGroup(const std::string& key_) : key(key_)
    { }
    bool operator()(const RosterNode* node)
    {
        return static_cast<const GroupNode*>(node)->get_name() == key;
    }
    std::string key;
};


void GroupedRosterStrategy::add_roster_item(jabberoo::Roster::Item& item)
{
    for(jabberoo::Roster::Item::iterator iit = item.begin(); 
        iit != item.end(); ++iit)
    {
        GroupNode* gnode = NULL;
        RosterModel::Path ins_path(2);

        // We have to make sure we have the group
        RosterNodeList::iterator rnit = std::find_if(mRoster.begin(), 
            mRoster.end(), FindGroup(*iit) );
        if(rnit == mRoster.end())
        {
            // We don't have it yet, so add it
            gnode = new GroupNode(*iit);
            rnit = std::lower_bound(mRoster.begin(), mRoster.end(), 
                gnode, GroupLowerBound);
            int pos = std::distance(mRoster.begin(), rnit);
            mRoster.insert(rnit, gnode);
            ins_path[0] = pos;
            RosterModel::Path gpath(1, pos);
            mModel.signal_node_inserted(*gnode, gpath);
        }
        else
        {
            gnode = static_cast<GroupNode*>(*rnit);
            ins_path[0] = std::distance(mRoster.begin(), rnit);
        }

        // Add the actual item node to the group
        RosterItemNode* new_node = new RosterItemNode(item, gnode);
        RosterNodeList& nl(gnode->get_children());
        RosterNodeList::iterator nlit = std::lower_bound(nl.begin(), 
            nl.end(), new_node, sortNicknameJID());
        ins_path[1] = std::distance(nl.begin(), nlit);
        nl.insert(nlit, new_node);
        mNodeMap.insert(RosterModel::ItemNodeMap::value_type(&item, new_node));
        mModel.signal_node_inserted(*new_node, ins_path);
    }
}

typedef std::pair<RosterNode*, RosterModel::Path> CleanupPair;

static bool DelPathSort(const CleanupPair& lhs_p, const CleanupPair& rhs_p)
{
    const RosterModel::Path& lhs(lhs_p.second);
    const RosterModel::Path& rhs(rhs_p.second);

    if(lhs[0] == rhs[0])
    {
        if(lhs.size() == 2 && rhs.size() == 2)
            return lhs[1] > rhs[1];
        else if(lhs.size() == 1 && rhs.size() == 2)
            return false;
        else
            return true;
    }

    return lhs[0] > rhs[0];
}

void GroupedRosterStrategy::del_roster_item(jabberoo::Roster::Item& item)
{
    // This needs to be a pair of node to path and remove the nodes, but don't
    // delete them.  Iterate them through the signal_node_removed and then loop
    // through and delete the nodes for real.  This allows the node specific
    // lookups from the model user.
    typedef std::vector<CleanupPair> CleanupArray;
    CleanupArray cleanups;

    for(RosterModel::ItemNodeMap::iterator it = mNodeMap.lower_bound(&item);
        it != mNodeMap.upper_bound(&item); ++it)
    {
        RosterItemNode* node = static_cast<RosterItemNode*>(it->second);
        GroupNode* gnode = static_cast<GroupNode*>(node->get_parent());
        RosterNodeList& children(gnode->get_children());

        cleanups.push_back(make_pair(node, mModel.get_path(*node)));
        children.remove(node);

        if(children.empty())
        {
            cleanups.push_back(make_pair(gnode, mModel.get_path(*gnode)));
            mRoster.remove(gnode);
        }
    }

    mNodeMap.erase(&item);

    std::sort(cleanups.begin(), cleanups.end(), DelPathSort);
    for(CleanupArray::iterator it = cleanups.begin(); 
            it != cleanups.end(); ++it)
    {
        mModel.signal_node_removed(*(it->first), it->second);
        delete it->first;
    }
}

void GroupedRosterStrategy::clear()
{
    RosterModel::Path path(2);
    path[0] = mRoster.size() - 1;
    RosterNodeList::reverse_iterator it = mRoster.rbegin();
    while(it != mRoster.rend())
    {
        RosterNodeList::reverse_iterator gnext = it;
        ++gnext;

        GroupNode* gnode = static_cast<GroupNode*>(*it);
        RosterNodeList& nodes( gnode->get_children() );
        path[1] = nodes.size() - 1;
        RosterNodeList::reverse_iterator iit = nodes.rbegin();
        while(iit != nodes.rend())
        {
            RosterNodeList::reverse_iterator next = iit;
            ++next;
            RosterNode* node(*iit);
            // XXX Needs a forward iterator but might not be needed
            //nodes.erase(iit);
            mModel.signal_node_removed(*node, path);
            delete node;
            --(path[1]);
            iit = next;
        }

        // XXX Needs a forward iterator but might not be needed
        //mRoster.erase(it);
        
        it = gnext;
        RosterModel::Path gpath(1);
        gpath[0] = path[0];
        mModel.signal_node_removed(*gnode, gpath);
        delete gnode;
        --(path[0]);
    }

    mRoster.clear();
    mNodeMap.clear();
}
