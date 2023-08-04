libmisc_sources = [
    'ev-page-action.c',
    'ev-page-action.h',
    'ev-page-action-widget.c',
    'ev-page-action-widget.h',
    libdoc_enums,
    libview_enums,
]

libmisc_deps = [
    libdocument_dep,
    glib,
    gtk,
]

libmisc = static_library(
    'misc',
    libmisc_sources,
    link_with: libview,
    include_directories: include_dirs,
    dependencies: libmisc_deps,
)

libmisc_dep = declare_dependency(
    link_with: libmisc,
    include_directories: include_dirs,
    dependencies: libmisc_deps,
)
