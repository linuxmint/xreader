SUBDIRS = reference

if ENABLE_HELP

include $(top_srcdir)/mate-doc-utils.make
dist-hook: doc-dist-hook

DOC_MODULE = atril
DOC_ENTITIES = legal.xml
DOC_INCLUDES = 
DOC_FIGURES = figures/atril_start_window.png

DOC_LINGUAS = bg ca cs de el en_GB es eu fi fr it ja nl oc pt_BR ru sl sr sv uk vi zh_CN

-include $(top_srcdir)/git.mk

DOCDIR = $(DESTDIR)$(docdir)

endif

htmldir=$(datadir)/doc/mate/atril/

html_DATA = C/atril.html \
            C/figures/atril_start_window.png

EXTRA_DIST = $(html_DATA)
