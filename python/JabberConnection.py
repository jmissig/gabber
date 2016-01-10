import Judo
import JabberOO

import TCPTransmitter
from ConfigWrapper import *

class JabberConnection(ConfigWrapper):
    ## Members
    session = None
    transmitter = None
    server = None

    ## Events
    evtConnecting=JabberOO.Signal0_void()
    evtLoggingIn=JabberOO.Signal0_void()
    evtConnected=JabberOO.Signal0_void()
    evtDisconnected=JabberOO.Signal0_void()
    evtReconnecting=JabberOO.Signal0_void()

    def __init__(self):
        ## init base class
        ConfigWrapper.__init__(self, Keys.Account.main_dir)

        ## Primary members
        self.session = JabberOO.Session()
        self.transmitter = TCPTransmitter.TCPTransmitter()

        ## Connect to TCPT Events
        self.transmitter.evtConnected.connect(self.on_transmitter_connected)
        self.transmitter.evtDisconnected.connect(self.on_transmitter_disconnected)
        self.transmitter.evtReconnect.connect(self.on_transmitter_reconnect)
        self.transmitter.evtError.connect(self.on_transmitter_error)
        self.transmitter.evtDataAvailable.connect(self.session.push)

        ## Connect to Session Events
        self.session.evtTransmitXML.connect(self.transmitter.send)
        self.session.evtConnected.connect(self.on_session_connected)
        self.session.evtDisconnected.connect(self.on_session_disconnected)
        #self.session.evtOnVersion.connect(self.on_session_version)
        #self.session.evtOnTime.connect(self.on_session_time)

    def __del__(self):
        del self.session
        del self.transmitter

    def connect(self, login=1):
        self.login = login

        self.evtConnecting()

        self.transmitter.connect(self.client.get_string(Keys.Account.server),
            self.client.get_int(Keys.Account.port), 0, 0)

        ## XXX start polling?

    ## Transmitter Events
    def on_transmitter_connected(self):
        ## default to auto auth negotiation (0k, digest, plain)
        atype = JabberOO.Session.AuthType.atAutoAuth
        
        self.evtLoggingIn()

        self.server = self.client.get_string(Keys.Account.server)
        ## Start the session up
        self.session.connect(self.server, atype,
            self.client.get_string(Keys.Account.username),
            self.client.get_string(Keys.Account.resource),
            self.client.get_string(Keys.Account.password), 0, 1)

    def on_transmitter_disconnected(self):
        if self.session:
            self.session.disconnect()

        self.evtDisconnected()

    def on_transmitter_reconnect(self):
        self.evtReconnecting()

    def on_transmitter_error(self, msg):
        self.transmitter.disconnect()

    ## Session event handlers
    def on_session_connected(self, element):
        self.session.browseDB().cache(self.server, self.on_server_browse)

    def on_session_roster(self):
        self.__roster_conn.disconnect()
        self.evtConnected()
        self.session.send(JabberOO.Presence("", 
            JabberOO.Presence.Type.ptAvailable, 
            JabberOO.Presence.Show.stOnline))

    def on_session_disconnected(self):
        if self.transmitter.state != TCPTransmitter.TCPTransmitter.Offline:
            self.transmitter.disconnect()

    def on_session_version(self, name, version, os):
        print "on_session_version"

    def on_session_time(self, time, timezone):
        print "on_session_time"

    def on_server_browse(self, item):
        self.__roster_conn = self.session.evtOnRoster.connect(self.on_session_roster)
