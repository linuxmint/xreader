AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libdocument		\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-DATRILDATADIR=\""$(datadir)"\"	\
	-DATRIL_COMPILATION			\
	$(BACKEND_CFLAGS)			\
	$(WARN_CFLAGS)				\
	$(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libimpressdocument.la

libimpressdocument_la_SOURCES =	\
	$(IMPOSTER_SOURCE_FILES)	\
	$(IMPOSTER_INCLUDE_FILES)	\
	impress-document.c		\
	impress-document.h

IMPOSTER_SOURCE_FILES =			\
	document.c			\
	f_oasis.c			\
	f_oo13.c			\
	iksemel.c			\
	r_back.c			\
	r_draw.c			\
	render.c			\
	r_geometry.c			\
	r_gradient.c			\
	r_style.c			\
	r_text.c			\
	zip.c
IMPOSTER_INCLUDE_FILES =		\
	common.h			\
	iksemel.h			\
	imposter.h			\
	internal.h			\
	zip.h
IMPOSTER_RENDER_SOURCE_FILES =		\
	render.c
IMPOSTER_RENDER_INCLUDE_FILES =		\
	render.h

libimpressdocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libimpressdocument_la_LIBADD =				\
	$(top_builddir)/libdocument/libatrildocument.la     \
	$(BACKEND_LIBS) -lz

backend_in_files = impressdocument.atril-backend.in
backend_DATA = $(backend_in_files:.atril-backend.in=.atril-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_ATRIL_BACKEND_RULE@

-include $(top_srcdir)/git.mk
