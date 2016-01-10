INCLUDES = \
	-DLOCALEDIR=\""$(datadir)/locale"\" 	\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/$(PACKAGE)"\" \
	-DPKGDATADIR=\""$(pkgdatadir)"\" \
	-DPKGLIBDIR=\""$(pkglibdir)"\" \
	@CXXFLAGS@ $(DEPS_CFLAGS) $(JABBEROO_CFLAGS) \
	$(SSL_CFLAGS) \
	-I$(GABBER_PLATFORM)

DIST_SUBDIRS=linux osx win32 modules
SUBDIRS = $(GABBER_PLATFORM) modules
bin_PROGRAMS = gabber

if WITH_SSL
ssl_sources=SSLAdapter.hh SSLAdapter.cc
else
ssl_sources=
endif

if GTKSPELL
spell_sources=gtkspell.h gtkspell.c
else
spell_sources=
endif

# Keep gabber files in alphabetical order

noinst_HEADERS = fwd.h intl.h BaseModule.hh ConfigPaths.hh ConfigWrapper.hh \
		Environment.hh FeatureNegotiation.hh PrettyText.hh \
		Singleton.hh Stream.hh SIProfile.hh

gabber_SOURCES = \
	AddContactDlg.hh AddContactDlg.cc \
	AutoAway.hh AutoAway.cc \
	AvatarManager.hh AvatarManager.cc \
	BaseGabberWindow.hh BaseGabberWindow.cc \
	Configurator.hh Configurator.cc \
	ContactInfo.hh ContactInfo.cc \
	FileTransferDlg.hh FileTransferDlg.cc \
	FileTransferManager.hh FileTransferManager.cc \
	FileTransferRecvDlg.hh FileTransferRecvDlg.cc \
	FileTransferSendDlg.hh FileTransferSendDlg.cc \
	FTProfile.hh FTProfile.cc \
	GabberApp.hh GabberApp.cc \
	GabberDialog.hh GabberDialog.cc \
	GabberUtility.hh GabberUtility.cc \
	GabberWidgets.hh GabberWidgets.cc \
	GabberWin.hh GabberWin.cc \
	GroupsEditor.hh GroupsEditor.cc \
	JabberConnection.hh JabberConnection.cc \
	LogManager.hh LogManager.cc \
	LogViewer.hh LogViewer.cc \
	Menus.hh Menus.cc \
	ModuleManager.hh ModuleManager.cc \
	PacketQueue.hh PacketQueue.cc \
	PacketQueueView.hh PacketQueueView.cc \
	PlainTextView.hh PlainTextView.cc \
	PrefsInterface.hh PrefsInterface.cc \
	PubSubNode.hh PubSubNode.cc \
	RegisterGateway.hh RegisterGateway.cc \
	ResourceManager.hh ResourceManager.cc \
	RosterController.hh RosterController.cc \
	RosterModel.hh RosterModel.cc \
	RosterView.hh RosterView.cc \
	S5B.hh S5B.cc \
	S10nRequestDlg.hh S10nRequestDlg.cc \
	StreamInitiation.hh StreamInitiation.cc \
	TCPTransmitter.hh TCPTransmitter.cc \
	TextParser.hh TextParser.cc \
	XData.hh XData.cc \
	XPathLog.hh XPathLog.cc \
	$(ssl_sources) \
	base64.hpp base64.cpp \
	eggtrayicon.h eggtrayicon.c \
	$(spell_sources) \
	md5.h md5.cpp \
	main.cc

gabber_LDFLAGS = -export-dynamic

gabber_LDADD = \
	$(DEPS_LIBS) $(SPELLER_LIB) $(JABBEROO_LIBS) $(SSL_LIBS) $(XSS_LIBS) \
	$(top_builddir)/src/$(GABBER_PLATFORM)/libplatform.a $(PLATFORM_LIBS) \
	-lstdc++
