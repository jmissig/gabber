INCLUDES = \
	-DLOCALEDIR=\""$(datadir)/locale"\" 	\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/$(PACKAGE)"\" \
	-DPKGDATADIR=\""$(pkgdatadir)"\" \
	-DPKGLIBDIR=\""$(pkglibdir)"\" \
	@CXXFLAGS@ $(DEPS_CFLAGS) $(JABBEROO_CFLAGS) -I$(top_builddir)/src

pkglib_LTLIBRARIES = chat.la standalone.la groupchat.la raw.la

noinst_HEADERS = ChatViewManager.hh \
                 StandaloneViewManager.hh \
                 GCJoinDlg.hh \
                 GCView.hh \
                 GCViewManager.hh \
		 RawViewManager.hh

chat_la_SOURCES = ChatViewManager.cc \
                  ChatView.hh ChatView.cc
chat_la_LDFLAGS = -module -export-dynamic -avoid-version

standalone_la_SOURCES = StandaloneViewManager.cc \
                        StandaloneView.hh StandaloneView.cc \
			StandaloneSendDlg.hh StandaloneSendDlg.cc
standalone_la_LDFLAGS = -module -export-dynamic -avoid-version

groupchat_la_SOURCES = GCViewManager.cc GCView.cc GCJoinDlg.cc
groupchat_la_LDFLAGS = -module -export-dynamic -avoid-version

raw_la_SOURCES = RawViewManager.cc \
		 RawInputDlg.hh RawInputDlg.cc
raw_la_LDFLAGS = -module -export-dynamic -avoid-version
