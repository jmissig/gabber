import gtk
import gobject
import Judo
import JabberOO
import types
from Environment import ENV_VARS
from ResourceManager import ResourceManager

COLUMN_NAME = 0
COLUMN_JID = 1
COLUMN_ICON = 2
COLUMN_SHOW = 3
COLUMN_STATUS = 4
COLUMN_ONLINE_COUNT = 5
COLUMN_ONLINE = 6

class RosterModelNode:
    def __init__(self, name, jid=None):
        ## Parent, None is the top most layer
        self.parent = None
        ## In case we have any under us
        self.children = []
        ## Passed in
        self.name = name
        self.jid = jid
        self.icon = None
        self.show = None
        self.status = None
        self.online_count = 0
        self.online = 0

    def __del__(self):
        ## XXX Stuff to do here later
        for child in self.children:
            del child

    def __getitem__(self, key):
        assert(type(key) is types.IntType, "Must be integer key")
        if key == COLUMN_NAME:
            if not self.jid:
                return self.name + " (" + str(self.online_count) + ")"
            return self.name
        if key == COLUMN_JID:
            return self.jid
        if key == COLUMN_ICON:
            return self.icon
        if key == COLUMN_SHOW:
            return self.show
        if key == COLUMN_STATUS:
            return self.status
        if key == COLUMN_ONLINE_COUNT:
            return self.online_count
        if key == COLUMN_ONLINE:
            return self.online

    def __setitem__(self, key, val):
        assert(type(key) is types.IntType, "Must be integer key")
        if key == COLUMN_ICON:
            self.icon = val
        if key == COLUMN_STATUS:
            self.status = val
        if key == COLUMN_SHOW:
            self.show = val
        if key == COLUMN_ONLINE_COUNT:
            self.online_count = val
        if key == COLUMN_ONLINE:
            self.online = val

    def __cmp__(self, node):
        return cmp(self.name, node.name)

    def append(self, node):
        ## Set their parent to us
        node.parent = self
        self.children.append(node)

    def sort(self):
        self.children.sort()

    def get_path(self):
        cur = self
        path = []
        while cur.parent:
            path.insert(0, cur.parent.children.index(cur))
            cur = cur.parent
        return tuple(path)

    def clear(self):
        for child in self.children:
            self.children.remove(child)
            del child

class RosterModel(gtk.GenericTreeModel):
    def __init__(self, app):
        gtk.GenericTreeModel.__init__(self)
        self.__app = app
        self.__types = (gobject.TYPE_STRING, gobject.TYPE_STRING,
            gtk.gdk.Pixbuf, gobject.TYPE_STRING, gobject.TYPE_STRING,
            gobject.TYPE_INT)
        self.__root = RosterModelNode(None, None)
        self.show_online_only = 1
        ## quick access to whole groups
        self.__groups = {}
        self.__users = {}

        ## Get references to the pixbufs from the global resource cache
        rm = app.resources
        self.__online_pixbuf = rm.get(rm.rtPIXBUF, "online")
        self.__away_pixbuf = rm.get(rm.rtPIXBUF, "away")
        self.__xa_pixbuf = rm.get(rm.rtPIXBUF, "xa")

        app.session.roster().evtRefresh.connect(self.refresh)
        app.session.roster().evtPresence.connect(self.__on_presence)

        app.session.registerXPath(
            "/iq[@type='set']/query[@xmlns='jabber:iq:roster']/item",
            self.__on_roster_set)

        self.__has_refresh = 0

    def on_get_flags(self):
        return 0

    def on_get_n_columns(self):
        return len(self.__types)

    def on_get_column_type(self, column):
        return self.__types[column]

    def on_get_path(self, node):
        return node.get_path()

    def on_get_value(self, node, column):
        return node[column]

    def on_get_iter(self, path):
        if not self.__root.children:
            return None
        val = self.__root
        for x in path:
            val = val.children[x]
        return val

    def on_iter_next(self, node):
        children = node.parent.children
        try:
            return children[children.index(node) + 1]
        except IndexError:
            return None

    def on_iter_children(self, node):
        return node.children[0]

    def on_iter_has_child(self, node):
        return node.children

    def on_iter_n_children(self, node):
        if node is None:
            return len(self.__root.children)
        return len(node.children)

    def on_iter_nth_child(self, node, n):
        if not node:
            return self.__root.children[n]
        return node.children[n]

    def on_iter_parent(self, node):
        return node.parent

    def __on_roster_set(self, elem):
        print "__on_roster_set", elem.toString()

        query_elem = elem.findElement("query")
        item_elem = query_elem.findElement("item")

        s10n = item_elem.getAttrib("subscription")
        jid = item_elem.getAttrib("jid")

        ## See if they removed us
        if s10n == "none":
            self.__app.session.roster().deleteUser(jid)
        elif s10n == "remove":
            self.clearUser(jid)


    ## XXX Look at this again
    def __on_presence(self, jid, online, prev_type):
        print "__on_presence for",jid, " Online: ", online, " prev_type: ", prev_type
        item = self.__app.session.roster()[jid]
        ## We don't have them yet, will this ever happen?
        if not self.__users.has_key(jid):
            if not online and self.show_online_only:
                return
            self.buildUser(item)

        ## If they went offline and it's show online only, nuke them
        if prev_type != JabberOO.Presence.Type.ptUnavailable and not online and self.show_online_only:
            self.clearUser(jid)
            return

        self.updatePres(jid, online)

    def updatePres(self, jid, online):
        icon = None
        show = None
        status = None

        print " Enter updatePres:  jid: ", jid, " online: ", online
        if online:
            try:
                pres = self.__app.session.presenceDB().find(jid)
            except:
                print "Major error, this person has no stored pres", jid
                return

            if pres.show == JabberOO.Presence.Show.stAway:
                icon = self.__away_pixbuf
            elif pres.show == JabberOO.Presence.Show.stXA:
                icon = self.__xa_pixbuf
            else:
                icon = self.__online_pixbuf

            show = pres.getShow_str()
            status = pres.status
        ## Process each of these
        for user in self.__users[jid]:
            if online and not user[COLUMN_ONLINE]:
                gnode = user.parent
                gnode[COLUMN_ONLINE_COUNT] = gnode[COLUMN_ONLINE_COUNT] + 1
                path = gnode.get_path()
                self.row_changed(path, self.get_iter(path))
            user[COLUMN_ONLINE] = online
            user[COLUMN_ICON] = icon
            user[COLUMN_SHOW] = show
            user[COLUMN_STATUS] = status
            path = user.get_path()
            self.row_changed(path, self.get_iter(path))

    def addItem(self, item):
        pass

    def addGroup(self, name):
        node = RosterModelNode(name)
        self.__root.append(node)
        path = node.get_path()
        self.__groups[name] = node
        self.row_inserted(path, self.get_iter(path))
        return node

    def remGroup(self, node):
        if len(node.children) > 0:
            for child in node.children:
                node.children.remove(child)
                del child
        path = node.get_path()
        self.__root.children.remove(node)
        del self.__groups[node.name]
        self.row_deleted(path)
        del node

    def addUser(self, item, gnode):
        jid = item.getJID()
        node = RosterModelNode(item.getNickname(), jid)
        gnode.append(node)
        path = node.get_path()
        ## Quick access
        if self.__users.has_key(jid):
            self.__users[jid].append(node)
        else:
            self.__users[jid] = [node]
        self.row_inserted(path, self.get_iter(path))

        ## Return it in case they want to play with it
        return node

    def remUser(self, node):
        gnode = node.parent

        path = node.get_path()
        self.__users[node.jid].remove(node)
        if len(self.__users[node.jid]) == 0:
            del self.__users[node.jid]
        self.row_deleted(path)

        if gnode:
            gnode.children.remove(node)
            gnode[COLUMN_ONLINE_COUNT] = gnode[COLUMN_ONLINE_COUNT] - 1
            if len(gnode.children) == 0:
                self.remGroup(gnode)
            else:
                path = gnode.get_path()
                self.row_changed(path, self.get_iter(path))
            node.parent = None

        del node

    def clearUser(self, jid):
        if not self.__users.has_key(jid):
            return

        for node in self.__users[jid]:
            self.remUser(node)


    def buildUser(self, item):
        if self.show_online_only and not item.isAvailable():
            return

        for group in item:
            if not self.__groups.has_key(group):
                gnode = self.addGroup(group)
            else:
                gnode = self.__groups[group]
            unode = self.addUser(item, gnode)
            self.updatePres(unode.jid, item.isAvailable())

    def refresh(self, force_reresh=0):
        if self.__has_refresh and not force_reresh:
            print "Skipping refresh"
            return

        self.__has_refresh = 1
        ## remove every row
        for user in self.__users.keys():
            self.clearUser(user)

        ## Make sure there are no straggling groups
        for group in self.__groups.keys():
            self.remGroup(self.__groups[group])

        ## Reset everything
        self.__root.clear()
        self.__groups.clear()
        self.__users.clear()

        ## Rebuild it
        for item in self.__app.session.roster():
            self.buildUser(item)
        
        ## make it look purty
        self.__root.sort()
