from BaseGabberWindow import BaseGabberWindow
from JabberOO import Presence
from LoginDlg import LoginDlg
from RosterModel import RosterModel
from RosterView import RosterView
import GabberApp
import gtk

class GabberWin(BaseGabberWindow):
    def __init__(self):
        BaseGabberWindow.__init__(self, "Gabber_win")

        self.__prev_show = 0
        self.__app = GabberApp.GabberApp()

        ## Get all our widgets
        self.__optPres = self.get_widget("Presence_optmenu")
        self.__optPres.set_relief(gtk.RELIEF_NONE)
        self.__appbarMain = self.get_widget("Gabber_appbar")

        ## Setup the roster view
        self.__rostermodel = RosterModel(self.__app)
        tv = self.get_widget("Roster_treeview")
        self.__rosterview = RosterView(tv, self.__rostermodel)

        tv.show()

        ## Our status dialog
        self.status_dlg = BaseGabberWindow("Status_dlg")
        self.status_dlg.window.connect("response", self.__on_status_dlg_response)

        ## Our Add JID dialog
        self.addjid_dlg = BaseGabberWindow("AddJID_dlg")
        self.addjid_dlg.window.connect("response",
            self.__on_addjid_dlg_response)
        w = self.addjid_dlg.get_widget("toggleAdvancedOptions")
        w.connect("toggled", self.__on_addjid_advanced_options)

        ## hookup all the menus
        self.__init_menus()

        ## Connect to events
        self.__app.evtConnecting.connect(self.__on_evtConnecting)
        self.__app.evtConnected.connect(self.__on_evtConnected)
        self.__app.evtDisconnected.connect(self.__on_evtDisconnected)
        self.__app.session.evtMyPresence.connect(self.__on_my_presence_event)

    def __del__(self):
        pass

    def close(self):
        self.__app.quit()

    def __init_menus(self):
        mi = self.get_widget("Quit_item")
        mi.connect("activate", self.__on_quit_item_activate)

        mi = self.get_widget("Login_item")
        mi.set_sensitive(1)
        mi.connect("activate", self.__on_login_item_activate)

        mi = self.get_widget("Logout_item")
        mi.set_sensitive(0)
        mi.connect("activate", self.__on_logout_item_activate)

        ## Add contact
        mi = self.get_widget("AddContact_item")
        mi.connect("activate", self.__on_addcontact_item_activate)

        self.__optPres.set_sensitive(0)
        self.__optPres.get_menu().set_active(0)
        self.__optPres.connect("changed", self.__on_pres_menu_changed)

        ## Hide Offline Contacts
        mi = self.get_widget("OfflineContacts_item")
        mi.connect("activate", self.__on_offlinecontacts_item_activate)

    def __on_evtConnecting(self):
        mi = self.get_widget("Login_item")
        mi.set_sensitive(0)
        mi = self.get_widget("Logout_item")
        mi.set_sensitive(1)

    def __on_evtConnected(self):
        self.__optPres.set_sensitive(1)

    def __on_evtDisconnected(self):
        mi = self.get_widget("Login_item")
        mi.set_sensitive(1)
        mi = self.get_widget("Logout_item")
        mi.set_sensitive(0)

    def __on_quit_item_activate(self, item):
        self.close()

    def __on_login_item_activate(self, item):
        ld = LoginDlg(1)
        ld.evtDoConnect = self.__on_login_dlg_do_connect

    def __on_login_dlg_do_connect(self):
        self.__app.connect()

    def __on_logout_item_activate(self, item):
        self.__app.session.disconnect()

    def __on_addcontact_item_activate(self, item):
        self.addjid_dlg.show()

    def __on_offlinecontacts_item_activate(self, item):
        if item.get_active():
            self.__rostermodel.show_online_only = 1
        else:
            self.__rostermodel.show_online_only = 0
        self.__rostermodel.refresh(1)

    def __on_my_presence_event(self, pres):
        show = pres.getShow()

        if (show == Presence.Show.stChat):
            self.__optPres.get_menu().set_active(1)
        elif (show == Presence.Show.stAway):
            self.__optPres.get_menu().set_active(2)
        elif (show == Presence.Show.stXA):
            self.__optPres.get_menu().set_active(3)
        elif (show == Presence.Show.stDND):
            self.__optPres.get_menu().set_active(4)
        else:
            self.__optPres.get_menu().set_active(0)

    def __on_pres_menu_changed(self, btn):
        if (self.__prev_show == self.__optPres.get_history()):
            return

        tv = self.status_dlg.get_widget("tvStatus")
        tv.get_buffer().set_text("")
        self.status_dlg.show()

    def __on_addjid_dlg_response(self, dlg, resp):
        self.addjid_dlg.hide()
        ent = self.addjid_dlg.get_widget("entJID")
        jid = ent.get_text()
        tv = self.addjid_dlg.get_widget("txtReason")
        tb = tv.get_buffer()
        reason = None
        if tb.get_char_count() > 0:
            reason = tb.get_text(tb.get_start_iter(), tb.get_end_iter(), 0)

        p = Presence(jid, Presence.Type.ptSubRequest)
        if reason:
            p.status = reason

        self.__app.session.send(p)

    def __on_addjid_advanced_options(self, widget):
        frame = self.addjid_dlg.get_widget("frameAdvancedOptions")
        arrow = self.addjid_dlg.get_widget("arrowAdvancedOptions")
        lbl = self.addjid_dlg.get_widget("lblAdvancedOptions")

        if widget.get_active():
            frame.show_all()
            arrow.set_property("arrow-type", gtk.ARROW_DOWN)
            lbl.set_text("Hide Advanced Options")
        else:
            frame.hide()
            arrow.set_property("arrow-type", gtk.ARROW_RIGHT)
            lbl.set_text("Show Advanced Options")

    def __on_status_dlg_response(self, dlg, resp):
        self.status_dlg.hide()

        if (resp == gtk.RESPONSE_CANCEL):
            self.__optPres.set_history(self.__prev_show)
            return

        tv = self.status_dlg.get_widget("tvStatus")
        tb = tv.get_buffer()

        status = tb.get_text(tb.get_start_iter(), tb.get_end_iter(), 0)

        cur = self.__optPres.get_history()
        show = None
        pt = Presence.Type.ptAvailable
        if (cur == 0):
            show = Presence.Show.stOnline
        elif (cur == 1):
            show = Presence.Show.stChat
        elif (cur == 2):
            show = Presence.Show.stAway
        elif (cur == 3):
            show = Presence.Show.stXA
        elif (cur == 4):
            show = Presence.Show.stDND
        elif (cur == 5):
            show = Presence.Show.stOnline
            pt = Presence.Type.ptInvisible

        self.__app.session.send(Presence("", pt, show, status))

        self.__prev_show = cur
