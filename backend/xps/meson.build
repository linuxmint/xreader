xps_sources = [
    'xps-document.c',
    'xps-document.h',
]

xps_deps = [
    cairo,
    glib,
    gtk,
    xps,
]

shared_module(
    'xpsdocument',
    xps_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: xps_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'xps_backend',
    input: 'xpsdocument.xreader-backend.in',
    output: 'xpsdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
