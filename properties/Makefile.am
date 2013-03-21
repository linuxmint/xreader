AM_CPPFLAGS=					\
	-DATRILDATADIR=\"$(pkgdatadir)\"	\
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

cajaextension_LTLIBRARIES = libatril-properties-page.la

libatril_properties_page_la_CFLAGS = -I$(top_srcdir)
libatril_properties_page_la_SOURCES = 	\
	ev-properties-main.c

libatril_properties_page_la_LIBADD = 		\
	libevproperties.la			\
	$(top_builddir)/libdocument/libatrildocument.la	\
	$(FRONTEND_LIBS)			\
	$(CAJA_LIBS)

libatril_properties_page_la_LDFLAGS = -module -avoid-version -no-undefined

endif # ENABLE_CAJA

-include $(top_srcdir)/git.mk
