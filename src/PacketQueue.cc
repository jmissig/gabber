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

#include "GabberUtility.hh"
#include "PacketQueue.hh"

#include <jabberoo/JID.hh>
#include <jabberoo/filestream.hh>

#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <functional>

#ifdef WIN32
#   define mkdir(a, b) ::mkdir(a)
#endif

using namespace Gabber;

PacketQueue::PacketQueue(jabberoo::Session& sess) : 
    _session(sess)
{
    // Make the directories if needed
    _gdir = Glib::build_filename(Glib::get_home_dir(),".Gabber");
    if (!Glib::file_test(_gdir, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_gdir.c_str(), 0700);
    }
    _gdir = Glib::build_filename(_gdir, "PacketQueue");
    if (!Glib::file_test(_gdir, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_gdir.c_str(), 0700);
    }
}

PacketQueue::~PacketQueue()
{
    saveToDisk();
    LoadedQueueMap::iterator it = _loaded_queues.begin();
    while (it != _loaded_queues.end())
    {
        LoadedQueueMap::iterator next = it;
        ++next;

        delete it->second.queue;
        it = next;
    }
}

void PacketQueue::push(judo::Element* elem, const Glib::ustring& icon,
                       const Glib::ustring& type)
{
    elem->putAttrib("gabber:queued", "true");
    std::ofstream fil;
    std::string userhost =
        jabberoo::JID::getUserHost(elem->getAttrib("from"));
    Glib::ustring filename = Glib::build_filename(_gdir, userhost);
    fil.open(filename.c_str(), std::ios::app);
    fil << elem->toString();
    fil.close();

    _queue.push_back(QueueInfo(userhost, icon, type));

    packet_queued_event(userhost, icon);

    return;
}

void PacketQueue::pop(const std::string& jid, int pos)
{
    std::string userhost(jabberoo::JID::getUserHost(jid));
    judo::Element* root = NULL;
    try
    {
        root = loadFromDisk(jid);
    }
    catch( invalidQueue& e )
    {
        std::cerr << "Invalid queue for " << jid << "." << std::endl;
    }
    catch( notQueued& e )
    {
        std::cerr << "Pop called on " << jid << " but queue is empty." << std::endl;
    }

    if (root && !root->empty())
    {
        judo::Element::iterator it = root->begin();
        int cur_pos = 0;
        while (cur_pos < pos) { ++it; ++cur_pos; }
        
        judo::Element* child = static_cast<judo::Element*>(root->detachChild(it));

        saveToDisk(jid, *root);
        delete root;
        
        _session.dispatch(child);

    }
    else
    {
        std::cerr << "There was an error retrieving the queue for " << jid << std::endl;
    }
}

void PacketQueue::pop()
{
    pop(_queue.front().jid);
}

void PacketQueue::pop(const std::string& jid)
{
    std::string userhost = jabberoo::JID::getUserHost(jid);
    _queue.erase(std::find(_queue.begin(), _queue.end(), userhost));
    pop(userhost, 0);
}

void PacketQueue::pop(queue_iterator it)
{
    int pos = 0;
    // XXX I don't like this (temas)
    queue_iterator qit = std::find(_queue.begin(), _queue.end(), it->jid);
    while (qit != it)
    {
        ++pos;
        qit = std::find(++qit, _queue.end(), it->jid);
    }

    std::string jid = it->jid;
    _queue.erase(qit);
    pop(jid, pos);
}

void PacketQueue::loadFromDisk()
{
    Glib::ustring filename = Glib::build_filename(_gdir, "packet_queue.xml");
    if (!Glib::file_test(filename, Glib::FILE_TEST_EXISTS))
    {
        return;
    }

    jabberoo::FileStream fs(filename.c_str());
    if (fs.ParseFile())
    {
        judo::Element* root = fs.getRoot();
        for(judo::Element::iterator it = root->begin(); it != root->end(); ++it)
        {
            if ( ((*it)->getType() == judo::Node::ntElement) || 
                 ((*it)->getName() != "item") )
            {
                judo::Element* child = static_cast<judo::Element*>(*it);
                _queue.push_back(QueueInfo(child->getAttrib("jid"),
                    child->getAttrib("icon"), child->getAttrib("type")));
                QueueInfo& info(_queue.back());
                packet_queued_event(info.jid, info.icon);
            }
        }
    }
    else
    {
        // XXX Warning dialog?
        std::cerr << "Error reading packet queue control file." << std::endl;
    }
}

void PacketQueue::saveToDisk()
{
    judo::Element queue("queue");

    for (pqueue::iterator it = _queue.begin(); it != _queue.end(); ++it)
    {
        judo::Element* child = queue.addElement("item");
        child->putAttrib("jid", it->jid);
        child->putAttrib("icon", it->icon);
        child->putAttrib("type", it->type);
    }

    std::ofstream ofs;
    Glib::ustring filename = Glib::build_filename(_gdir, "packet_queue.xml");
    ofs.open(filename.c_str());
    ofs << queue.toString();
    ofs.close();
}

bool PacketQueue::isQueued(const std::string& jid)
{
    pqueue::iterator it = std::find(_queue.begin(), _queue.end(),
        jabberoo::JID::getUserHost(jid));
    return (it !=  _queue.end());
}
QueueIterator PacketQueue::open(const std::string& jid)
{
    std::string userhost(jabberoo::JID::getUserHost(jid));
    LoadedQueueMap::iterator it = _loaded_queues.find(userhost);
    if (it == _loaded_queues.end())
    {
        judo::Element* root=NULL;
        try
        {
            root = loadFromDisk(jid);
        }
        catch ( notQueued& e )
        {
        }
        catch ( invalidQueue& e )
        {
        }
        std::pair<LoadedQueueMap::iterator, bool> res = 
            _loaded_queues.insert(
                LoadedQueueMap::value_type(userhost, LoadedQueue(root)));
        it = res.first;
    }
        
    return QueueIterator(it->second.queue->begin());
}

QueueIterator PacketQueue::end(const std::string& jid)
{
    std::string userhost(jabberoo::JID::getUserHost(jid));
    LoadedQueueMap::iterator it = _loaded_queues.find(userhost);
    assert(it != _loaded_queues.end());
    
    return QueueIterator(it->second.queue->end());
}

QueueIterator& PacketQueue::erase(QueueIterator& it)
{
    std::string userhost(jabberoo::JID::getUserHost((*it).getAttrib("from")));
    LoadedQueueMap::iterator lqit = _loaded_queues.find(userhost);
    assert(lqit != _loaded_queues.end());

    LoadedQueue& lq(lqit->second);
    int pos = std::distance(lq.queue->begin(), it._cur);

    pqueue::iterator pqit = std::find(_queue.begin(), _queue.end(), userhost);
    for (int i = 0; i < pos; ++i)
    {
        pqit = std::find(++pqit, _queue.end(), userhost);
    }
    _queue.erase(pqit);

    judo::Element::iterator next = it._cur;
    ++next;
    lq.queue->erase(it._cur);
    it._cur = next;
    lq.dirty = true;

    return it;
}

void PacketQueue::close(const std::string& jid)
{
    std::string userhost = jabberoo::JID::getUserHost(jid);
    LoadedQueueMap::iterator it = _loaded_queues.find(userhost);
    assert(it != _loaded_queues.end());

    LoadedQueue& lq(it->second);
    if (lq.dirty)
    {
        saveToDisk(userhost, *lq.queue);
    }

    delete lq.queue;
    _loaded_queues.erase(userhost);
}

void PacketQueue::flush()
{
    queue_flushing_event();
}

judo::Element* PacketQueue::loadFromDisk(const std::string& jid)
{
    Glib::ustring filename = Glib::build_filename(_gdir, 
        jabberoo::JID::getUserHost(jid));
    jabberoo::FileStream fs(filename.c_str());
    if (fs.ParseFile("queue"))
    {
        return fs.getRoot();
    }
    else
    {
        throw invalidQueue();
    }
}

void PacketQueue::saveToDisk(const std::string& jid, const judo::Element& queue)
{
    if (queue.getName() != "queue")
    {
        throw invalidQueue();
    }

    std::string userhost(jabberoo::JID::getUserHost(jid));
    Glib::ustring filename = Glib::build_filename(_gdir, userhost);

    if (queue.empty())
    {
        unlink(filename.c_str());
        _queue.erase(std::remove(_queue.begin(), _queue.end(), userhost), _queue.end());
        queue_emptied_event(userhost);
    }
    else
    {
        std::ofstream fil;
        fil.open(filename.c_str());
        std::for_each(queue.begin(), queue.end(), saveToFile(fil));
        fil.close();
        pqueue::iterator it = std::find(_queue.begin(), _queue.end(), userhost);
        queue_changed_event(*it, _queue.front());
    }
}

// PacketQueue::saveToFile struct
PacketQueue::saveToFile::saveToFile(std::ofstream& ofs) : _ofs(ofs)
{ }

void PacketQueue::saveToFile::operator()(const judo::Node* node)
{
    _ofs << node->toString();
}
