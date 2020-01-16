#!/usr/bin/python3

# Test reloading a document.

from testCommon import run_app, bail

from dogtail.procedural import *

try:

	run_app(file='test-page-labels.pdf')

	focus.widget('page-label-entry')
	focus.widget.text = "iii"
	activate()

	if focus.widget.text != "III":
		click('File', roleName='menu')
		click('Close', roleName='menu item')
		exit (1)

	# Close xreader
	click('File', roleName='menu')
	click('Close', roleName='menu item')

except:
	bail()
