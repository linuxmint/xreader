ps_sources = [
    'ev-spectre.c',
    'ev-spectre.h'
]

ps_deps = [
    cairo,
    glib,
    gtk,
    spectre,
]

shared_module(
    'psdocument',
    ps_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: ps_deps,
    install: true,
    install_dir: backendsdir,
)

i18n.merge_file(
    input: 'psdocument.xreader-backend.desktop.in',
    output: 'psdocument.xreader-backend',
    po_dir: po_dir,
    type: 'desktop',
    args: ['--keyword=TypeDescription'],
    install: true,
    install_dir: backendsdir,
)
