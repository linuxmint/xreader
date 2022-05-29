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

i18n.merge_file(
    input: 'comicsdocument.xreader-backend.desktop.in',
    output: 'comicsdocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
