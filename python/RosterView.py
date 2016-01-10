import Judo
import JabberOO
import gtk
import RosterModel
import GabberApp
import Popups

class RosterView:
    def __init__(self, tv, rm):
        self.__view = tv
        self.__model = rm

        tv.set_model(rm)

        self.__selection = tv.get_selection()
        self.__selection.set_mode(gtk.SELECTION_BROWSE)

        column = gtk.TreeViewColumn('Name')
        cell = gtk.CellRendererPixbuf()
        column.pack_start(cell, 0)
        column.set_attributes(cell, pixbuf=RosterModel.COLUMN_ICON)
        cell = gtk.CellRendererText()
        column.pack_start(cell, 1)
        column.set_attributes(cell, text=RosterModel.COLUMN_NAME)
        tv.append_column(column)

        tv.connect("cursor-changed", self.on_cursor_changed)
        tv.connect("button-press-event", self.on_button_press_event)
        tv.connect("row-activated", self.on_row_activated)

        del_user_item = Popups.User().getItem("delete_this_contact")
        del_user_item.connect("activate", self.__on_delete_contact_activate)

        self.__cur_jid = None

    def on_cursor_changed(self, tv):
        store, iter = self.__selection.get_selected()
        jid = store.get_value(iter, RosterModel.COLUMN_JID)

        ## Doh, it's a group
        if not jid:
            self.__cur_jid = None
            return

        self.__cur_jid = self.__get_full_jid(jid)

    def on_button_press_event(self, tv, event):
        if event.button == 3:
            ret = tv.get_path_at_pos(event.x, event.y)
            if not ret or not ret[0]:
                return

            path = ret[0]

            model = self.__model
            jid = model.get_value(model.get_iter(path), RosterModel.COLUMN_JID)
            if not jid:
                return

            if not self.__cur_jid:
                self.__cur_jid = self.__get_full_jid(jid)

            Popups.User().updateSelectedJID(self.__cur_jid)
            Popups.User().popup(None, None, None, event.button, event.time)

    def on_row_activated(self, tv, path, column):
        model = self.__model
        jid = model.get_value(model.get_iter(path), RosterModel.COLUMN_JID)
        ## It's a group, expand/contract it
        if not jid:
            if tv.row_expanded(path):
                tv.collapse_row(path)
            else:
                tv.expand_row(path, 0)
            return

        Popups.User().updateSelectedJID(self.__cur_jid)
        Popups.User().activate()

    def __on_delete_contact_activate(self, item):
        session = GabberApp.GabberApp().session
        item = session.roster()[self.__cur_jid]
        s10n = item.getSubsType()

        ## See if we need to do send unsubscribed
        if s10n == JabberOO.Roster.Subscription.rsFrom or s10n == JabberOO.Roster.Subscription.rsBoth:
            session.send(JabberOO.Presence(self.__cur_jid, JabberOO.Presence.Type.ptUnsubscribed))

        session.roster().deleteUser(self.__cur_jid)

    def __get_full_jid(self, jid):
        full_jid = jid
        try:
            full_jid = GabberApp.GabberApp().session.presenceDB().find(jid).sender
            print "Found",full_jid
            return full_jid
        except IndexError, e:
            return jid
