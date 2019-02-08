version_conf = configuration_data()
version_conf.set('VERSION', version)

version_xml = configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf,
)

libdoc_doc_deps = declare_dependency(
    include_directories: [libdoc_include, include_directories('.')],
    link_with: libdocument,
    dependencies: libdocument_deps,
)

gnome.gtkdoc(
    'libxreaderdocument',
    mode: 'xml',
    main_xml: 'libxreaderdocument-docs.xml',
    gobject_typesfile: files('libxreaderdocument.types'),
    src_dir: [
        join_paths(meson.source_root(), 'libdocument'),
    ],
    ignore_headers: [libdocument_private_headers],
    dependencies: libdoc_doc_deps,
    mkdb_args: ['--xml-mode', '--output-format=xml', '--name-space=ev'],
    fixxref_args: fixref_args,
    install: true,
    install_dir: 'libxreaderdocument-' + api_version,
)
