from JabberConnection import JabberConnection
import gtk
import gnome
import LoginDlg
from Environment import ENV_VARS
from LoginDlg import LoginDlg
from GabberWin import GabberWin
from PluginManager import PluginManager
from Singleton import Singleton
import new
import JabberOO
from ResourceManager import ResourceManager

class GabberApp(Singleton, JabberConnection):
    _instance = None
    
    def __init__(self):
        pass

    def init_once(self):
        print "Starting " + ENV_VARS.package + " v" + ENV_VARS.version
        JabberConnection.__init__(self)

        gnome.init(ENV_VARS.package, ENV_VARS.version)

        self.pman = PluginManager()
        self.resources = ResourceManager()
        self.__load_resources()
        
        ld = LoginDlg(1)
        ld.evtDoConnect = self.startup
        ld.evtCancel = self.quit
        ld.show()

        ## Show us more info if we are debugging
        if __debug__:
            self.session.evtTransmitXML.connect(self.__on_transmit_xml)
            self.session.evtRecvXML.connect(self.__on_recv_xml)

        self.session.evtPresenceRequest.connect(self.__on_presence_request)

    def __del__(self):
        self.main_window.close()

    def startup(self):
        self.main_window = GabberWin()
        self.connect()

    def run(self):
        gtk.main()

    def quit(self):
        if not self.session.disconnect():
            self.transmitter.disconnect()

        gtk.main_quit()

    def __on_transmit_xml(self, xml):
        if __debug__:
            print "SEND:",xml

    def __on_recv_xml(self, xml):
        if __debug__:
            print "RECV:",xml

    def __on_presence_request(self, pres):
        jid = pres.sender
        type = pres.type

        p = JabberOO.Presence(jid, JabberOO.Presence.Type.ptSubscribed)
        if type == JabberOO.Presence.Type.ptSubRequest:
            ## See if the user will allow them
            dlg = gtk.MessageDialog(self.main_window.window, gtk.DIALOG_DESTROY_WITH_PARENT | gtk.DIALOG_MODAL, gtk.MESSAGE_QUESTION,  gtk.BUTTONS_YES_NO, "%s would like to subscribe to your presence.  Allow them?" % jid)

            resp = dlg.run()
            dlg.hide()
            del dlg

            ## Generate a response
            if resp == gtk.RESPONSE_NO:
                p.type = JabberOO.Presence.Type.ptUnsubscribed
                print "Should deny",jid

        elif type == JabberOO.Presence.Type.ptUnsubRequest:
            p.type = JabberOO.Presence.Type.ptUnsubscribed

        self.session.send(p)

    ## Load the common resources
    ## XXX temas: are there any?
    def __load_resources(self):
        pass
