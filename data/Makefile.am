# This file is based on a Makefile.am from the gweled project.

ICONS = \
    apps_16x16_connection.png \
    apps_24x24_connection.png \
    apps_48x48_connection.png \
    apps_scalable_connection.svg

desktopdir = $(datadir)/applications
dist_desktop_DATA = connection.desktop

EXTRA_DIST = $(ICONS)

install-icons:
	for icon in $(ICONS); do \
		CONTEXT=`echo $$icon | cut -d_ -f1`; \
		SIZE=`echo $$icon | cut -d_ -f2`; \
		ICONFILE=`echo $$icon | cut -d_ -f3`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

uninstall-icons:
	-for icon in $(ICONS); do \
		CONTEXT=`echo $$icon | cut -d_ -f1`; \
		SIZE=`echo $$icon | cut -d_ -f2`; \
		ICONFILE=`echo $$icon | cut -d_ -f3`; \
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

install-data-local: install-icons
uninstall-local: uninstall-icons
