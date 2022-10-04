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

i18n.merge_file(
    input: 'epubdocument.xreader-backend.desktop.in',
    output: 'epubdocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
