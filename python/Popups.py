from Singleton import Singleton
import gtk
from gtk.glade import XML
from Environment import ENV_VARS

class Base:
    __widget = None
    __first_item = None
    def __init__(self, name):
        self.__xml = XML(ENV_VARS.pkgdatadir + name + ".glade", name)
        self.__widget = self.__xml.get_widget(name)
        self.__widget.show()

    def getItem(self, name):
        return self.__xml.get_widget(name)
        
    def addItem(self, mi):
        mi.show()
        self.__first_item = mi
        self.__widget.prepend(mi)

    def activate(self):
        if self.__first_item:
            self.__first_item.activate()

    def popup(self, *args):
        self.__widget.popup(*args)

class User(Singleton, Base):
    def __init__(self):
        pass

    def init_once(self):
        Base.__init__(self, "Roster_menu_user")
        self.__curent_jid = None

    def getSelectedJID(self):
        return self.__curent_jid
    
    def updateSelectedJID(self, jid):
        self.__curent_jid = jid
