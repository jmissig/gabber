SUBDIRS = src pixmaps po ui

# Desktop entry
desktopdir		= $(datadir)/applications
desktop_in_files	= gabber.desktop.in
desktop_DATA 		= $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

# Gconf schema
schemadir   		= @GCONF_SCHEMA_FILE_DIR@
schema_in_files 	= gabber.schemas.in
schema_DATA 		= $(schema_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

dist_intltool		= intltool-extract.in intltool-merge.in intltool-update.in

dist_noinst_SCRIPTS	= autogen.sh
dist_noinst_DATA	= HACKING HACKING.ideas po/LINGUAS $(dist_intltool) $(desktop_in_files) $(schema_in_files)

CLEANFILES		= $(desktop_DATA) $(schema_DATA)
DISTCLEANFILES		= intltool-extract intltool-merge intltool-update

if GCONF_SCHEMAS_INSTALL
install-data-hook: install-schemas
uninstall-hook: uninstall-schemas
else
install-data-hook:
uninstall-hook:
endif

gconftool_command = GCONF_CONFIG_SOURCE='$(GCONF_SCHEMA_CONFIG_SOURCE)' $(GCONFTOOL)

install-schemas: $(schema_DATA)
	@$(POST_INSTALL)
	@file='$(schema_DATA)'; test -f "$$file" || file="$(srcdir)/$$file"; \
	echo " $(gconftool_command) --makefile-install-rule $$file"; \
	$(gconftool_command) --makefile-install-rule $$file
	@echo; \
	echo "*** NOTE ***"; \
	echo "If you executed src/gabber before running 'make install' as privileged user,"; \
	echo "it might be necessary to restart the GConf daemon now in order for the default"; \
	echo "configuration values to take effect.  Do to so, run (as ordinary user):"; \
	echo; \
	echo "  gconftool-2 --shutdown; gconftool-2 --spawn"; \
	echo

uninstall-schemas: $(schema_DATA)
	@$(NORMAL_UNINSTALL)
	$(gconftool_command) --makefile-uninstall-rule $(schema_DATA)
