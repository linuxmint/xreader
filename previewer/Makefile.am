
bin_PROGRAMS = atril-previewer

man_MANS = atril-previewer.1

atril_previewer_SOURCES = \
	ev-previewer.c		\
	ev-previewer-window.h	\
	ev-previewer-window.c

atril_previewer_CPPFLAGS = \
	-DATRILDATADIR=\"$(pkgdatadir)\"	\
	-I$(top_srcdir)                         \
	-I$(top_builddir) 			\
        -I$(top_srcdir)/libdocument             \
        -I$(top_srcdir)/libview                 \
	-I$(top_srcdir)/libmisc			\
	$(AM_CPPFLAGS)

atril_previewer_CFLAGS = \
	$(PREVIEWER_CFLAGS)			\
        $(WARN_CFLAGS)                          \
        $(DISABLE_DEPRECATED)			\
	$(AM_CFLAGS)    \
    $(WEBKIT_CFLAGS)

atril_previewer_LDFLAGS = $(AM_LDFLAGS)

atril_previewer_LDADD = \
	$(top_builddir)/libdocument/libatrildocument.la	\
	$(top_builddir)/libview/libatrilview.la		\
	$(top_builddir)/libmisc/libevmisc.la		\
	$(PREVIEWER_LIBS)                           \
    $(WEBKIT_LIBS)

EXTRA_DIST = $(man_MANS)

-include $(top_srcdir)/git.mk
