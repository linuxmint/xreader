#!/usr/bin/python3

# This test opens the View menu and test zoom features.

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
