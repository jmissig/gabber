#include "AvatarManager.hh"
#include "PubSubNode.hh"
#include "GabberApp.hh"
#include "ConfigPaths.hh"
#include "FTProfile.hh"
#include "S5B.hh"
#include "base64.hpp"
#include "md5.h"
#include <jabberoo/session.hh>

#include <sigc++/bind.h>

#include <fstream>
#include <vector>
#include <string>

#include <sys/stat.h>

#ifdef WIN32
#  include <io.h>
#  define mkdir(a, b) ::mkdir(a)
#endif

using namespace Gabber;

AvatarManager::AvatarManager(jabberoo::Session& sess) : _session(sess)
{
    _avatar_dir = Glib::build_filename(Glib::get_home_dir(),".Gabber");
    if (!Glib::file_test(_avatar_dir, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_avatar_dir.c_str(), 0700);
    }
    _avatar_dir = Glib::build_filename(_avatar_dir, "Avatars");
    if (!Glib::file_test(_avatar_dir, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR))
    {
        mkdir(_avatar_dir.c_str(), 0700);
    }
    _avatar_filename = G_App.getConfigurator().get_string(Keys::avatar.filename);
    _pubsub_jid = G_App.getConfigurator().get_string(Keys::avatar.jid);
    _pubsub_node = G_App.getConfigurator().get_string(Keys::avatar.node);
    _pubsub_hash = G_App.getConfigurator().get_string(Keys::avatar.hash);
    _avatar_iq_xpath = _session.registerXPath(
        "/iq[@type='get']/avatar[@xmlns='http://jabber.org/protocol/avatar']",
        SigC::slot(*this, &AvatarManager::on_avatar_info_request));
    _avatar_pubsub_xpath = _session.registerXPath(
        "/message/events/items/item/avatar[@xmlns='http://jabber.org/protocol/avatar']",
        SigC::slot(*this, &AvatarManager::on_avatar_pubsub_node));
    _avatar_disco_xpath = _session.registerXPath(
        "/iq[@type='get']/query[@xmlns='http://jabber.org/protocol/disco#items' and @node='http://jabber.org/protocol/avatar']",
        SigC::slot(*this, &AvatarManager::on_disco_items_request));

        
}

AvatarManager::~AvatarManager()
{
    _session.unregisterXPath(_avatar_pubsub_xpath);
}

void AvatarManager::publish(const std::string& filename)
{
    // This is a configuration error
    if(_pubsub_jid.empty())
        return;

    if(_pubsub_node.empty())
    {
        do_create(filename);
        return;
    }

    // If we don't have a stored hash we can go ahead and publish
    do_publish(filename);
}

Glib::RefPtr<Gdk::Pixbuf> AvatarManager::get(const Glib::ustring& jid)
{
    AvatarMap::iterator it = _avatars.find(jid);
    if(it == _avatars.end())
        throw NotFound();
    return it->second.avatar;
}

void AvatarManager::retrieve(const Glib::ustring& jid, AvatarCallback cb)
{
    std::string ujid = jabberoo::JID::getUserHost(jid);
    _callbacks.insert(CallbackMap::value_type(ujid, cb));
    
    // First step in getting an avatar is discoing to figure out the pubsub
    // servers node and jid
    try
    {
        jabberoo::DiscoDB::Item& item(G_App.getSession().discoDB()[jid]);
        on_disco_result(&item, ujid);
    }
    catch(jabberoo::DiscoDB::XCP_NotCached& e)
    {
        G_App.getSession().discoDB().cache(jid,
            "http://jabber.org/protocol/avatar", 
            SigC::bind(SigC::slot(*this, 
                &AvatarManager::on_disco_result), ujid), true);
    }
}

void AvatarManager::on_avatar_pubsub_node(const judo::Element& node)
{
    // XXX TODO: process the node and see if it has an inline element.  If it
    // does we pull it out, add it to our map and then fire the changed
    // signal.  If it's only sipub nodes we have to impl that later.
    std::cout << "Got avatar node: " << node.toString() << std::endl;
}

void AvatarManager::on_create_result(const judo::Element& node, std::string filename)
{
    if(node.getAttrib("type") == "error")
    {
        const judo::Element* error = node.findElement("error");

        if(!error || error->getAttrib("code") != "409")
        {
            std::cout << "info create error: " << node.toString() << std::endl;
            // XXX TODO:  Handle this more fully
            _pubsub_node.clear();
            return;
        }
    }

    G_App.getConfigurator().set(Keys::avatar.node, _pubsub_node);
    std::cout << "Create result: " << node.toString() << std::endl;
    do_publish(filename);
}

void AvatarManager::do_create(const std::string& filename)
{
    // We need to create the nodes to publish to
    std::string jid = jabberoo::JID::getUserHost(_session.getLocalJID());
    _pubsub_node = jid + "/avatar";

    PubSubNode* pnode = PubSubNode::create(_pubsub_node);
    pnode->setTo(_pubsub_jid);
    std::string id = _session.getNextID();
    _session.registerIQ(id, SigC::bind(SigC::slot(*this,
        &AvatarManager::on_create_result), filename));
    pnode->setID(id);
    std::cout << "Creating node: " << pnode->toString() << std::endl;
    _session << *pnode;
    delete pnode;
}

void AvatarManager::on_publish_result(const judo::Element& node,
    std::string filename)
{
    if(node.cmpAttrib("type", "error"))
    {
        const judo::Element* error = node.findElement("error");
        if(error == NULL) return;
        // We can probably recover from a 404
        if(error->cmpAttrib("code", "404"))
        {
            do_create(filename);
            return;
        }
    }

    G_App.getConfigurator().set(Keys::avatar.filename, _avatar_filename);

    std::cout << "Avatar info published: " << node.toString() << std::endl;
}

void AvatarManager::do_publish(const std::string& filename)
{
    FTProfile::FileInfo info;
    info.suggested_name = filename;
    std::ifstream in(_avatar_filename.c_str(), ios::binary);
    info.hash = getMD5(in);
    struct stat s;
    stat(_avatar_filename.c_str(), &s);
    info.size = s.st_size;
    info.desc = "current avatar";
    G_App.getFileTransferManager().publish("current-avatar", info);
    _avatar_filename = filename;
    PubSubNode* pnode = PubSubNode::publish(_pubsub_node, "current-avatar", get_avatar_info_node());
    pnode->setTo(_pubsub_jid);
    std::string id = _session.getNextID();
    pnode->setID(id);
    _session.registerIQ(id, SigC::bind(SigC::slot(*this,
        &AvatarManager::on_publish_result), filename));
    std::cout << "Avatar data publish: " << pnode->toString() << std::endl;
    _session << *pnode;
    delete pnode;
}

void AvatarManager::on_disco_items_request(const judo::Element& elem)
{
    judo::Element iq("iq");
    iq.putAttrib("to", elem.getAttrib("from"));
    iq.putAttrib("type", "result");
    iq.putAttrib("id", elem.getAttrib("id"));
    judo::Element* q = iq.addElement("query");
    q->putAttrib("node", "http://jabber.org/protocol/avatar");
    q->putAttrib("xmlns", "http://jabber.org/protocol/disco#items");
    judo::Element* item = q->addElement("item");
    item->putAttrib("node", _pubsub_node);
    item->putAttrib("jid", _pubsub_jid);

    G_App.getSession() << iq;
}

void AvatarManager::on_disco_result(const jabberoo::DiscoDB::Item* item,
    std::string jid)
{
    std::cout << "on_disco_result for " << jid << std::endl;

    if(item == NULL || !item->hasChildren())
    {
        retrieve_error(jid, "Unable to get pubsub information.");
        return;
    }
    
    jabberoo::DiscoDB::Item* child(*(item->begin()));
    subscribe(child->getJID(), child->getNode());

    judo::Element iq("iq");
    iq.putAttrib("to", item->getJID());
    iq.putAttrib("type", "get");
    std::string id = _session.getNextID();
    iq.putAttrib("id", id);
    judo::Element* avatar = iq.addElement("avatar");
    avatar->putAttrib("xmlns", "http://jabber.org/protocol/avatar");

    _session.registerIQ(id, SigC::slot(*this,
        &AvatarManager::on_avatar_info_result));

    std::cout << "Sending out: " << iq.toString() << std::endl;
    _session << iq;
}

void AvatarManager::subscribe(const std::string& jid, const std::string& node)
{
    PubSubNode* pnode = PubSubNode::subscribe(node,
        jabberoo::JID::getUserHost(_session.getLocalJID()));
    pnode->setTo(jid);
    std::string id = _session.getNextID();
    _session.registerIQ(id, 
        SigC::bind(SigC::slot(*this, &AvatarManager::on_subscribe_result), 
            jid));
    pnode->setID(id);
    _session << *pnode;

    delete pnode;
}

// callback from AVM::subscribe for the result node of a pubsub subscribe
void AvatarManager::on_subscribe_result(const judo::Element& elem, std::string jid)
{
    if(elem.cmpAttrib("type", "error"))
    {
        // XXX TODO:  Make this more descriptive from the error
        retrieve_error(jid, "Unable to retrieve avatar info.");
        return;
    }

    std::cout << "Subscribed to avatar: " << elem.toString() << std::endl;
}

// Tell the user we could not retrieve the avatar and clean it up
void AvatarManager::retrieve_error(const std::string& jid, 
    const std::string& error)
{
    _callbacks[jid](Glib::RefPtr<Gdk::Pixbuf>(), error);
    _callbacks.erase(jid);
}

// reply to people requesting our avatar
void AvatarManager::on_avatar_info_request(const judo::Element& elem)
{
    judo::Element iq("iq");
    iq.putAttrib("id", elem.getAttrib("id"));
    iq.putAttrib("to", elem.getAttrib("from"));
    if(_avatar_filename.empty())
    {
        iq.putAttrib("type", "error");
        judo::Element* error = iq.addElement("error");
        // XXX TODO:  What error code to use here?
        error->putAttrib("code", "409");
    }
    else
    {
        iq.putAttrib("type", "result");
        iq.appendChild(get_avatar_info_node());
    }

    std::cout << "Sending avatar info request: " << iq.toString() << std::endl;
    _session << iq;
}

judo::Element* AvatarManager::get_avatar_info_node()
{
    judo::Element* avatar = new judo::Element("avatar");
    avatar->putAttrib("xmlns", "http://jabber.org/protocol/avatar");
    judo::Element* sipub = avatar->addElement("sipub");
    sipub->putAttrib("xmlns", "http://jabber.org/protocol/si-pub");
    sipub->putAttrib("id", "current-avatar");
    sipub->putAttrib("profile", "http://jabber.org/protocol/si/profile/file-transfer");
    // XXX TODO:  Actually get the mime type
    sipub->putAttrib("mime-type", "image/png");

    FTProfile prof;
    FTProfile::FileInfo& info(prof.getFileInfo());
    info.suggested_name = Glib::path_get_basename(_avatar_filename);
    std::ifstream in(_avatar_filename.c_str(), ios::binary);
    info.hash = getMD5(in);
    struct stat s;
    stat(_avatar_filename.c_str(), &s);
    info.size = s.st_size;
    avatar->appendChild(prof.buildNode());

    return avatar;
}

// callback for the result of an avatar info request
void AvatarManager::on_avatar_info_result(const judo::Element& elem)
{
    if(elem.cmpAttrib("type", "error"))
    {
        retrieve_error(jabberoo::JID::getUserHost(elem.getAttrib("from")), 
                    "Error retrieving avatar info.");
        return;
    }

    // XXX TODO: These should be retrieve errors
    const judo::Element* avatar = elem.findElement("avatar");
    const judo::Element* sipub = avatar->findElement("sipub");
    //const judo::Element* ftprofile = sipub->findElement("file");

    std::string jid = elem.getAttrib("from");
    
    judo::Element iq("iq");
    iq.putAttrib("type", "get");
    iq.putAttrib("to", jid);
    std::string id = _session.getNextID();
    iq.putAttrib("id", id);
    judo::Element* req_sipub = iq.addElement("start");
    req_sipub->putAttrib("id", sipub->getAttrib("id"));
    req_sipub->putAttrib("xmlns", "http://jabber.org/protocol/sipub");

    _session.registerIQ(id, SigC::slot(*this,
        &AvatarManager::on_sipub_result));

    _session << iq;
}

#if 0
void AvatarManager::on_retrieve_data_result(const judo::Element& elem,
    std::string jid)
{
    if(elem.cmpAttrib("type", "error"))
    {
        // XXX TODO:  Make this more descriptive from the error
        retrieve_error(jid, "Unable to retrieve avatar info.");
        return;
    }

    const judo::Element* node = elem.findElement("pubsub");
    node = node->findElement("items");
    node = node->findElement("item");
    node = node->findElement("avatar");
    node = node->findElement("data");

    std::string txt(node->getCDATA());

    Glib::ustring filename = Glib::build_filename(_avatar_dir, "/" + jid);
    ofstream out(filename.c_str(), ios::binary);

    base64::decode(txt.begin(), txt.end(), ostream_iterator<char>(out));
    Glib::RefPtr<Gdk::Pixbuf> avatar =
        Gdk::Pixbuf::create_from_file(filename);
    ifstream in(filename.c_str());
    _avatars.insert(AvatarMap::value_type(jid, AvatarInfo(getMD5(in), avatar)));
    _callbacks[jid](avatar, "");
    _callbacks.erase(jid);
}
#endif

// callback for sending of the sipub request
void AvatarManager::on_sipub_result(const judo::Element& elem)
{
    if(elem.cmpAttrib("type", "error"))
    {
        retrieve_error(elem.getAttrib("from"), "Unable to start avatar file transfer.");
        return;
    }

    const judo::Element* starting = elem.findElement("starting");
    G_App.getFileTransferManager().wait_for(starting->getAttrib("sid"),
        SigC::slot(*this, &AvatarManager::setup_transfer));

    std::cout << "si-pub result: " << elem.toString() << std::endl;
}

void AvatarManager::setup_transfer(const std::string& jid, 
    const std::string& id, SI& si)
{
    FTProfile* ftprofile = static_cast<FTProfile*>(si.getProfile());
    std::cout << "Setting up transfer for " << jid << " id: " << id << std::endl;
    AvatarReceiver* av = new AvatarReceiver(*this, jid,
        ftprofile->getFileInfo().size);
    _receivers.insert(AvatarReceiverMap::value_type(jid, av));

    std::string fil;
    fil = Glib::build_filename(_avatar_dir, jabberoo::JID::getUserHost(jid));
    // XXX For now we assume S5B
    Stream* stream = new S5B(si.getID());
    G_App.getFileTransferManager().receive(jid, fil, stream, si.getID());
    G_App.getFileTransferManager().add_listener(si.getID(), av);
    
    FeatureNegotiation* fneg = si.getFeatureNegotiation();
    fneg->choose("stream-method", "http://jabber.org/protocol/bytestreams");

    // Send the reply
    judo::Element iq("iq");
    iq.putAttrib("to", jid);
    iq.putAttrib("id", id);
    iq.putAttrib("type", "result");
    judo::Element* si_res = si.buildResultNode();
    iq.appendChild(si_res);
    G_App.getSession() << iq;
}

// ------------------------ AvatarReceiver Impl ----------------------
AvatarManager::AvatarReceiver::AvatarReceiver(AvatarManager& avm, const std::string& jid, 
    int sz) : _avm(avm), _jid(jid), _tot_sz(sz), _sz(0)
{
}

void AvatarManager::AvatarReceiver::transfer_update(int sz)
{
    _sz += sz;
}

void AvatarManager::AvatarReceiver::transfer_error(const std::string& msg)
{
    _avm.retrieve_error(_jid, msg);
}

void AvatarManager::AvatarReceiver::transfer_closed()
{
    if(_tot_sz >= _sz)
    {
        std::cout << "Transfer completed" << std::endl;
        // XXX TODO:  Finish this part
    }
    delete this;
}
