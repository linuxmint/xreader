zoom_sources = [
    'ephy-zoom-action.h',
    'ephy-zoom-action.c',
    'ephy-zoom-control.c',
    'ephy-zoom-control.h',
    'ephy-zoom.c',
    'ephy-zoom.h',
]

zoom_deps = [
    config_h,
    glib,
    gtk,
    math
]

libephyzoom = static_library(
    'libephyzoom',
    zoom_sources,
    include_directories: include_root,
    dependencies: zoom_deps,
)

libephyzoom_dep = declare_dependency(
    link_with: libephyzoom,
    include_directories: [include_root, include_directories('.')],
    dependencies: zoom_deps,
)
