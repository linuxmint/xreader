libdocument_private_headers = [
    'ev-debug.h',
    'ev-module.h'
]

libdocument_headers = [
    'ev-annotation.h',
    'ev-async-renderer.h',
    'ev-attachment.h',
    'ev-backends-manager.h',
    'ev-document-factory.h',
    'ev-document-annotations.h',
    'ev-document-attachments.h',
    'ev-document-find.h',
    'ev-document-fonts.h',
    'ev-document-forms.h',
    'ev-document.h',
    'ev-document-images.h',
    'ev-document-info.h',
    'ev-document-layers.h',
    'ev-document-links.h',
    'ev-document-misc.h',
    'ev-document-print.h',
    'ev-document-security.h',
    'ev-document-thumbnails.h',
    'ev-document-transition.h',
    'ev-document-text.h',
    'ev-file-exporter.h',
    'ev-file-helpers.h',
    'ev-form-field.h',
    'ev-image.h',
    'ev-init.h',
    'ev-layer.h',
    'ev-link-action.h',
    'ev-link-dest.h',
    'ev-link.h',
    'ev-macros.h',
    'ev-mapping-list.h',
    'ev-page.h',
    'ev-render-context.h',
    'ev-selection.h',
    'ev-transition-effect.h',
]

libdocument_sources = [
    'ev-annotation.c',
    'ev-async-renderer.c',
    'ev-attachment.c',
    'ev-backends-manager.c',
    'ev-layer.c',
    'ev-link.c',
    'ev-link-action.c',
    'ev-link-dest.c',
    'ev-image.c',
    'ev-init.c',
    'ev-document.c',
    'ev-document-annotations.c',
    'ev-document-attachments.c',
    'ev-document-factory.c',
    'ev-document-thumbnails.c',
    'ev-document-fonts.c',
    'ev-document-layers.c',
    'ev-document-links.c',
    'ev-document-images.c',
    'ev-document-print.c',
    'ev-document-security.c',
    'ev-document-find.c',
    'ev-document-transition.c',
    'ev-document-forms.c',
    'ev-document-text.c',
    'ev-form-field.c',
    'ev-debug.c',
    'ev-file-exporter.c',
    'ev-file-helpers.c',
    'ev-mapping-list.c',
    'ev-module.c',
    'ev-page.c',
    'ev-render-context.c',
    'ev-selection.c',
    'ev-transition-effect.c',
    'ev-document-misc.c',
    libdocument_headers,
]

libdoc_header_dir = join_paths(header_dir, 'libdocument')

libdoc_include = include_directories('.')

libdoc_enums = gnome.mkenums(
    'ev-document-type-builtins',
    sources : libdocument_headers,
    c_template : 'ev-document-type-builtins.c.template',
    h_template : 'ev-document-type-builtins.h.template',
    identifier_prefix : 'Ev',
    symbol_prefix : 'ev',
    install_header: true,
    install_dir: join_paths(includedir, libdoc_header_dir),
)

libdocument_sources += libdoc_enums

libdoc_version_conf = configuration_data()
libdoc_version_conf.set('EV_MAJOR_VERSION', major_version)
libdoc_version_conf.set('EV_MINOR_VERSION', minor_version)
libdoc_version_conf.set('EV_MICRO_VERSION', micro_version)

libdoc_version = configure_file(
    input: 'ev-version.h.in',
    output: 'ev-version.h',
    configuration: libdoc_version_conf,
    install: true,
    install_dir: join_paths(includedir, libdoc_header_dir),
)

libdocument_sources += libdoc_version

libdocument_deps = [
    config_h,
    cairo,
    glib,
    gmodule,
    gtk,
    math,
]

libdocument = library(
    'xreaderdocument',
    libdocument_private_headers + libdocument_sources,
    dependencies: libdocument_deps,
    include_directories: [include_root, synctex_includes],
    link_whole: [libsynctex],
    soversion: binary_major_version,
    version: binary_version,
    install: true,
)

install_headers(
    libdocument_headers,
    subdir: libdoc_header_dir,
)

if get_option('introspection')
    introspection_includes = [
        'Gdk-3.0',
        'GdkPixbuf-2.0',
        'Gio-2.0',
        'GLib-2.0',
        'GObject-2.0',
        'Gtk-3.0',
    ]

    libdocument_gir = gnome.generate_gir(
        libdocument,
        namespace: 'XreaderDocument',
        nsversion: api_version,
        sources: libdocument_sources,
        identifier_prefix: 'Ev',
        symbol_prefix: 'ev',
        includes: introspection_includes,
        install: true,
        install_dir_typelib: join_paths(libdir, 'girepository-1.0'),
        install_dir_gir: join_paths(datadir, 'gir-1.0'),
        export_packages: 'xreader-document-' + api_version,
    )
endif
