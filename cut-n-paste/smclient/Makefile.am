noinst_LTLIBRARIES = libsmclient.la

libsmclient_la_SOURCES = \
	eggsmclient.c \
	eggsmclient.h \
	eggsmclient-private.h \
	eggdesktopfile.c \
	eggdesktopfile.h \
	eggsmclient-xsmp.c \
	$(NULL)

libsmclient_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	-DEGG_SM_CLIENT_BACKEND_XSMP \
	$(DISABLE_DEPRECATED) \
	$(AM_CPPFLAGS)

libsmclient_la_CFLAGS = \
	$(SMCLIENT_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(AM_CFLAGS)

libsmclient_la_LIBADD = \
	$(SMCLIENT_LIBS) \
	-lICE

-include $(top_srcdir)/git.mk
