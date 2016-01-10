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

#include "FileTransferManager.hh"
#include "FileTransferRecvDlg.hh"
#include "StreamInitiation.hh"
#include "FTProfile.hh"
#include "FeatureNegotiation.hh"
#include "XData.hh"
#include "S5B.hh"
#include "GabberApp.hh"
#include "intl.h"

#include <sigc++/bind.h>
#include <sigc++/slot.h>

#include <sys/stat.h>

using namespace Gabber;

FileTransferManager::FileTransferManager()
{
    _si_xpath = G_App.getSession().registerXPath(
        "/iq[@type='set']/si[@xmlns='http://jabber.org/protocol/si']", 
        SigC::slot(*this, &FileTransferManager::on_stream_initiation));
    _sipub_xpath = G_App.getSession().registerXPath(
        "/iq[@type='get']/start[@xmlns='http://jabber.org/protocol/sipub']",
        SigC::slot(*this, &FileTransferManager::on_sipub));
}

FileTransferManager::~FileTransferManager()
{
   G_App.getSession().unregisterXPath(_si_xpath);
   G_App.getSession().unregisterXPath(_sipub_xpath);
    for(TransfersMap::iterator it = _transfers.begin(); 
        it != _transfers.end(); ++it)
    {
        delete it->second;
    }
}

std::string FileTransferManager::initiate(const std::string& jid, 
    const std::string& filename, FTProfile::FileInfo& info, 
    const std::string& id, const std::string& proxy)
{
    Transfer* transfer = new Transfer;
    transfer->id = id;
    transfer->jid = jid;
    transfer->proxy.jid = proxy;
    transfer->file.open(filename.c_str(), std::ios::in|std::ios::binary);
    transfer->stream = NULL;
    transfer->stop = false;
    transfer->filename = filename;
    transfer->info = info;

    _transfers.insert(TransfersMap::value_type(transfer->id, transfer));

    // XXX: yo temas, this should try to pull from cache first with a try-catch, no?
    G_App.getSession().discoDB().cache(jid, SigC::bind(
        SigC::slot(*this, &FileTransferManager::on_disco_node), transfer));

    return transfer->id;
}

void FileTransferManager::receive(const std::string& jid, 
    const std::string& filename, Stream* stream, const std::string& id)
{
    Transfer* transfer = new Transfer;
    transfer->jid = jid;
    transfer->filename = Glib::path_get_basename(filename);
    transfer->file.open(filename.c_str(), std::ios::out|std::ios::binary);
    transfer->stream = stream;
    transfer->id = id;
    transfer->stop = false;

    _transfers.insert(TransfersMap::value_type(transfer->id, transfer));

    stream->signal_data_available.connect(SigC::bind(
        SigC::slot(*this, &FileTransferManager::on_receiver_data_available), 
        transfer));
    stream->signal_closed.connect(SigC::bind(
        SigC::slot(*this, &FileTransferManager::on_transfer_closed), transfer));
}

void FileTransferManager::publish(const std::string& id, 
    const FTProfile::FileInfo& info)
{
    std::cout << "Publishing " << id << std::endl;
    _publishes.insert(PublishMap::value_type(id, info));
}

void FileTransferManager::wait_for(const std::string& id, TransferSetupCB cb)
{
    _pending_transfers.insert(WaitForMap::value_type(id, cb));
}

void FileTransferManager::stop(const std::string& id)
{
    TransfersMap::iterator it = _transfers.find(id);
    if (it == _transfers.end())
        return;
    Transfer* transfer = it->second;
    transfer->stop = true;
}
    
bool FileTransferManager::has_listener(const std::string& id)
{
    return (_transfers.count(id) > 0);
}

void FileTransferManager::add_listener(const std::string& id, 
                                       FileTransferListener* listener)
{
    _transfers[id]->listeners.push_back(listener);
}

void FileTransferManager::on_disco_node(const jabberoo::DiscoDB::Item* item, 
    Transfer* transfer)
{
    const jabberoo::DiscoDB::Item::FeatureList& features = item->getFeatureList();
    if( std::find(features.begin(), features.end(), 
                  "http://jabber.org/protocol/si") == features.end() )
    {
        // XXX do oob here
        //XXX TODO error
        printf("No SI\n");
        return;
    }

    if( std::find(features.begin(), features.end(), 
            "http://jabber.org/protocol/si/profile/file-transfer") == 
        features.end() )
    {
        printf("No flie-transfer profile\n");
        // XXX We have to bail, no file transfer
        // XXX TODO error
        return;
    }
    
    SI si;
    FTProfile* ft_profile = new FTProfile;
    ft_profile->getFileInfo() = transfer->info;
    si.setProfile(ft_profile);
    si.setID(transfer->id);
    FeatureNegotiation* fneg = si.getFeatureNegotiation();
    XData::Field* field = new XData::Field;
    field->setVar("stream-method");
    field->setType("list-single");
    XData::Field::Option* opt = new XData::Field::Option;
    opt->label = "s5b";
    opt->value = "http://jabber.org/protocol/bytestreams";
    field->addOption(opt);
    // XXX Support IBB
    fneg->getXData().addField(field);

    judo::Element iq("iq");
    iq.appendChild(si.buildNode());
    iq.putAttrib("type", "set");
    std::string id = G_App.getSession().getNextID();
    iq.putAttrib("id", id);
    iq.putAttrib("to", transfer->jid);

    G_App.getSession().registerIQ(id, SigC::bind(
        SigC::slot(*this, &FileTransferManager::on_si_result_node), transfer));
    
    G_App.getSession() << iq.toString().c_str();
}

void FileTransferManager::send_s5b_offer(Transfer* transfer)
{
    // XXX This has a ton of assumptions, like only S5B
    S5B* s5b = new S5B(transfer->id);
    if (!transfer->proxy.jid.empty())
    {
        s5b->addStreamHost(transfer->proxy.jid, transfer->proxy.host,
                           transfer->proxy.port, true);
    }
    s5b->start(transfer->jid);
    transfer->stream = s5b;

    transfer->stream->signal_state_change.connect(SigC::bind(
        SigC::slot(*this, &FileTransferManager::on_initiator_state_changed), 
        transfer));
}

void FileTransferManager::on_streamhost_result_node(const judo::Element& elem,
    Transfer* transfer)
{
    if (elem.getAttrib("type") == "error")
    {
        // XXX Spew the error and keep going?
        return;
    }
    else if (elem.getAttrib("type") != "result")
    {
        // XXX blow up on invalid results?
        return;
    }

    const judo::Element* query = elem.findElement("query");
    if (!query)
    {
        // XXX just try and go on?
        return;
    }
    const judo::Element* streamhost = query->findElement("streamhost");
    if (!streamhost)
    {
        return;
    }

    if (!streamhost->getAttrib("zeroconf").empty())
    {
        // XXX handle zeroconf?
        return;
    }

    transfer->proxy.host = streamhost->getAttrib("host");
    transfer->proxy.port = atoi(streamhost->getAttrib("port").c_str());

    send_s5b_offer(transfer);
}

void FileTransferManager::on_si_result_node(const judo::Element& elem, 
    Transfer* transfer)
{
    if (elem.cmpAttrib("type", "error"))
    {
        const judo::Element* err_elem = elem.findElement("error");
        if (err_elem && err_elem->cmpAttrib("code", "403"))
        {
            transfer_error(transfer, _("Offered file was declined."));
        }
        else
        {
            transfer_error(transfer, _("An error occured during trying to establish the file transfer."));
        }
        // XXX TODO fire more specific errors
        on_transfer_closed(transfer);
        return;
    }

    // If there is a potential proxy then we need to get its info before
    // preceeding.
    if (transfer->proxy.jid.empty())
    {
        send_s5b_offer(transfer);
    }
    else
    {
        judo::Element iq("iq");
        iq.putAttrib("type", "get");
        iq.putAttrib("to", transfer->proxy.jid);
        std::string id = G_App.getSession().getNextID();
        iq.putAttrib("id", id);

        judo::Element* query = iq.addElement("query");
        query->putAttrib("xmlns", "http://jabber.org/protocol/bytestreams");
        
        G_App.getSession().registerIQ(id, SigC::bind(
            SigC::slot(*this, &FileTransferManager::on_streamhost_result_node), 
            transfer));

        G_App.getSession() << iq.toString().c_str();
    }

}

void FileTransferManager::on_initiator_state_changed(Stream::State state,
    Transfer* transfer)
{
    if ( state == Stream::Activated )
    {
        transfer->stream->signal_data_sent.connect(SigC::bind(
            SigC::slot(*this, &FileTransferManager::on_initiator_data_sent), 
            transfer));
        transfer->stream->signal_can_send_more.connect(SigC::bind(
            SigC::slot(*this, &FileTransferManager::on_initiator_can_send_more),
            transfer));
        transfer->stream->signal_closed.connect(SigC::bind(
            SigC::slot(*this, &FileTransferManager::on_transfer_closed),
            transfer));
        transfer->stream->need_write();
        on_initiator_can_send_more(transfer);
    }
}

void FileTransferManager::on_initiator_data_sent(int sz, Transfer* transfer)
{
    for(ListenerList::iterator it = transfer->listeners.begin(); 
        it != transfer->listeners.end(); ++it)
    {
        (*it)->transfer_update(sz);
    }

    if (transfer->stop && transfer->stream)
    {
        transfer->stream->close();
        transfer->stream = NULL;
    }
}

void FileTransferManager::on_initiator_can_send_more(Transfer* transfer)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    if (transfer->stop)
    {
        if (transfer->stream)
        {
            transfer->stream->close();
            transfer->stream = NULL;
        }
        return;
    }

    char buf[1024];
    int len = 0;
    if (transfer->file.is_open() && !transfer->file.eof())
    {
        std::cout << "Sending some" << std::endl;
        transfer->file.read(buf, 1024);
        len = transfer->file.gcount();
        transfer->stream->write(buf, len);
    }
    else
    {
        std::cout << "can_send_more is closing down" << std::endl;
        if (transfer->stream)
        {
            transfer->stop = true;
            transfer->stream->close();
            transfer->stream = NULL;
        }
    }
}

void FileTransferManager::on_receiver_data_available(const char* data, int sz,
    Transfer* transfer)
{
    transfer->file.write(data, sz);
    // XXX Can this use for_each?
    for(ListenerList::iterator it = transfer->listeners.begin(); 
        it != transfer->listeners.end(); ++it)
    {
        (*it)->transfer_update(sz);
    }

    if (transfer->stop && transfer->stream)
    {
        transfer->stream->close();
        transfer->stream = NULL;
    }
}

void FileTransferManager::on_transfer_closed(Transfer* transfer)
{
    for(ListenerList::iterator it = transfer->listeners.begin(); 
        it != transfer->listeners.end(); ++it)
    {
        (*it)->transfer_closed();
    }
    
    transfer->file.close();
    _transfers.erase(transfer->id);
    delete transfer;
}

void FileTransferManager::transfer_error(Transfer* transfer,
    const std::string& errmsg)
{
    for(ListenerList::iterator it = transfer->listeners.begin(); 
        it != transfer->listeners.end(); ++it)
    {
        (*it)->transfer_error(errmsg);
    }
}

void FileTransferManager::on_stream_initiation(const judo::Element& elem)
{
    // XXX See if this is a type of profile we'll support, if not we can still
    // do a generic save dialog.
    const judo::Element* si_elem = elem.findElement("si");
    SI* si = new SI(*si_elem);

    if ( si->getProfile() && si->getProfile()->getNamespace() == 
            "http://jabber.org/protocol/si/profile/file-transfer" )
    {
        FeatureNegotiation* fneg = si->getFeatureNegotiation();
        if (fneg && !fneg->check("stream-method", 
                "http://jabber.org/protocol/bytestreams"))
        {
            // We don't have a valid stream, send an error
            judo::Element* err = SI::buildInvalidStreamsNode();
            err->putAttrib("to", elem.getAttrib("from"));
            err->putAttrib("id", elem.getAttrib("id"));
            err->putAttrib("type", "error");
            G_App.getSession() << err->toString().c_str();
            delete err;
            delete si;
            return;
        }

        WaitForMap::iterator it = _pending_transfers.find(si->getID());
        if(it != _pending_transfers.end())
        {
            // This callback will set up the transfer for us
            it->second(elem.getAttrib("from"), elem.getAttrib("id"), *si);
            _pending_transfers.erase(si->getID());
            delete si;
        }
        else
        {
            // Figure out the name to save it as
            printf("SI Node: %s\n", elem.toString().c_str());
            FileTransferRecvDlg* dlg = new FileTransferRecvDlg(
                elem.getAttrib("from"), elem.getAttrib("id"), si);
            dlg->show();
        }
    }
    else
    {
        // XXX Should show a save dialog or some such?

        // We can't understand the profile so bail
        judo::Element* err = SI::buildInvalidProfileNode();
        err->putAttrib("to", elem.getAttrib("from"));
        err->putAttrib("id", elem.getAttrib("id"));
        err->putAttrib("type", "error");
        G_App.getSession() << err->toString().c_str();
        delete err;
        delete si;
        return;
    }
}

void FileTransferManager::on_sipub(const judo::Element& elem)
{
    const judo::Element* start = elem.findElement("start");
    std::string sipub_id = start->getAttrib("id");

    judo::Element iq("iq");
    iq.putAttrib("to", elem.getAttrib("from"));
    iq.putAttrib("id", elem.getAttrib("id"));

    PublishMap::iterator it = _publishes.find(sipub_id);
    std::cout << "Looking for publish: " << sipub_id << std::endl;
    if(it == _publishes.end())
    {
        // They supplied an invalid published id, send an error
        iq.putAttrib("type", "error");
        judo::Element* e_start = iq.addElement("start");
        e_start->putAttrib("xmlns", "http://jabber.org/protocol/sipub");
        e_start->addCDATA(sipub_id.c_str(), sipub_id.size());
        judo::Element* e = iq.addElement("error");
        e->putAttrib("code", "405");
        e->putAttrib("type", "modify");
        judo::Element* e_msg = e->addElement("not-acceptable");
        e_msg->putAttrib("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

        G_App.getSession() << iq;

        return;
    }

    // First we send a valid result.
    iq.putAttrib("type", "result");
    judo::Element* starting = iq.addElement("starting");
    std::string id = G_App.getSession().getNextID();
    starting->putAttrib("sid", id);
    G_App.getSession() << iq;

    initiate(elem.getAttrib("from"), it->second.suggested_name, it->second, id);
}
