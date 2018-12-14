icons = [
	'16x16/actions/object-rotate-right.png',
	'16x16/actions/zoom.png',
	'16x16/actions/zoom-fit-width.png',
	'16x16/actions/view-page-facing.png',
	'16x16/actions/view-page-continuous.png',
	'16x16/actions/object-rotate-left.png',
    '16x16/actions/zoom-fit-height.png',
    '16x16/mimetypes/x-office-presentation.png',
    '22x22/actions/object-rotate-right.png',
    '22x22/actions/zoom-fit-page.png',
    '22x22/actions/zoom.png',
    '22x22/actions/zoom-fit-width.png',
    '22x22/actions/view-page-facing.png',
    '22x22/actions/view-page-continuous.png',
    '22x22/actions/object-rotate-left.png',
    '22x22/actions/eye.png',
    '22x22/mimetypes/x-office-presentation.png',
    '24x24/actions/object-rotate-right.png',
    '24x24/actions/stock_filters-invert.png',
    '24x24/actions/zoom.png',
    '24x24/actions/zoom-fit-width.png',
    '24x24/actions/view-page-facing.png',
    '24x24/actions/view-page-continuous.png',
    '24x24/actions/object-rotate-left.png',
    '24x24/actions/zoom-fit-height.png',
    '24x24/mimetypes/x-office-presentation.png',
    '32x32/actions/object-rotate-right.png',
    '32x32/actions/view-page-facing.png',
    '32x32/actions/view-page-continuous.png',
    '32x32/actions/object-rotate-left.png',
    '32x32/mimetypes/x-office-presentation.png',
    '48x48/actions/resize-se.png',
    '48x48/actions/resize-sw.png',
    '48x48/actions/view-page-facing.png',
    '48x48/actions/view-page-continuous.png',
    '48x48/actions/close.png',
    'scalable/actions/object-rotate-left.svg',
    'scalable/actions/object-rotate-right.svg',
    'scalable/mimetypes/x-office-presentation.svg',
]

foreach icon : icons
    parts = icon.split('/')
    size = parts[0]
    context = parts[1]
    filename = parts[2]

    install_data(
        icon,
        install_dir: join_paths(datadir, meson.project_name(), 'icons', 'hicolor', size, context)
    )
endforeach

app_icons = [
	'16x16/apps/xreader.png',
    '22x22/apps/xreader.png',
    '24x24/apps/xreader.png',
    '48x48/apps/xreader.png',
    'scalable/apps/xreader.svg',
]

foreach icon : app_icons
    parts = icon.split('/')
    size = parts[0]
    context = parts[1]
    filename = parts[2]

    install_data(
        icon,
        install_dir: join_paths(datadir, 'icons', 'hicolor', size, context)
    )
endforeach
