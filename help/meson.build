# developer docs
if get_option('docs')
    glib_prefix = glib.get_pkgconfig_variable('prefix')
    gtk_prefix = gtk.get_pkgconfig_variable('prefix')

    fixref_args = [
        '--extra-dir=' + join_paths(glib_prefix, gnome.gtkdoc_html_dir('glib')),
        '--extra-dir=' + join_paths(glib_prefix, gnome.gtkdoc_html_dir('gio')),
        '--extra-dir=' + join_paths(gtk_prefix, gnome.gtkdoc_html_dir('gdk')),
        '--extra-dir=' + join_paths(gtk_prefix, gnome.gtkdoc_html_dir('gdk-pixbuf')),
        '--extra-dir=' + join_paths(gtk_prefix, gnome.gtkdoc_html_dir('gtk')),
    ]

    subdir('reference/libdocument')
    subdir('reference/libview')
    subdir('reference/shell')
endif

# help files
if get_option('help_files')
  help_sources = [
      'index.docbook',
      'legal.xml',
  ]

  help_media = [
      'figures/xreader_start_window.png',
  ]

  gnome.yelp(
      meson.project_name(),
      sources: help_sources,
      media: help_media,
  )
endif
