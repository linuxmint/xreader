
bin_PROGRAMS = xreader-previewer

man_MANS = xreader-previewer.1

xreader_previewer_SOURCES = \
	ev-previewer.c		\
	ev-previewer-window.h	\
	ev-previewer-window.c

xreader_previewer_CPPFLAGS = \
	-DXREADERDATADIR=\"$(pkgdatadir)\"	\
	-I$(top_srcdir)                         \
	-I$(top_builddir) 			\
        -I$(top_srcdir)/libdocument             \
        -I$(top_srcdir)/libview                 \
	-I$(top_srcdir)/libmisc			\
	$(AM_CPPFLAGS)

xreader_previewer_CFLAGS = \
	$(PREVIEWER_CFLAGS)			\
        $(WARN_CFLAGS)                          \
        $(DISABLE_DEPRECATED)			\
	$(AM_CFLAGS)    \
    $(WEBKIT_CFLAGS)

xreader_previewer_LDFLAGS = $(AM_LDFLAGS)

xreader_previewer_LDADD = \
	$(top_builddir)/libdocument/libxreaderdocument.la	\
	$(top_builddir)/libview/libxreaderview.la		\
	$(top_builddir)/libmisc/libevmisc.la		\
	$(PREVIEWER_LIBS)                           \
    $(WEBKIT_LIBS)

EXTRA_DIST = $(man_MANS)

-include $(top_srcdir)/git.mk
