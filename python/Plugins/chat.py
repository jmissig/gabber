from ChatViewManager import ChatViewManager

__cvm = None
## Configure the plugin correctly
def init():
    global __cvm, p
    if __cvm:
        print "Already have a CVM loaded!"
        return

    __cvm = ChatViewManager()
