thumbnailer_sources = [
    'xreader-thumbnailer.c',
]

thumbnailer_deps = [
    libdocument_dep,
    glib,
    gtk,
]

executable(
    'xreader-thumbnailer',
    thumbnailer_sources,
    include_directories: include_dirs,
    dependencies: thumbnailer_deps,
    install: true,
)

install_data(
    'xreader-thumbnailer.1',
    install_dir : join_paths(prefix, get_option('mandir'), 'man1')
)

mime_conf = configuration_data()
mime_conf.set('THUMBNAILER_MIME_TYPES', thumbnailer_mime_types)

mime_file = configure_file(
    input: 'xreader.thumbnailer.in',
    output: 'xreader.thumbnailer',
    configuration: mime_conf,
    install_dir: join_paths(datadir, 'thumbnailers')
)
