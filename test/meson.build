test_numbers = ['1', '2', '3', '4', '5', '6', '7']

foreach number : test_numbers
    test_script = find_program('test@0@.py'.format(number))

    test(
        'test' + number,
        test_script,
        is_parallel: false,
        depends: 'xreader',
        workdir: join_paths(prefix, bindir),
    )
endforeach
