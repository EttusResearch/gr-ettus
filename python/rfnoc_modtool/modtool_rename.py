#!/usr/bin/env python
#
# Copyright 2014-2017 Free Software Foundation, Inc.
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
# Boston, MA 02110-1301, USA.
#
""" Module to rename blocks """
from __future__ import print_function
from builtins import input
import os
import re
from .util_functions import  ask_yes_no
from .modtool_base import ModTool, ModToolException

class ModToolRename(ModTool):
    """ Rename a block in the RFNoC out-of-tree module. """
    name = 'rename'
    description = 'Rename a block in the RFNoC out-of-tree module'
    aliases = ('mv',)

    def __init__(self):
        ModTool.__init__(self)
        self._add_cc_qa = False
        self._add_py_qa = False
        self._skip_cmakefiles = False
        self._license_file = None

    def setup_parser(self):
        parser = ModTool.setup_parser(self)
        agroup = parser.add_argument_group("Rename module options")
        agroup.add_argument("-o", "--old-name", default=None, help="Current name of the block to rename.")
        agroup.add_argument("-u", "--new-name", default=None, help="New name of the block.")
        return parser

    def setup(self, args, positional):
        ModTool.setup(self, args, positional)

        if ((self._skip_subdirs['lib'] and self._info['lang'] == 'cpp')
             or (self._skip_subdirs['python'] and self._info['lang'] == 'python')):
            raise ModToolException('Missing or skipping relevant subdir.')

        # first make sure the old block name is provided
        self._info['oldname'] = args.old_name
        if self._info['oldname'] is None:
            if len(positional) >= 2:
                self._info['oldname'] = positional[1]
            else:
                self._info['oldname'] = input("Enter name of block/code to rename (without module name prefix): ")
        if not re.match('[a-zA-Z0-9_]+', self._info['oldname']):
            raise ModToolException('Invalid block name.')
        print("Block/code to rename identifier: " + self._info['oldname'])
        self._info['fulloldname'] = self._info['modname'] + '_' + self._info['oldname']

        # now get the new block name
        self._info['newname'] = args.new_name
        if self._info['newname'] is None:
            if len(positional) >= 2:
                self._info['newname'] = positional[2]
            else:
                self._info['newname'] = input("Enter name of block/code (without module name prefix): ")
        if not re.match('[a-zA-Z0-9_]+', self._info['newname']):
            raise ModToolException('Invalid block name.')
        print("Block/code identifier: " + self._info['newname'])
        self._info['fullnewname'] = self._info['modname'] + '_' + self._info['newname']

    def run(self):
        """ Go, go, go. """
        module = self._info['modname']
        oldname = self._info['oldname']
        newname = self._info['newname']
        print("In module '{}' rename block '{}' to '{}'".format(module, oldname, newname))
        self._run_swig_rename(self._file['swig'], oldname, newname)
        self._run_grc_rename(self._info['modname'], oldname, newname)
        self._run_python_qa(oldname, newname)
        self._run_python(oldname, newname)
        self._run_lib(self._info['modname'], oldname, newname)
        self._run_include(self._info['modname'], oldname, newname)
        self._run_rfnoc_blocks(oldname, newname)
        self._run_rfnoc_fpga(oldname, newname)
        self._run_rfnoc_testbench(oldname, newname)
        return

    def _run_swig_rename(self, swigfilename, old, new):
        """ Rename SWIG includes and block_magic """
        nsubs = self._run_file_replace(swigfilename, old, new)
        if nsubs < 1:
            print("Couldn't find '{}' in file '{}'.".format(old, swigfilename))
        if nsubs == 2:
            print("Changing 'noblock' type file")
        if nsubs > 3:
            print("Hm, changed more than expected while editing {}.".format(swigfilename))
        return False

    def _run_lib(self, module, old, new):
        ccfile = './lib/' + old + '_impl.cc'
        cppfile = './lib/' + old + '_block_ctrl_impl.cpp'
        hfile = './lib/' + old + '_impl.h'
        self._run_file_replace(ccfile, old, new)
        self._run_file_replace(cppfile, old, new)
        self._run_file_replace(hfile, old, new)
        self._run_file_replace(hfile, old.upper(), new.upper())  # take care of include guards
        self._run_cmakelists('./lib/', old, new)
        self._run_file_rename('./lib/', old, new)
        self._run_cpp_qa(module, old, new)

    def _run_cpp_qa(self, module, old, new):
        path = './lib/'
        filename = 'qa_' + module + '.cc'
        nsubs = self._run_file_replace(path + filename, old, new)
        if nsubs > 0:
            print("C++ QA code detected, renaming...")
            filename = 'qa_' + old + '.cc'
            self._run_file_replace(path + filename, old, new)
            filename = 'qa_' + old + '.h'
            self._run_file_replace(path + filename, old, new)
            self._run_file_replace(path + filename, old.upper(), new.upper())
            self._run_file_rename(path, 'qa_' + old, 'qa_' + new)
        else:
            print("No C++ QA code detected, skipping...")

    def _run_rfnoc_blocks(self, old, new):
        xmlfile = './rfnoc/blocks/' + old + '.xml'
        self._run_file_replace(xmlfile, old, new)
        self._run_cmakelists('./rfnoc/blocks/', old, new)
        self._run_file_rename('./rfnoc/blocks/', old, new)

    def _run_rfnoc_fpga(self, old, new):
        vfile = './rfnoc/fpga-src/noc_block_' + old + '.v'
        makefile = './rfnoc/fpga-src/Makefile.srcs'
        self._run_file_replace(vfile, old, new)
        self._run_file_replace(makefile, old, new)
        self._run_file_rename('./rfnoc/fpga-src/', 'noc_block_'+ old, 'noc_block_'+ new)

    def _run_rfnoc_testbench(self, old, new):
        " Renames the related files from the OOT in the testbench folder "
        tb_path = os.path.join('.', 'rfnoc', 'testbenches')
        tb_file = os.path.join(tb_path, 'noc_block_' + old + '_tb', 'noc_block_' + old + '_tb.sv')
        cmakefile = os.path.join(tb_path, 'CMakeLists.txt')
        makefile = os.path.join(tb_path, 'noc_block_' + old + '_tb', 'Makefile')
        old_tb_dir = os.path.join(tb_path, 'noc_block_' + old + '_tb')
        new_tb_dir = os.path.join(tb_path, 'noc_block_' + new + '_tb')
        self._run_file_replace(tb_file, old, new)
        self._run_file_replace(cmakefile, old, new)
        self._run_file_replace(makefile, old, new)
        self._run_file_rename(tb_path, old, new)
        self._run_dir_rename(old_tb_dir, new_tb_dir)

    def _run_include(self, module, old, new):
        path = './include/' + module + '/'
        filename = path + old + '.h'
        filenamehpp = path + old + '_block_ctrl.hpp'
        self._run_file_replace(filename, old, new)
        self._run_file_replace(filenamehpp, old, new)
        self._run_file_replace(filename, old.upper(), new.upper())  # take care of include guards
        self._run_file_replace(filenamehpp, old.upper(), new.upper())  # take care of include guards
        self._run_cmakelists(path, old, new)
        self._run_file_rename(path, old, new)

    def _run_python(self, old, new):
        path = './python/'
        filename = '__init__.py'
        nsubs = self._run_file_replace(path + filename, old, new)
        if nsubs > 0:
            print("Python block detected, renaming...")
            filename = old + '.py'
            self._run_file_replace(path + filename, old, new)
            self._run_cmakelists(path, old, new)
            self._run_file_rename(path, old, new)
        else:
            print("Not a Python block, nothing to do here...")

    def _run_python_qa(self, old, new):
        new = 'qa_' + new
        old = 'qa_' + old
        filename = './python/' + old + '.py'
        self._run_file_replace(filename, old, new)
        self._run_cmakelists('./python/', old, new)
        self._run_file_rename('./python/', old, new)

    def _run_grc_rename(self, module, old, new):
        grcfile = './grc/' + module + '_' + old + '.xml'
        self._run_file_replace(grcfile, old, new)
        self._run_cmakelists('./grc/', old, new)
        self._run_file_rename('./grc/', module + '_' + old, module + '_' + new)

    def _run_cmakelists(self, path, first, second):
        filename = path + 'CMakeLists.txt'
        nsubs = self._run_file_replace(filename, first, second)
        if nsubs < 1:
            print("'{}' wasn't in '{}'.".format(first, filename))

    def _run_file_rename(self, path, old, new):
        files = os.listdir(path)
        for fil in files:
            if fil.find(old) > -1 and fil.find(old) < 3:
                nl = fil.replace(old, new)
                src = path + fil
                dst = path + nl
                print("Renaming file '{}' to '{}'.".format(src, dst))
                os.rename(src, dst)

    def _run_file_replace(self, filename, old, new):
        if not os.path.isfile(filename):
            return False
        else:
            print("In '{}' renaming occurences of '{}' to '{}'".format(filename, old, new))
        cfile = open(filename).read()
        (cfile, nsubs) = re.subn(old, new, cfile)

        open(filename, 'w').write(cfile)
        self.scm.mark_file_updated(filename)
        return nsubs

    def _run_dir_rename(self, old_dirname, new_dirname):
        " Renames a directory"
        if not os.path.isdir(old_dirname):
            return False
        else:
            print("Renaming {} directory to {}".format(old_dirname, new_dirname))
        os.rename(old_dirname, new_dirname)
