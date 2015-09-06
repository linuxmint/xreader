SUBDIRS = icons
NULL=

install-data-local: update-icon-cache

#
# man file
#

man_MANS=atril.1

#
# UI descriptions
#

uidir = $(pkgdatadir)
ui_DATA =				\
	atril-ui.xml			\
	atril-toolbar.xml		\
	hand-open.png

if ENABLE_PREVIEWER
ui_DATA += atril-previewer-ui.xml
endif

#
# Desktop file
#

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES= atril.desktop.in.in
DESKTOP_FILES= $(DESKTOP_IN_FILES:.desktop.in.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

#
# DBus servide file
#
if ENABLE_DBUS
servicedir = $(datadir)/dbus-1/services
service_in_files = org.mate.atril.Daemon.service.in
service_DATA = $(service_in_files:.service.in=.service)

$(service_DATA): $(service_in_files) Makefile
	$(AM_V_GEN) $(SED) -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@
endif

#
# App data file
#
@INTLTOOL_XML_NOMERGE_RULE@
appdatadir = $(datadir)/appdata
appdata_in_files = atril.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)

#
# GSettings schema
#
gsettings_SCHEMAS = org.mate.Atril.gschema.xml

.PRECIOUS: $(gsettings_SCHEMAS)

# include the appropriate makefile rules for schema handling
@GSETTINGS_RULES@

gsettingsconvertdir = $(datadir)/MateConf/gsettings
gsettingsconvert_DATA = atril.convert



#
# GTK icon cache
#

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache

update-icon-cache:
	if test -z "$(DESTDIR)"; then \
	        echo "Updating Gtk icon cache."; \
	        $(gtk_update_icon_cache); \
	else \
	        echo "*** Icon cache not updated.  After (un)install, run this:"; \
	        echo "***   $(gtk_update_icon_cache)"; \
	        fi


#
# Extra files to be included in the tarball
#

EXTRA_DIST =					\
	$(ui_DATA)				\
	$(DESKTOP_IN_FILES)			\
	$(appdata_in_files)			\
	$(gsettings_SCHEMAS)			\
	org.mate.atril.Daemon.service.in	\
	$(man_MANS)				\
	atril.ico				\
	atril.convert				\
	$(NULL)

#
# Clean up properly
#

CLEANFILES = \
	atril.appdata.xml

DISTCLEANFILES = \
	$(DESKTOP_FILES)	\
	$(service_DATA)

-include $(top_srcdir)/git.mk
