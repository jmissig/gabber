import Plugins
import gobject
from ConfigWrapper import * 

class PluginManager(ConfigWrapper):
    def __init__(self):
        ConfigWrapper.__init__(self, Keys.Plugin.main_dir)

        self.__plugins = self.client.get_list(Keys.Plugin.pluginlist, gconf.VALUE_STRING)
        for plugin in self.__plugins:
            try:
                p = Plugins.load(plugin, self)
                p.init()
            except ImportError, e:
                print "Unable to load plugin", plugin
