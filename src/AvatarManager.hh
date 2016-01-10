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
 *  Copyright (c) 2004 Thomas Muldowney and Julian Missig
 */

#ifndef INCL_GABBER_AVATARMANAGER_HH
#define INCL_GABBER_AVATARMANAGER_HH

#include "FileTransferManager.hh"
#include "StreamInitiation.hh"

#include <jabberoo/XPath.h>
#include <jabberoo/discoDB.hh>
#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <sigc++/slot.h>
#include <sigc++/signal.h>

#include <map>

namespace jabberoo {
    class Session;
};

namespace Gabber {

class AvatarManager : public SigC::Object
{
public:
    typedef SigC::Slot2<void, Glib::RefPtr<Gdk::Pixbuf>, Glib::ustring> AvatarCallback;
    struct NotFound { };
    struct InvalidAvatar { };
    
    AvatarManager(jabberoo::Session& sess);
    ~AvatarManager();

    /**
    * Publish the specified avatar.
    * If the avatar is not valid for publishing an InvalidAvatar exception
    * will be thrown with a descriptive error.
    */
    void publish(const std::string& filename);

    /**
    * Get the Pixbuf associated with the JID.
    * If there is no information for the JID currently a NotFound exception is
    * thrown and the retrieve method can be used to get it.
    */
    Glib::RefPtr<Gdk::Pixbuf> get(const Glib::ustring& jid);

    void retrieve(const Glib::ustring& jid, AvatarCallback cb);

    /// Signal for when a user's avatar has changed.
    SigC::Signal2<void, const Glib::ustring&, Glib::RefPtr<Gdk::Pixbuf> > signal_avatar_changed;

private:
    struct AvatarInfo
    {
        AvatarInfo(const std::string& _hash, Glib::RefPtr<Gdk::Pixbuf> _pb) :
            hash(_hash), avatar(_pb)
        { }
        std::string hash;
        Glib::RefPtr<Gdk::Pixbuf> avatar;
    };

    class AvatarReceiver : public FileTransferManager::FileTransferListener
    {
    public:
        AvatarReceiver(AvatarManager& avm, const std::string& jid, int sz);
        virtual ~AvatarReceiver() { }
        void transfer_update(int sz);
        void transfer_closed();
        void transfer_error(const std::string& msg);
    private:
        AvatarManager& _avm;
        std::string _jid;
        int _tot_sz;
        int _sz;
    };

    friend class AvatarReceiver;

    typedef std::map<Glib::ustring, AvatarInfo> AvatarMap;
    typedef std::map<Glib::ustring, AvatarCallback> CallbackMap;
    typedef std::map<Glib::ustring, AvatarReceiver*> AvatarReceiverMap;

    AvatarMap _avatars;
    CallbackMap _callbacks;
    AvatarReceiverMap _receivers;
    jabberoo::Session& _session;
    std::string _avatar_dir;
    std::string _avatar_filename;
    Glib::ustring _pubsub_jid;
    Glib::ustring _pubsub_node;
    Glib::ustring _pubsub_hash;
    judo::XPath::Query* _avatar_pubsub_xpath;
    judo::XPath::Query* _avatar_iq_xpath;
    judo::XPath::Query* _avatar_disco_xpath;

    // pubsub creation
    void do_create(const std::string& filename);
    void on_create_result(const judo::Element& node, std::string filename);
    // publishing
    void do_publish(const std::string& filename);
    void on_publish_result(const judo::Element& node, std::string filename);
    // retrieval
    void on_disco_items_request(const judo::Element& elem);
    void on_disco_result(const jabberoo::DiscoDB::Item* item, std::string jid);
    void subscribe(const std::string& jid, const std::string& node);
    void on_subscribe_result(const judo::Element& node, std::string jid);
    void retrieve_complete(const std::string& jid);
    void retrieve_error(const std::string& jid, const std::string& error);
    void on_avatar_info_result(const judo::Element& node);
    void on_sipub_result(const judo::Element& elem);
    void setup_transfer(const std::string& jid, const std::string& id, SI& si);
    // callbacks
    void on_avatar_info_request(const judo::Element& elem);
    void on_avatar_pubsub_node(const judo::Element& node);
    // util
    judo::Element* get_avatar_info_node(void);
};

}; // namespace Gabber

#endif // INCL_GABBER_AVATARMANAGER_HH
