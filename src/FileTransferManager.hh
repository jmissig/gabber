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
 *  Based on Gabber, Copyright (c) 1999-2003 Dave Smith & Julian Missig
 *  Copyright (c) 2002-2003 Julian Missig
 */

#ifndef INCL_FILETRANSFERMANAGER_HH
#define INCL_FILETRANSFERMANAGER_HH

#include "Stream.hh"
#include "FTProfile.hh"
#include "StreamInitiation.hh"

#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <jabberoo/discoDB.hh>

#include <list>
#include <string>
#include <map>
#include <fstream>

namespace Gabber {

class FileTransferManager : public SigC::Object
{
public:
    class FileTransferListener
    {
    public:
        virtual void transfer_update(int sz) = 0;
        virtual void transfer_closed() = 0;
        virtual void transfer_error(const std::string& msg) = 0;
    };

    typedef std::list<FileTransferListener*> ListenerList;
    typedef SigC::Slot3<void, const std::string&, const std::string&, SI&> TransferSetupCB;
public:
    FileTransferManager();
    ~FileTransferManager();

    /** Initiate a transfer of the file to the specified jid.
     * @param jid The jid to send the file to
     * @param filename The actual system file to send
     * @returns string The id for the transfer
     */
    std::string initiate(const std::string& jid, const std::string& filename,
                         FTProfile::FileInfo& info, const std::string& id,
                         const std::string& proxy = "");
    void receive(const std::string& jid, const std::string& filename,
                 Stream* stream, const std::string& id);
    /**
    * Publish a file that can be retrieved via sipub.
    */
    void publish(const std::string& id, const FTProfile::FileInfo& file);
    /**
    * Wait for the incoming id and use the given callback to setup the transfer
    * 
    * In the callback the stream is picked and receive() is called with the
    * correct settings.
    * 
    * @param id The Stream Initiation id to wait for
    * @param cb The callback that will setup the transfer.
    */
    void wait_for(const std::string& id, TransferSetupCB cb);
    void stop(const std::string& sid);
    bool has_listener(const std::string& id);
    void add_listener(const std::string& id, FileTransferListener* listener);
private:
    struct ProxyInfo
    {
        std::string jid;
        std::string host;
        int port;
        std::string srvid;
    };

    struct Transfer
    {
        std::string id;
        std::string jid;
        std::string filename;
        ProxyInfo proxy;
        std::fstream file; 
        Stream* stream;
        ListenerList listeners;
        bool stop;
        FTProfile::FileInfo info;
    };

    typedef std::map<std::string, Transfer*> TransfersMap;
    typedef std::map<std::string, TransferSetupCB> WaitForMap;
private:
    typedef std::map<std::string, FTProfile::FileInfo> PublishMap;

    TransfersMap _transfers;
    PublishMap _publishes;
    WaitForMap _pending_transfers;
    judo::XPath::Query* _si_xpath;
    judo::XPath::Query* _sipub_xpath;

    // Initiation callbacks
    void on_disco_node(const jabberoo::DiscoDB::Item* item, Transfer* transfer);
    void on_streamhost_result_node(const judo::Element& elem, Transfer* transfer);
    void on_si_result_node(const judo::Element& elem, Transfer* transfer);
    void on_initiator_state_changed(Stream::State state, Transfer* transfer);
    void on_initiator_data_sent(int sz, Transfer* transfer);
    void on_initiator_can_send_more(Transfer* transfer);

    // Receiver callbacks
    void on_receiver_data_available(const char* data, int sz, 
                                    Transfer* transfer);

    // Common callbacks
    void on_transfer_closed(Transfer* transfer);

    // Other functions
    void transfer_error(Transfer* transfer, const std::string& errmsg);
    void send_s5b_offer(Transfer* transfer);

     void on_stream_initiation(const judo::Element& elem);
     void on_sipub(const judo::Element& elem);

};

};
#endif //INCL_FILETRANSFERMANAGER_HH
