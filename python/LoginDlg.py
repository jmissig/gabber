from ConfigWrapper import ConfigWrapper, Keys
from BaseGabberWindow import BaseGabberWindow

import JabberOO

class LoginDlg(BaseGabberWindow, ConfigWrapper):
    evtDoConnect = JabberOO.Signal0_void()
    evtCancel = JabberOO.Signal0_void()

    def __init__(self, is_startup=0):
        BaseGabberWindow.__init__(self, "Login_dlg")
        ConfigWrapper.__init__(self, Keys.Account.main_dir)

        b = self.get_widget("Cancel_btn")
        b.connect("clicked", self.on_Cancel_clicked)

        self.__btnLogIn = self.get_widget("LogIn_btn")
        self.__btnLogIn.connect("clicked", self.on_LogIn_clicked)

        self.__entUsername = self.get_widget("Username_ent")
        self.__entUsername.connect("changed", self.on_Changed)

        self.__entServer = self.get_widget("Server_ent")
        self.__entServer.connect("changed", self.on_Changed)

        self.__entPassword = self.get_widget("Password_ent")
        self.__entPassword.connect("changed", self.on_Changed)

        self.__entResource = self.get_widget("Resource_ent")
        self.__entResource.connect("changed", self.on_Changed)

        self.__spinPort = self.get_widget("Port_spin")
        self.__spinPort.set_width_chars(4)
        self.__spinPort.connect("changed", self.on_Changed)

        self.__spinPriority = self.get_widget("Priority_spin")
        self.__spinPriority.connect("changed", self.on_Changed)

        self.__ckPassword = self.get_widget("SavePassword_chk")
        self.__ckPassword.connect("toggled", self.on_Changed)

        self.loadConfig()

        ## If this is opened during startup, it is the only window available
        if is_startup:
            self.window.set_modal(1)

    def __del__(self):
        ## Do something?
        pass

    def loadConfig(self):
        self.__entUsername.set_text(self.client.get_string(Keys.Account.username))
        self.__entServer.set_text(self.client.get_string(Keys.Account.server))
        self.__entPassword.set_text(self.client.get_string(Keys.Account.password))
        self.__entResource.set_text(self.client.get_string(Keys.Account.resource))
        self.__spinPort.set_value(self.client.get_int(Keys.Account.port))
        self.__spinPriority.set_value(self.client.get_int(Keys.Account.priority))
        self.__ckPassword.set_active(self.client.get_bool(Keys.Account.savepassword))

    def saveConfig(self):
        self.client.set_string(Keys.Account.username, self.__entUsername.get_text())
        self.client.set_string(Keys.Account.server, self.__entServer.get_text())
        self.client.set_string(Keys.Account.resource, self.__entResource.get_text())
        self.client.set_string(Keys.Account.password, self.__entPassword.get_text())
        self.client.set_int(Keys.Account.port, self.__spinPort.get_value_as_int())
        self.client.set_int(Keys.Account.priority, self.__spinPriority.get_value_as_int())
        self.client.set_bool(Keys.Account.savepassword, self.__ckPassword.get_active())

    def on_LogIn_clicked(self, btn):
        self.saveConfig()

        self.evtDoConnect()

        self.close()

    def on_Cancel_clicked(self, btn):
        self.evtCancel()

        self.close()

    def on_Changed(self, widget):
        pass
