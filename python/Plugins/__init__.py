import os.path

__all__ = []

def load(name, pman):
    if not os.path.exists("Plugins/" + name + ".py"):
        raise ImportError

    exec "import Plugins." + name + " as plugin"
    return plugin
