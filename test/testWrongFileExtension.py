#!/usr/bin/python3

# Test opening a file with wrong extenstion.

from testCommon import run_app, bail

from dogtail.procedural import *

try:

	run_app(file='test-mime.bin')

	# Close xreader
	click('File', roleName='menu')
	click('Close', roleName='menu item')

except:
	bail()
