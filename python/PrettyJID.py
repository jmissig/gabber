import Judo
import JabberOO
import GabberApp
import gobject
import gtk

TARGET_JID = 0
TARGET_ROSTER_ITEM = 1

class PrettyJID(gtk.EventBox):
    """Generic JID displayer and interaction point.

    This provides a simple way to display a jid or jid selector, as well as a
    drag and drop point."""

    ## Display Types
    dtNick = 0
    dtNickRes = 1
    dtJID = 2
    dtJIDRes = 3
    dtNickJID = 4
    dtNickJIDRes = 5

    def __init__(self, jid, dt, max_size=0):
        gtk.EventBox.__init__(self)
        ## public attributes
        self.select_resource = 0

        ## private
        self.__app = GabberApp.GabberApp()
        self.__jid = jid
        self.__resource = JabberOO.JID.getResource(jid)
        self.__default_jid = jid

        ## get a nickname
        self.__nickname = JabberOO.JID.getUser(jid)
        try:
            self.__nickname = self.__app.session.roster()[JabberOO.JID.getUserHost(jid)].getNickname()
        except IndexError, e:
            pass

        if not self.__resource:
            try:
                self.__default_jid = self.__app.session.presenceDB().find(self.__jid).sender
            except IndexError, e:
                pass

        ## Setup our drag and drop support
        targets_accept = [("text/x-jabber-id", 0, TARGET_JID)]
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_DROP,
            targets_accept, gtk.gdk.ACTION_COPY | gtk.gdk.ACTION_MOVE)

        targets_send = [("text/x-jabber-roster-item", 0, TARGET_ROSTER_ITEM), ("text/x-jabber-id", 0, TARGET_JID)]
        self.drag_source_set(gtk.gdk.BUTTON1_MASK | gtk.gdk.BUTTON3_MASK,
            targets_send, gtk.gdk.ACTION_COPY | gtk.gdk.ACTION_MOVE)

        self.connect("drag-data-received", self.__on_drag_data_received)
        self.connect("drag-data-get", self.__on_drag_data_get)
        self.connect("drag-begin", self.__on_drag_begin)

        ## Create the widgets we'll always have
        self.__hboxPJ = gtk.HBox(0, 4)
        self.add(self.__hboxPJ)
        self.__hboxPJ.show()

        self.__lblPJ = gtk.Label("")
        self.__lblPJ.set_alignment(0.0, 0.5)
        self.__hboxPJ.pack_end(self.__lblPJ, 1, 1, 0)
        self.__lblPJ.show()

        self.__pixPJ = gtk.Image()
        rm = self.__app.resources
        self.__pixPJ.set_from_pixbuf(rm.get(rm.rtPIXBUF, "online"))
        self.__hboxPJ.pack_end(self.__pixPJ, 0, 0, 0)
        self.__pixPJ.show()

        ## Get presence updates for the current default jid
        self.__pres_query = self.__app.session.registerXPath(
            "/presence[@from='" + self.__default_jid + "']", self.__on_presence)

        try:
            p = self.__app.session.presenceDB().findExact(self.__default_jid)
            self.__on_presence(p.getBaseElement())
        except IndexError, e:
            pass

        self.set_display_type(dt, max_size)

    def set_display_type(self, type, max_size = 0):
        self.__type = type

        dt = self.__jid
        
        if type == PrettyJID.dtNickRes and self.__resource and not self.select_resource:
            dt = self.__nickname + "/" + self.__resource
        elif type == PrettyJID.dtNick:
            dt = self.__nickname
        elif type == PrettyJID.dtJID and self.__resource and not self.select_resource:
            dt = JabberOO.JID.getUserHost(self.__jid)
        elif type == PrettyJID.dtJIDRes:
            dt = self.__jid
        elif type == PrettyJID.dtNickJIDRes and self.__resource:
            dt = self.__nickname + "/" + self.__resource
            dt += " (" + JabberOO.JID.getUserHost(self.__jid) + ")"
        elif type == PrettyJID.dtNickJID:
            dt = self.__nickname
            dt += " (" + JabberOO.JID.getUserHost(self.__jid) + ")"

        if max_size and len(dt) > max_size:
            dt = dt[0:max_size] + "..."

        print "Setting display text to ", dt
        self.__lblPJ.set_text(dt)

    def show(self):
        if self.select_resource:
            self.__cboResource = gtk.Combo()
            self.__cboResource.set_usize(100, 0)
            self.__hboxPJ.pack_end(self.__cboResource)
            self.__cboResource.show()

        ## Call our base
        gtk.EventBox.show(self)

    def __on_presence(self, elem):
        show = elem.getChildCData("show")
        if not show:
            show = "online"
        ## update the img icon
        rm = self.__app.resources
        self.__pixPJ.set_from_pixbuf(rm.get(rm.rtPIXBUF, show))

    def __on_drag_data_received(self, w, ctx, x, y, data, info, time):
        ## XXX FIXME
        print "Should make a contact send dialog"

    def __on_drag_data_get(self, w, ctx, data, info, time):
        if info == TARGET_ROSTER_ITEM:
            ## XXX FIXME
            print "Need to send a roster item!"
        else:
            dndata = "jabber:" + self.__jid + "\n"
            data.set(data.target, 8, dndata)

    def __on_drag_begin(self, w, ctx):
        self.drag_source_set_icon_pixbuf(self.__pixPJ.get_pixbuf())
