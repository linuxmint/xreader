AM_CPPFLAGS =                                      \
        -I$(top_srcdir)                         \
        -I$(top_srcdir)/libdocument             \
        -DXREADER_LOCALE_DIR=\"$(datadir)/locale\"   \
        -DXREADER_COMPILATION                     \
        $(BACKEND_CFLAGS)                       \
        $(WARN_CFLAGS)                          \
        $(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libxpsdocument.la

libxpsdocument_la_SOURCES =    \
        xps-document.c         \
        xps-document.h

libxpsdocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libxpsdocument_la_LIBADD =                             \
        $(top_builddir)/libdocument/libxreaderdocument.la \
        $(BACKEND_LIBS)                 \
        $(GXPS_LIBS)

backend_in_files = xpsdocument.xreader-backend.in
backend_DATA = $(backend_in_files:.xreader-backend.in=.xreader-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_XREADER_BACKEND_RULE@

-include $(top_srcdir)/git.mk
