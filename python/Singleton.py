class Singleton(object):
    def __new__(cls, *args, **kwds):
        it = cls.__dict__.get("_real_instance")
        if it is not None:
            return it
        cls._real_instance = it = object.__new__(cls)
        it.init_once()
        return it
    
    def init_once(cls):
        """Override this method with code that should only happen after the
        initial creation.  An example is base class initialization."""
        pass
