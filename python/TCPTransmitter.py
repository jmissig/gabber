import gobject
import socket
import Judo
import JabberOO

class TCPTransmitter:
    """TCP handling class.  This is designed to interact with Gtk+"""
    #State definitions
    Offline=0
    Connecting=1
    Reconnecting=2
    Connected=3
    Listening=4
    Accepting=5
    Error=6
    ProxyConnecting=7
        
    ## Event properties
    evtError = JabberOO.Signal1_v_ccp()
    evtReconnect = JabberOO.Signal0_void()
    evtDisconnected = JabberOO.Signal0_void()
    evtConnected = JabberOO.Signal0_void()
    evtAccepted = JabberOO.Signal0_void()
    evtDataAvailable = JabberOO.Signal2_v_ccp_i()
    evtCanSendMore = JabberOO.Signal0_void()
    evtDataSent = JabberOO.Signal1_v_i()
    send_buffer = "" 
        
    def __init__(self):
        self.state = TCPTransmitter.Offline
        self.reconnect_count = 0

    def __del__(self):
        self.disconnect()

    def connect(self, host, port, use_ssl=0, autoreconnect=1):
        self.host = host
        self.port = port
        self.use_ssl = use_ssl
        self.autoreconnect = autoreconnect
        self.state = TCPTransmitter.Connecting
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setblocking(0)
        try:
            self.socket.connect((host, port))
        except socket.error, e:
            # Make sure we're performing a non blocking connect
            if e[0] != 115:
                raise e
        self.io_channel = gobject.io_add_watch(self.socket, gobject.IO_IN | gobject.IO_OUT | gobject.IO_ERR | gobject.IO_HUP, self.handleConnect)

    def listen(self, host, port):
        ## XXX implement me!
        pass

    def disconnect(self):
        ## And lame-o was his name-o
        if (self.state == TCPTransmitter.Offline):
            return

        ## Stop any pending event
        if (self.io_channel):
            gobject.source_remove(self.io_channel)

        self.send_buffer = ""
            
        self.socket.close()
        self.state = TCPTransmitter.Offline
        self.evtDisconnected()

    def send(self, data, sz=0):
        if self.state == TCPTransmitter.Offline:
            return

        if sz:
            data = data[0:sz]
        self.send_buffer += data
        gobject.source_remove(self.io_channel)
        self.io_channel = gobject.io_add_watch(self.socket, gobject.IO_IN | gobject.IO_OUT | gobject.IO_ERR | gobject.IO_HUP, self.handleSocketEvent)

    ## IO Callbacks
    def handleError(self, errMsg):
        self.disconnect()

        ## Reconnect if they want us to
        if (self.autoreconnect and self.reconnect_count < 5):
            self.reconnect_count += 1
            self.evtReconnect()
            gobject.timeout_add(10000, self.doReconnect)
            self.state = TCPTransmitter.Reconnecting
            return

            self.state = TCPTransmitter.Error
            self.evtError(errMsg)
        else:
            print errMsg

    def handleSocketEvent(self, sock, cond):
        assert (self.state == TCPTransmitter.Connected), "Not connected"

        if cond & gobject.IO_IN:
            try:
                in_buf = self.socket.recv(2048)
            except socket.error, e:
                print "Exception reading:",e
            self.evtDataAvailable(in_buf, len(in_buf))
            return 1
        elif cond & gobject.IO_OUT:
            try:
                out_count = self.socket.send(self.send_buffer)
            except socket.error, e:
                print "Exception writing:", e

            if (out_count == len(self.send_buffer)):
                ## Clean up and don't come back in here
                self.send_buffer = ""
                gobject.source_remove(self.io_channel)
                self.io_channel = gobject.io_add_watch(self.socket, gobject.IO_IN | gobject.IO_ERR | gobject.IO_HUP, self.handleSocketEvent)
            else:
                ## Advance the send buffer
                self.send_buffer = self.send_buffer[out_count:]

            ## Tell the world we wrote
            self.evtDataSent(out_count)
            
            return 1
        else:
            self.handleError("There was an error handling socket event %d" % cond)
            return 0

    def handleConnect(self, sock, cond):
        ## Make sure we don't magically get callsed again
        gobject.source_remove(self.io_channel)

        ## Check for an error
        has_err = self.socket.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
        if has_err:
            self.handleError("Unable to connect")
            return 0
        
        self.reconnect_count = 0

        ## We're clear, switch to SSL mode if we want to
        if self.use_ssl:
            self.ssl_socket = socket.ssl(self.socket)

        self.state = TCPTransmitter.Connected

        self.io_channel = gobject.io_add_watch(self.socket, gobject.IO_IN | gobject.IO_OUT | gobject.IO_ERR | gobject.IO_HUP, self.handleSocketEvent)

        self.evtConnected()

    def doReconnect(self):
        self.connect(self.host, self.port, self.use_ssl, self.autoreconnect)
        return 0
