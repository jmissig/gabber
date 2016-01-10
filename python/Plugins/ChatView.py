from BaseGabberWindow import BaseGabberWindow
from PlainTextView import PlainTextView
from time import time
import GabberApp
from JabberOO import Message
import gtk
from PrettyJID import PrettyJID

class ChatView(BaseGabberWindow):
    __sent_composing = 0
    __composing_id = ""
    __composing_msg = None
    __last_msg_time = 0

    def __init__(self, mgr, jid, node=None):
        BaseGabberWindow.__init__(self, "OOOChat_win")
        self.__manager = mgr
        self.__jid = jid

        self.__init()
        ## See what type to start
        if node:
            self.display(Message(node))

    def __init(self):
        #btn = self.get_widget("AddContact_btn")
        #print btn
        #img = gtk.Image()
        #rm = GabberApp.GabberApp().resources
        #img.set_from_pixbuf(rm.get(rm.rtPIXBUF, "add-user"))
        #btn.add(img)

        self.__pjJIDInfo = PrettyJID(self.__jid, PrettyJID.dtNickRes)
        hbox = self.get_widget("JIDInfo_hbox")
        hbox.pack_end(self.__pjJIDInfo)
        self.__pjJIDInfo.show()
        hbox.show()

        self.__txtMessage = self.get_widget("Message_txt")
        ## XXX Disabled due to issues detailed on __on_drag_data_received function
        #targets = [("text/x-jabber-id", 0, 255)]
        #self.__txtMessage.drag_dest_set( gtk.DEST_DEFAULT_ALL, targets, 
        #    gtk.gdk.ACTION_COPY | gtk.gdk.ACTION_MOVE)
        #self.__txtMessage.connect("drag-data-received", self.__on_drag_data_received)

        sw = self.get_widget("Chatview_scroll")
        self.__txtChatview = PlainTextView(sw)
        sw.show_all()
        
        self.window.set_title("Chat with " + self.__jid + " - Gabber")
        self.window.connect("event", self.__on_window_event)

        self.__app = GabberApp.GabberApp()
        sess = self.__app.session
        self.__chat_query = sess.registerXPath("/message[@type='chat' and @from='"+self.__jid+"']", self.__on_chat_node)
        self.__event_query = sess.registerXPath("/message[@type='chat' and @from='" + self.__jid + "']/x[@xmlns='jabber:x:event']", self.__on_event_node)

    def close(self):
        self.__app.session.unregisterXPath(self.__chat_query)
        self.__app.session.unregisterXPath(self.__event_query)

        self.__manager.release(self.__jid)

        ## let it fully clean us up
        BaseGabberWindow.close(self)

    def __del__(self):
        del self.__txtChatview

    def on_delete_event(self, win, event):
        now = time()
        ## XXX This timer is arbitrary
        if (now - self.__last_msg_time) < 3:
            dlg = gtk.Dialog("Recent Message", self.window,
                gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT, 
                (gtk.STOCK_NO, 0, gtk.STOCK_YES, 1))
            lbl = gtk.Label("A message was received recently, really close this chat?")
            dlg.vbox.set_border_width(4)
            dlg.vbox.set_spacing(8)
            dlg.vbox.pack_start(lbl)
            dlg.show_all()
            resp = dlg.run()
            dlg.destroy()
            del dlg
            if not resp:
                return 1

        BaseGabberWindow.on_delete_event(self, win, event)

    def __on_window_event(self, win, ev):
        if not ev.type == gtk.gdk.KEY_PRESS:
            return

        keyval = gtk.gdk.keyval_name(ev.keyval)
        if keyval == "KP_Enter":
            keyval = "Return"

        if keyval == "Escape":
            self.on_delete_event(None, None)

        if keyval == "Return":
            if ev.state & gtk.gdk.SHIFT_MASK:
                return

            self.__on_send()
            return 1

        if self.__composing_id:
            char_count = self.__txtMessage.get_buffer().get_char_count()

            if (char_count == 0 and self.__sent_composing):
                ## They deleted stop the event
                m = Message(self.__jid, "", Message.Type.mtChat)
                x = m.addX("jabber:x:event")
                x.addElement("id", self.__composing_id)

                self.__app.session.send(m)

                self.__sent_composing = 0
            elif (char_count > 0 and not self.__sent_composing):
                m = Message(self.__jid, "", Message.Type.mtChat)
                x = m.addX("jabber:x:event")
                x.addElement("composing")
                x.addElement("id", self.__composing_id)

                self.__app.session.send(m)

                self.__sent_composing = 1

    def __on_chat_node(self, node):
        self.display(Message(node))

    def __on_event_node(self, node):
        m = Message(node)

        x = m.findX("jabber:x:event")
        delivered = x.findElement("delivered")
        composing = x.findElement("composing")
        id = x.findElement("id")

        if composing and id:
            self.__txtChatview.composing(m.sender)
        elif not composing and id:
            self.__txtChatview.cancel_composing(m.sender)
        elif composing and not id:
            if self.__composing_id:
                msg = Message(e.getAttrib("from"), "", Message.Type.mtChat)
                ix = msg.addX("jabber:x:event")
                ix.addElement("composing")
                ix.addElement("id", self.__composing_id)

                self.__app.session.send(msg)

            self.__composing_id = m.getID()
        elif delivered:
            self.__app.session.send(m.delivered())

    def __on_send(self):
        tb = self.__txtMessage.get_buffer()
        body = tb.get_text(tb.get_start_iter(), tb.get_end_iter(), 0)
        if not body:
            return

        m = Message(self.__jid, body, Message.Type.mtChat)
        m.requestComposing()
        m.id = self.__app.session.getNextID()

        self.__app.session.send(m)

        self.__txtChatview.append(m, 1)
        
        tb.set_text("")
        
    def display(self, msg):
        self.__last_msg_time = time()
        
        if not msg.body:
            return

        self.__txtChatview.append(msg)
        x = msg.findX("jabber:x:event")
        if x and x.findElement("displayed"):
            self.__app.session.send(msg.displayed())

    ## XXX Currently disabled due to issues with this getting called twice
    def __on_drag_data_received(self, w, ctx, x, y, data, info, time):
        print "__on_drag_data_received", self, w, ctx, x, y, data, info, time
        if data and data.data:
            ## Gabber1 sucks
            jids = data.data.replace("\x00", " ").strip().splitlines()
            print jids
            delim = "\n"
            buff = w.get_buffer()
            buff.insert(buff.get_end_iter(), delim.join(jids))
            ctx.finish(1, 0, time)
        else:
            ctx.finish(0, 0, time)
