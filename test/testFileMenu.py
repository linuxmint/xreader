#!/usr/bin/python3

# Test that the File menu and menu items work correctly.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='test-links.pdf')

    # Open a file
    click('File', roleName='menu')
    click('Open…', roleName='menu item')
    click('Cancel', roleName='push button')

    # Save a Copy
    focus.frame('test-links.pdf')
    click('File', roleName='menu')
    click('Save a Copy…', roleName='menu item')
    click('Cancel', roleName='push button')

    # Print
    focus.frame('test-links.pdf')
    click('File', roleName='menu')
    click('Print…', roleName='menu item')
    focus.dialog('Print')
    click('Cancel', roleName='push button')

    # Properties
    focus.frame('test-links.pdf')
    click('File', roleName='menu')
    click('Properties', roleName='menu item')
    click('Fonts', roleName='page tab')
    click('General', roleName='page tab')
    focus.dialog('Properties')
    click('Close', roleName='push button')

    # Close All Windows
    focus.frame('test-links.pdf')
    click('File', roleName='menu')
    click('Close All Windows', roleName='menu item')

except:
    bail()
