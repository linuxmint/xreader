NULL =

AM_CPPFLAGS=							\
	-DXREADERDATADIR=\"$(pkgdatadir)\"				\
	-DXREADER_LOCALE_DIR=\"$(datadir)/locale\"	\
	-DMATEDATADIR=\"$(datadir)\"				\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/cut-n-paste/zoom-control/		\
	-I$(top_srcdir)/cut-n-paste/totem-screensaver/		\
	-I$(top_srcdir)/cut-n-paste/smclient/			\
	-I$(top_srcdir)/libdocument				\
	-I$(top_builddir)/libdocument				\
	-I$(top_srcdir)/libview					\
	-I$(top_builddir)/libview				\
	-I$(top_srcdir)/libmisc					\
	-DMATEICONDIR=\""$(datadir)/pixmaps"\" 		\
	-DBINDIR=\""$(bindir)"\"				\
	-DLIBEXECDIR=\""$(libexecdir)"\"			\
	-DXREADER_COMPILATION					\
	$(SHELL_CFLAGS)						\
	$(MATE_DESKTOP_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED)

bin_PROGRAMS=xreader

if ENABLE_DBUS
libexec_PROGRAMS = xreaderd
endif

xreader_SOURCES=				\
	eggfindbar.c			\
	eggfindbar.h			\
	ev-annotation-properties-dialog.h \
	ev-annotation-properties-dialog.c \
	ev-bookmarks.h			\
	ev-bookmarks.c			\
	ev-bookmark-action.h		\
	ev-bookmark-action.c		\
	ev-application.c		\
	ev-application.h		\
	ev-file-monitor.h		\
	ev-file-monitor.c		\
	ev-history.c			\
	ev-history.h			\
	ev-history-action.c 	\
	ev-history-action.h 	\
	ev-history-action-widget.c \
	ev-history-action-widget.h \
	ev-keyring.h			\
	ev-keyring.c			\
	ev-marshal.c			\
	ev-marshal.h			\
	ev-message-area.c		\
	ev-message-area.h		\
	ev-metadata.c			\
	ev-metadata.h			\
	ev-password-view.h		\
	ev-password-view.c		\
	ev-progress-message-area.h	\
	ev-progress-message-area.c	\
	ev-properties-dialog.c		\
	ev-properties-dialog.h		\
	ev-properties-fonts.c		\
	ev-properties-fonts.h		\
	ev-properties-license.c		\
	ev-properties-license.h		\
    ev-properties-view.c        \
	ev-properties-view.h		\
	ev-preferences-dialog.c		\
	ev-preferences-dialog.h		\
	ev-open-recent-action.c		\
	ev-open-recent-action.h		\
	ev-resources.c \
	ev-resources.h \
	ev-toolbar.c 		\
	ev-toolbar.h 		\
	ev-utils.c			\
	ev-utils.h			\
	ev-window.c			\
	ev-window.h			\
	ev-window-title.c		\
	ev-window-title.h		\
	ev-sidebar.c			\
	ev-sidebar.h			\
	ev-sidebar-annotations.c	\
	ev-sidebar-annotations.h	\
	ev-sidebar-attachments.c	\
	ev-sidebar-attachments.h	\
	ev-sidebar-bookmarks.h		\
	ev-sidebar-bookmarks.c		\
	ev-sidebar-layers.c		\
	ev-sidebar-layers.h		\
	ev-sidebar-links.c		\
	ev-sidebar-links.h		\
	ev-sidebar-page.c		\
	ev-sidebar-page.h		\
	ev-sidebar-thumbnails.c		\
	ev-sidebar-thumbnails.h		\
	main.c

if ENABLE_DBUS
nodist_xreader_SOURCES = \
	ev-gdbus-generated.c \
	ev-gdbus-generated.h \
	$(NULL)
endif

xreader_LDFLAGS = $(AM_LDFLAGS)

xreader_LDADD=										\
	$(top_builddir)/cut-n-paste/zoom-control/libephyzoom.la				\
	$(top_builddir)/cut-n-paste/totem-screensaver/libtotemscrsaver.la		\
	$(top_builddir)/cut-n-paste/smclient/libsmclient.la				\
	$(top_builddir)/libdocument/libxreaderdocument.la					\
	$(top_builddir)/libview/libxreaderview.la						\
	$(top_builddir)/libmisc/libevmisc.la						\
	$(SHELL_LIBS)									\
	$(MATE_DESKTOP_LIBS)

if ENABLE_CAJA
    xreader_LDADD+=$(top_builddir)/properties-caja/libevproperties.la
endif

if ENABLE_NEMO
    xreader_LDADD+=$(top_builddir)/properties-nemo/libevproperties.la
endif


BUILT_SOURCES = \
	ev-marshal.h \
	ev-marshal.c \
	ev-resources.c

if ENABLE_DBUS
BUILT_SOURCES += \
	ev-gdbus-generated.c \
	ev-gdbus-generated.h \
	ev-daemon-gdbus-generated.c \
	ev-daemon-gdbus-generated.h
endif

if ENABLE_DBUS
xreaderd_SOURCES = \
	ev-daemon.c \
	$(NULL)

nodist_xreaderd_SOURCES = \
	ev-daemon-gdbus-generated.c \
	ev-daemon-gdbus-generated.h
	$(NULL)

xreaderd_CFLAGS=				\
	-DXREADERDATADIR=\"$(pkgdatadir)\"			\
	-DMATEDATADIR=\"$(datadir)\"				\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-DLIBEXECDIR=\""$(libexecdir)"\"			\
	-DXREADER_COMPILATION					\
	$(EV_DAEMON_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED)

xreaderd_LDADD=				\
	$(EV_DAEMON_LIBS)
endif

EXTRA_DIST = \
	ev-marshal.list \
	xreader-ui.xml \
	xreader.gresource.xml \
	ev-gdbus.xml \
	ev-daemon-gdbus.xml

ev-marshal.h: $(srcdir)/ev-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=ev_marshal $(srcdir)/ev-marshal.list --header > ev-marshal.h

ev-marshal.c: $(srcdir)/ev-marshal.list
	echo '#include "ev-marshal.h"' > ev-marshal.c
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=ev_marshal $(srcdir)/ev-marshal.list --body >> ev-marshal.c

ev-resources.c: xreader.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/xreader.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate-source --c-name ev $<

ev-gdbus-generated.c ev-gdbus-generated.h: ev-gdbus.xml Makefile
	$(AM_V_GEN) $(GDBUS_CODEGEN) \
			--interface-prefix=org.x.reader \
			--c-namespace=Ev \
			--c-generate-object-manager \
			--generate-c-code ev-gdbus-generated \
			$<

ev-daemon-gdbus-generated.c ev-daemon-gdbus-generated.h: ev-daemon-gdbus.xml Makefile
	$(AM_V_GEN) $(GDBUS_CODEGEN) \
			--interface-prefix=org.x.reader \
			--c-namespace=Ev \
			--c-generate-object-manager \
			--generate-c-code ev-daemon-gdbus-generated \
			$<

DISTCLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
