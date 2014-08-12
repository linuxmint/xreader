SUBDIRS = minizip

AM_CPPFLAGS = 		       			\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libdocument		\
	-I$(srcdir)/minizip			\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-DATRIL_COMPILATION			\
	$(BACKEND_CFLAGS)			\
	$(WARN_CXXFLAGS)			\
	$(EPUB_CFLAGS)			\
	$(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libepubdocument.la

libepubdocument_la_SOURCES =	\
	epub-document.c		\
	epub-document.h		

libepubdocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libepubdocument_la_LIBADD =				\
	$(top_builddir)/libdocument/libatrildocument.la	\
	minizip/libminizip.la				\
	$(EPUB_LIBS)					\
	$(BACKEND_LIBS)					
	

backend_in_files = epubdocument.atril-backend.in
backend_DATA = $(backend_in_files:.atril-backend.in=.atril-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_ATRIL_BACKEND_RULE@

-include $(top_srcdir)/git.mk
