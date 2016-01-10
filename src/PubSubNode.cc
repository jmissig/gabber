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

#include "PubSubNode.hh"

using namespace Gabber;

PubSubNode::PubSubNode(const std::string& node) : Packet("iq"), _node(node)
{
}

PubSubNode::~PubSubNode()
{
}

PubSubNode* PubSubNode::create(const std::string& node)
{
    PubSubNode* res = new PubSubNode(node);
    judo::Element& pnode = res->getBaseElement();
    pnode.putAttrib("type", "set");
    judo::Element* pubsub = pnode.addElement("pubsub");
    pubsub->putAttrib("xmlns", "http://jabber.org/protocol/pubsub");
    judo::Element* create = pubsub->addElement("create");
    if(!node.empty())
        create->putAttrib("node", node);

    return res;
}

PubSubNode* PubSubNode::publish(const std::string& node, 
        const std::string& id, judo::Element* payload)
{
    PubSubNode* res = new PubSubNode(node);
    judo::Element& pnode = res->getBaseElement();
    pnode.putAttrib("type", "set");
    judo::Element* pubsub = pnode.addElement("pubsub");
    pubsub->putAttrib("xmlns", "http://jabber.org/protocol/pubsub");
    judo::Element* publish = pubsub->addElement("publish");
    publish->putAttrib("node", node);
    judo::Element* item = publish->addElement("item");
    item->putAttrib("id", id);
    item->appendChild(payload);

    return res;
}

PubSubNode* PubSubNode::retract(const std::string& node, const std::string& id)
{
    PubSubNode* res = new PubSubNode(node);
    judo::Element& pnode = res->getBaseElement();
    pnode.putAttrib("type", "set");
    judo::Element* pubsub = pnode.addElement("pubsub");
    pubsub->putAttrib("xmlns", "http://jabber.org/protocol/pubsub");
    judo::Element* retract = pubsub->addElement("retract");
    retract->putAttrib("node", node);
    judo::Element* item = retract->addElement("item");
    item->putAttrib("id", id);

    return res;
}

PubSubNode* PubSubNode::subscribe(const std::string& node, const std::string& jid)
{
    PubSubNode* res = new PubSubNode(node);
    judo::Element& pnode = res->getBaseElement();
    pnode.putAttrib("type", "set");
    judo::Element* pubsub = pnode.addElement("pubsub");
    pubsub->putAttrib("xmlns", "http://jabber.org/protocol/pubsub");
    judo::Element* subscribe = pubsub->addElement("subscribe");
    subscribe->putAttrib("node", node);
    subscribe->putAttrib("jid", jid);

    return res;
}

PubSubNode* PubSubNode::get_items(const std::string& node, 
        std::vector<Glib::ustring>& items)
{
    PubSubNode* res = new PubSubNode(node);
    judo::Element& pnode = res->getBaseElement();
    pnode.putAttrib("type", "get");
    judo::Element* pubsub = pnode.addElement("pubsub");
    pubsub->putAttrib("xmlns", "http://jabber.org/protocol/pubsub");
    judo::Element* itemsn = pubsub->addElement("items");
    itemsn->putAttrib("node", node);
    for(std::vector<Glib::ustring>::iterator it = items.begin(); 
            it != items.end(); ++it)
    {
        judo::Element* item = itemsn->addElement("item");
        item->putAttrib("id", *it);
    }

    return res;
}
