#!/usr/bin/python3

# This test opens the Go menu and test menu items.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app()

    # Open a file
    click('File', roleName='menu')
    click('Open…', roleName='menu item')
    click('test-links.pdf', roleName='table cell')
    click('Open', roleName='push button')

    focus.frame('test-links.pdf')
    click('Go', roleName='menu')
    click('Next Page', roleName='menu item')

    click('Go', roleName='menu')
    click('Previous Page', roleName='menu item')

    click('Go', roleName='menu')
    click('Last Page', roleName='menu item')

    click('Go', roleName='menu')
    click('First Page', roleName='menu item')

    # Close
    click('File', roleName='menu')
    click('Close', roleName='menu item')

except:
    bail()
