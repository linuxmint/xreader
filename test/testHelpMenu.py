#!/usr/bin/python3

# This test opens the Help menu and runs through the menu items.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app()

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
except:
    bail()
