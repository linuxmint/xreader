subdir('icons')

data_dir = include_directories('.')

install_data(
    'xreader.1',
    install_dir : join_paths(get_option('prefix'), get_option('mandir'), 'man1')
)

service_conf = configuration_data()
service_conf.set('libexecdir', join_paths(prefix, libdir))

configure_file(
    input: 'org.x.reader.Daemon.service.in',
    output: 'org.x.reader.Daemon.service',
    configuration: service_conf,
    install_dir: join_paths(datadir, 'dbus-1', 'services')
)

schema_file = install_data(
    'org.x.reader.gschema.xml',
    install_dir: join_paths(datadir, 'glib-2.0', 'schemas')
)

desktop_conf = configuration_data()
desktop_conf.set('XREADER_MIME_TYPES', xreader_mime_types)

desktop = configure_file(
    input: 'xreader.desktop.in.in',
    output: 'xreader.desktop.in',
    configuration: desktop_conf,
)

custom_target(
    'desktop',
    input: desktop,
    output: 'xreader.desktop',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: desktopdir,
)

appdata = custom_target(
    'appdata',
    input: 'xreader.appdata.xml.in',
    output: 'xreader.appdata.xml',
    command: [intltool_merge, '-x', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: join_paths(datadir, 'appdata'),
)

install_data(
    'hand-open.png',
    install_dir: join_paths(prefix, datadir, meson.project_name())
)
