#!/usr/bin/python3

# Test opening a password encrypted file and unlocking it.

from testCommon import run_app, bail

from dogtail.procedural import *

try:

	run_app(file='test-encrypt.pdf')

	# Try an incorrect password first
	focus.dialog('Enter password')
	type('wrong password')
	click('Unlock Document', roleName='push button')
	focus.dialog('Enter password')
	click('Cancel', roleName='push button')

	# Try again with the correct password
	focus.frame('test-encrypt.pdf — Password Required')
	click('Unlock Document', roleName='push button')
	type('Foo')
	focus.dialog('Enter password')
	click('Unlock Document', roleName='push button')

	# Close xreader
	focus.frame('test-encrypt.pdf — Dokument1')
	click('File', roleName='menu')
	click('Close', roleName='menu item')

except:
	bail()
