test_cases = ['testFileMenu.py', 'testEditMenu.py', 'testHelpMenu.py',
		'testZoom.py', 'testGoMenu.py', 'testBookmarksMenu.py']

foreach case : test_cases
    test_script = find_program(case)

    test(
        case,
        test_script,
        is_parallel: false,
        depends: xreader,
        workdir: join_paths(prefix, bindir),
    )
endforeach
