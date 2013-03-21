AM_CPPFLAGS =                                      \
        -I$(top_srcdir)                         \
        -I$(top_srcdir)/libdocument             \
        -DMATELOCALEDIR=\"$(datadir)/locale\"   \
        -DATRIL_COMPILATION                     \
        $(BACKEND_CFLAGS)                       \
        $(WARN_CFLAGS)                          \
        $(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libxpsdocument.la

libxpsdocument_la_SOURCES =    \
        xps-document.c         \
        xps-document.h

libxpsdocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libxpsdocument_la_LIBADD =                             \
        $(top_builddir)/libdocument/libatrildocument.la \
        $(BACKEND_LIBS)                 \
        $(GXPS_LIBS)

backend_in_files = xpsdocument.atril-backend.in
backend_DATA = $(backend_in_files:.atril-backend.in=.atril-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_ATRIL_BACKEND_RULE@

-include $(top_srcdir)/git.mk
