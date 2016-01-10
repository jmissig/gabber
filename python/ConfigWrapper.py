import gconf

class Keys:
    class Account:
        main_dir = "/apps/gabber/account"
        savepassword = main_dir + "/savepassword"
        password = main_dir + "/password"
        port = main_dir + "/port"
        priority = main_dir + "/priority"
        resource = main_dir + "/resource"
        server = main_dir + "/server"
        username = main_dir + "/username"
    class Plugin:
        main_dir = "/apps/gabber/plugins"
        pluginlist = main_dir + "/pluginlist"

class ConfigWrapper:
    def __init__(self, main_dir, preload=gconf.CLIENT_PRELOAD_ONELEVEL):
        self.__main_dir=main_dir
        self.client = gconf.client_get_default()
        self.client.add_dir(main_dir, preload)

    def __del__(self):
        self.client.suggest_sync()
        self.client.remove_dir(self.__main_dir)
        del self.client
