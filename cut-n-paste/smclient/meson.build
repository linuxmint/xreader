smclient_sources = [
    'eggsmclient.c',
    'eggsmclient.h',
    'eggsmclient-private.h',
    'eggdesktopfile.c',
    'eggdesktopfile.h',
    'eggsmclient-xsmp.c',
]

smclient_deps = [
    config_h,
    gio,
    glib,
    gtk,
    ice,
    sm,
]

libsmclient = static_library(
    'smclient',
    smclient_sources,
    dependencies: smclient_deps,
    include_directories: include_dirs,
)

libsmclient_dep = declare_dependency(
    link_with: libsmclient,
    dependencies: smclient_deps,
    include_directories: [include_root, include_directories('.')],
)
