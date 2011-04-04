dist_pkgdata_DATA = connection.glade

bin_PROGRAMS = connection
connection_CFLAGS = $(GTK_CFLAGS) -DLOCALEDIR="\"${localedir}\"" -DPKGDATADIR="\"${pkgdatadir}\""
connection_LDFLAGS = $(GTK_LIBS) $(LIBINTL) -export-dynamic -lm
connection_SOURCES = conn.c \
                     conn-ui.c \
                     conn-ui.h \
                     conn-utils.c \
                     conn-utils.h \
                     conn-hex.c \
                     conn-hex.h \
                     conn-hex-widget.c \
                     conn-hex-widget.h \
                     conn-marshallers.c \
                     conn-marshallers.h

conn-hex-widget.c: conn-marshallers.c conn-marshallers.h
conn-hex-widget.c: conn-marshallers.c conn-marshallers.h

conn-marshallers.c: conn-marshallers.list
	glib-genmarshal --prefix conn_marshal --body $< > $@

conn-marshallers.h: conn-marshallers.list
	glib-genmarshal --prefix conn_marshal --header $< > $@
