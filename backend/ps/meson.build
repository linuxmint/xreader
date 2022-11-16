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

custom_target(
    'ps_backend',
    input: 'psdocument.xreader-backend.in',
    output: 'psdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
