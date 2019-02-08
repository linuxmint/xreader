synctex_sources = [
    'synctex_parser.c',
    'synctex_parser.h',
    'synctex_parser_utils.h',
    'synctex_parser_utils.c',
]

synctex_deps = [
    zlib,
]

libsynctex = static_library(
    'synctex',
    synctex_sources,
    dependencies: synctex_deps,
)
