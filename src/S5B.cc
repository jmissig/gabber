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

#include "S5B.hh"
#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "intl.h"
#include <string>

#include <jabberoo/sha.h>

using namespace Gabber;

S5B::S5B(const std::string& sid) :
    _sid(sid), _state(Stream::Disconnected), _bytes_read(0), _resetting(false)
{
    _xpath_query = GabberApp::getSingleton().getSession().registerXPath(
        "/iq/query[@sid='" + _sid + "' and @xmlns='http://jabber.org/protocol/bytestreams']", SigC::slot(*this, &S5B::on_s5b_node));

    _tcpt.evtConnected.connect(SigC::slot(*this, &S5B::on_connected));
    _tcpt.evtError.connect(SigC::slot(*this, &S5B::on_error));
    _tcpt.evtDisconnected.connect(SigC::slot(*this, &S5B::on_disconnected));
}

S5B::~S5B()
{
    GabberApp::getSingleton().getSession().unregisterXPath(_xpath_query);
    for_each(_hosts.begin(), _hosts.end(), Util::ListDeleter());
}

void S5B::on_s5b_node(const judo::Element& elem)
{
    printf("on_s5b_node: %s\n", elem.toString().c_str());
    switch(_state)
    {
    case Stream::Disconnected:
        process_initial_node(elem);
        break;
    case Stream::Authenticated:
        process_activate_node(elem);
        break;
    default:
        // XXX What is this?
        return;
    };
}

void S5B::close()
{
    printf("S5B::close\n");
    _tcpt.disconnect();
}

void S5B::write(const std::string& buf)
{
    _tcpt.send(buf.c_str());
}

void S5B::write(const char* out, int len)
{
    _tcpt.sendsz(out, len);
}

void S5B::need_write(bool need)
{
    _tcpt.needSend(need);
}

void S5B::connect()
{
    StreamHost* host = NULL;
    // Find a valid host
    while (!host)
    {
        host = _hosts.front();
        if (!host->srvid.empty() && host->port < 0)
        {
            _hosts.pop_front();
            delete host;
            host = NULL;

            // If we run out of hosts bail
            if (_hosts.empty())
            {
                on_error(_("No valid StreamHost found"));
                return;
            }
        }
    }

    do_connect(host);
}

void S5B::do_connect(StreamHost* host)
{
    printf("Connecting to %s\n", host->jid.c_str());
    _resetting = false;
    std::string dest = _sid;
    if ( host->proxy )
        dest += _our_jid + _their_jid;
    else
        dest += _their_jid + _our_jid;
    std::string dest_hash = shahash(dest.c_str());
    _tcpt.setProxy("SOCKS5", host->host, host->port, "", "", false);
    _state = Stream::Connecting;
    _used_host = host;
    _tcpt.connect(dest_hash, 0, false, false);
}

void S5B::listen()
{
    _resetting = false;
    _tcpt.listen("", 0);
    _hosts.push_back(new StreamHost(G_App.getSession().getLocalJID(),
                                    G_App.getTransmitter().getsockname(), "",
                                    _tcpt.getPort(), false));
}   

void S5B::start(const std::string& jid)
{
    _tcpt.evtDataAvailable.connect(SigC::slot(*this, &S5B::on_data_available));
    listen();

    judo::Element iq("iq");
    iq.putAttrib("to", jid);
    std::string id = G_App.getSession().getNextID();
    iq.putAttrib("id", id);
    iq.putAttrib("type", "set");
    iq.appendChild(buildNode());

    G_App.getSession().registerIQ(id, 
        SigC::slot(*this, &S5B::on_host_used_node));

    G_App.getSession() << iq.toString().c_str();

    _their_jid = jid;
    _our_jid = G_App.getSession().getLocalJID();
}

void S5B::addStreamHost(const std::string& jid, const std::string& host, 
                        int port, bool proxy)
{
    _hosts.push_back(new StreamHost(jid, host, "", port, proxy));
}

void S5B::addStreamHost(const std::string& jid, const std::string& srvid, 
                        bool proxy)
{
    _hosts.push_back(new StreamHost(jid, "", srvid, 0, proxy));
}
    
judo::Element* S5B::buildNode()
{
    // XXX get streamhost proxies from the server agent list
    judo::Element* query = new judo::Element("query");
    query->putAttrib("xmlns", "http://jabber.org/protocol/bytestreams");
    query->putAttrib("sid", _sid);
    std::for_each(_hosts.begin(), _hosts.end(), StreamHostNodeBuilder(query));

    return query;
}

void S5B::process_initial_node(const judo::Element& elem)
{
    printf("Processing Initial Node\n");
    const judo::Element* query = elem.findElement("query");

    _id = elem.getAttrib("id");
    _our_jid = elem.getAttrib("to");
    _their_jid = elem.getAttrib("from");
    
    // XXX We just snag one for now
    for(judo::Element::const_iterator it = query->begin(); 
        it != query->end(); ++it)
    {
        if ( (*it)->getName() == "streamhost" && 
             (*it)->getType() == judo::Node::ntElement )
        {
            const judo::Element* streamhost = static_cast<const judo::Element*>(*it);
            int port = -1;
            std::string tmp = streamhost->getAttrib("port");
            if (!tmp.empty())
            {
                port = ::atoi(tmp.c_str());
            }
            _hosts.push_back(new StreamHost(streamhost->getAttrib("jid"),
                streamhost->getAttrib("host"), streamhost->getAttrib("srvid"),
                port, false));
        }
    }

    // This is malformed
    if (_hosts.empty())
    {
        // XXX screw this one
        on_error(_("Malformed packet, no hosts offered."));
        return;
    }

    connect();
}

void S5B::process_activate_node(const judo::Element& elem)
{
    printf("processing activate node\n");
    const judo::Element* query = elem.findElement("query");
    if (!query->findElement("activate"))
    {
        // XXX This is an error!
        on_error(_("No activate found"));
        return;
    }

    _state = Stream::Activated;

    judo::Element iq("iq");
    iq.putAttrib("type", "result");
    iq.putAttrib("to", elem.getAttrib("from"));
    iq.putAttrib("id", elem.getAttrib("id"));

    GabberApp::getSingleton().getSession() << iq;

    _state = Stream::Activated;
    signal_state_change(_state);
}

void S5B::on_connected()
{
    printf("S5B::on_connected\n");

    if (_used_host->proxy)
    {
        printf("Activating proxy\n");
        judo::Element iq("iq");
        iq.putAttrib("type", "set");
        iq.putAttrib("to", _used_host->jid);
        std::string id = G_App.getSession().getNextID();
        iq.putAttrib("id", id);
        judo::Element* query = iq.addElement("query");
        query->putAttrib("xmlns", "http://jabber.org/protocol/bytestreams");
        query->putAttrib("sid", _sid);
        query->addElement("activate", _their_jid);

        jabberoo::Session& sess(G_App.getSession());
        sess.registerIQ(id, SigC::slot(*this, &S5B::on_proxy_activated));
        sess << iq.toString().c_str();
    }
    else
    {
        _tcpt.evtCanSendMore.connect(signal_can_send_more.slot());
        _tcpt.evtDataAvailable.connect(signal_data_available.slot());
        _tcpt.evtDataSent.connect(signal_data_sent.slot());

        judo::Element iq("iq");
        iq.putAttrib("type", "result");
        iq.putAttrib("to", _their_jid);
        iq.putAttrib("id", _id);
        judo::Element* query = iq.addElement("query");
        query->putAttrib("xmlns", "http://jabber.org/protocol/bytestreams");
        query->putAttrib("sid", _sid);
        judo::Element* tmp = query->addElement("streamhost-used");
        tmp->putAttrib("jid", _used_host->jid);

        G_App.getSession() << iq;

        _state = Stream::Authenticated;
        signal_state_change(_state);
    }
}

void S5B::on_error(const std::string& msg)
{
    printf("S5B::on_error: %s\n", msg.c_str());
    if (_state == Stream::Connecting && !_hosts.empty())
    {
        // This host sucked, try another
        StreamHost* host = _hosts.front();
        _hosts.pop_front();
        delete host;

        connect();
        return;
    }

    _state = Stream::Error;
    signal_error(msg);
}

void S5B::on_host_used_node(const judo::Element& elem)
{
    if (elem.cmpAttrib("type", "error"))
    {
        on_error(_("Error negotiating transfer."));
        return;
    }

    const judo::Element* q = elem.findElement("query");
    const judo::Element* shu = q->findElement("streamhost-used");
    std::string shu_jid = shu->getAttrib("jid");
    HostsList::iterator it = std::find_if(_hosts.begin(), _hosts.end(), 
                                          StreamHostFinder(shu_jid));
    if (it == _hosts.end())
    {
        on_error(_("Invalid host selected."));
        return;
    }

    _used_host = *it;
    if ( _used_host->proxy )
    {
        printf("Resetting to use proxy\n");
        _resetting = true;
        _tcpt.disconnect();
        do_connect( (*it) );
    }
    else
    {
        printf("Activated\n");
        _state = Stream::Activated;
        signal_state_change(Stream::Activated);
    }
}

void S5B::on_data_available(const char* data, int sz)
{
    _bytes_read += sz;
    _hdr_buf += std::string(data, sz);

    printf("Bytes read is now %d\n", _bytes_read);

    if (_bytes_read == 3)
    {
        _tcpt.sendsz("\005\000", 2);
        printf("Sent reply header\n");
        return;
    }
    else if (_bytes_read == 50)
    {
        std::string hash_str(_sid + _our_jid + _their_jid);
        std::string hash = std::string(shahash(hash_str.c_str()));
        printf("sid(%s) them(%s) us(%s)\n", _sid.c_str(), _their_jid.c_str(), _our_jid.c_str());
        printf("hash: %s\n", _hdr_buf.substr(8, 40).c_str());
        printf("computed hash: %s\n", shahash(hash_str.c_str()));
        printf("CMD: %d\n", _hdr_buf.substr(4,1)[0]);
        printf("ATYPE: %d\n", _hdr_buf.substr(6,1)[0]);
        if (_hdr_buf.substr(4,1)[0] == 1 && _hdr_buf.substr(6,1)[0] == 3 &&
            hash == _hdr_buf.substr(8, 40))
        {
            _tcpt.sendsz("\005\000\000\003\050", 5);
            _tcpt.send(hash.c_str());
            _tcpt.sendsz("\000\000", 2);
            printf("Sent response header\n");
            _state = Stream::Open;
            signal_state_change(_state);
        }
        else
        {
            printf("Closing conn, bad header\n");
            _tcpt.disconnect();
            on_error(_("Unable to negotiate transfer."));
        }
        _hdr_buf.clear();
        return;
    }
}

void S5B::on_disconnected()
{
    printf("S5B::on_disconnected\n");
    if (!_resetting)
    {
        signal_closed();
        // No more at this point
        delete this;
    }
}

void S5B::on_proxy_activated(const judo::Element& elem)
{
    printf("Proxy activated\n");
    if (elem.getAttrib("type") != "result")
    {
        _state = Error;
        on_error(_("Could not activate proxy."));
        return;
    }

    _state = Stream::Activated;
    signal_state_change(_state);

    _tcpt.evtCanSendMore.connect(signal_can_send_more.slot());
    _tcpt.evtDataAvailable.connect(signal_data_available.slot());
    _tcpt.evtDataSent.connect(signal_data_sent.slot());
}

void S5B::proxy_conn_complete(const char* buf, int bytes)
{
}
