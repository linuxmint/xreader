pixbuf_sources = [
    'pixbuf-document.c',
    'pixbuf-document.h'
]

pixbuf_deps = [
    cairo,
    glib,
    gtk,
]

shared_module(
    'pixbufdocument',
    pixbuf_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: pixbuf_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'pixbuf_backend',
    input: 'pixbufdocument.xreader-backend.in',
    output: 'pixbufdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
