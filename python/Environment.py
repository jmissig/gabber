import os.path

class Environment:
    def __init__(self):
        ## XXX GENERATE THESE!!!
        self.pixmapdir = self.findPath("pixmaps/", "/usr/local/share/pixmaps/gabber", "show-online.png")
        self.pkgdatadir = self.findPath("ui/", "/usr/local/share/gabber", "Gabber_win.glade")
        self.package = "gabber"
        self.version = "1.9"
    
    def findPath(self, rel_dir, sys_dir, file):
        paths = ["./"+rel_dir, "../" + rel_dir, "./", sys_dir]
        for path in paths:
            if os.path.exists(path):
                return path

        return None

ENV_VARS = Environment()
