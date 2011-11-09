
bin_PROGRAMS = atril-previewer

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
	$(AM_CFLAGS)

atril_previewer_LDFLAGS = $(AM_LDFLAGS)

if PLATFORM_WIN32
atril_previewer_LDFLAGS += -mwindows
endif

atril_previewer_LDADD = \
	$(top_builddir)/libdocument/libevdocument.la	\
	$(top_builddir)/libview/libevview.la		\
	$(top_builddir)/libmisc/libevmisc.la		\
	$(PREVIEWER_LIBS)


-include $(top_srcdir)/git.mk
