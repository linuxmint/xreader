subdir('mdvi-lib')

dvi_sources = [
    'dvi-document.c',
    'dvi-document.h',
    'cairo-device.c',
    'cairo-device.h',
    'fonts.c',
    'fonts.h',
]

dvi_deps = [
    cairo,
    glib,
    gtk,
    libmdvi_dep,
    math,
    spectre,
    mdvi_deps,
]

shared_module(
    'dvidocument',
    dvi_sources,
    link_with: [libdocument],
    link_whole: libmdvi,
    c_args: mdvi_c_args,
    include_directories: include_dirs,
    dependencies: dvi_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'dvi_backend',
    input: 'dvidocument.xreader-backend.in',
    output: 'dvidocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
