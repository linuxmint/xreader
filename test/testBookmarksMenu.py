#!/usr/bin/python3

# This test opens the Bookmarks menu.

import os
os.environ['LANG']='C'

from dogtail.procedural import *

import dogtail.config
dogtail.config.config.logDebugToStdOut = True
dogtail.config.config.logDebugToFile = False

run('xreader')

# Open a file
click('File', roleName='menu')
click('Open…', roleName='menu item')
click('test-links.pdf', roleName='table cell')
click('Open', roleName='push button')

focus.frame('test-links.pdf')
click('Bookmarks', roleName='menu')
click('Add Bookmark', roleName='menu item')

click('Bookmarks', roleName='menu')
click('Page 1', roleName='menu item')

# Close
click('File', roleName='menu')
click('Close', roleName='menu item')
