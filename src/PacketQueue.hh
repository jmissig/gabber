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

#ifndef INCL_GABBER_PACKETQUEUE_HH
#define INCL_GABBER_PACKETQUEUE_HH

#include <jabberoo/session.hh>
#include <jabberoo/judo.hpp>
#include <jabberoo/JID.hh>

#include <glibmm/ustring.h>

#include <fstream>
#include <string>
#include <list>
#include <map>

namespace Gabber{

class QueueIterator
{
public:
    QueueIterator(judo::Element::iterator begin) : _cur(begin) { }
    ~QueueIterator() { }
    QueueIterator& operator++()
    { ++_cur; return *this; }
    const judo::Element& operator*()
    { return (*(static_cast<judo::Element*>(*_cur))); }
    bool equal(const QueueIterator& other) const
    { return (_cur == other._cur); }

private:
    friend class PacketQueue;
    judo::Element::iterator _cur;
};

inline bool operator!=(const QueueIterator& lhs, const QueueIterator& rhs)
{ return !lhs.equal(rhs); }
inline bool operator==(const QueueIterator& lhs, const QueueIterator& rhs)
{ return lhs.equal(rhs); }

/**
 * Queues incoming packets based on jids.
 */
class PacketQueue
{
public:
    // Exception structs
    struct notQueued { };
    struct invalidQueue { };

    // Data structs
    struct QueueInfo
    {
        QueueInfo(const Glib::ustring& jid, const Glib::ustring& icon, 
                  const Glib::ustring& type):
            jid(jid), icon(icon), type(type)
        { }

        bool operator==(const QueueInfo& rhs)
        {
            return compareUserHostJID(rhs.jid);
        }

        bool operator==(const std::string& rhs)
        {
            return compareUserHostJID(rhs);
        }
        bool compareUserHostJID(const std::string& rhs)
        {
            return jabberoo::JID::getUserHost(jid) == jabberoo::JID::getUserHost(rhs);
        }

        Glib::ustring jid;
        Glib::ustring icon;
        Glib::ustring type;
    };

    struct QueueInfoTypeCompare : std::unary_function<bool, const QueueInfo&>
    {
        QueueInfoTypeCompare(const std::string& type) : type(type) { }
        bool operator()(const QueueInfo& qi)
        { return qi.type == type; }
        std::string type;
    };
        
    typedef QueueIterator iterator;
    typedef std::list<QueueInfo> pqueue;
    typedef pqueue::iterator queue_iterator;

public:
    PacketQueue(jabberoo::Session& sess);
    ~PacketQueue();

    /**
    * Push the element on to the back of the packet list.
    * @param elem The element to add to the list.
    * @param icon The icon that should be associated with this packet.
    */
    void push(judo::Element* elem, const Glib::ustring& icon, 
              const Glib::ustring& type);

    /**
    * Pops the first packet from the queue, for the specified JID,
    * and hands it off to the PacketDispatcher.
    * @param jid The JID (only user@host used) to process for.
    */
    void pop(const std::string& jid);

    /**
    * Pops the first packet from the queue.
    */
    void pop();

    /**
    * Pops the entry specified by the queue iterator.
    */
    void pop(queue_iterator it);

    /**
    * Erase the specified queue entry.
    */
    QueueIterator& erase(QueueIterator& it);

    /**
    * Returns if the queue is empty.
    * @returns true When the queue is fully empty.
    */
    bool empty() const
    { return _queue.empty(); }

    /**
    * Load the pending queue from the disk and fire events for it.
    * This can be called whenever a refresh of the pending events is needed.
    */
    void loadFromDisk();

    /**
    * See if the specified JID has queued packets.
    * @param jid The user@host jid to check.
    * @returns bool true if there are packets queued for the jid.
    */
    bool isQueued(const std::string& jid);

    /**
    * Open a queue and get an iterator to the first entry.
    * close() must be called after usage is complete to ensure there is no leak.
    * @param jid The user@host jid to open.
    */
    QueueIterator open(const std::string& jid);
    /**
    * Get the end iterator for the specified queue.
    * The queue must have been previously opened using the open() call.
    * @param jid The user@host jid to get the queue for.
    */
    QueueIterator end(const std::string& jid);

    /**
    * Close an opened queue, saving the data if it was modified.
    * @param jid The user@host jid to close for.
    */
    void close(const std::string& jid);

    /**
    * Pops all pending queue events.
    */
    void flush();

    pqueue::iterator begin()
    { return _queue.begin(); }

    pqueue::iterator end()
    { return _queue.end(); }

    /// Event that is fired whenever a packet is queued for the specified user.
    SigC::Signal2<void, const std::string&, const std::string&> packet_queued_event;

    /// Event that is fired whenever a packet is emptied for the specified user.
    SigC::Signal1<void, const std::string&> queue_emptied_event;

    /**
    * Fired when a packet is causing queue changes.
    * The first argument is the next packet for the same jid as the changed
    * packet.  The second argument is the first packet on the queue.
    */
    SigC::Signal2<void, const QueueInfo&, const QueueInfo&> queue_changed_event;

    /// Event that is fired whenever the queue wants to be flushed.
    SigC::Signal0<void> queue_flushing_event;
private:
    // types
    struct LoadedQueue
    {
        LoadedQueue(judo::Element* queue) : queue(queue), dirty(false) { }
        judo::Element* queue;
        bool dirty;
    };

    typedef std::map<std::string, LoadedQueue> LoadedQueueMap;

    // Helper functions
    struct fileToElems : public std::unary_function<judo::Node*, void>
    {
        fileToElems(judo::Element& elem) : _elem(elem) {}
        void operator() (judo::Node* node)
        {
            if (node->getType() == judo::Node::ntElement)
            {
                _elem.appendChild(node);
            }
        }
        judo::Element& _elem;
    };

    struct saveToFile : public std::unary_function<judo::Node*, void>
    {
        saveToFile(std::ofstream& ofs);
        void operator() (const judo::Node* node);
        std::ofstream& _ofs;
    };

    // Vars
    jabberoo::Session& _session;
    std::string _gdir;
    pqueue _queue;
    LoadedQueueMap _loaded_queues;

    // Functions
    judo::Element* loadFromDisk(const std::string& jid);
    // Save all to disk
    void saveToDisk();
    void saveToDisk(const std::string& jid, const judo::Element& queue);
    void pop(const std::string& jid, int pos);
};

}; // namespace Gabber

#endif // INCL_GABBER_PACKETQUEUE_HH
