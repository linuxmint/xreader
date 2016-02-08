#!/usr/bin/python

# This test opens a file with wrong extenstion.

import os
os.environ['LANG']='C'
srcdir = os.environ['srcdir']

from dogtail.procedural import *

run('xreader', arguments=' '+srcdir+'/test-mime.bin')

# Close xreader
click('File', roleName='menu')
click('Close', roleName='menu item')
