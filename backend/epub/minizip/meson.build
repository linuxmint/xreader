minizip_sources = [
    'unzip.c',
    'ioapi.c',
    'unzip.h',
    'ioapi.h',
]

minizip_deps = [
    zlib
]

minizip = static_library(
    'minizip',
    minizip_sources,
    dependencies: minizip_deps,
)

minizip_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: minizip,
)
