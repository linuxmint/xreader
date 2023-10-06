backendsdir = join_paths(prefix, libdir, meson.project_name(), binary_major_version, 'backends')
backend_includes = []

xreader_mime_types = ''

if get_option('pdf')
    subdir('pdf')
    xreader_mime_types += 'application/pdf;application/x-bzpdf;application/x-gzpdf;application/x-xzpdf;'
endif

if get_option('ps')
    subdir('ps')
    xreader_mime_types += 'application/postscript;application/x-bzpostscript;application/x-gzpostscript;image/x-eps;image/x-bzeps;image/x-gzeps;'
endif

if get_option('dvi')
    subdir('dvi')
    xreader_mime_types += 'application/x-dvi;application/x-bzdvi;application/x-gzdvi;'
endif

if get_option('djvu')
    subdir('djvu')
    xreader_mime_types += 'image/vnd.djvu;image/vnd.djvu+multipage;'
endif

if get_option('tiff')
    subdir('tiff')
    xreader_mime_types += 'image/tiff;'
endif

if get_option('comics')
    subdir('comics')
    xreader_mime_types += 'application/vnd.comicbook-rar;application/vnd.comicbook+zip;application/x-cb7;application/x-cbr;application/x-cbt;application/x-cbz;application/x-ext-cb7;application/x-ext-cbr;application/x-ext-cbt;application/x-ext-cbz;'
endif

if get_option('pixbuf')
    subdir('pixbuf')
    xreader_mime_types += 'image/*;'
endif

if get_option('xps')
    subdir('xps')
    xreader_mime_types += 'application/oxps;application/vnd.ms-xpsdocument;'
endif

if get_option('epub')
    subdir('epub')
    xreader_mime_types += 'application/epub+zip;'
endif
