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

#ifndef INCL_S5B_HH
#define INCL_S5B_HH

#include "Stream.hh"
#include "TCPTransmitter.hh"
#include "GabberUtility.hh"

#include <sigc++/object.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <jabberoo/judo.hpp>
#include <jabberoo/XPath.h>

#include <list>
#include <string>

namespace Gabber {

/**
* Implements SOCKS5 Bytestreams (JEP-0065)
*/
class S5B : public SigC::Object, public Stream
{
public:
    S5B(const std::string& sid);
    ~S5B();

    /**
    * Opens a connection to the StreamHost
    */
    void open()
    { }

    void close();

    void write(const std::string& buf);

    void write(const char* out, int len);

    void need_write(bool need);

    /** Start the stream for the given jid. */
    void start(const std::string& jid);

    void addStreamHost(const std::string& jid, const std::string& host, int port, bool proxy=false);
    void addStreamHost(const std::string& jid, const std::string& zeroconf, bool proxy=false);

    /** Build the initial offer node. */
    judo::Element* buildNode();
    judo::Element* buildResultNode();
protected:
    /**
    * Authenticate the stream.
    * This will generate a state change event or an error event.
    */
    void authenticate();
private:

    struct StreamHost
    {
        StreamHost(const std::string& j, const std::string& h, 
                   const std::string& s, int p, bool proxy) : 
            jid(j), host(h), srvid(s), port(p), proxy(proxy)
        { }
        std::string     jid;
        std::string     host;
        std::string     srvid;
        int             port;
        bool            proxy;
    };

    struct StreamHostFinder : public std::unary_function<StreamHost*, bool>
    {
        StreamHostFinder(const std::string& jid_) : jid(jid_) { }
        bool operator()(StreamHost* host) { return host->jid == jid; }
        std::string jid;
    };

    struct StreamHostNodeBuilder
    {
        StreamHostNodeBuilder(judo::Element* q_) : q(q_) { }
        void operator()(StreamHost* host)
        {
            judo::Element* h = q->addElement("streamhost");
            h->putAttrib("jid", host->jid);
            h->putAttrib("host", host->host);
            h->putAttrib("port", Util::int2string(host->port));
        }
        judo::Element* q;
    };

    typedef std::list<StreamHost*> HostsList;

    std::string     _sid;
    std::string     _id;
    std::string     _their_jid;
    std::string     _our_jid;
    std::string     _error_msg;
    Stream::State   _state;
    HostsList       _hosts;
    TCPTransmitter  _tcpt;
    judo::XPath::Query* _xpath_query;
    int _bytes_read;
    std::string _hdr_buf;
    bool _resetting;
    StreamHost* _used_host;
    
    // callbacks
    void on_s5b_node(const judo::Element& elem);
    void on_connected();
    void on_error(const std::string& msg);

    void connect();
    void do_connect(StreamHost* host);
    void listen();
    void process_initial_node(const judo::Element& elem);
    void process_activate_node(const judo::Element& elem);
    void on_host_used_node(const judo::Element& elem);
    void on_data_available(const char* data, int sz);
    void on_disconnected();
    void on_proxy_activated(const judo::Element& elem);
    void proxy_conn_complete(const char* buf, int len);

}; // class S5B

}; // namespace Gabber

#endif // INCL_S5B_HH
