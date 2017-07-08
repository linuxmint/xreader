NULL =

bin_PROGRAMS = xreader-previewer

man_MANS = xreader-previewer.1

xreader_previewer_SOURCES = \
	ev-previewer.c \
	ev-previewer-resources.c \
	ev-previewer-window.h \
	ev-previewer-window.c \
	$(NULL)

xreader_previewer_CPPFLAGS = \
	-DXREADER_LOCALE_DIR=\"$(datadir)/locale\" \
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
    
ev-previewer-resources.c: previewer.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/previewer.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate-source --c-name ev_previewer $<

EXTRA_DIST = \
	$(man_MANS) \
	previewer.gresource.xml \
	previewer-ui.xml \
	$(NULL)
	
CLEANFILES = \
	ev-previewer-resources.c \
	$(NULL)

-include $(top_srcdir)/git.mk
