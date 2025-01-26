version_conf = configuration_data()
version_conf.set('VERSION', version)

version_xml = configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf,
)

gnome.gtkdoc(
    'libxreaderview',
    mode: 'xml',
    main_xml: 'libxreaderview-docs.xml',
    gobject_typesfile: files('libxreaderview.types'),
    src_dir: [
        join_paths(meson.build_root(), 'libview'),
    ],
    ignore_headers: [libview_private_headers],
    dependencies: libview_dep,
    mkdb_args: ['--xml-mode', '--output-format=xml', '--name-space=ev'],
    c_args: '-DXREADER_COMPILATION',
    fixxref_args: fixref_args,
    install: true,
    install_dir: 'libxreaderview-' + api_version,
)
