totem_screensaver_sources = [
    'totem-scrsaver.h',
    'totem-scrsaver.c',
]

totem_screensaver_deps = [
    config_h,
    gtk,
    X11,
]

libtotemscrsaver = static_library(
    'totemscrsaver',
    totem_screensaver_sources,
    dependencies: totem_screensaver_deps,
    include_directories: include_root,
)

libtotemscrsaver_dep = declare_dependency(
    link_with: libtotemscrsaver,
    dependencies: totem_screensaver_deps,
    include_directories: [include_root, include_directories('.')],
)
