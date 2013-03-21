AM_CPPFLAGS = \
	-I$(top_srcdir)						\
	-I$(top_srcdir)/libdocument				\
	-DMATEICONDIR=\""${prefix}/${DATADIRNAME}/pixmaps"\" 	\
	-DMATELOCALEDIR=\"$(datadir)/locale\"			\
	-DATRIL_COMPILATION			\
	$(BACKEND_CFLAGS)					\
	$(DJVU_CFLAGS)						\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libdjvudocument.la

libdjvudocument_la_SOURCES = 	\
	djvu-document.c		\
	djvu-document.h 	\
	djvu-document-private.h \
	djvu-links.c		\
	djvu-links.h		\
	djvu-text-page.c 	\
	djvu-text-page.h

libdjvudocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libdjvudocument_la_LIBADD =				\
	$(top_builddir)/libdocument/libatrildocument.la	\
	$(BACKEND_LIBS)					\
	$(DJVU_LIBS)

backend_in_files = djvudocument.atril-backend.in
backend_DATA = $(backend_in_files:.atril-backend.in=.atril-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_ATRIL_BACKEND_RULE@

-include $(top_srcdir)/git.mk
