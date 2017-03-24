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
# Boston, MA 02110-1301, USA.
#
""" Remove blocks module """
from __future__ import print_function
import os
import re
import sys
import glob

from .util_functions import remove_pattern_from_file
from .modtool_base import ModTool
from .cmakefile_editor import CMakeFileEditor


class ModToolRemove(ModTool):
    """ Remove block (delete files and remove Makefile entries) """
    name = 'remove'
    aliases = ('rm', 'del')

    def __init__(self):
        ModTool.__init__(self)

    def setup(self, args, positional):
        ModTool.setup(self, args, positional)
        if args.block_name is not None:
            self._info['pattern'] = args.block_name
        elif len(positional) >= 2:
            self._info['pattern'] = positional[1]
        else:
            self._info['pattern'] = raw_input('Which blocks do you want to delete? (Regex): ')
        if len(self._info['pattern']) == 0:
            self._info['pattern'] = '.'

    def run(self):
        """ Go, go, go! """
        def _remove_cc_test_case(filename=None, ed=None):
            """ Special function that removes the occurrences of a qa*.cc file
            from the CMakeLists.txt. """
            if filename[:2] != 'qa':
                return
            if self._info['version'] == '37':
                (base, ext) = os.path.splitext(filename)
                if ext == '.h':
                    remove_pattern_from_file(self._file['qalib'],
                                             '^#include "%s"\s*$' % filename)
                    remove_pattern_from_file(self._file['qalib'],
                                             '^\s*s->addTest\(gr::%s::%s::suite\(\)\);\s*$' % (
                                                 self._info['modname'], base)
                                            )
                    self.scm.mark_file_updated(self._file['qalib'])
                elif ext == '.cc':
                    ed.remove_value('list',
                                    '\$\{CMAKE_CURRENT_SOURCE_DIR\}/%s' % filename,
                                    to_ignore_start='APPEND test_%s_sources' % self._info['modname'])
                    self.scm.mark_file_updated(ed.filename)
            else:
                filebase = os.path.splitext(filename)[0]
                ed.delete_entry('add_executable', filebase)
                ed.delete_entry('target_link_libraries', filebase)
                ed.delete_entry('GR_ADD_TEST', filebase)
                ed.remove_double_newlines()
                self.scm.mark_file_updated(ed.filename)

        def _remove_py_test_case(filename=None, ed=None):
            """ Special function that removes the occurrences of a qa*.{cc,h} file
            from the CMakeLists.txt and the qa_$modname.cc. """
            if filename[:2] != 'qa':
                return
            filebase = os.path.splitext(filename)[0]
            ed.delete_entry('GR_ADD_TEST', filebase)
            ed.remove_double_newlines()

        def _make_swig_regex(filename):
            filebase = os.path.splitext(filename)[0]
            pyblockname = filebase.replace(self._info['modname'] + '_', '')
            regexp = r'(^\s*GR_SWIG_BLOCK_MAGIC2?\(%s,\s*%s\);|^\s*.include\s*"(%s/)?%s"\s*)' % \
                    (self._info['modname'], pyblockname, self._info['modname'], filename)
            return regexp
        # Go, go, go!
        if not self._skip_subdirs['lib']:
            self._run_subdir('lib', ('*.cc', '*.h','*.cpp','*.hpp',), ('add_library', 'list'),
                             cmakeedit_func=_remove_cc_test_case)
        if not self._skip_subdirs['include']:
            incl_files_deleted = self._run_subdir(self._info['includedir'], ('*.h','*.hpp',), ('install',))
        if not self._skip_subdirs['swig']:
            swig_files_deleted = self._run_subdir('swig', ('*.i',), ('install',))
            for f in incl_files_deleted + swig_files_deleted:
                # TODO do this on all *.i files
                remove_pattern_from_file(self._file['swig'], _make_swig_regex(f))
                self.scm.mark_file_updated(self._file['swig'])
        if not self._skip_subdirs['python']:
            py_files_deleted = self._run_subdir('python', ('*.py',), ('GR_PYTHON_INSTALL',),
                                                cmakeedit_func=_remove_py_test_case)
            for f in py_files_deleted:
                remove_pattern_from_file(self._file['pyinit'], '.*import\s+%s.*' % f[:-3])
                remove_pattern_from_file(self._file['pyinit'], '.*from\s+%s\s+import.*\n' % f[:-3])
        if not self._skip_subdirs['grc']:
            self._run_subdir('grc', ('*.xml',), ('install',))
        if not self._skip_subdirs['rfnoc']:
            self._run_subdir(os.path.join('rfnoc', 'blocks'),('*.xml',), ('install',))
            self._run_subdir_mk(os.path.join('rfnoc', 'fpga-src'),('*.v',),)
            self._run_tb(os.path.join('rfnoc', 'testbenches'))

    def _run_subdir(self, path, globs, makefile_vars, cmakeedit_func=None):
        """ Delete all files that match a certain pattern in path.
        path - The directory in which this will take place
        globs - A tuple of standard UNIX globs of files to delete (e.g. *.xml)
        makefile_vars - A tuple with a list of CMakeLists.txt-variables which
                        may contain references to the globbed files
        cmakeedit_func - If the CMakeLists.txt needs special editing, use this
        """
        # 1. Create a filtered list
        files = []
        for g in globs:
            files = files + glob.glob("%s/%s"% (path, g))
        files_filt = []
        print("Searching for matching files in {}/:".format(path))
        for f in files:
            if re.search(self._info['pattern'], os.path.basename(f)) is not None:
                files_filt.append(f)
        if len(files_filt) == 0:
            print("None found.")
            return []
        # 2. Delete files, Makefile entries and other occurrences
        files_deleted = []
        ed = CMakeFileEditor('%s/CMakeLists.txt' % path)
        yes = self._info['yes']
        for f in files_filt:
            b = os.path.basename(f)
            if not yes:
                ans = raw_input("Really delete %s? [Y/n/a/q]: " % f).lower().strip()
                if ans == 'a':
                    yes = True
                if ans == 'q':
                    sys.exit(0)
                if ans == 'n':
                    continue
            files_deleted.append(b)
            print("Deleting {}.".format(f))
            self.scm.remove_file(f)
            os.unlink(f)
            print("Deleting occurrences of {} from {}/CMakeLists.txt...".format(b, path))
            for var in makefile_vars:
                ed.remove_value(var, b)
            if cmakeedit_func is not None:
                cmakeedit_func(b, ed)
        ed.write()
        self.scm.mark_files_updated(('%s/CMakeLists.txt' % path,))
        return files_deleted

    def _run_subdir_mk(self, path, globs):
        """
        Runs almost like _run_subdir function, but intended for filename occurrences
        at  the rfnoc/fpga-src folder
        """
        # 1. Create a filtered list
        files = []
        for g in globs:
            files = files + glob.glob("%s/%s"% (path, g))
        files_filt = []
        print("Searching for matching files in {}/:".format(path))
        for f in files:
            if re.search(self._info['pattern'], os.path.basename(f)) is not None:
                files_filt.append(f)
        if len(files_filt) == 0:
            print("None found.")
            return []
        # 2. Delete files, Makefile entries and other occurrences
        files_deleted = []
        yes = self._info['yes']
        for f in files_filt:
            b = os.path.basename(f)
            if not yes:
                ans = raw_input("Really delete %s? [Y/n/a/q]: " % f).lower().strip()
                if ans == 'a':
                    yes = True
                if ans == 'q':
                    sys.exit(0)
                if ans == 'n':
                    continue
            files_deleted.append(b)
            print("Deleting {}.".format(f))
            self.scm.remove_file(f)
            os.unlink(f)
            print("Deleting occurrences of {} from {}/Makefile.srcs...".format(b, path))
            b = re.escape(b +' \\\n')
            remove_pattern_from_file(os.path.join(path, 'Makefile.srcs'),b)
        self.scm.mark_files_updated(('%s/Makefile.srcs' % path,))
        return files_deleted

    def _run_tb(self, path):
        """
        Remove the directories that match the blockname ocurrence. Intended for
        testbench directory removal
        """
        import shutil
        pattern = self._info['pattern']
        dirs_in_path = []
        dir_to_delete = []
        print("Searching for matching directories in {}".format(path))
        dirs_in_path = [dirs[0] for dirs in os.walk(path)]
        dir_to_delete.extend([dirs for dirs in dirs_in_path if re.search(pattern, dirs)])
        yes = self._info['yes']
        for item in dir_to_delete:
            if not yes:
                ans = raw_input("Really delete {}? [Y/n/a/q]: ".format(item)).lower().strip()
                if ans == 'a':
                    yes = True
                if ans == 'q':
                    sys.exit(0)
                if ans == 'n':
                    continue
            print("Deleting {}".format(item))
            shutil.rmtree(item)
            base = os.path.basename(item)
            print("Deleting occurrences of {} from {}/" + \
                  "CMakeLists.txt...".format(base, item))
            cmake_pattern = re.escape("add_subdirectory("+ base + ")\n")
            print(cmake_pattern)
            remove_pattern_from_file(os.path.join(path, 'CMakeLists.txt'), cmake_pattern)
