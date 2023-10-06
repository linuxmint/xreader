backend_desktop_conf = configuration_data()
backend_desktop_conf.set('COMICS_MIME_TYPES', comic_mimetypes)

backend_desktop = configure_file(
    input: 'comicsdocument.xreader-backend.in',
    output: 'comicsdocument.xreader-backend',
    configuration: backend_desktop_conf,
    install: true,
    install_dir: backendsdir,
)

comics_sources = files(
  'comics-document.c',
  'ev-archive.c',
)

comics_deps = [
  cairo,
  glib,
  gtk,
  libarchive_dep,
]

shared_module(
    'comicsdocument',
    comics_sources,
    link_with: [libdocument],
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: include_dirs,
    dependencies: comics_deps,
    install: true,
    install_dir: backendsdir
)



test_name = 'test-ev-archive'

test_sources = files(
  'ev-archive.c',
  'test-ev-archive.c',
)

executable(
  test_name,
  test_sources,
  include_directories: include_dirs,
  dependencies: comics_deps,
)
