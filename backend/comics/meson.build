comics_sources = [
    'comics-document.c',
    'comics-document.h'
]

comics_deps = [
    glib,
    cairo,
    gtk
]

shared_module(
    'comicsdocument',
    comics_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: comics_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'comics_backend',
    input: 'comicsdocument.xreader-backend.in',
    output: 'comicsdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
