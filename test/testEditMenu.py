#!/usr/bin/python3

# This test opens the Edit menu and runs through the menu items.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='test-links.pdf')

    # Begin to run through Edit options
    focus.frame('test-links.pdf')
    click('Edit', roleName='menu')

    click('Select All', roleName='menu item')

    click('Edit', roleName='menu')
    click('Find…', roleName='menu item')

    focus.frame('test-links.pdf')
    type('link')
    click('Find Previous', roleName='push button')

    click('Edit', roleName='menu')
    click('Find Next', roleName='menu item')

    click('Edit', roleName='menu')
    click('Find Previous', roleName='menu item')

    click('Edit', roleName='menu')
    click('Rotate Left', roleName='menu item')

    click('Edit', roleName='menu')
    click('Rotate Right', roleName='menu item')

    click('Edit', roleName='menu')
    click('Save Current Settings as Default', roleName='menu item')

    click('Edit', roleName='menu')
    click('Preferences', roleName='menu item')

    focus.frame('Preferences')
    click('Close', roleName='push button')

    focus.frame('test-links.pdf')
    click('File', roleName='menu')
    click('Close', roleName='menu item')

except:
    bail()
