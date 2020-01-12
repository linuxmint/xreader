#!/usr/bin/python3

# This test opens the View menu and test zoom features.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='test-links.pdf')

    # Zoom In
    focus.frame('test-links.pdf')
    click('View', roleName='menu')
    click('Zoom In', roleName='menu item')

    # Zoom Out
    click('View', roleName='menu')
    click('Zoom Out', roleName='menu item')

    # Close
    click('File', roleName='menu')
    click('Close', roleName='menu item')

except:
    bail()
