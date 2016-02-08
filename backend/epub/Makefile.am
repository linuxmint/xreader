SUBDIRS = minizip

AM_CPPFLAGS = 		       			\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libdocument		\
	-I$(srcdir)/minizip			\
	-DMATELOCALEDIR=\"$(datadir)/locale\"	\
	-DXREADER_COMPILATION			\
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
	$(top_builddir)/libdocument/libxreaderdocument.la	\
	minizip/libminizip.la				\
	$(EPUB_LIBS)					\
	$(BACKEND_LIBS)					
	

backend_in_files = epubdocument.xreader-backend.in
backend_DATA = $(backend_in_files:.xreader-backend.in=.xreader-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_XREADER_BACKEND_RULE@

-include $(top_srcdir)/git.mk
