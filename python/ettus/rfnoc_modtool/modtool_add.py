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
""" Module to add new blocks """

from __future__ import print_function
from builtins import input
import os
import re
from .util_functions import append_re_line_sequence, ask_yes_no, id_process
from .cmakefile_editor import CMakeFileEditor
from .modtool_base import ModTool, ModToolException
from .templates import Templates
from .code_generator import render_template
from collections import OrderedDict
from ruamel import yaml

class ModToolAdd(ModTool):
    """ Add block to the out-of-tree module. """
    name = 'add'
    aliases = ('insert',)

    def __init__(self):
        ModTool.__init__(self)
        self._add_cc_qa = False
        self._add_py_qa = False
        self._skip_cmakefiles = False
        self._skip_block_ctrl = False
        self._skip_block_interface = False
        self._license_file = None

    def setup_parser(self):
        parser = ModTool.setup_parser(self)
        agroup = parser.add_argument_group('add', "Add module options")
        agroup.add_argument("--license-file", default=None,
                            help="File containing the license header for every source code file.")
        agroup.add_argument("--noc_id", default=None,
                            help="The ID number with which the RFNoC block will" +
                            "identify itself at the SW/Hw interface")
        agroup.add_argument("--copyright", default=None,
                            help="Name of the copyright holder (you or your " + \
                                 "company) MUST be a quoted string.")
        agroup.add_argument("--argument-list", default=None,
                            help="The argument list for the constructor and make functions.")
        agroup.add_argument("--add-python-qa", action="store_true", default=None,
                            help="If given, Python QA code is automatically added if possible.")
        agroup.add_argument("--add-cpp-qa", action="store_true", default=None,
                            help="If given, C++ QA code is automatically added if possible.")
        agroup.add_argument("--skip-cmakefiles", action="store_true", default=False,
                            help="If given, only source files are written, " + \
                                 "but CMakeLists.txt files are left unchanged.")
        agroup.add_argument("--skip-block-ctrl", action="store_true", default=None,
                            help="If given, skips the generation of the RFNoC Block Controllers.")
        agroup.add_argument("--skip-block-interface", action="store_true", default=None,
                            help="If given, skips the generation of the RFNoC interface files.")
        return parser

    def setup(self, args, positional):
        ModTool.setup(self, args, positional)
        self._info['blocktype'] = 'rfnoc'
        self._info['lang'] = 'cpp'
        if (self._skip_subdirs['lib']) or (self._skip_subdirs['python']):
            raise ModToolException('Missing or skipping relevant subdir.')
        if self._info['blockname'] is None:
            if len(positional) >= 2:
                self._info['blockname'] = positional[1]
            else:
                self._info['blockname'] = input("Enter name of block/code (without module name prefix): ")
        if not re.match('^([a-zA-Z]+[0-9a-zA-Z_]*)$', self._info['blockname']):
            raise ModToolException('Invalid block name.')
        print("Block/code identifier: " + self._info['blockname'])
        self._info['fullblockname'] = self._info['modname'] + '_' + self._info['blockname']
        if not args.license_file:
            self._info['copyrightholder'] = args.copyright
            if self._info['copyrightholder'] is None:
                self._info['copyrightholder'] = '<+YOU OR YOUR COMPANY+>'
            elif self._info['is_component']:
                print("For GNU Radio components the FSF is added as copyright holder")
        self._license_file = args.license_file
        self._info['license'] = self.setup_choose_license()
        if args.argument_list is not None:
            self._info['arglist'] = args.argument_list
        else:
            self._info['arglist'] = input('Enter valid argument list, including default arguments: ')
        self._add_py_qa = args.add_python_qa
        self._add_cc_qa = args.add_cpp_qa
        if self._add_py_qa is None:
            self._add_py_qa = ask_yes_no('Add Python QA code?', False)
        if self._add_cc_qa is None:
            self._add_cc_qa = ask_yes_no('Add C++ QA code?', False)

        self._skip_cmakefiles = args.skip_cmakefiles
        if self._info['version'] == 'autofoo' and not self._skip_cmakefiles:
            print("Warning: Autotools modules are not supported. ",)
            print("Files will be created, but Makefiles will not be edited.")
            self._skip_cmakefiles = True
        #NOC ID parse
        self._info['noc_id'] = args.noc_id
        if self._info['noc_id'] is None:
            self._info['noc_id'] = id_process(input("Block NoC ID (Hexadecimal): "))
        if not re.match(r'\A[0-9A-Fa-f]+\Z', self._info['noc_id']):
            raise ModToolException('Invalid NoC ID - Only Hexadecimal Values accepted.')
        self._skip_block_ctrl = args.skip_block_ctrl
        if self._skip_block_ctrl is None:
            self._skip_block_ctrl = ask_yes_no('Skip Block Controllers Generation? [UHD block ctrl files]', False)
        self._skip_block_interface = args.skip_block_interface
        if self._skip_block_interface is None:
            self._skip_block_interface = ask_yes_no('Skip Block interface files Generation? [GRC block ctrl files]', False)

    def setup_choose_license(self):
        """ Select a license by the following rules, in this order:
        1) The contents of the file given by --license-file
        2) The contents of the file LICENSE or LICENCE in the modules
           top directory
        3) The default license. """
        if self._license_file is not None \
            and os.path.isfile(self._license_file):
            return open(self._license_file).read()
        elif os.path.isfile('LICENSE'):
            return open('LICENSE').read()
        elif os.path.isfile('LICENCE'):
            return open('LICENCE').read()
        elif self._info['is_component']:
            return Templates['grlicense']
        else:
            return render_template('defaultlicense', **self._info)

    def _write_tpl(self, tpl, path, fname):
        """ Shorthand for writing a substituted template to a file"""
        path_to_file = os.path.join(path, fname)
        print("Adding file '%s'..." % path_to_file)
        open(path_to_file, 'w').write(render_template(tpl, **self._info))
        self.scm.add_files((path_to_file,))

    def run(self):
        """ Go, go, go. """
        has_swig = (self._info['lang'] == 'cpp'
                    and not self._skip_subdirs['swig']
                   )
        has_grc = False
        self._run_lib()
        has_grc = has_swig
        if has_swig:
            self._run_swig()
        if self._add_py_qa:
            self._run_python_qa()
        if has_grc and not self._skip_subdirs['grc']:
            self._run_grc()
        if not self._skip_subdirs['examples']:
            self._run_examples()
        if not self._skip_subdirs['rfnoc']:
            self._run_rfnoc()

    def _run_lib(self):
        """ Do everything that needs doing in the subdir 'lib' and 'include'.
        - add .cc and .h files
        - include them into CMakeLists.txt
        - check if C++ QA code is req'd
        - if yes, create qa_*.{cc,h} and add them to CMakeLists.txt
        """
        def _add_qa():
            " Add C++ QA files "
            fname_qa_h = 'qa_{}.h'.format(self._info['blockname'])
            fname_qa_cc = 'qa_{}.cc'.format(self._info['blockname'])
            self._write_tpl('qa_cpp', 'lib', fname_qa_cc)
            self._write_tpl('qa_h', 'lib', fname_qa_h)
            if not self._skip_cmakefiles:
                try:
                    append_re_line_sequence(self._file['cmlib'],
                                            '\$\{CMAKE_CURRENT_SOURCE_DIR\}/qa_%s.cc.*\n' % self._info['modname'],
                                            '    ${CMAKE_CURRENT_SOURCE_DIR}/qa_%s.cc' % self._info['blockname'])
                    append_re_line_sequence(self._file['qalib'],
                                            '#include.*\n',
                                            '#include "%s"' % fname_qa_h)
                    append_re_line_sequence(self._file['qalib'],
                                            '(addTest.*suite.*\n|new CppUnit.*TestSuite.*\n)',
                                            '  s->addTest(gr::%s::qa_%s::suite());' % (self._info['modname'],
                                                                                       self._info['blockname'])
                                           )
                    self.scm.mark_files_updated((self._file['qalib'],))
                except IOError:
                    print("Can't add C++ QA files.")
        fname_cc = None
        fname_h = None
        if self._info['version'] == '37':
            #RFNoC block Interface
            if self._skip_block_interface is False:
                fname_h = self._info['blockname'] + '.h'
                fname_cc = self._info['blockname'] + '.cc'
                fname_cc = self._info['blockname'] + '_impl.cc'
                self._write_tpl('block_impl_h', 'lib', self._info['blockname'] + '_impl.h')
                self._write_tpl('block_impl_cpp', 'lib', fname_cc)
                self._write_tpl('block_def_h', self._info['includedir'], fname_h)
                if not self._skip_cmakefiles:
                    ed = CMakeFileEditor(self._file['cmlib'])
                    cmake_list_var = '[a-z]*_?' + self._info['modname'] + '_sources'
                    if not ed.append_value('list',
                                           fname_cc,
                                           to_ignore_start='APPEND ' + cmake_list_var):
                        ed.append_value('add_library', fname_cc)
                    ed.write()
                    ed = CMakeFileEditor(self._file['cminclude'])
                    ed.append_value('install', fname_h, to_ignore_end='DESTINATION[^()]+')
                    ed.write()
                    self.scm.mark_files_updated((self._file['cminclude'], self._file['cmlib']))
            #RFNoC block Controllers
            if self._skip_block_ctrl is False:
                fname_ctrl_cpp = self._info['blockname'] + '_block_ctrl_impl.cpp'
                fname_ctrl_hpp = self._info['blockname'] + '_block_ctrl.hpp'
                self._write_tpl('block_ctrl_hpp', self._info['includedir'], fname_ctrl_hpp)
                self._write_tpl('block_ctrl_cpp', 'lib', fname_ctrl_cpp)
                if not self._skip_cmakefiles:
                    ed = CMakeFileEditor(self._file['cmlib'])
                    cmake_list_var = '[a-z]*_?' + self._info['modname'] + '_sources'
                    ed.append_value('list', fname_ctrl_cpp, to_ignore_start='APPEND ' + cmake_list_var)
                    ed.write()
                    ed = CMakeFileEditor(self._file['cminclude'])
                    ed.append_value('install', fname_ctrl_hpp, to_ignore_end='DESTINATION[^()]+')
                    ed.write()
                    self.scm.mark_files_updated((self._file['cminclude'], self._file['cmlib']))
        else: # Pre-3.7 or autotools
            fname_h = self._info['fullblockname'] + '.h'
            fname_cc = self._info['fullblockname'] + '.cc'
            self._write_tpl('block_h36', self._info['includedir'], fname_h)
            self._write_tpl('block_cpp36', 'lib', fname_cc)
        if self._add_cc_qa:
            _add_qa()

    def _run_swig(self):
        """ Do everything that needs doing in the subdir 'swig'.
        - Edit main *.i file
        """
        if self._get_mainswigfile() is None:
            print('Warning: No main swig file found.')
            return
        print("Editing %s..." % self._file['swig'])
        mod_block_sep = '/'
        if self._info['version'] == '36':
            mod_block_sep = '_'
        swig_block_magic_str = render_template('swig_block_magic', **self._info)
        open(self._file['swig'], 'a').write(swig_block_magic_str)
        include_str = '#include "%s%s%s.h"' % (
            {True: 'gnuradio/' + self._info['modname'], False: self._info['modname']}[self._info['is_component']],
            mod_block_sep,
            self._info['blockname'])
        if re.search('#include', open(self._file['swig'], 'r').read()):
            append_re_line_sequence(self._file['swig'], '^#include.*\n', include_str)
        else: # I.e., if the swig file is empty
            oldfile = open(self._file['swig'], 'r').read()
            regexp = re.compile(r'^%\{\n', re.MULTILINE)
            oldfile = regexp.sub('%%{\n%s\n' % include_str, oldfile, count=1)
            open(self._file['swig'], 'w').write(oldfile)
        self.scm.mark_files_updated((self._file['swig'],))

    def _run_python_qa(self):
        """ Do everything that needs doing in the subdir 'python' to add QA code.
        - add .py files
        - include in CMakeLists.txt
        """
        fname_py_qa = 'qa_' + self._info['blockname'] + '.py'
        self._write_tpl('qa_python', self._info['pydir'], fname_py_qa)
        os.chmod(os.path.join(self._info['pydir'], fname_py_qa), 0o755)
        self.scm.mark_files_updated((os.path.join(self._info['pydir'], fname_py_qa),))
        if self._skip_cmakefiles or CMakeFileEditor(self._file['cmpython']).check_for_glob('qa_*.py'):
            return
        print("Editing %s/CMakeLists.txt..." % self._info['pydir'])
        open(self._file['cmpython'], 'a').write(
            'GR_ADD_TEST(qa_%s ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/%s)\n' % \
                  (self._info['blockname'], fname_py_qa))
        self.scm.mark_files_updated((self._file['cmpython'],))

    def _run_grc(self):
        """ Do everything that needs doing in the subdir 'grc' to add
        a GRC bindings YML file.
        - add .yml file
        - add to .tree.yml file
        - include in CMakeLists.txt
        """
        fname_grc = self._info['fullblockname'] + '.block.yml'
        self._write_tpl('grc_yml', 'grc', fname_grc)
        ed = CMakeFileEditor(self._file['cmgrc'], '\n    ')
        if self._skip_cmakefiles or ed.check_for_glob('*.yml'):
            return
        print("Editing grc/CMakeLists.txt...")
        ed.append_value('install', fname_grc, to_ignore_end='DESTINATION[^()]+')
        ed.write()

        fname_tree = self._info['modname'] + '.tree.yml'
        print("Editing grc/" + fname_tree)
        tree_yml = yaml.YAML()
        tree = tree_yml.load(open(os.path.join('grc', fname_tree), 'r').read())
        tree_key = list(tree.keys())[0]
        blocks = []
        if tree[tree_key] != None:
            blocks = tree[tree_key]
        blocks.append(self._info['fullblockname'])
        tree.update(OrderedDict([(tree_key, blocks)]))
        tree_yml.dump(tree, open(os.path.join('grc', fname_tree), 'w'))

        self.scm.mark_files_updated((self._file['cmgrc'],))

    def _run_examples(self):
        """ Add example flowgraph """
        fname_grc_example = self._info['blockname'] + '.grc'
        self._write_tpl('grc_example', 'examples', fname_grc_example)

    def _run_rfnoc(self):
        """ Do everything that needs doing in the subdir 'rfnoc'
        Adds skeleton files for
        - Block description YAML file
        - Verilog block files (including noc shell)
        - Systemverilog testbench
        - Image builder YAML file
        - include in CMakeLists.txt
        - adds verilog name to Makefile.srcs
        - Calls _run_testbenches()
        - Runs build (test)
        """
        filename_block_yml       = self._info['blockname'] + '.yml'
        filename_noc_shell       = 'noc_shell_' + self._info['blockname'] + '.v'
        filename_rfnoc_block     = 'rfnoc_block_' + self._info['blockname'] + '.v'
        filename_rfnoc_testbench = 'rfnoc_block_' + self._info['blockname'] + '_tb.sv'
        filename_icore_yml       = self._info['blockname'] + '_x310_rfnoc_image_core.yml'

        dirname_rfnoc       = 'rfnoc/'
        dirname_blocks      = dirname_rfnoc + 'blocks/'
        dirname_icores      = dirname_rfnoc + 'icores/'
        dirname_fpga        = dirname_rfnoc + 'fpga/'
        dirname_rfnoc_block = dirname_fpga  + 'rfnoc_block_' + self._info['blockname']

        # Add directory for block HDL & testbench
        if (not os.path.exists(dirname_rfnoc_block)):
            os.mkdir(dirname_rfnoc_block)

        self._write_tpl('block_cmake', dirname_blocks, 'CMakeLists.txt')
        self._write_tpl('block_yml', dirname_blocks, filename_block_yml)
        self._write_tpl('fpga_cmake', dirname_fpga, 'CMakeLists.txt')
        self._write_tpl('fpga_makefile_srcs', dirname_fpga, 'Makefile.srcs')
        self._write_tpl('fpga_rfnoc_block_cmake', dirname_rfnoc_block, 'CMakeLists.txt')
        self._write_tpl('fpga_rfnoc_block_makefile_srcs', dirname_rfnoc_block, 'Makefile.srcs')
        self._write_tpl('fpga_rfnoc_block_makefile', dirname_rfnoc_block, 'Makefile')
        self._write_tpl('fpga_rfnoc_block_noc_shell', dirname_rfnoc_block, filename_noc_shell)
        self._write_tpl('fpga_rfnoc_block', dirname_rfnoc_block, filename_rfnoc_block)
        self._write_tpl('fpga_rfnoc_testbench', dirname_rfnoc_block, filename_rfnoc_testbench)
        self._write_tpl('icore_cmake', dirname_icores, 'CMakeLists.txt')
        self._write_tpl('icore_yml', dirname_icores, filename_icore_yml)

        # Update file timestamp to force cmake to update makefiles
        self.scm.mark_files_updated((dirname_rfnoc + 'CMakeLists.txt',))
