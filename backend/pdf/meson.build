pdf_sources = [
    'ev-poppler.cc',
    'ev-poppler.h'
]

pdf_deps = [
    gtk,
    xml,
    poppler,
]

shared_module(
    'pdfdocument',
    pdf_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: pdf_deps,
    install: true,
    install_dir: backendsdir,
)

custom_target(
    'pdf_backend',
    input: 'pdfdocument.xreader-backend.in',
    output: 'pdfdocument.xreader-backend',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: backendsdir,
)
