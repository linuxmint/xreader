version_conf = configuration_data()
version_conf.set('VERSION', version)

version_xml = configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf,
)

xreader_doc_deps = declare_dependency(
    link_with: libshell.get_shared_lib(),
)

gnome.gtkdoc(
    meson.project_name(),
    mode: 'xml',
    main_xml: 'xreader-docs.xml',
    gobject_typesfile: files('xreader.types'),
    src_dir: [
        join_paths(meson.source_root(), 'shell'),
        join_paths(meson.source_root(), 'libmisc'),
    ],
    ignore_headers: [libview_private_headers],
    dependencies: xreader_doc_deps,
    mkdb_args: ['--xml-mode', '--output-format=xml', '--name-space=ev'],
    c_args: '-DXREADER_COMPILATION',
    fixxref_args: fixref_args,
    install: true,
    install_dir: meson.project_name(),
)
