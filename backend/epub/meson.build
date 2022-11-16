subdir('minizip')

epub_sources = [
    'epub-document.c',
    'epub-document.h',
]

epub_deps = [
    cairo,
    glib,
    gtk,
    xml,
    minizip_dep,
]

shared_module(
    'epubdocument',
    epub_sources,
    link_with: [libdocument],
    link_whole: minizip,
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: epub_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'epub_backend',
    input: 'epubdocument.xreader-backend.in',
    output: 'epubdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
