#!/usr/bin/python3

# This test opens the Bookmarks menu.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='test-links.pdf')

    focus.frame('test-links.pdf')
    click('Bookmarks', roleName='menu')
    click('Add Bookmark', roleName='menu item')

    click('Bookmarks', roleName='menu')
    click('Page 1', roleName='menu item')

    # Close
    click('File', roleName='menu')
    click('Close', roleName='menu item')

except:
    bail()
