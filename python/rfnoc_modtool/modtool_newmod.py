#
# Copyright 2013 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
#   Boston, MA 02110-1301, USA.
#
""" Create a whole new out-of-tree module """

from __future__ import print_function
import shutil
import os
import re
from builtins import input
from gnuradio import gr
from .modtool_base import ModTool, ModToolException

class ModToolNewModule(ModTool):
    """ Create a new out-of-tree module """
    name = 'newmod'
    aliases = ('nm', 'create')
    def __init__(self):
        ModTool.__init__(self)

    def setup_parser(self):
        " Initialise the option parser for 'rfnoc_modtool newmod'"
        parser = ModTool.setup_parser(self)
        agroup = parser.add_argument_group('newmod', "New out-of-tree module options")
        agroup.add_argument("--srcdir",
                            help="Source directory for the module template.")
        return parser

    def setup(self, args, positional):
        # Don't call ModTool.setup(), that assumes an existing module.
        self._info['modname'] = args.module_name
        if self._info['modname'] is None:
            if len(positional) >= 2:
                self._info['modname'] = positional[1]
            else:
                self._info['modname'] = input('Name of the new module: ')
        if not re.match('[a-zA-Z0-9_]+$', self._info['modname']):
            raise ModToolException('Invalid module name.')
        self._dir = args.directory
        if self._dir == '.':
            self._dir = './rfnoc-%s' % self._info['modname']
        try:
            os.stat(self._dir)
        except OSError:
            pass # This is what should happen
        else:
            raise ModToolException('The given directory exists.')

        # If --srcdir isn't specified, use the following logic to try to
        # determine the rfnoc-newmod path.
        if args.srcdir is None:
            # Try a few ways to get the rfnoc-newmod path automatically
            # Prioritize path in GNU Radio configuration file if it exists
            gr_prefs_srcdir = gr.prefs().get_string('rfnocmodtool', 'newmod_path', "")
            if gr_prefs_srcdir:
                args.srcdir = gr_prefs_srcdir
            else:
                post_path = os.path.join('share', 'gr-ettus', 'rfnoc_modtool', 'rfnoc-newmod')
                if os.environ.get('PYBOMBS_PREFIX'):
                    args.srcdir = os.path.join(os.environ.get('PYBOMBS_PREFIX'), post_path)
                else:
                    args.srcdir = os.path.join('/usr', 'local', post_path)
        self._srcdir = args.srcdir

        if not os.path.isdir(self._srcdir):
            raise ModToolException("Could not find rfnoc-newmod source dir. \n"
                    "Please run: \n\n\t $ rfnocmodtool newmod [NAME] --srcdir"
                    " {path/to/rfnoc-newmod/}\n\n"
                    "Specifying the path where the newmod template is located.")
        self.args = args
        self._setup_scm(mode='new')

    def run(self):
        """
        * Copy the example dir recursively
        * Open all files, rename rfnoc_example and RFNOC_EXAMPLE to the module name
        * Rename files and directories that contain the word rfnoc_example
        """
        print("Creating out-of-tree module in {}...".format(self._dir), end=" ")
        try:
            shutil.copytree(self._srcdir, self._dir)
            os.chdir(self._dir)
        except OSError:
            raise ModToolException('Could not create directory {}.'.format(self._dir))
        for root, _, files in  os.walk('.'):
            for filename in files:
                f = os.path.join(root, filename)
                s = open(f, 'r').read()
                s = s.replace('rfnoc_example', self._info['modname'])
                s = s.replace('RFNOC_EXAMPLE', self._info['modname'].upper())
                open(f, 'w').write(s)
                if filename.find('rfnoc_example') != -1:
                    os.rename(f, os.path.join(root,
                                              filename.replace('rfnoc_example',
                                                               self._info['modname'])))
            if os.path.basename(root) == 'rfnoc_example':
                os.rename(root, os.path.join(os.path.dirname(root), self._info['modname']))
        print("Done.")
        if self.scm.init_repo(path_to_repo="."):
            print("Created repository... you might want to commit before continuing.")
        print("Use 'rfnocmodtool add' to add a new block to this currently empty module.")
