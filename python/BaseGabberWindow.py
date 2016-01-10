from gtk.glade import XML
from Environment import ENV_VARS

class BaseGabberWindow:
    def __init__(self, widget_name):
        self.__xml = XML(ENV_VARS.pkgdatadir + widget_name + ".glade", widget_name)
        self.window = self.__xml.get_widget(widget_name)
        self.window.connect("delete-event", self.on_delete_event)

    def __del__(self):
        del self.window
        del self.__xml

    def show(self):
        self.window.show()

    def hide(self):
        self.window.hide()

    def get_widget(self, name):
        return self.__xml.get_widget(name)

    ## XXX this seems icky
    def close(self):
        self.hide()
        del self


    def on_delete_event(self, win, event):
        self.close()
