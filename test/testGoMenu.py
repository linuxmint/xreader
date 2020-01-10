#!/usr/bin/python3

# This test opens the Go menu and test menu items.

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
