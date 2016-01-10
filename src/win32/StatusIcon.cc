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
 *  Copyright (c) 2003 Julian Missig
 */

#include "StatusIcon.hh"

#include "GabberApp.hh"
#include "JabberConnection.hh"
#include "ResourceManager.hh"
#include "ConfigPaths.hh"

#include "intl.h"
#include <jabberoo/presence.hh>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>

static HWND systray_hwnd=0;
static HICON sysicon=0;
static NOTIFYICONDATA nid;

static Gabber::StatusIcon* statusIcon;

namespace Gabber {

StatusIcon::StatusIcon(GabberWin* gwin)
     : _gwin(gwin), _blinking(false)
{
     // XXX: Find out if we have a status tray *at all*
     // if not, then every minute or so we should check again and then init
     init();
    statusIcon = this;
}

StatusIcon::~StatusIcon()
{
    _timer.disconnect();
    systray_remove_nid();
	DestroyWindow(systray_hwnd);
	if (_blinking)
	    DestroyIcon(_queue_icon);
    DestroyIcon(_normal_icon);
}

void StatusIcon::clicked()
{ 
    if (_blinking)
    {
        G_App.getPacketQueue().pop();
    }
    else
    {
        _gwin->toggle_visibility(); 
    }
}
void StatusIcon::init()
{
	/* dummy window to process systray messages */
	systray_hwnd = systray_create_hiddenwin();

	/* Load icons, and init systray notify icon */
    std::string fname = Glib::build_filename( G_App.getConfigurator().get_string(Keys::acct.pixmapdir), "online.ico");

    _normal_msg = "Gabber";
	_normal_icon = (HICON)LoadImage(GetModuleHandle(NULL), fname.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

	/* Create icon in systray */
	systray_init_icon(systray_hwnd, _normal_icon);

     // Now that we've established all the widgets and whatnot
     // let's hook up to the events we want
     G_App.getSession().evtMyPresence.connect(SigC::slot(*this, &StatusIcon::on_my_presence));
     //_app.evtConnecting.connect(  SigC::slot(*this, &GabberWin::on_evtConnecting));
     //_app.evtConnected.connect(   SigC::slot(*this, &GabberWin::on_evtConnected));
     //_app.evtDisconnected.connect(SigC::slot(*this, &GabberWin::on_evtDisconnected));
    G_App.getPacketQueue().packet_queued_event.connect(SigC::slot(*this, &StatusIcon::on_packet_queued));
    G_App.getPacketQueue().queue_changed_event.connect(SigC::slot(*this, &StatusIcon::on_queue_changed));
    G_App.getPacketQueue().queue_emptied_event.connect(SigC::slot(*this, &StatusIcon::on_queue_emptied));
}

void StatusIcon::on_my_presence(const jabberoo::Presence& p)
{
     // Set the labels
     std::string filename;
     if((p.getShow() == jabberoo::Presence::stInvalid) ||
        (p.getShow() == jabberoo::Presence::stOffline) ||
        p.getShow_str().empty())
          filename = "offline.ico";
     else
          filename = p.getShow_str() + ".ico";
    std::string fname = Glib::build_filename( G_App.getConfigurator().get_string(Keys::acct.pixmapdir), filename);   

    _normal_icon = (HICON)LoadImage(GetModuleHandle(NULL), fname.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    _normal_msg = Gabber::Util::getShowName(p.getShow()) + ": " + p.getStatus();

    systray_change_icon(_normal_icon, _normal_msg.c_str());
    if (!_blinking)
        DestroyIcon(_prev_icon);
}

static LRESULT CALLBACK systray_mainmsg_handler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	static UINT taskbarRestartMsg; 

	switch(msg) {
	case WM_CREATE:
		taskbarRestartMsg = RegisterWindowMessage("TaskbarCreated");
		break;
		
	case WM_TIMER:
		break;

	case WM_DESTROY:
		break;

	case WM_TRAYMESSAGE:
	{
		int type = 0;

		/* Single click */
		if( lparam == WM_LBUTTONUP )
            statusIcon->clicked();
		else
			break;

		break;
	}
	default: 
		if (msg == taskbarRestartMsg) {
			/* explorer crashed and left us hanging... 
			   This will put the systray icon back in it's place, when it restarts */
			Shell_NotifyIcon(NIM_ADD,&nid);
		}
		break;
	}/* end switch */

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
void StatusIcon::on_packet_queued(const std::string& jid, 
                                  const std::string& icon)
{
    std::cout << "PQ" << std::endl;
    if (_blinking)
    {
        return;
    }
    
    _blinking = true;
    std::string filename = icon.substr(0, icon.find("."));
    filename += ".ico";
    std::string fname = Glib::build_filename( G_App.getConfigurator().get_string(Keys::acct.pixmapdir), filename);   
    _queue_icon = (HICON)LoadImage(GetModuleHandle(NULL), fname.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    _queue_msg = jid;
    systray_change_icon(_queue_icon, jid.c_str());
    _timer = Glib::signal_timeout().connect(SigC::slot(*this, &StatusIcon::swapIcons), 500);
}

void StatusIcon::on_queue_changed(const PacketQueue::QueueInfo& jid_first, 
    const PacketQueue::QueueInfo& first)
{
    std::string filename = first.icon.substr(0, first.icon.find("."));
    filename += ".ico";
    std::string fname = Glib::build_filename( G_App.getConfigurator().get_string(Keys::acct.pixmapdir), filename);   
    HICON tmpicon = (HICON)LoadImage(GetModuleHandle(NULL), fname.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

    systray_change_icon(_normal_icon, _normal_msg.c_str());
    DestroyIcon(_queue_icon);
    _queue_icon = tmpicon;
    _queue_msg = first.jid;
    systray_change_icon(_queue_icon, _queue_msg.c_str()); 
}

void StatusIcon::on_queue_emptied(const std::string& jid)
{
    _timer.disconnect();
    _blinking = false;
    
    PacketQueue& pq(G_App.getPacketQueue());
    PacketQueue::queue_iterator it = pq.begin();
    if (it == pq.end())
    {
        systray_change_icon(_normal_icon, _normal_msg.c_str());
        DestroyIcon(_queue_icon);
    }
    else
    {
        on_packet_queued(it->jid, it->icon);
    }
}

bool StatusIcon::swapIcons()
{
    static bool normal = false;
    
    if (!_blinking)
        return false;

    systray_change_icon(normal ? _normal_icon : _queue_icon, 
                        normal ? _normal_msg.c_str() : _queue_msg.c_str());
                        
    normal = !normal;
    
    return true;
}

/* Create hidden window to process systray messages */
HWND StatusIcon::systray_create_hiddenwin() {
	WNDCLASSEX wcex;
	TCHAR wname[32];

	strcpy(wname, "GabberSystrayWin");

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style	        = 0;
	wcex.lpfnWndProc	= (WNDPROC)systray_mainmsg_handler;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon		= NULL;
	wcex.hCursor		= NULL,
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= wname;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	// Create the window
	return (CreateWindow(wname, "", 0, 0, 0, 0, 0, GetDesktopWindow(), NULL, GetModuleHandle(NULL), 0));
}

void StatusIcon::systray_init_icon(HWND hWnd, HICON icon) {
	char* locenc=NULL;

	ZeroMemory(&nid,sizeof(nid));
	nid.cbSize=sizeof(NOTIFYICONDATA);
	nid.hWnd=hWnd;
	nid.uID=0;
	nid.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage=WM_TRAYMESSAGE;
	nid.hIcon=icon;
	locenc=g_locale_from_utf8("Gabber", -1, NULL, NULL, NULL);
	strcpy(nid.szTip, locenc);
	g_free(locenc);
	Shell_NotifyIcon(NIM_ADD,&nid);
}

void StatusIcon::systray_change_icon(HICON icon, const char* text) {
	char *locenc=NULL;
	nid.hIcon = icon;
	locenc = g_locale_from_utf8(text, -1, NULL, NULL, NULL);
	lstrcpy(nid.szTip, locenc);
	g_free(locenc);
	Shell_NotifyIcon(NIM_MODIFY,&nid);
	_prev_icon = sysicon;
    sysicon = icon;
}


void StatusIcon::systray_remove_nid(void) {
	Shell_NotifyIcon(NIM_DELETE,&nid);
}

/*
void 
tntc_tray_create () 
{
}

void tntc_tray_destroy()
{
	systray_remove_nid();
	DestroyWindow(systray_hwnd);
	docklet_remove(TRUE);
}
*/

} // namespace Gabber
