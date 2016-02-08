AM_CPPFLAGS=					\
	-DXREADERDATADIR=\"$(pkgdatadir)\"	\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(FRONTEND_CFLAGS)			\
	$(NEMO_CFLAGS)			\
	$(DISABLE_DEPRECATED)			\
	$(WARN_CFLAGS)

noinst_LTLIBRARIES = libevproperties.la

libevproperties_la_SOURCES=			\
	ev-properties-view.c			\
	ev-properties-view.h

if ENABLE_NEMO

nemoextension_LTLIBRARIES = libxreader-properties-page.la

libxreader_properties_page_la_CFLAGS = -I$(top_srcdir)
libxreader_properties_page_la_SOURCES = 	\
	ev-properties-main.c

libxreader_properties_page_la_LIBADD = 		\
	libevproperties.la			\
	$(top_builddir)/libdocument/libxreaderdocument.la	\
	$(FRONTEND_LIBS)			\
	$(NEMO_LIBS)

libxreader_properties_page_la_LDFLAGS = -module -avoid-version -no-undefined

extensiondir = $(datadir)/nemo/extensions
extension_in_files = libxreader-properties-page.nemo-extension.in
extension_DATA = $(extension_in_files:.nemo-extension.in=.nemo-extension)
%.nemo-extension: %.nemo-extension.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(AM_V_GEN) LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

EXTRA_DIST = $(extension_in_files)

endif # ENABLE_NEMO

-include $(top_srcdir)/git.mk
