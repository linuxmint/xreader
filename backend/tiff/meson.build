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

custom_target(
    'tiff_backend',
    input: 'tiffdocument.xreader-backend.in',
    output: 'tiffdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
