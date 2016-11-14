bin_PROGRAMS = xreader-thumbnailer

man_MANS = xreader-thumbnailer.1

xreader_thumbnailer_SOURCES = \
	xreader-thumbnailer.c

xreader_thumbnailer_CPPFLAGS = \
	-DXREADERDATADIR=\"$(pkgdatadir)\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-DMATEICONDIR=\""$(datadir)/pixmaps"\" \
	$(AM_CPPFLAGS)

xreader_thumbnailer_CFLAGS = \
	$(FRONTEND_CFLAGS)	\
	$(DISABLE_DEPRECATED)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

xreader_thumbnailer_LDFLAGS = $(AM_LDFLAGS)

xreader_thumbnailer_LDADD = \
	$(top_builddir)/libdocument/libxreaderdocument.la	\
	$(FRONTEND_LIBS)

thumbnailerdir = $(datadir)/thumbnailers
thumbnailer_in_files = xreader.thumbnailer.in
thumbnailer_DATA = $(thumbnailer_in_files:.thumbnailer.in=.thumbnailer)


xreader.thumbnailer: $(thumbnailer_in_files)
	$(AM_V_GEN)sed \
		-e "s|\@XREADER_MIME_TYPES\@|$(XREADER_MIME_TYPES)|" \
		-e "s|application/epub+zip;||" \
		$< > $@

EXTRA_DIST =         	\
	$(thumbnailer_in_files) \
	$(man_MANS)

DISTCLEANFILES = \
	$(thumbnailer_DATA)

-include $(top_srcdir)/git.mk
