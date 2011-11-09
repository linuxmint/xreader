INCLUDES=					\
	-DEVINCEDATADIR=\"$(pkgdatadir)\"	\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(FRONTEND_CFLAGS)			\
	$(CAJA_CFLAGS)			\
	$(DISABLE_DEPRECATED)			\
	$(WARN_CFLAGS)

noinst_LTLIBRARIES = libevproperties.la

libevproperties_la_SOURCES=			\
	ev-properties-view.c			\
	ev-properties-view.h

if ENABLE_CAJA

cajaextension_LTLIBRARIES = libevince-properties-page.la

libevince_properties_page_la_CFLAGS = -I$(top_srcdir)
libevince_properties_page_la_SOURCES = 	\
	ev-properties-main.c

libevince_properties_page_la_LIBADD = 		\
	libevproperties.la			\
	$(top_builddir)/libdocument/libevdocument.la	\
	$(FRONTEND_LIBS)			\
	$(CAJA_LIBS)

libevince_properties_page_la_LDFLAGS = -module -avoid-version -no-undefined

endif # ENABLE_CAJA

-include $(top_srcdir)/git.mk
