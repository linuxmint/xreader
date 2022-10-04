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

i18n.merge_file(
    input: 'xpsdocument.xreader-backend.desktop.in',
    output: 'xpsdocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
