from GabberApp import GabberApp
import gtk

class PlainTextView:
    __composers = {}

    def __init__(self, sw):
        self.__app = GabberApp()
        self.__nickname = self.__app.session.getUserName()
        self.__view = gtk.TextView()
        self.__buffer = self.__view.get_buffer()
        self.__start_of_last_msg = self.__buffer.create_mark("start_of_last",
            self.__buffer.get_end_iter(), 0)
        self.__end_mark = self.__buffer.create_mark("END", 
            self.__buffer.get_end_iter(), 0)

        ## setup the view
        self.__view.set_wrap_mode(gtk.WRAP_WORD)
        self.__view.set_editable(0)
        self.__view.set_cursor_visible(0)
        self.__view.set_left_margin(5)
        self.__view.set_right_margin(5)

        sw.add(self.__view)

        ## buffer related setup
        tag = self.__buffer.create_tag("remote")
        tag.set_property("foreground", "red")
        tag = self.__buffer.create_tag("local")
        tag.set_property("foreground", "blue")


    def __del__(self):
        del self.__view
        
    def append(self, msg, local_msg=0):
        msg_from = msg.sender
        nick = self.__nickname
        tag = "local"
        end = self.__buffer.get_end_iter

        if not local_msg:
            if msg_from in self.__composers:
                self.cancel_composing(msg_from)

            nick = self.__get_nick(msg_from)
            tag = "remote"
        
        self.__buffer.move_mark(self.__start_of_last_msg, end())

        self.__buffer.insert_with_tags_by_name(end(), "<", tag)
        self.__buffer.insert(end(), nick)
        self.__buffer.insert_with_tags_by_name(end(), "> ", tag)
        self.__buffer.insert(end(), msg.body + "\n")

        self.__buffer.move_mark(self.__end_mark, end())

        self.__view.scroll_to_mark(self.__end_mark, 0)

    def composing(self, jid):
        if self.__composers.has_key(jid):
            ## XXX Do something else?
            return

        nick = self.__get_nick(jid)
        end = self.__buffer.get_end_iter

        self.__buffer.create_mark(jid + "_start", end(), 1)
        self.__buffer.insert(end(), "<" + nick + "> ...")
        self.__buffer.create_mark(jid + "_end", end(), 0)
        self.__buffer.insert(end(), "\n")

        self.__composers[jid] = jid
        
    
    def cancel_composing(self, jid):
        if not self.__composers.has_key(jid):
            ## XXX Do something else?
            return

        start = self.__buffer.get_mark(jid + "_start")
        end = self.__buffer.get_mark(jid + "_end")

        if not start or not end:
            print "No marks found for", jid
            return

        self.__buffer.delete(self.__buffer.get_iter_at_mark(start),
            self.__buffer.get_iter_at_mark(end))

        self.__buffer.delete_mark(start)
        self.__buffer.delete_mark(end)

        del self.__composers[jid]

    def __get_nick(self, jid):
        return jid[:jid.find("@")]

    ## Return the viewable widget
    def get_widget(self):
        return self.__view
