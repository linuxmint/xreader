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

i18n.merge_file(
    input: 'dvidocument.xreader-backend.desktop.in',
    output: 'dvidocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
