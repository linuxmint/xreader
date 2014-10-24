AM_CPPFLAGS = \
	$(WARN_CFLAGS) \
	$(ZLIB_CFLAGS)

noinst_LTLIBRARIES = libminizip.la

libminizip_la_SOURCES = \
	unzip.c		\
	ioapi.c

libminizip_la_SOURCES = \
	unzip.h		\
	ioapi.h

libminizip_la_LIBADD =  \
	$(ZLIB_LIBS)

-include $(top_srcdir)/git.mk
