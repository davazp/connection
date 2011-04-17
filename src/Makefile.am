dist_pkgdata_DATA = connection.ui

bin_PROGRAMS = connection
connection_CFLAGS = $(GTK_CFLAGS) \
                    $(LOUDMOUTH_CFLAGS) \
                    -DLOCALEDIR="\"${localedir}\"" \
                    -DPKGDATADIR="\"${pkgdatadir}\""

connection_LDFLAGS = $(GTK_LIBS) $(LIBINTL) $(LOUDMOUTH_LIBS) -export-dynamic -lm
connection_SOURCES = conn.c \
                     utils.h \
                     conn-ui.c \
                     conn-ui.h \
                     conn-hex.c \
                     conn-hex.h \
                     conn-hex-widget.c \
                     conn-hex-widget.h \
                     conn-marshallers.c \
                     conn-marshallers.h \
                     conn-xmpp.c \
                     conn-xmpp.h

conn-hex-widget.c: conn-marshallers.c conn-marshallers.h
conn-hex-widget.c: conn-marshallers.c conn-marshallers.h

conn-marshallers.c: conn-marshallers.list
	glib-genmarshal --prefix conn_marshal --body $< > $@

conn-marshallers.h: conn-marshallers.list
	glib-genmarshal --prefix conn_marshal --header $< > $@

EXTRA_DIST = conn-marshallers.list

