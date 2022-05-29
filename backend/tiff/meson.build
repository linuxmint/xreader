tiff_sources = [
    'tiff-document.c',
    'tiff-document.h',
    'tiff2ps.c',
    'tiff2ps.h',
]

tiff_deps = [
    cairo,
    glib,
    gtk,
    math,
    tiff,
]

shared_module(
    'tiffdocument',
    tiff_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: tiff_deps,
    install: true,
    install_dir: backendsdir,
)

i18n.merge_file(
    input: 'tiffdocument.xreader-backend.desktop.in',
    output: 'tiffdocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
