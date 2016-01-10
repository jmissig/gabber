from ChatView import ChatView
from GabberApp import GabberApp
from gtk import MenuItem
import Popups

class ChatViewManager:
    def __init__(self):
        self.__app = GabberApp()
        self.__chats = {}
        self.__query = self.__app.session.registerXPath("/message[@type='chat']", self.__on_chat_node)
        self.__menu_item = MenuItem("One-on-one _Chat...")
        self.__menu_item.connect("activate", self.__on_chat_menu_activate)
        Popups.User().addItem(self.__menu_item)

    def __del__(self):
        self.__app.session.unregisterXPath(self.__query)
        for chat in chats:
            del chat

    def __on_chat_node(self, node):
        msg_from = node.getAttrib("from")
        if not self.__chats.has_key(msg_from):
            self.__chats[msg_from] = ChatView(self, msg_from, node)

    def __on_chat_menu_activate(self, menu):
        to = Popups.User().getSelectedJID()
        if not self.__chats.has_key(to):
            self.__chats[to] = ChatView(self, to)

    def release(self, jid):
        del self.__chats[jid]
