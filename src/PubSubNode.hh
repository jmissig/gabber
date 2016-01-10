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
 *  Copyright (c) 2004 Julian Missig and Thomas Muldowney
 */
#ifndef INCL_GABBER_PUBSUBNODE_HH
#define INCL_GABBER_PUBSUBNODE_HH

#include <jabberoo/packet.hh>

#include <glibmm/ustring.h>

#include <string>
#include <vector>

namespace Gabber {

/// Functionality on pubsub nodes
class PubSubNode : public jabberoo::Packet
{
public:
    PubSubNode(const std::string& node);
    ~PubSubNode();

    /**
    * Generate a creation packet for this node.
    */
    static PubSubNode* create(const std::string& node);
    //static void create(PubSubConfig* config);
    /**
    * Generate a publish packet with the specified payload.
    */
    static PubSubNode* publish(const std::string& node, const std::string& id, 
        judo::Element* payload);
    /**
    * Generate an item retraction from the node.
    * @param id The item id to to remove from the node.
    */
    static PubSubNode* retract(const std::string& node, const std::string& id);
    /**
    * Generate a subscription to the node for the specified jid.
    * jis shoudl be either a bare JID or a full resourced JID.
    */
    static PubSubNode* subscribe(const std::string& node, const std::string& jid);
    /**
    * Generate a request for items in the node.
    */
    static PubSubNode* get_items(const std::string& node,
        std::vector<Glib::ustring>& items);

    const std::string& getNode(void) const
    { return _node; }
    void setNode(const std::string& node)
    { _node = node; }
private:
    std::string _node;
};

}; // namespace Gabber

#endif // INCL_GABBER_PUBSUBNODE_HH
