#!/usr/bin/python

# This test opens a file with wrong extenstion.

import os
os.environ['LANG']='C'
srcdir = os.environ['srcdir']

from dogtail.procedural import *

run('atril', arguments=' '+srcdir+'/test-mime.bin')

# Close atril
click('File', roleName='menu')
click('Close', roleName='menu item')
