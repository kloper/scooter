# -*- mode:python -*-
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
# Copyright (c) Dimitry Kloper <kloper@users.sf.net> 2002-2012
# 
# This file is a part of the Scooter project. 
# 
# SConstruct -- main construction file for Scooter
#

import os
import sys
import atexit

from SCons.Script import ARGUMENTS

dgscons_path = ARGUMENTS.get('dgsconsdir')
dgscons_tools_path = os.path.join(dgscons_path, 'tools')

sys.path.append(dgscons_path)

import dgscons
import dgscons.version
import dgscons.build_status

boost = Tool('boost', [ dgscons_tools_path ])
hardlink = Tool('hardlink', [ dgscons_tools_path ])

env = dgscons.setup_environment(tools = ['textfile', boost, hardlink])

version = dgscons.version.version()
version.incr()

env['CRSHPREFIX'] = 'cr{0}'.format(version.version['stable'])

Export('env', 'version')

SConscript( ['zlib/SConscript' ,
             'libpng/SConscript',
             'libjpeg/SConscript',
             'libxml2/SConscript',
             'openvrml/SConscript',
             'libcurl/SConscript',
             'common/SConscript',
             'dgd/SConscript',
             'boxfish/SConscript',
             'test/SConscript'] )

atexit.register(dgscons.build_status.handle_build_atexit, version)
