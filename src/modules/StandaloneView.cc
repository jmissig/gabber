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
 *  Copyright (c) 2002-2004 Julian Missig
 */

#include "StandaloneView.hh"

#include "StandaloneSendDlg.hh"
#include "StandaloneViewManager.hh"
#include "GabberApp.hh"
#include "GabberUtility.hh"
#include "GabberWidgets.hh"
#include "LogManager.hh"
#include "PacketQueue.hh"
#include "TextParser.hh"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>
#include "intl.h"

using namespace Gabber;

StandaloneView::StandaloneView(StandaloneViewManager& mgr, 
                               const judo::Element& msg) : 
    BaseGabberWindow("StandaloneMsgRecv_dlg"), _mgr(mgr),
    _jid(msg.getAttrib("from"))
{
     _tbl_uri = NULL;
     
    // Add the PrettyJID
    Gtk::HBox* hbox;
    get_widget("JIDInfo_hbox", hbox);
    PrettyJID* pj = Gtk::manage(new PrettyJID(_jid, "", PrettyJID::dtNickJID));
    hbox->pack_end(*pj);
    getGtkWindow()->set_title(
        Util::substitute(_("Message from %s"), pj->get_nickname()));
    pj->show();

    // Disable the buttons
    get_widget("Next_btn", _next_button);
    _next_button->set_sensitive(false);
    _next_button->signal_clicked().connect(
        SigC::slot(*this, &StandaloneView::on_next_clicked));
    get_widget("Previous_btn", _prev_button);
    _prev_button->set_sensitive(false);
    _prev_button->signal_clicked().connect(
        SigC::slot(*this, &StandaloneView::on_prev_clicked));

    // Hook up our other buttons
    Gtk::Button* btn;
    get_widget("Reply_btn", btn);
    btn->signal_clicked().connect(
        SigC::slot(*this, &StandaloneView::on_reply_clicked));
    get_widget("Close_btn", btn);
    btn->signal_clicked().connect(
        SigC::slot(*this, &StandaloneView::on_close_clicked));

    // Pull out the other main widgets
    get_widget("Subject_lbl", _subject_lbl);
    get_widget("Subject_lbl_lbl", _subject_lbl_lbl);
    get_widget("Time_lbl", _time_lbl);
    get_widget("Time_lbl_lbl", _time_lbl_lbl);
    get_widget("Message_txtview", _message_txtview);
    
    _buffer = _message_txtview->get_buffer();

    // initialize buffer uri/smiley parser
    _bufferparser = new TextBufferParser(_buffer);
    _startmark = _buffer->create_mark("startmark", _buffer->begin());
    _endmark   = _buffer->create_mark("endmark", _buffer->end());
    
    // Hook up to the good ol' PacketQueue to be notified of incoming messages
    G_App.getPacketQueue().packet_queued_event.connect(SigC::slot(*this, &StandaloneView::on_packet_queued));
    
    // Add the msg and then display it
    display_message(msg);
}

StandaloneView::~StandaloneView()
{
    delete _bufferparser;
    for_each(_msgs.begin(), _msgs.end(), Util::ListDeleter());
}

void StandaloneView::close()
{
    _mgr.releaseView(_jid);

    BaseGabberWindow::close();
}

void StandaloneView::display_message(const judo::Element& msg)
{
    G_App.getLogManager().log(_jid) << msg.toString() << std::endl;

    _cur_msg = _msgs.insert(_msgs.end(), new jabberoo::Message(msg));
    
    display();
}

void StandaloneView::display()
{
     jabberoo::Message* msg = *_cur_msg;
     
     if (_tbl_uri)
     {
          delete _tbl_uri;
          _tbl_uri = NULL;
     }
     
     // if we're at the start, prev is not sensitive
     _prev_button->set_sensitive(_cur_msg != _msgs.begin() && _msgs.size() > 1);
     ViewMsgMap::iterator next = _cur_msg;
     ++next;
     // if we're at the end, next is not sensitive
     if (next == _msgs.end() && !G_App.getPacketQueue().isQueued(jabberoo::JID::getUserHost(_jid)))
          _next_button->set_sensitive(false);
     else
          _next_button->set_sensitive(true);
     
     // display subject
     const Glib::ustring& subject = msg->getSubject();
     if (subject.empty())
     {
          _subject_lbl->hide();
          _subject_lbl_lbl->hide();
     }
     else
     {
          _subject_lbl->set_text(msg->getSubject());
          _subject_lbl->show();
          _subject_lbl_lbl->show();
     }
     
     // display time
     const Glib::ustring& time = msg->getDateTime();
     if (time.empty())
     {
          _time_lbl->hide();
          _time_lbl_lbl->hide();
     }
     else
     {
          _time_lbl->set_text(msg->getDateTime());
          _time_lbl->show();
          _time_lbl_lbl->show();
     }
     
     // display URIs, if any
     display_uris();
     
     // display body
     _buffer->set_text(msg->getBody());
     _buffer->move_mark(_startmark, _buffer->begin());
     _buffer->move_mark(_endmark, _buffer->end());
     _bufferparser->parse_buffer(_startmark, _endmark);
}

void StandaloneView::on_close_clicked()
{
    close();
}

void StandaloneView::on_reply_clicked()
{
     jabberoo::Message* msg = *_cur_msg;
     StandaloneSendDlg::display(G_App, *_thisWindow, *msg);
}

void StandaloneView::on_prev_clicked()
{
    --_cur_msg;

    display();
}

void StandaloneView::on_next_clicked()
{
     ++_cur_msg;
     
     // _cur_msg should only be the end iter if we need to pop a queued message
     if (_cur_msg == _msgs.end())
     {
          G_App.getPacketQueue().pop(jabberoo::JID::getUserHost(_jid));
     }

    display();
}

void StandaloneView::on_packet_queued(const std::string& jid, const std::string& icon)
{
     if (jabberoo::JID::getUserHost(jid) == jabberoo::JID::getUserHost(_jid)
         && icon == "message-standalone.png") // this is cheating, but we know it's ours...
     {
          _next_button->set_sensitive(true);
     }
}

// this should only be called from display()!
// separated just because it's OMG huge
void StandaloneView::display_uris()
{
     // we have to manually walk the element list because there may be several URIs
     int row = 0;
     Gtk::Label* lbl;
     PrettyURI* puri;
     judo::Element::const_iterator it = (*_cur_msg)->getBaseElement().begin();
     for (; it != (*_cur_msg)->getBaseElement().end(); ++it)
     {
          // Case the child element as a tag
          if ((*it)->getType() != judo::Node::ntElement)
               continue;
          judo::Element& x = *static_cast<judo::Element*>(*it);
          
          const std::string& xmlns = x.getAttrib("xmlns");
          if (xmlns.empty())
               continue;
          else if (xmlns == "jabber:x:oob")
          {
               // we have a URI
               
               // Create table if necessary
               if (!_tbl_uri)
               {
                    _tbl_uri = manage(new Gtk::Table(1, 2));
                    _tbl_uri ->set_row_spacings(3);
                    _tbl_uri ->set_col_spacings(3);
                    _tbl_uri ->set_border_width(0);
                    Gtk::VBox* vb;
                    get_widget("Display_vbox", vb);
                    vb->pack_end(*_tbl_uri, false, true, 0);
               }
               
               // Create side label
               lbl = manage(new Gtk::Label(_("Attached URI:"), 0.0, 0.0));
               _tbl_uri->attach(*lbl, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL);
               lbl->show();
               
               // Create URI display
               puri = manage(new PrettyURI(x.getChildCData("url"), x.getChildCData("desc")));
               _tbl_uri->attach(*puri, 1, 2, row, row+1, Gtk::EXPAND|Gtk::FILL, Gtk::AttachOptions(0));
               puri->show();
               
               row++;
          }
          // else if other multiple x namespaces if needed
     }
     
     if (_tbl_uri)
          _tbl_uri->show();
}
