shell_sources = [
    'eggfindbar.c',
    'eggfindbar.h',
    'ev-annotation-properties-dialog.h',
    'ev-annotation-properties-dialog.c',
    'ev-annotations-toolbar.h',
    'ev-annotations-toolbar.c',
    'ev-bookmarks.h',
    'ev-bookmarks.c',
    'ev-bookmark-action.h',
    'ev-bookmark-action.c',
    'ev-application.c',
    'ev-application.h',
    'ev-file-monitor.h',
    'ev-file-monitor.c',
    'ev-history.c',
    'ev-history.h',
    'ev-history-action.c',
    'ev-history-action.h',
    'ev-history-action-widget.c',
    'ev-history-action-widget.h',
    'ev-keyring.h',
    'ev-keyring.c',
    'ev-message-area.c',
    'ev-message-area.h',
    'ev-metadata.c',
    'ev-metadata.h',
    'ev-password-view.h',
    'ev-password-view.c',
    'ev-progress-message-area.h',
    'ev-progress-message-area.c',
    'ev-properties-dialog.c',
    'ev-properties-dialog.h',
    'ev-properties-fonts.c',
    'ev-properties-fonts.h',
    'ev-properties-license.c',
    'ev-properties-license.h',
    'ev-properties-view.c',
    'ev-properties-view.h',
    'ev-preferences-dialog.c',
    'ev-preferences-dialog.h',
    'ev-open-recent-action.c',
    'ev-open-recent-action.h',
    'ev-toolbar.c',
    'ev-toolbar.h',
    'ev-utils.c',
    'ev-utils.h',
    'ev-window.c',
    'ev-window.h',
    'ev-window-title.c',
    'ev-window-title.h',
    'ev-sidebar.c',
    'ev-sidebar.h',
    'ev-sidebar-annotations.c',
    'ev-sidebar-annotations.h',
    'ev-sidebar-attachments.c',
    'ev-sidebar-attachments.h',
    'ev-sidebar-bookmarks.h',
    'ev-sidebar-bookmarks.c',
    'ev-sidebar-layers.c',
    'ev-sidebar-layers.h',
    'ev-sidebar-links.c',
    'ev-sidebar-links.h',
    'ev-sidebar-page.c',
    'ev-sidebar-page.h',
    'ev-sidebar-thumbnails.c',
    'ev-sidebar-thumbnails.h',
    'ev-zoom-action.c',
    'ev-zoom-action.h',
]

ev_marshal = gnome.genmarshal(
    'ev-marshal',
    sources: 'ev-marshal.list',
    prefix: 'ev_marshal'
)

shell_sources += ev_marshal

ev_resources = gnome.compile_resources(
    'xreader-resources',
    'xreader.gresource.xml',
    source_dir: '../data',
    c_name: 'ev',
)

shell_sources += ev_resources

xreader_deps = [
    libdocument_dep,
    config_h,
    gio,
    glib,
    gtk,
    libsecret,
    math,
    xapp,
]

if get_option('enable_dbus')
    dbus_generated = gnome.gdbus_codegen(
        'ev-gdbus-generated',
        'ev-gdbus.xml',
        interface_prefix: 'org.x.reader',
        namespace: 'Ev',
        object_manager: true,
    )

    dbus_daemon_generated = gnome.gdbus_codegen(
        'ev-daemon-gdbus-generated',
        'ev-daemon-gdbus.xml',
        interface_prefix: 'org.x.reader',
        namespace: 'Ev',
        object_manager: true,
    )

    shell_sources += dbus_generated

    xreaderd_sources = [
        'ev-daemon.c',
    ]

    xreaderd_sources += dbus_daemon_generated

    executable(
        'xreaderd',
        xreaderd_sources,
        include_directories: include_dirs,
        dependencies: xreader_deps,
        install: true,
        install_dir: libexecdir,
    )
endif

libshell_deps = [
    xreader_deps,
    libview_dep,
    libmisc_dep,
    libtotemscrsaver_dep,
    libsmclient_dep,
    libephyzoom_dep,
]

libshell = static_library(
    'shell',
    shell_sources,
    dependencies: libshell_deps,
    include_directories: include_dirs,
)

libshell_dep = declare_dependency(
  link_whole: libshell,  # Need the whole lib for gresource lookup
  dependencies: libshell_deps,
  include_directories: include_dirs,
)

xreader = executable(
    'xreader',
    'main.c',
    dependencies: libshell_dep,
    install: true,
)
