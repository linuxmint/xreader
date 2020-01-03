#!/usr/bin/python

# This test opens the Help menu and runs through the menu items.

import os
os.environ['LANG']='C'

from dogtail.procedural import *

run('xreader')

click('Help', roleName='menu')
click('About', roleName='menu item')
focus.dialog('About Xreader')
click('License', roleName='toggle button')
click('Close', roleName='push button')

focus.frame('Recent Documents')
click('Help', roleName='menu')
click('Contents', roleName='menu item')

keyCombo('<Control>w')

focus.frame('Recent Documents')
click('File', roleName='menu')
click('Close', roleName='menu item')
