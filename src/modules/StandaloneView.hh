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

#ifndef INCL_STANDALONE_VIEW_HH
#define INCL_STANDALONE_VIEW_HH

#include "fwd.h"
#include "BaseGabberWindow.hh"
#include "TextParser.hh"

namespace Gabber {

class StandaloneViewManager;

/**
* The view of standalone msgs from a single jid
*/
class StandaloneView : public BaseGabberWindow
{
public:
    StandaloneView(StandaloneViewManager& mgr, const judo::Element& msg);
    StandaloneView(StandaloneViewManager& mgr, const std::string& jid);
    ~StandaloneView();
    
    void close();
    void display_message(const judo::Element& msg);

protected:
    // Button callbacks
    void on_close_clicked();
    void on_reply_clicked();
    void on_next_clicked();
    void on_prev_clicked();
    void on_packet_queued(const std::string& jid, const std::string& icon);
    void display_uris();

private:
    typedef std::list<jabberoo::Message*> ViewMsgMap;
    StandaloneViewManager&      _mgr;
    std::string                 _jid;
    ViewMsgMap                  _msgs;
    ViewMsgMap::iterator        _cur_msg;
    Gtk::Button*                _next_button;
    Gtk::Button*                _prev_button;
    Gtk::Label*                 _subject_lbl;
    Gtk::Label*                 _subject_lbl_lbl;
    Gtk::Label*                 _time_lbl;
    Gtk::Label*                 _time_lbl_lbl;
    Gtk::Table*                 _tbl_uri;
    Gtk::TextView*              _message_txtview;
    Glib::RefPtr<Gtk::TextBuffer> _buffer;
    Glib::RefPtr<Gtk::TextMark> _startmark;
    Glib::RefPtr<Gtk::TextMark> _endmark;
    TextBufferParser*           _bufferparser;

    // Display the current message
    void display();
}; // class StandaloneView

}; // namespace Gabber

#endif // INCL_STANDALONE_VIEW_HH
