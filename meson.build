project('xreader', 'c', 'cpp', version : '2.0.0')

gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

version = meson.project_version()
version_list = version.split('.')
major_version = version_list[0]
minor_version = version_list[1]
micro_version = version_list[2]

api_version = '1.5'
binary_version = '3.0.0'
binary_major_version = binary_version.split('.')[0]

xreader_conf = configuration_data()
xreader_conf.set_quoted('VERSION', version)
xreader_conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())

# directories
prefix = get_option('prefix')
bindir = get_option('bindir')
datadir = get_option('datadir')
libdir = get_option('libdir')
includedir = get_option('includedir')
libexecdir = get_option('libexecdir')
desktopdir = join_paths(datadir, 'applications')
schema_dir = join_paths(datadir, 'glib-2.0', 'schemas')
header_dir = join_paths(meson.project_name(), api_version)

po_dir = join_paths(meson.source_root(), 'po')

# dependencies
cairo = dependency('cairo', version: '>= 1.14.0')
gail = dependency('gail-3.0')
gio = dependency('gio-2.0', version: '>= 2.36.0')
glib_version = '2.36.0'
glib = dependency('glib-2.0', version: '>= ' + glib_version)
gmodule = dependency('gmodule-2.0')
gtk_version = '3.14.0'
gtk_api_version = '3.0'
gtk = dependency('gtk+-' + gtk_api_version, version: '>= ' + gtk_version)
ice = dependency('ice')
sm = dependency('sm')
spectre = dependency('libspectre', version: '>= 0.2.0')
webkit = dependency('webkit2gtk-4.0', version: '>= 2.4.3')
X11 = dependency('x11')
xapp = dependency('xapp', version: '>= 1.1.0')
xml = dependency('libxml-2.0', version: '>= 2.5.0')
zlib = dependency('zlib')

if get_option('djvu')
    djvu = dependency('ddjvuapi', version: '>= 3.5.17')
endif
if get_option('dvi')
    kpathsea = dependency('kpathsea')
    if get_option('t1lib')
        t1lib = dependency('t1', required: false)
        t1_enabled = t1lib.found()
    else
        t1_enabled = false
    endif
endif
if get_option('pdf')
    poppler = dependency('poppler-glib')
endif
if get_option('tiff')
    tiff = dependency('libtiff-4')
endif
if get_option('xps')
    xps = dependency('libgxps', version: '>= 0.2.1')
endif

# on some systems we need to find the math lib to make sure it builds
cc = meson.get_compiler('c')
math = cc.find_library('m', required: false)

intltool_merge = find_program('intltool-merge')

# generate config.h
config_h_file = configure_file(
    output : 'config.h',
    configuration : xreader_conf
)

config_h = declare_dependency(
    sources: config_h_file
)

include_dirs = include_directories('.', 'libdocument', 'libview', 'libmisc')
include_root = include_directories('.')

xreader_document_header = [
    'xreader-document.h'
]

xreader_view_header = [
    'xreader-view.h'
]

# compiler flags
c_args = [
    '-DDATADIR="@0@"'.format(join_paths(prefix, datadir)),
    '-DLIBDIR="@0@"'.format(join_paths(prefix, libdir)),
    '-DBINDIR="@0@"'.format(join_paths(prefix, bindir)),
    '-DEV_BACKENDSDIR="@0@"'.format(join_paths(prefix, libdir, meson.project_name(), binary_major_version, 'backends')),
    '-DXREADERDATADIR="@0@"'.format(join_paths(prefix, datadir, meson.project_name())),
]

cpp_args = [
    '-DXREADER_LOCALE_DIR="@0@"'.format(join_paths(prefix, datadir, 'locale')),
    '-DXREADER_COMPILATION',
    '-DHAVE_CONFIG_H',
]

if get_option('debug')
    c_args += '-DEV_ENABLE_DEBUG'
endif

if get_option('epub')
    c_args += '-DENABLE_EPUB'
endif

if not get_option('deprecated_warnings')
    c_args += [
        '-Wno-deprecated-declarations',
        '-Wno-deprecated',
        '-Wno-declaration-after-statement',
    ]
endif

add_global_arguments(c_args, language: 'c')
add_global_arguments(cpp_args, language: ['c', 'cpp'])

subdir('cut-n-paste')
subdir('libdocument')
subdir('libview')
subdir('libmisc')
# 'backend' needs to be run before 'data' because the mimetypes defined in 'backend'
# are used to generate the .desktop file
subdir('backend')
subdir('data')
if get_option('thumbnailer')
    subdir('thumbnailer')
endif
if get_option('previewer')
    subdir('previewer')
endif
subdir('shell')
subdir('po')
subdir('help')
# tests are currently outdated and will not work with current xreader.
# to reenable tests, change "false" to "get_option('tests')"
if false
    subdir('test')
endif

xreader_headers = [
    'xreader-document.h',
    'xreader-view.h',
]

install_headers(
    xreader_headers,
    subdir: header_dir
)

pc_conf = configuration_data()
pc_conf.set('prefix', prefix)
pc_conf.set('exec_prefix', '${prefix}')
pc_conf.set('libdir', join_paths('${prefix}', libdir))
pc_conf.set('includedir', join_paths('${prefix}', includedir))
pc_conf.set('EV_API_VERSION', api_version)
pc_conf.set('EV_BINARY_VERSION', binary_major_version)
pc_conf.set('VERSION', version)
pc_conf.set('GLIB_REQUIRED', glib_version)
pc_conf.set('GTK_API_VERSION', gtk_api_version)
pc_conf.set('GTK_REQUIRED', gtk_version)

configure_file(
    input: 'xreader-document.pc.in',
    output: 'xreader-document-@0@.pc'.format(api_version),
    configuration: pc_conf,
    install_dir: join_paths(libdir, 'pkgconfig')
)

configure_file(
    input: 'xreader-view.pc.in',
    output: 'xreader-view-@0@.pc'.format(api_version),
    configuration: pc_conf,
    install_dir: join_paths(libdir, 'pkgconfig')
)

message('prefix = @0@'.format(prefix))
message('bindir = @0@'.format(bindir))
message('datadir = @0@'.format(datadir))
message('libdir = @0@'.format(libdir))
message('includedir = @0@'.format(includedir))
message('libexecdir = @0@'.format(libexecdir))
message('desktopdir = @0@'.format(desktopdir))
message('schema_dir = @0@'.format(schema_dir))
