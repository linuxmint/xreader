bin_PROGRAMS = atril-thumbnailer

atril_thumbnailer_SOURCES = \
	atril-thumbnailer.c

atril_thumbnailer_CPPFLAGS = \
	-DATRILDATADIR=\"$(pkgdatadir)\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-DMATEICONDIR=\""$(datadir)/pixmaps"\" \
	$(AM_CPPFLAGS)

atril_thumbnailer_CFLAGS = \
	$(FRONTEND_CFLAGS)	\
	$(DISABLE_DEPRECATED)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

atril_thumbnailer_LDFLAGS = $(AM_LDFLAGS)

if PLATFORM_WIN32
atril_thumbnailer_LDFLAGS += -mwindows
endif

atril_thumbnailer_LDADD = \
	$(top_builddir)/libdocument/libatrildocument.la	\
	$(FRONTEND_LIBS)

thumbnailerdir = $(datadir)/thumbnailers
thumbnailer_in_files = atril.thumbnailer.in
thumbnailer_DATA = $(thumbnailer_in_files:.thumbnailer.in=.thumbnailer)


atril.thumbnailer: $(thumbnailer_in_files)
	$(AM_V_GEN)sed \
		-e "s|\@ATRIL_MIME_TYPES\@|$(ATRIL_MIME_TYPES)|" \
		$< > $@

schema_files = \
	atril-thumbnailer-dvi.schemas.in  \
	atril-thumbnailer-djvu.schemas.in \
	atril-thumbnailer-comics.schemas.in \
	atril-thumbnailer-ps.schemas.in \
	atril-thumbnailer-xps.schemas.in \
	atril-thumbnailer.schemas.in

if WITH_MATECONF

@INTLTOOL_SCHEMAS_RULE@

schema_DATA =

if ENABLE_PDF
schema_DATA += atril-thumbnailer.schemas
endif

if ENABLE_PS
schema_DATA += atril-thumbnailer-ps.schemas
endif

if ENABLE_DJVU
schema_DATA += atril-thumbnailer-djvu.schemas
endif

if ENABLE_DVI
schema_DATA += atril-thumbnailer-dvi.schemas
endif

if ENABLE_COMICS
schema_DATA += atril-thumbnailer-comics.schemas
endif

if ENABLE_XPS
schema_DATA += atril-thumbnailer-xps.schemas
endif

schemadir = $(MATECONF_SCHEMA_FILE_DIR)

install-data-local:
if MATECONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $(schema_DATA) ; do \
	MATECONF_CONFIG_SOURCE=$(MATECONF_SCHEMA_CONFIG_SOURCE) $(MATECONFTOOL) --makefile-install-rule $(top_builddir)/thumbnailer/$$p >&1 > /dev/null; \
	done \
	fi
endif

endif # WITH_MATECONF

EXTRA_DIST =         	\
	$(schema_files) \
	$(thumbnailer_in_files)

DISTCLEANFILES = \
	$(schema_DATA) \
	$(thumbnailer_DATA)

-include $(top_srcdir)/git.mk
