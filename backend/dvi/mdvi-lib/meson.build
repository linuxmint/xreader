mdvi_sources = [
    'afmparse.c',
    'afmparse.h',
    'bitmap.c',
    'bitmap.h',
    'color.c',
    'color.h',
    'common.c',
    'common.h',
    'defaults.h',
    'dviopcodes.h',
    'dviread.c',
    'files.c',
    'font.c',
    'fontmap.c',
    'fontmap.h',
    'fontsrch.c',
    'gf.c',
    'hash.c',
    'hash.h',
    'list.c',
    'mdvi.h',
    'pagesel.c',
    'paper.c',
    'paper.h',
    'pk.c',
    'private.h',
    'setup.c',
    'special.c',
    'sp-epsf.c',
    'sysdeps.h',
    't1.c',
    'tfm.c',
    'tfmfile.c',
    'tt.c',
    'util.c',
    'vf.c',
]

mdvi_c_args = [
    '-DSIZEOF_INT=__SIZEOF_INT__',
    '-DSIZEOF_LONG=__SIZEOF_LONG__',
    '-DSIZEOF_SHORT=__SIZEOF_SHORT__',
    '-DSIZEOF_VOID_P=__SIZEOF_POINTER__',
]

mdvi_deps = [
    kpathsea,
]

if t1_enabled
    mdvi_c_args += '-DWITH_TYPE1_FONTS'
    mdvi_deps += t1lib
endif

libmdvi = static_library(
    'mdvi',
    mdvi_sources,
    c_args: mdvi_c_args,
    include_directories: include_dirs,
    dependencies: mdvi_deps,
)

libmdvi_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: libmdvi,
)
