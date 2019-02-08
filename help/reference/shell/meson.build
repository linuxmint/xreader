version_conf = configuration_data()
version_conf.set('VERSION', version)

version_xml = configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf,
)

xreader_doc_deps = declare_dependency(
    include_directories: [include_root, libdoc_include, libview_include, include_directories('.')],
    link_with: [libdocument, libview],
    dependencies: xreader_deps,
)

gnome.gtkdoc(
    meson.project_name(),
    mode: 'xml',
    main_xml: 'xreader-docs.xml',
    gobject_typesfile: files('xreader.types'),
    src_dir: [
        join_paths(meson.source_root(), 'libview'),
    ],
    ignore_headers: [libview_private_headers],
    dependencies: xreader_doc_deps,
    mkdb_args: ['--xml-mode', '--output-format=xml', '--name-space=ev'],
    cflags: '-DXREADER_COMPILATION',
    fixxref_args: fixref_args,
    install: true,
    install_dir: meson.project_name(),
)
