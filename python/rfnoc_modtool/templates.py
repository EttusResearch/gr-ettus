#!/usr/bin/env python
#
# Copyright 2013-2017 Free Software Foundation, Inc.
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


''' All the templates for skeleton files (needed by ModToolAdd) '''

from datetime import datetime

Templates = {}


# Default licence
Templates['defaultlicense'] = '''
Copyright {0} <+YOU OR YOUR COMPANY+>.

This is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street,
Boston, MA 02110-1301, USA.
'''.format(datetime.now().year)

Templates['grlicense'] = '''
Copyright {0} Free Software Foundation, Inc.

This file is part of GNU Radio

GNU Radio is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GNU Radio is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Radio; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street,
Boston, MA 02110-1301, USA.
'''.format(datetime.now().year)

# Header file of a sync/decimator/interpolator block
Templates['block_impl_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}
#pragma once

#include <${include_dir_prefix}/${blockname}.h>
#include <${include_dir_prefix}/${blockname}_block_ctrl.hpp>

namespace gr {
  namespace ${modname} {

    class ${blockname}_impl : public ${blockname}
    {
     public:
      ${blockname}_impl(::uhd::rfnoc::noc_block_base::sptr block_ref);
      ~${blockname}_impl();

      // Where all the action really happens

     private:
      ::uhd::rfnoc::${blockname}_block_ctrl::sptr d_${blockname}_ref;
    };

  } // namespace ${modname}
} // namespace gr

'''

# C++ file of a GR block
Templates['block_impl_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "${blockname}_impl.h"
namespace gr {
  namespace ${modname} {
    ${blockname}::sptr
    ${blockname}::make(
        const gr::ettus::rfnoc_graph::sptr graph,
        const ::uhd::device_addr_t& block_args,
        const int device_select,
        const int instance
    )
    {
      return gnuradio::get_initial_sptr(
        new ${blockname}_impl(
          gr::ettus::rfnoc_block::make_block_ref(
            graph, block_args, "${blockname}", device_select, instance)));
    }

    /*
     * The private constructor
     */
    ${blockname}_impl::${blockname}_impl(
        ::uhd::rfnoc::noc_block_base::sptr block_ref) :
      gr::ettus::rfnoc_block(block_ref),
      d_${blockname}_ref(get_block_ref<::uhd::rfnoc::${blockname}_block_ctrl>())
    {
    }

    /*
     * Our virtual destructor.
     */
    ${blockname}_impl::~${blockname}_impl()
    {
    }

  } /* namespace ${modname} */
} /* namespace gr */

'''

# Block definition header file (for include/)
Templates['block_def_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}

#pragma once

#include <${include_dir_prefix}/api.h>
#include <ettus/rfnoc_block.h>
#include <ettus/rfnoc_graph.h>

namespace gr {
  namespace ${modname} {

    /*!
     * \\brief <+description of block+>
     * \ingroup ${modname}
     *
     */
    class ${modname.upper()}_API ${blockname} : virtual public gr::ettus::rfnoc_block
    {
     public:
      typedef boost::shared_ptr<${blockname}> sptr;

      /*!
       * \\brief Return a shared_ptr to a new instance of ${modname}::${blockname}.
       *
       * To avoid accidental use of raw pointers, ${modname}::${blockname}'s
       * constructor is in a private implementation
       * class. ${modname}::${blockname}::make is the public interface for
       * creating new instances.
       */
      static sptr make(
        const gr::ettus::rfnoc_graph::sptr graph,
        const ::uhd::device_addr_t& block_args,
        const int device_select,
        const int instance);
    };
  } // namespace ${modname}
} // namespace gr


'''
# Header for RFNoC Block Controller (UHD Host-part)
Templates['block_ctrl_hpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}

#pragma once

#include <uhd/config.hpp>
#include <uhd/rfnoc/noc_block_base.hpp>

namespace uhd {
    namespace rfnoc {

/*! \\brief Block controller for the standard copy RFNoC block.
 *
 */
class UHD_API ${blockname}_block_ctrl : public noc_block_base
{
public:
    RFNOC_DECLARE_BLOCK(${blockname}_block_ctrl)

    static const uint32_t REG_USER_ADDR;
    static const uint32_t REG_USER_DEFAULT;

}; /* class ${blockname}_block_ctrl*/

}} /* namespace uhd::rfnoc */

'''

# RFNoC Block Controller (UHD Host-part)
Templates['block_ctrl_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}

#include <${include_dir_prefix}/${blockname}_block_ctrl.hpp>
#include <uhd/rfnoc/registry.hpp>

using namespace uhd::rfnoc;

// Note: Register addresses should increment by 4
const uint32_t ${blockname}_block_ctrl::REG_USER_ADDR    = 0;
const uint32_t ${blockname}_block_ctrl::REG_USER_DEFAULT = 0;

class ${blockname}_block_ctrl_impl : public ${blockname}_block_ctrl
{
public:
    RFNOC_BLOCK_CONSTRUCTOR(${blockname}_block_ctrl)
    {
        _register_props();
    }
private:
    void _register_props()
    {
        register_property(&_user_reg, [this]() {
            int user_reg = this->_user_reg.get();
            this->regs().poke32(REG_USER_ADDR, user_reg);
        });

        // register edge properties
        register_property(&_type_in);
        register_property(&_type_out);

        // add resolvers for type (keeps it constant)
        add_property_resolver({&_type_in}, {&_type_in}, [& type_in = _type_in]() {
            type_in.set(IO_TYPE_SC16);
        });
        add_property_resolver({&_type_out}, {&_type_out}, [& type_out = _type_out]() {
            type_out.set(IO_TYPE_SC16);
        });
    }

    property_t<int> _user_reg{"user_reg", REG_USER_DEFAULT, {res_source_info::USER}};

    property_t<std::string> _type_in = property_t<std::string>{
        PROP_KEY_TYPE, IO_TYPE_SC16, {res_source_info::INPUT_EDGE}};
    property_t<std::string> _type_out = property_t<std::string>{
        PROP_KEY_TYPE, IO_TYPE_SC16, {res_source_info::OUTPUT_EDGE}};

};

UHD_RFNOC_BLOCK_REGISTER_DIRECT(${blockname}_block_ctrl, 0x${noc_id}, "${blockname}", CLOCK_KEY_GRAPH, "bus_clk");
'''

# C++ file for QA
Templates['qa_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}

#include <gnuradio/attributes.h>
#include <cppunit/TestAssert.h>
#include "qa_${blockname}.h"
#include <${include_dir_prefix}/${blockname}.h>

namespace gr {
  namespace ${modname} {

    void
    qa_${blockname}::t1()
    {
      // Put test here
    }

  } /* namespace ${modname} */
} /* namespace gr */

'''

# Header file for QA
Templates['qa_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment(license)}

#ifndef _QA_${blockname.upper()}_H_
#define _QA_${blockname.upper()}_H_

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

namespace gr {
  namespace ${modname} {

    class qa_${blockname} : public CppUnit::TestCase
    {
    public:
      CPPUNIT_TEST_SUITE(qa_${blockname});
      CPPUNIT_TEST(t1);
      CPPUNIT_TEST_SUITE_END();

    private:
      void t1();
    };

  } /* namespace ${modname} */
} /* namespace gr */

#endif /* _QA_${blockname.upper()}_H_ */

'''

# Python QA code TODO check this
Templates['qa_python'] = '''#!/usr/bin/env python
# -*- coding: utf-8 -*-
${str_to_python_comment(license)}


from gnuradio import gr, gr_unittest
from gnuradio import blocks
import ${modname}_swig as ${modname}

class qa_${blockname} (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        self.tb.run ()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_${blockname}, "qa_${blockname}.xml")
'''

Templates['grc_yml'] = '''id: ${modname}_${blockname}
label: RFNoC ${blockname}

templates:
  imports: |-
    import ${modname}
  make: |-
    ${modname}.${blockname}(
      self.rfnoc_graph,
      uhd.device_addr(${'${block_args}'}),
      ${'${device_select}'},
      ${'${instance_select}'})
    self.${'${id}'}.set_int_property('user_reg', ${'${user_reg}'})
  callbacks:
  - set_int_property('user_reg', ${'${user_reg}'})

# Make one 'parameter' node for every Parameter you want settable from the GUI.

parameters:
- id: user_reg
  label: User Register
  dtype: int
  default: 0
- id: block_args
  label: Block Args
  dtype: string
  default: ""
- id: device_select
  label: Device Select
  dtype: int
  default: -1
- id: instance_select
  label: Instance Select
  dtype: int
  default: -1

# Make one 'inputs' node per input. Include:
#    label (an identifier for the GUI)
#    dtype (data type of expected data)
#    optional (set to 1 for optional inputs)
inputs:
- domain: rfnoc
  label: in
  dtype: 'sc16'

# Make out 'outputs' node per output.
#    label (an identifier for the GUI)
#    dtype (data type of expected data)
#    optional (set to 1 for optional outputs)
outputs:
- domain: rfnoc
  label: out
  dtype: 'sc16'

file_format: 1
'''

# Example GRC flowgraph
Templates['grc_example'] = '''
options:
  parameters:
    author: ''
    category: '[GRC Hier Blocks]'
    cmake_opt: ''
    comment: ''
    copyright: ''
    description: ''
    gen_cmake: 'On'
    gen_linking: dynamic
    generate_options: qt_gui
    hier_block_src_path: '.:'
    id: rfnoc_${blockname}
    max_nouts: '0'
    output_language: python
    placement: (0,0)
    qt_qss_theme: ''
    realtime_scheduling: ''
    run: 'True'
    run_command: '{python} -u {filename}'
    run_options: prompt
    sizing_mode: fixed
    thread_safe_setters: ''
    title: 'RFNoC: ${blockname} Example'
    window_size: ''
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [8, 8]
    rotation: 0
    state: enabled

blocks:
- name: ettus_rfnoc_graph
  id: ettus_rfnoc_graph
  parameters:
    alias: ''
    clock_source_0: ''
    clock_source_1: ''
    clock_source_2: ''
    clock_source_3: ''
    clock_source_4: ''
    clock_source_5: ''
    clock_source_6: ''
    clock_source_7: ''
    comment: ''
    dev_addr: ''
    dev_args: type=x300
    num_mboards: '1'
    time_source_0: ''
    time_source_1: ''
    time_source_2: ''
    time_source_3: ''
    time_source_4: ''
    time_source_5: ''
    time_source_6: ''
    time_source_7: ''
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [224, 8.0]
    rotation: 0
    state: true
- name: samp_rate
  id: variable_qtgui_entry
  parameters:
    comment: ''
    gui_hint: ''
    label: Sampling Rate (Hz)
    type: real
    value: 1e6
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [224, 80.0]
    rotation: 0
    state: true
- name: variable_qtgui_range_amplitude
  id: variable_qtgui_range
  parameters:
    comment: ''
    gui_hint: ''
    label: Amplitude
    min_len: '1000'
    orient: Qt.Horizontal
    rangeType: float
    start: '0'
    step: 1/1000
    stop: '1'
    value: '1'
    widget: counter_slider
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [456, 8.0]
    rotation: 0
    state: true
- name: variable_qtgui_range_freq
  id: variable_qtgui_range
  parameters:
    comment: ''
    gui_hint: ''
    label: Frequency
    min_len: '1000'
    orient: Qt.Horizontal
    rangeType: float
    start: -samp_rate/2
    step: samp_rate/1000
    stop: samp_rate/2
    value: samp_rate/10
    widget: counter_slider
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [648, 8.0]
    rotation: 0
    state: true
- name: variable_qtgui_range_user_reg
  id: variable_qtgui_range
  parameters:
    comment: ''
    gui_hint: ''
    label: User Reg
    min_len: '1000'
    orient: Qt.Horizontal
    rangeType: int
    start: -2**15-1
    step: '1'
    stop: 2**15-1
    value: '1'
    widget: counter_slider
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [840, 8.0]
    rotation: 0
    state: true
- name: analog_sig_source_x_0
  id: analog_sig_source_x
  parameters:
    affinity: ''
    alias: ''
    amp: variable_qtgui_range_amplitude
    comment: ''
    freq: variable_qtgui_range_freq
    maxoutbuf: '0'
    minoutbuf: '0'
    offset: '0'
    phase: '0'
    samp_rate: samp_rate
    type: complex
    waveform: analog.GR_COS_WAVE
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [32, 200.0]
    rotation: 0
    state: enabled
- name: blocks_throttle_0
  id: blocks_throttle
  parameters:
    affinity: ''
    alias: ''
    comment: ''
    ignoretag: 'True'
    maxoutbuf: '0'
    minoutbuf: '0'
    samples_per_second: samp_rate
    type: complex
    vlen: '1'
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [272, 240.0]
    rotation: 0
    state: true
- name: ettus_rfnoc_rx_streamer_0
  id: ettus_rfnoc_rx_streamer
  parameters:
    affinity: ''
    alias: ''
    args: ''
    comment: ''
    maxoutbuf: '0'
    minoutbuf: '0'
    num_chans: '1'
    otw: sc16
    output_type: fc32
    vlen: '1'
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [896, 248.0]
    rotation: 0
    state: true
- name: ettus_rfnoc_tx_streamer_0
  id: ettus_rfnoc_tx_streamer
  parameters:
    affinity: ''
    alias: ''
    args: ''
    comment: ''
    input_type: fc32
    maxoutbuf: '0'
    minoutbuf: '0'
    num_chans: '1'
    otw: sc16
    vlen: '1'
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [480, 248.0]
    rotation: 0
    state: true
- name: qtgui_time_sink_x_0
  id: qtgui_time_sink_x
  parameters:
    affinity: ''
    alias: ''
    alpha1: '1.0'
    alpha10: '1.0'
    alpha2: '1.0'
    alpha3: '1.0'
    alpha4: '1.0'
    alpha5: '1.0'
    alpha6: '1.0'
    alpha7: '1.0'
    alpha8: '1.0'
    alpha9: '1.0'
    autoscale: 'False'
    axislabels: 'True'
    color1: blue
    color10: dark blue
    color2: red
    color3: green
    color4: black
    color5: cyan
    color6: magenta
    color7: yellow
    color8: dark red
    color9: dark green
    comment: ''
    ctrlpanel: 'False'
    entags: 'True'
    grid: 'False'
    gui_hint: ''
    label1: Signal 1
    label10: Signal 10
    label2: Signal 2
    label3: Signal 3
    label4: Signal 4
    label5: Signal 5
    label6: Signal 6
    label7: Signal 7
    label8: Signal 8
    label9: Signal 9
    legend: 'True'
    marker1: '-1'
    marker10: '-1'
    marker2: '-1'
    marker3: '-1'
    marker4: '-1'
    marker5: '-1'
    marker6: '-1'
    marker7: '-1'
    marker8: '-1'
    marker9: '-1'
    name: '""'
    nconnections: '1'
    size: '1024'
    srate: samp_rate
    stemplot: 'False'
    style1: '1'
    style10: '1'
    style2: '1'
    style3: '1'
    style4: '1'
    style5: '1'
    style6: '1'
    style7: '1'
    style8: '1'
    style9: '1'
    tr_chan: '0'
    tr_delay: '0'
    tr_level: '0.0'
    tr_mode: qtgui.TRIG_MODE_FREE
    tr_slope: qtgui.TRIG_SLOPE_POS
    tr_tag: '""'
    type: complex
    update_time: '0.10'
    width1: '1'
    width10: '1'
    width2: '1'
    width3: '1'
    width4: '1'
    width5: '1'
    width6: '1'
    width7: '1'
    width8: '1'
    width9: '1'
    ylabel: Amplitude
    ymax: '1'
    ymin: '-1'
    yunit: '""'
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [1120, 224.0]
    rotation: 0
    state: true
- name: tutorial_${blockname}_0
  id: tutorial_${blockname}
  parameters:
    affinity: ''
    alias: ''
    block_args: ''
    comment: ''
    device_select: '-1'
    instance_select: '-1'
    maxoutbuf: '0'
    minoutbuf: '0'
    user_reg: variable_qtgui_range_user_reg
  states:
    bus_sink: false
    bus_source: false
    bus_structure: null
    coordinate: [688, 216.0]
    rotation: 0
    state: true

connections:
- [analog_sig_source_x_0, '0', blocks_throttle_0, '0']
- [blocks_throttle_0, '0', ettus_rfnoc_tx_streamer_0, '0']
- [ettus_rfnoc_rx_streamer_0, '0', qtgui_time_sink_x_0, '0']
- [ettus_rfnoc_tx_streamer_0, '0', tutorial_${blockname}_0, '0']
- [tutorial_${blockname}_0, '0', ettus_rfnoc_rx_streamer_0, '0']

metadata:
  file_format: 1

'''

# RFNoC block description YAML file
Templates['block_yml'] = '''
schema: rfnoc_modtool_args
module_name: ${blockname}
version: 1.0
rfnoc_version: 1.0
chdr_width: 64
noc_id: 0x${noc_id}
makefile_srcs: "${modpath}/rfnoc/fpga/rfnoc_block_${blockname}/Makefile.srcs"

clocks:
  - name: rfnoc_chdr
    freq: "[]"
  - name: rfnoc_ctrl
    freq: "[]"
  - name: ce
    freq: "[]"

control:
  sw_iface: nocscript
  fpga_iface: ctrlport
  interface_direction: slave
  fifo_depth: 32
  clk_domain: ce
  ctrlport:
    byte_mode: False
    timed: False
    has_status: False

data:
  fpga_iface: axis_data
  clk_domain: ce
  inputs:
    in:
      item_width: 32
      nipc: 1
      info_fifo_depth: 32
      payload_fifo_depth: 32
      format: int32
      mdata_sig: ~
  outputs:
    out:
      item_width: 32
      nipc: 1
      info_fifo_depth: 32
      payload_fifo_depth: 32
      format: int32
      mdata_sig: ~
'''

# RFNoC block description CMakeLists.txt
Templates['block_cmake'] = '''
# Reminder: This won't auto-update when you add a file, you need to re-run CMake
# to re-generate the glob. Or, you add the files directly into the install()
# statement below.
file(GLOB yml_files "*.yml")
# List all header files here (UHD and GNU Radio)
install(
    FILES
    ${"${yml_files}"}
    DESTINATION share/uhd/rfnoc/blocks
    COMPONENT blocks
)

'''

# RFNoC fpga CMakeLists.txt
Templates['fpga_cmake'] = '''
# List Makefile.srcs here (which needs to point to the individual blocks!) as
# well as any non-block specific HDL files that should get installed alongside
# the rest of the FPGA/Verilog/VHDL/HDL files. Only list files that are required
# for synthesis, testbench-specific files do not get installed and thus do not
# have to be listed (it won't hurt, it will just clutter your share/ directory).
# Don't list the files in the block subdirectories, though, they will get added
# below.
install(FILES
    Makefile.srcs
    DESTINATION ${"${PROJECT_DATA_DIR}"}/fpga
    COMPONENT fpga
)

# Call add_subdirectory() for every subdirectory
SUBDIRLIST(subdirs ${"${CMAKE_CURRENT_SOURCE_DIR}"})
foreach(subdir ${"${subdirs}"})
    add_subdirectory(${"${subdir}"})
endforeach()

'''

# RFNoC fpga Makefile.srcs
Templates['fpga_makefile_srcs'] = '''
# Current path
RFNOC_FPGA_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Include Makefile.srcs for all subdirectories automatically
SUBDIRS_MAKEFILE_SRCS = $(wildcard $(RFNOC_FPGA_DIR)/*/Makefile.srcs)
include $(SUBDIRS_MAKEFILE_SRCS)

'''

# RFNoC fpga rfnoc block Makefile.srcs
Templates['fpga_rfnoc_block_makefile_srcs'] = '''
##################################################
# RFNoC Block Sources
##################################################
# Here, list all the files that are necessary to synthesize this block. Don't
# include testbenches!
# Make sure that the source files are nicely detectable by a regex. Best to put
# one on each line.
# The first argument to addprefix is the current path to this Makefile, so the
# path list is always absolute, regardless of from where we're including or
# calling this file. RFNOC_OOT_SRCS needs to be a simply expanded variable
# (not a recursively expanded variable), and we take care of that in the build
# infrastructure.
RFNOC_OOT_SRCS += $(addprefix $(dir $(abspath $(lastword $(MAKEFILE_LIST)))), \
rfnoc_block_${blockname}.v \
noc_shell_${blockname}.v \
)

'''

# RFNoC fpga block source CMakeLists.txt
Templates['fpga_rfnoc_block_cmake'] = '''
# This macro will tell CMake that this directory contains an RFNoC block. It
# will parse Makefile.srcs to see which files need to be installed, and it will
# register a testbench target for this directory.
RFNOC_REGISTER_BLOCK_DIR()

# This will do the same, but it will skip the testbench target.
#RFNOC_REGISTER_BLOCK_DIR(NOTESTBENCH)

'''

# RFNoC fpga block source Makefile
Templates['fpga_rfnoc_block_makefile'] = '''
#-------------------------------------------------
# Top-of-Makefile
#-------------------------------------------------
# Define BASE_DIR to point to the "top" dir. Note:
# UHD_FPGA_DIR must be passed into this Makefile.
ifndef UHD_FPGA_DIR
$(error "UHD_FPGA_DIR is not set! Must point to UHD FPGA repository!")
endif
BASE_DIR = $(UHD_FPGA_DIR)/usrp3/top
# Include viv_sim_preample after defining BASE_DIR
include $(BASE_DIR)/../tools/make/viv_sim_preamble.mak

#-------------------------------------------------
# Design Specific
#-------------------------------------------------
# Include makefiles and sources for the DUT and its
# dependencies.
include $(BASE_DIR)/../lib/rfnoc/core/Makefile.srcs
include $(BASE_DIR)/../lib/rfnoc/utils/Makefile.srcs
include Makefile.srcs

DESIGN_SRCS += $(abspath \
$(RFNOC_CORE_SRCS) \
$(RFNOC_UTIL_SRCS) \
$(RFNOC_OOT_SRCS)  \
)

#-------------------------------------------------
# Testbench Specific
#-------------------------------------------------
SIM_TOP = rfnoc_block_${blockname}_tb
SIM_SRCS = \
$(abspath rfnoc_block_${blockname}_tb.sv) \

#-------------------------------------------------
# Bottom-of-Makefile
#-------------------------------------------------
# Include all simulator specific makefiles here
# Each should define a unique target to simulate
# e.g. xsim, vsim, etc and a common "clean" target
include $(BASE_DIR)/../tools/make/viv_simulator.mak

'''

# RFNoC fpga block source noc_shell
Templates['fpga_rfnoc_block_noc_shell'] = '''
//
// Module: noc_shell_${blockname}
//
// Description:
//
//   This is a tool-generated NoC-shell for the ${blockname} block.
//   See the RFNoC specification for more information about NoC shells.
//
// Parameters:
//
//   THIS_PORTID : Control crossbar port to which this block is connected
//   CHDR_W      : AXIS-CHDR data bus width
//   MTU         : Maximum transmission unit (i.e., maximum packet size in
//

`default_nettype none


module noc_shell_${blockname} #(
  parameter [9:0] THIS_PORTID     = 10'd0,
  parameter       CHDR_W          = 64,
  parameter [5:0] MTU             = 10
) (
  //---------------------
  // Framework Interface
  //---------------------

  // RFNoC Framework Clocks
  input  wire rfnoc_chdr_clk,
  input  wire rfnoc_ctrl_clk,

  // NoC Shell Generated Resets
  output wire rfnoc_chdr_rst,
  output wire rfnoc_ctrl_rst,

  // RFNoC Backend Interface
  input  wire [511:0]          rfnoc_core_config,
  output wire [511:0]          rfnoc_core_status,

  // AXIS-CHDR Input Ports (from framework)
  input  wire [(1)*CHDR_W-1:0] s_rfnoc_chdr_tdata,
  input  wire [(1)-1:0]        s_rfnoc_chdr_tlast,
  input  wire [(1)-1:0]        s_rfnoc_chdr_tvalid,
  output wire [(1)-1:0]        s_rfnoc_chdr_tready,
  // AXIS-CHDR Output Ports (to framework)
  output wire [(1)*CHDR_W-1:0] m_rfnoc_chdr_tdata,
  output wire [(1)-1:0]        m_rfnoc_chdr_tlast,
  output wire [(1)-1:0]        m_rfnoc_chdr_tvalid,
  input  wire [(1)-1:0]        m_rfnoc_chdr_tready,

  // AXIS-Ctrl Control Input Port (from framework)
  input  wire [31:0]           s_rfnoc_ctrl_tdata,
  input  wire                  s_rfnoc_ctrl_tlast,
  input  wire                  s_rfnoc_ctrl_tvalid,
  output wire                  s_rfnoc_ctrl_tready,
  // AXIS-Ctrl Control Output Port (to framework)
  output wire [31:0]           m_rfnoc_ctrl_tdata,
  output wire                  m_rfnoc_ctrl_tlast,
  output wire                  m_rfnoc_ctrl_tvalid,
  input  wire                  m_rfnoc_ctrl_tready,

  //---------------------
  // Client Interface
  //---------------------

  // CtrlPort Clock and Reset
  output wire               ctrlport_clk,
  output wire               ctrlport_rst,
  // CtrlPort Master
  output wire               m_ctrlport_req_wr,
  output wire               m_ctrlport_req_rd,
  output wire [19:0]        m_ctrlport_req_addr,
  output wire [31:0]        m_ctrlport_req_data,
  input  wire               m_ctrlport_resp_ack,
  input  wire [31:0]        m_ctrlport_resp_data,

  // AXI-Stream Payload Context Clock and Reset
  output wire               axis_data_clk,
  output wire               axis_data_rst,
  // Payload Stream to User Logic: in
  output wire [32*1-1:0]    m_in_payload_tdata,
  output wire [1-1:0]       m_in_payload_tkeep,
  output wire               m_in_payload_tlast,
  output wire               m_in_payload_tvalid,
  input  wire               m_in_payload_tready,
  // Context Stream to User Logic: in
  output wire [CHDR_W-1:0]  m_in_context_tdata,
  output wire [3:0]         m_in_context_tuser,
  output wire               m_in_context_tlast,
  output wire               m_in_context_tvalid,
  input  wire               m_in_context_tready,
  // Payload Stream from User Logic: out
  input  wire [32*1-1:0]    s_out_payload_tdata,
  input  wire [0:0]         s_out_payload_tkeep,
  input  wire               s_out_payload_tlast,
  input  wire               s_out_payload_tvalid,
  output wire               s_out_payload_tready,
  // Context Stream from User Logic: out
  input  wire [CHDR_W-1:0]  s_out_context_tdata,
  input  wire [3:0]         s_out_context_tuser,
  input  wire               s_out_context_tlast,
  input  wire               s_out_context_tvalid,
  output wire               s_out_context_tready
);

  //---------------------------------------------------------------------------
  //  Backend Interface
  //---------------------------------------------------------------------------

  wire         data_i_flush_en;
  wire [31:0]  data_i_flush_timeout;
  wire [63:0]  data_i_flush_active;
  wire [63:0]  data_i_flush_done;
  wire         data_o_flush_en;
  wire [31:0]  data_o_flush_timeout;
  wire [63:0]  data_o_flush_active;
  wire [63:0]  data_o_flush_done;

  backend_iface #(
    .NOC_ID        (32'h${noc_id}),
    .NUM_DATA_I    (1),
    .NUM_DATA_O    (1),
    .CTRL_FIFOSIZE ($clog2(32)),
    .MTU           (MTU)
  ) backend_iface_i (
    .rfnoc_chdr_clk       (rfnoc_chdr_clk),
    .rfnoc_chdr_rst       (rfnoc_chdr_rst),
    .rfnoc_ctrl_clk       (rfnoc_ctrl_clk),
    .rfnoc_ctrl_rst       (rfnoc_ctrl_rst),
    .rfnoc_core_config    (rfnoc_core_config),
    .rfnoc_core_status    (rfnoc_core_status),
    .data_i_flush_en      (data_i_flush_en),
    .data_i_flush_timeout (data_i_flush_timeout),
    .data_i_flush_active  (data_i_flush_active),
    .data_i_flush_done    (data_i_flush_done),
    .data_o_flush_en      (data_o_flush_en),
    .data_o_flush_timeout (data_o_flush_timeout),
    .data_o_flush_active  (data_o_flush_active),
    .data_o_flush_done    (data_o_flush_done)
  );

  //---------------------------------------------------------------------------
  //  Control Path
  //---------------------------------------------------------------------------

  assign ctrlport_clk = rfnoc_chdr_clk;
  assign ctrlport_rst = rfnoc_chdr_rst;

  ctrlport_endpoint #(
    .THIS_PORTID      (THIS_PORTID),
    .SYNC_CLKS        (0),
    .AXIS_CTRL_MST_EN (0),
    .AXIS_CTRL_SLV_EN (1),
    .SLAVE_FIFO_SIZE  ($clog2(32))
  ) ctrlport_endpoint_i (
    .rfnoc_ctrl_clk            (rfnoc_ctrl_clk),
    .rfnoc_ctrl_rst            (rfnoc_ctrl_rst),
    .ctrlport_clk              (ctrlport_clk),
    .ctrlport_rst              (ctrlport_rst),
    .s_rfnoc_ctrl_tdata        (s_rfnoc_ctrl_tdata),
    .s_rfnoc_ctrl_tlast        (s_rfnoc_ctrl_tlast),
    .s_rfnoc_ctrl_tvalid       (s_rfnoc_ctrl_tvalid),
    .s_rfnoc_ctrl_tready       (s_rfnoc_ctrl_tready),
    .m_rfnoc_ctrl_tdata        (m_rfnoc_ctrl_tdata),
    .m_rfnoc_ctrl_tlast        (m_rfnoc_ctrl_tlast),
    .m_rfnoc_ctrl_tvalid       (m_rfnoc_ctrl_tvalid),
    .m_rfnoc_ctrl_tready       (m_rfnoc_ctrl_tready),
    .m_ctrlport_req_wr         (m_ctrlport_req_wr),
    .m_ctrlport_req_rd         (m_ctrlport_req_rd),
    .m_ctrlport_req_addr       (m_ctrlport_req_addr),
    .m_ctrlport_req_data       (m_ctrlport_req_data),
    .m_ctrlport_req_byte_en    (),
    .m_ctrlport_req_has_time   (),
    .m_ctrlport_req_time       (),
    .m_ctrlport_resp_ack       (m_ctrlport_resp_ack),
    .m_ctrlport_resp_status    (2'b0),
    .m_ctrlport_resp_data      (m_ctrlport_resp_data),
    .s_ctrlport_req_wr         (1'b0),
    .s_ctrlport_req_rd         (1'b0),
    .s_ctrlport_req_addr       (20'b0),
    .s_ctrlport_req_portid     (10'b0),
    .s_ctrlport_req_rem_epid   (16'b0),
    .s_ctrlport_req_rem_portid (10'b0),
    .s_ctrlport_req_data       (32'b0),
    .s_ctrlport_req_byte_en    (4'hF),
    .s_ctrlport_req_has_time   (1'b0),
    .s_ctrlport_req_time       (64'b0),
    .s_ctrlport_resp_ack       (),
    .s_ctrlport_resp_status    (),
    .s_ctrlport_resp_data      ()
  );

  //---------------------------------------------------------------------------
  //  Data Path
  //---------------------------------------------------------------------------

  genvar i;

  assign axis_data_clk = rfnoc_chdr_clk;
  assign axis_data_rst = rfnoc_chdr_rst;

  //---------------------
  // Input Data Paths
  //---------------------

  chdr_to_axis_pyld_ctxt #(
    .CHDR_W              (CHDR_W),
    .ITEM_W              (32),
    .NIPC                (1),
    .SYNC_CLKS           (1),
    .CONTEXT_FIFO_SIZE   ($clog2(2)),
    .PAYLOAD_FIFO_SIZE   ($clog2(2)),
    .CONTEXT_PREFETCH_EN (1)
  ) chdr_to_axis_pyld_ctxt_in_in (
    .axis_chdr_clk         (rfnoc_chdr_clk),
    .axis_chdr_rst         (rfnoc_chdr_rst),
    .axis_data_clk         (axis_data_clk),
    .axis_data_rst         (axis_data_rst),
    .s_axis_chdr_tdata     (s_rfnoc_chdr_tdata[(0)*CHDR_W+:CHDR_W]),
    .s_axis_chdr_tlast     (s_rfnoc_chdr_tlast[0]),
    .s_axis_chdr_tvalid    (s_rfnoc_chdr_tvalid[0]),
    .s_axis_chdr_tready    (s_rfnoc_chdr_tready[0]),
    .m_axis_payload_tdata  (m_in_payload_tdata),
    .m_axis_payload_tkeep  (m_in_payload_tkeep),
    .m_axis_payload_tlast  (m_in_payload_tlast),
    .m_axis_payload_tvalid (m_in_payload_tvalid),
    .m_axis_payload_tready (m_in_payload_tready),
    .m_axis_context_tdata  (m_in_context_tdata),
    .m_axis_context_tuser  (m_in_context_tuser),
    .m_axis_context_tlast  (m_in_context_tlast),
    .m_axis_context_tvalid (m_in_context_tvalid),
    .m_axis_context_tready (m_in_context_tready),
    .flush_en              (data_i_flush_en),
    .flush_timeout         (data_i_flush_timeout),
    .flush_active          (data_i_flush_active[0]),
    .flush_done            (data_i_flush_done[0])
  );

  //---------------------
  // Output Data Paths
  //---------------------

  axis_pyld_ctxt_to_chdr #(
    .CHDR_W              (CHDR_W),
    .ITEM_W              (32),
    .NIPC                (1),
    .SYNC_CLKS           (1),
    .CONTEXT_FIFO_SIZE   ($clog2(2)),
    .PAYLOAD_FIFO_SIZE   ($clog2(2)),
    .MTU                 (MTU),
    .CONTEXT_PREFETCH_EN (1)
  ) axis_pyld_ctxt_to_chdr_out_out (
    .axis_chdr_clk         (rfnoc_chdr_clk),
    .axis_chdr_rst         (rfnoc_chdr_rst),
    .axis_data_clk         (axis_data_clk),
    .axis_data_rst         (axis_data_rst),
    .m_axis_chdr_tdata     (m_rfnoc_chdr_tdata[(0)*CHDR_W+:CHDR_W]),
    .m_axis_chdr_tlast     (m_rfnoc_chdr_tlast[0]),
    .m_axis_chdr_tvalid    (m_rfnoc_chdr_tvalid[0]),
    .m_axis_chdr_tready    (m_rfnoc_chdr_tready[0]),
    .s_axis_payload_tdata  (s_out_payload_tdata),
    .s_axis_payload_tkeep  (s_out_payload_tkeep),
    .s_axis_payload_tlast  (s_out_payload_tlast),
    .s_axis_payload_tvalid (s_out_payload_tvalid),
    .s_axis_payload_tready (s_out_payload_tready),
    .s_axis_context_tdata  (s_out_context_tdata),
    .s_axis_context_tuser  (s_out_context_tuser),
    .s_axis_context_tlast  (s_out_context_tlast),
    .s_axis_context_tvalid (s_out_context_tvalid),
    .s_axis_context_tready (s_out_context_tready),
    .framer_errors         (),
    .flush_en              (data_o_flush_en),
    .flush_timeout         (data_o_flush_timeout),
    .flush_active          (data_o_flush_active[0]),
    .flush_done            (data_o_flush_done[0])
  );

endmodule // noc_shell_${blockname}


`default_nettype wire

'''

# RFNoC fpga block source rfnoc_block
Templates['fpga_rfnoc_block'] = '''${str_to_verilog_comment(license)}
//
// Module: rfnoc_block_${blockname}
//
// Description:
//
//   This is a skeleton file for a RFNoC block. It passes incoming samples
//   to the output without any modification. A read/write user register is
//   instantiated, but left unused.
//
// Parameters:
//
//   THIS_PORTID : Control crossbar port to which this block is connected
//   CHDR_W      : AXIS-CHDR data bus width
//   MTU         : Maximum transmission unit (i.e., maximum packet size in
//                 CHDR words is 2**MTU).
//

`default_nettype none


module rfnoc_block_${blockname} #(
  parameter [9:0] THIS_PORTID     = 10'd0,
  parameter       CHDR_W          = 64,
  parameter [5:0] MTU             = 10
)(
  // RFNoC Framework Clocks and Resets
  input  wire                   rfnoc_chdr_clk,
  input  wire                   rfnoc_ctrl_clk,
  input  wire                   ce_clk,
  // RFNoC Backend Interface
  input  wire [511:0]           rfnoc_core_config,
  output wire [511:0]           rfnoc_core_status,
  // AXIS-CHDR Input Ports (from framework)
  input  wire [(1)*CHDR_W-1:0]  s_rfnoc_chdr_tdata,
  input  wire [(1)-1:0]         s_rfnoc_chdr_tlast,
  input  wire [(1)-1:0]         s_rfnoc_chdr_tvalid,
  output wire [(1)-1:0]         s_rfnoc_chdr_tready,
  // AXIS-CHDR Output Ports (to framework)
  output wire [(1)*CHDR_W-1:0]  m_rfnoc_chdr_tdata,
  output wire [(1)-1:0]         m_rfnoc_chdr_tlast,
  output wire [(1)-1:0]         m_rfnoc_chdr_tvalid,
  input  wire [(1)-1:0]         m_rfnoc_chdr_tready,
  // AXIS-Ctrl Input Port (from framework)
  input  wire [31:0]            s_rfnoc_ctrl_tdata,
  input  wire                   s_rfnoc_ctrl_tlast,
  input  wire                   s_rfnoc_ctrl_tvalid,
  output wire                   s_rfnoc_ctrl_tready,
  // AXIS-Ctrl Output Port (to framework)
  output wire [31:0]            m_rfnoc_ctrl_tdata,
  output wire                   m_rfnoc_ctrl_tlast,
  output wire                   m_rfnoc_ctrl_tvalid,
  input  wire                   m_rfnoc_ctrl_tready
);

  //---------------------------------------------------------------------------
  // Signal Declarations
  //---------------------------------------------------------------------------

  // Clocks and Resets
  wire               ctrlport_clk;
  wire               ctrlport_rst;
  wire               axis_data_clk;
  wire               axis_data_rst;
  // CtrlPort Master
  wire               m_ctrlport_req_wr;
  wire               m_ctrlport_req_rd;
  wire [19:0]        m_ctrlport_req_addr;
  wire [31:0]        m_ctrlport_req_data;
  reg                m_ctrlport_resp_ack;
  reg  [31:0]        m_ctrlport_resp_data;
  // Payload Stream to User Logic: in
  wire [32*1-1:0]    m_in_payload_tdata;
  wire [1-1:0]       m_in_payload_tkeep;
  wire               m_in_payload_tlast;
  wire               m_in_payload_tvalid;
  wire               m_in_payload_tready;
  // Context Stream to User Logic: in
  wire [CHDR_W-1:0]  m_in_context_tdata;
  wire [3:0]         m_in_context_tuser;
  wire               m_in_context_tlast;
  wire               m_in_context_tvalid;
  wire               m_in_context_tready;
  // Payload Stream from User Logic: out
  wire [32*1-1:0]    s_out_payload_tdata;
  wire [0:0]         s_out_payload_tkeep;
  wire               s_out_payload_tlast;
  wire               s_out_payload_tvalid;
  wire               s_out_payload_tready;
  // Context Stream from User Logic: out
  wire [CHDR_W-1:0]  s_out_context_tdata;
  wire [3:0]         s_out_context_tuser;
  wire               s_out_context_tlast;
  wire               s_out_context_tvalid;
  wire               s_out_context_tready;

  //---------------------------------------------------------------------------
  // NoC Shell
  //---------------------------------------------------------------------------

  noc_shell_${blockname} #(
    .CHDR_W      (CHDR_W),
    .THIS_PORTID (THIS_PORTID),
    .MTU         (MTU)
  ) noc_shell_${blockname}_i (
    //---------------------
    // Framework Interface
    //---------------------

    // Clock Inputs
    .rfnoc_chdr_clk      (rfnoc_chdr_clk),
    .rfnoc_ctrl_clk      (rfnoc_ctrl_clk),
    // Reset Outputs
    .rfnoc_chdr_rst      (),
    .rfnoc_ctrl_rst      (),
    // RFNoC Backend Interface
    .rfnoc_core_config   (rfnoc_core_config),
    .rfnoc_core_status   (rfnoc_core_status),
    // CHDR Input Ports  (from framework)
    .s_rfnoc_chdr_tdata  (s_rfnoc_chdr_tdata),
    .s_rfnoc_chdr_tlast  (s_rfnoc_chdr_tlast),
    .s_rfnoc_chdr_tvalid (s_rfnoc_chdr_tvalid),
    .s_rfnoc_chdr_tready (s_rfnoc_chdr_tready),
    // CHDR Output Ports (to framework)
    .m_rfnoc_chdr_tdata  (m_rfnoc_chdr_tdata),
    .m_rfnoc_chdr_tlast  (m_rfnoc_chdr_tlast),
    .m_rfnoc_chdr_tvalid (m_rfnoc_chdr_tvalid),
    .m_rfnoc_chdr_tready (m_rfnoc_chdr_tready),
    // AXIS-Ctrl Input Port (from framework)
    .s_rfnoc_ctrl_tdata  (s_rfnoc_ctrl_tdata),
    .s_rfnoc_ctrl_tlast  (s_rfnoc_ctrl_tlast),
    .s_rfnoc_ctrl_tvalid (s_rfnoc_ctrl_tvalid),
    .s_rfnoc_ctrl_tready (s_rfnoc_ctrl_tready),
    // AXIS-Ctrl Output Port (to framework)
    .m_rfnoc_ctrl_tdata  (m_rfnoc_ctrl_tdata),
    .m_rfnoc_ctrl_tlast  (m_rfnoc_ctrl_tlast),
    .m_rfnoc_ctrl_tvalid (m_rfnoc_ctrl_tvalid),
    .m_rfnoc_ctrl_tready (m_rfnoc_ctrl_tready),

    //---------------------
    // Client Interface
    //---------------------

    // CtrlPort Clock and Reset
    .ctrlport_clk         (ctrlport_clk),
    .ctrlport_rst         (ctrlport_rst),
    // CtrlPort Master
    .m_ctrlport_req_wr    (m_ctrlport_req_wr),
    .m_ctrlport_req_rd    (m_ctrlport_req_rd),
    .m_ctrlport_req_addr  (m_ctrlport_req_addr),
    .m_ctrlport_req_data  (m_ctrlport_req_data),
    .m_ctrlport_resp_ack  (m_ctrlport_resp_ack),
    .m_ctrlport_resp_data (m_ctrlport_resp_data),

    // AXI-Stream Payload Context Clock and Reset
    .axis_data_clk        (axis_data_clk),
    .axis_data_rst        (axis_data_rst),
    // Payload Stream to User Logic: in
    .m_in_payload_tdata   (m_in_payload_tdata),
    .m_in_payload_tkeep   (m_in_payload_tkeep),
    .m_in_payload_tlast   (m_in_payload_tlast),
    .m_in_payload_tvalid  (m_in_payload_tvalid),
    .m_in_payload_tready  (m_in_payload_tready),
    // Context Stream to User Logic: in
    .m_in_context_tdata   (m_in_context_tdata),
    .m_in_context_tuser   (m_in_context_tuser),
    .m_in_context_tlast   (m_in_context_tlast),
    .m_in_context_tvalid  (m_in_context_tvalid),
    .m_in_context_tready  (m_in_context_tready),
    // Payload Stream from User Logic: out
    .s_out_payload_tdata  (s_out_payload_tdata),
    .s_out_payload_tkeep  (s_out_payload_tkeep),
    .s_out_payload_tlast  (s_out_payload_tlast),
    .s_out_payload_tvalid (s_out_payload_tvalid),
    .s_out_payload_tready (s_out_payload_tready),
    // Context Stream from User Logic: out
    .s_out_context_tdata  (s_out_context_tdata),
    .s_out_context_tuser  (s_out_context_tuser),
    .s_out_context_tlast  (s_out_context_tlast),
    .s_out_context_tvalid (s_out_context_tvalid),
    .s_out_context_tready (s_out_context_tready)
  );

  //---------------------------------------------------------------------------
  // User Registers
  //---------------------------------------------------------------------------
  //
  // There's only one register now, but we'll structure the register code to
  // make it easier to add more registers later.
  // Register use the ctrlport_clk clock.
  //
  //---------------------------------------------------------------------------

  // Note: Register addresses increment by 4
  localparam REG_USER_ADDR    = 0; // Address for example user register
  localparam REG_USER_DEFAULT = 0; // Default value for user register

  reg [31:0] reg_user = REG_USER_DEFAULT;

  always @(posedge ctrlport_clk) begin
    if (ctrlport_rst) begin
      reg_user = REG_USER_DEFAULT;
    end else begin
      // Default assignment
      m_ctrlport_resp_ack <= 0;

      // Read user register
      if (m_ctrlport_req_rd) begin // Read request
        case (m_ctrlport_req_addr)
          REG_USER_ADDR: begin
            m_ctrlport_resp_ack  <= 1;
            m_ctrlport_resp_data <= reg_user;
          end
        endcase
      end

      // Write user register
      if (m_ctrlport_req_wr) begin // Write requst
        case (m_ctrlport_req_addr)
          REG_USER_ADDR: begin
            m_ctrlport_resp_ack <= 1;
            reg_user            <= m_ctrlport_req_data[31:0];
          end
        endcase
      end
    end
  end

  //---------------------------------------------------------------------------
  // User Logic
  //---------------------------------------------------------------------------
  //
  // User logic uses the axis_data_clk clock. While the registers above use the
  // ctrlport_clk clock, in the block YAML configuration file both the control
  // and data interfaces are specified to use the rfnoc_chdr clock. Therefore,
  // we do not need to cross clock domains when using user registers with
  // user logic.
  //
  //---------------------------------------------------------------------------

  // Sample data, pass through unchanged
  assign s_out_payload_tdata  = m_in_payload_tdata;
  assign s_out_payload_tlast  = m_in_payload_tlast;
  assign s_out_payload_tvalid = m_in_payload_tvalid;
  assign m_in_payload_tready  = s_out_payload_tready;

  // Context data, we are not doing anything with the context
  // (the CHDR header info) so we can simply pass through unchanged
  assign s_out_context_tdata  = m_in_context_tdata;
  assign s_out_context_tuser  = m_in_context_tuser;
  assign s_out_context_tlast  = m_in_context_tlast;
  assign s_out_context_tvalid = m_in_context_tvalid;
  assign m_in_context_tready  = s_out_context_tready;

  // Only 1-sample per clock, so tkeep should always be asserted
  assign s_out_payload_tkeep = 1'b1;

endmodule // rfnoc_block_${blockname}

`default_nettype wire

'''

# RFNoC fpga block source testbench
Templates['fpga_rfnoc_testbench'] = '''${str_to_verilog_comment(license)}
//
// Module: rfnoc_block_${blockname}_tb
//
// Description: Testbench for the ${blockname} RFNoC block.
//

`default_nettype none


module rfnoc_block_${blockname}_tb;

  `include "test_exec.svh"

  import PkgTestExec::*;
  import PkgChdrUtils::*;
  import PkgRfnocBlockCtrlBfm::*;
  import PkgRfnocItemUtils::*;

  //---------------------------------------------------------------------------
  // Testbench Configuration
  //---------------------------------------------------------------------------

  localparam [ 9:0] THIS_PORTID     = 10'h123;
  localparam [31:0] NOC_ID          = 32'h${noc_id};
  localparam int    CHDR_W          = 64;
  localparam int    ITEM_W          = 32;
  localparam int    NUM_PORTS_I     = 1;
  localparam int    NUM_PORTS_O     = 1;
  localparam int    MTU             = 13;
  localparam int    SPP             = 64;
  localparam int    PKT_SIZE_BYTES  = SPP * (ITEM_W/8);
  localparam int    STALL_PROB      = 25;      // Default BFM stall probability
  localparam real   CHDR_CLK_PER    = 5.0;     // 200 MHz
  localparam real   CTRL_CLK_PER    = 25.0;    // 40 MHz

  //---------------------------------------------------------------------------
  // Clocks and Resets
  //---------------------------------------------------------------------------

  bit rfnoc_chdr_clk;
  bit rfnoc_ctrl_clk;

  sim_clock_gen #(CHDR_CLK_PER) rfnoc_chdr_clk_gen (.clk(rfnoc_chdr_clk), .rst());
  sim_clock_gen #(CTRL_CLK_PER) rfnoc_ctrl_clk_gen (.clk(rfnoc_ctrl_clk), .rst());

  //---------------------------------------------------------------------------
  // Bus Functional Models
  //---------------------------------------------------------------------------

  // Backend Interface
  RfnocBackendIf backend (rfnoc_chdr_clk, rfnoc_ctrl_clk);

  // AXIS-Ctrl Interface
  AxiStreamIf #(32) m_ctrl (rfnoc_ctrl_clk, 1'b0);
  AxiStreamIf #(32) s_ctrl (rfnoc_ctrl_clk, 1'b0);

  // AXIS-CHDR Interfaces
  AxiStreamIf #(CHDR_W) m_chdr [NUM_PORTS_I] (rfnoc_chdr_clk, 1'b0);
  AxiStreamIf #(CHDR_W) s_chdr [NUM_PORTS_O] (rfnoc_chdr_clk, 1'b0);

  // Block Controller BFM
  RfnocBlockCtrlBfm #(CHDR_W, ITEM_W) blk_ctrl = new(backend, m_ctrl, s_ctrl);

  // CHDR word and item/sample data types
  typedef ChdrData #(CHDR_W, ITEM_W)::chdr_word_t chdr_word_t;
  typedef ChdrData #(CHDR_W, ITEM_W)::item_t      item_t;

  // Connect block controller to BFMs
  for (genvar i = 0; i < NUM_PORTS_I; i++) begin : gen_bfm_input_connections
    initial begin
      blk_ctrl.connect_master_data_port(i, m_chdr[i], PKT_SIZE_BYTES);
      blk_ctrl.set_master_stall_prob(i, STALL_PROB);
    end
  end
  for (genvar i = 0; i < NUM_PORTS_O; i++) begin : gen_bfm_output_connections
    initial begin
      blk_ctrl.connect_slave_data_port(i, s_chdr[i]);
      blk_ctrl.set_slave_stall_prob(i, STALL_PROB);
    end
  end

  //---------------------------------------------------------------------------
  // Device Under Test (DUT)
  //---------------------------------------------------------------------------

  // DUT Slave (Input) Port Signals
  logic [CHDR_W*NUM_PORTS_I-1:0] s_rfnoc_chdr_tdata;
  logic [       NUM_PORTS_I-1:0] s_rfnoc_chdr_tlast;
  logic [       NUM_PORTS_I-1:0] s_rfnoc_chdr_tvalid;
  logic [       NUM_PORTS_I-1:0] s_rfnoc_chdr_tready;

  // DUT Master (Output) Port Signals
  logic [CHDR_W*NUM_PORTS_O-1:0] m_rfnoc_chdr_tdata;
  logic [       NUM_PORTS_O-1:0] m_rfnoc_chdr_tlast;
  logic [       NUM_PORTS_O-1:0] m_rfnoc_chdr_tvalid;
  logic [       NUM_PORTS_O-1:0] m_rfnoc_chdr_tready;

  // Map the array of BFMs to a flat vector for the DUT connections
  for (genvar i = 0; i < NUM_PORTS_I; i++) begin : gen_dut_input_connections
    // Connect BFM master to DUT slave port
    assign s_rfnoc_chdr_tdata[CHDR_W*i+:CHDR_W] = m_chdr[i].tdata;
    assign s_rfnoc_chdr_tlast[i]                = m_chdr[i].tlast;
    assign s_rfnoc_chdr_tvalid[i]               = m_chdr[i].tvalid;
    assign m_chdr[i].tready                     = s_rfnoc_chdr_tready[i];
  end
  for (genvar i = 0; i < NUM_PORTS_O; i++) begin : gen_dut_output_connections
    // Connect BFM slave to DUT master port
    assign s_chdr[i].tdata        = m_rfnoc_chdr_tdata[CHDR_W*i+:CHDR_W];
    assign s_chdr[i].tlast        = m_rfnoc_chdr_tlast[i];
    assign s_chdr[i].tvalid       = m_rfnoc_chdr_tvalid[i];
    assign m_rfnoc_chdr_tready[i] = s_chdr[i].tready;
  end

  rfnoc_block_${blockname} #(
    .THIS_PORTID         (THIS_PORTID),
    .CHDR_W              (CHDR_W),
    .MTU                 (MTU)
  ) dut (
    .rfnoc_chdr_clk      (rfnoc_chdr_clk),
    .rfnoc_ctrl_clk      (rfnoc_ctrl_clk),
    .rfnoc_core_config   (backend.cfg),
    .rfnoc_core_status   (backend.sts),
    .s_rfnoc_chdr_tdata  (s_rfnoc_chdr_tdata),
    .s_rfnoc_chdr_tlast  (s_rfnoc_chdr_tlast),
    .s_rfnoc_chdr_tvalid (s_rfnoc_chdr_tvalid),
    .s_rfnoc_chdr_tready (s_rfnoc_chdr_tready),
    .m_rfnoc_chdr_tdata  (m_rfnoc_chdr_tdata),
    .m_rfnoc_chdr_tlast  (m_rfnoc_chdr_tlast),
    .m_rfnoc_chdr_tvalid (m_rfnoc_chdr_tvalid),
    .m_rfnoc_chdr_tready (m_rfnoc_chdr_tready),
    .s_rfnoc_ctrl_tdata  (m_ctrl.tdata),
    .s_rfnoc_ctrl_tlast  (m_ctrl.tlast),
    .s_rfnoc_ctrl_tvalid (m_ctrl.tvalid),
    .s_rfnoc_ctrl_tready (m_ctrl.tready),
    .m_rfnoc_ctrl_tdata  (s_ctrl.tdata),
    .m_rfnoc_ctrl_tlast  (s_ctrl.tlast),
    .m_rfnoc_ctrl_tvalid (s_ctrl.tvalid),
    .m_rfnoc_ctrl_tready (s_ctrl.tready)
  );

  //---------------------------------------------------------------------------
  // Main Test Process
  //---------------------------------------------------------------------------
  initial begin : tb_main

    // Initialize the test exec object for this testbench
    test.start_tb("rfnoc_block_${blockname}_tb");

    // Start the BFMs running
    blk_ctrl.run();

    //--------------------------------
    // Reset
    //--------------------------------

    test.start_test("Flush block then reset it", 10us);
    blk_ctrl.flush_and_reset();
    test.end_test();

    //--------------------------------
    // Verify Block Info
    //--------------------------------

    test.start_test("Verify Block Info", 2us);
    `ASSERT_ERROR(blk_ctrl.get_noc_id() == NOC_ID, "Incorrect NOC_ID Value");
    `ASSERT_ERROR(blk_ctrl.get_num_data_i() == NUM_PORTS_I, "Incorrect NUM_DATA_I Value");
    `ASSERT_ERROR(blk_ctrl.get_num_data_o() == NUM_PORTS_O, "Incorrect NUM_DATA_O Value");
    `ASSERT_ERROR(blk_ctrl.get_mtu() == MTU, "Incorrect MTU Value");
    test.end_test();

    //--------------------------------
    // Test Sequences
    //--------------------------------

    begin
      // Read and write the user register to make sure it updates correctly.
      logic [31:0] write_val, read_val;
      test.start_test("Verify user register", 5us);

      // Test user register has a default value
      blk_ctrl.reg_read(dut.REG_USER_ADDR, read_val);
      `ASSERT_ERROR(
        read_val == dut.REG_USER_DEFAULT, "Incorrect default value for user register");

      // Test writing and read user register works
      write_val = $random();
      blk_ctrl.reg_write(dut.REG_USER_ADDR, write_val);
      blk_ctrl.reg_read(dut.REG_USER_ADDR, read_val);
      `ASSERT_ERROR(
        read_val == write_val, "Initial value for user register is incorrect");

      test.end_test();
    end

    begin
      int          num_bytes;
      item_t send_samples[$];
      item_t recv_samples[$];

      test.start_test("Test passing through samples", 10us);

      // Generate a payload of random samples
      send_samples = {};
      for (int i = 0; i < SPP; i++) begin
        send_samples.push_back($random()); // 32-bit I,Q
      end

      // Queue a packet for transfer
      blk_ctrl.send_items(0, send_samples);

      // Receive the output packet
      blk_ctrl.recv_items(0, recv_samples);

      // Check the resulting payload size
      `ASSERT_ERROR(recv_samples.size() == SPP,
        "Received payload didn't match size of payload sent");

      // Check the resulting samples
      for (int i = 0; i < SPP; i++) begin
        item_t sample_in;
        item_t sample_out;

        sample_in  = send_samples[i];
        sample_out = recv_samples[i];

        `ASSERT_ERROR(
          sample_out == sample_in,
          $sformatf("Sample %4d, received 0x%08X, expected 0x%08X",
                    i, sample_out, sample_in));
      end

      test.end_test();
    end

    //--------------------------------
    // Finish Up
    //--------------------------------

    // Display final statistics and results
    test.end_tb();
  end : tb_main

endmodule : rfnoc_block_${blockname}_tb


`default_nettype wire

'''

# RFNoC image core CMakeLists.txt
Templates['icore_cmake'] = '''
# Grab all image core YAML filenames and register them
file(GLOB icore_yml_files RELATIVE ${"${CMAKE_CURRENT_SOURCE_DIR}"} "*_rfnoc_image_core.yml")
foreach (icore_yml ${"${icore_yml_files}"})
    RFNOC_REGISTER_IMAGE_CORE(SRC ${"${icore_yml}"})
endforeach()

'''

# RFNoC image core YAML
Templates['icore_yml'] = '''
# General parameters
# -----------------------------------------
schema: rfnoc_imagebuilder_args         # Identifier for the schema used to validate this file
copyright: ''                           # Copyright information used in file headers
license: 'SPDX-License-Identifier: LGPL-3.0-or-later' # License information used in file headers
version: 1.0                            # File version
rfnoc_version: 1.0                      # RFNoC protocol version
chdr_width: 64                          # Bit width of the CHDR bus for this image
device: 'x310'
default_target: 'X310_HG'

# A list of all stream endpoints in design
# ----------------------------------------
stream_endpoints:
  ep0:                       # Stream endpoint name
    ctrl: True                      # Endpoint passes control traffic
    data: True                      # Endpoint passes data traffic
    buff_size: 32768                # Ingress buffer size for data
  ep1:                       # Stream endpoint name
    ctrl: False                     # Endpoint passes control traffic
    data: True                      # Endpoint passes data traffic
    buff_size: 0                    # Ingress buffer size for data
  ep2:                       # Stream endpoint name
    ctrl: False                     # Endpoint passes control traffic
    data: True                      # Endpoint passes data traffic
    buff_size: 32768                # Ingress buffer size for data
  ep3:                       # Stream endpoint name
    ctrl: False                     # Endpoint passes control traffic
    data: True                      # Endpoint passes data traffic
    buff_size: 0                    # Ingress buffer size for data
  ep4:                       # Stream endpoint name
    ctrl: False                     # Endpoint passes control traffic
    data: True                      # Endpoint passes data traffic
    buff_size: 32768                # Ingress buffer size for data

# A list of all NoC blocks in design
# ----------------------------------
noc_blocks:
  duc0:                      # NoC block name
    block_desc: 'duc.yml'    # Block device descriptor file
    parameters:
      NUM_PORTS: 1
  ddc0:
    block_desc: 'ddc.yml'
    parameters:
      NUM_PORTS: 2
  radio0:
    block_desc: 'radio_2x64.yml'
  duc1:
    block_desc: 'duc.yml'
    parameters:
      NUM_PORTS: 1
  ddc1:
    block_desc: 'ddc.yml'
    parameters:
      NUM_PORTS: 2
  radio1:
    block_desc: 'radio_2x64.yml'
  ${blockname}0:
    block_desc: '${blockname}.yml'

# A list of all static connections in design
# ------------------------------------------
# Format: A list of connection maps (list of key-value pairs) with the following keys
#         - srcblk  = Source block to connect
#         - srcport = Port on the source block to connect
#         - dstblk  = Destination block to connect
#         - dstport = Port on the destination block to connect
connections:
  # ep0 to radio0(0) - RFA TX
  - { srcblk: ep0,    srcport: out0,  dstblk: duc0,   dstport: in_0 }
  - { srcblk: duc0,   srcport: out_0, dstblk: radio0, dstport: in_0 }
  # radio0(0) to ep0 - RFA RX
  - { srcblk: radio0, srcport: out_0, dstblk: ddc0,   dstport: in_0 }
  - { srcblk: ddc0,   srcport: out_0, dstblk: ep0,    dstport: in0  }
  # radio0(1) to ep1 - RFA RX
  - { srcblk: radio0, srcport: out_1, dstblk: ddc0,   dstport: in_1 }
  - { srcblk: ddc0,   srcport: out_1, dstblk: ep1,    dstport: in0  }
  # ep2 to radio1(0) - RFB TX
  - { srcblk: ep2,    srcport: out0,  dstblk: duc1,   dstport: in_0 }
  - { srcblk: duc1,   srcport: out_0, dstblk: radio1, dstport: in_0 }
  # radio1(0) to ep2 - RFB RX
  - { srcblk: radio1, srcport: out_0, dstblk: ddc1,   dstport: in_0 }
  - { srcblk: ddc1,   srcport: out_0, dstblk: ep2,    dstport: in0  }
  # radio1(1) to ep3 - RFB RX
  - { srcblk: radio1, srcport: out_1, dstblk: ddc1,   dstport: in_1 }
  - { srcblk: ddc1,   srcport: out_1, dstblk: ep3,    dstport: in0  }
  # Custom block connection: ep4 to ${blockname}0 and ${blockname}0 to ep4
  - { srcblk: ep4, srcport: out0, dstblk: ${blockname}0, dstport: in }
  - { srcblk: ${blockname}0, srcport: out, dstblk: ep4, dstport: in0 }
  # BSP Connections
  - { srcblk: radio0, srcport: ctrl_port, dstblk: _device_, dstport: ctrlport_radio0 }
  - { srcblk: radio1, srcport: ctrl_port, dstblk: _device_, dstport: ctrlport_radio1 }
  - { srcblk: _device_, srcport: x300_radio0, dstblk: radio0, dstport: x300_radio }
  - { srcblk: _device_, srcport: x300_radio1, dstblk: radio1, dstport: x300_radio }
  - { srcblk: _device_, srcport: time_keeper, dstblk: radio0, dstport: time_keeper }
  - { srcblk: _device_, srcport: time_keeper, dstblk: radio1, dstport: time_keeper }

# A list of all clock domain connections in design
# ------------------------------------------
# Format: A list of connection maps (list of key-value pairs) with the following keys
#         - srcblk  = Source block to connect (Always "_device"_)
#         - srcport = Clock domain on the source block to connect
#         - dstblk  = Destination block to connect
#         - dstport = Clock domain on the destination block to connect
clk_domains:
    - { srcblk: _device_, srcport: radio, dstblk: radio0, dstport: radio }
    - { srcblk: _device_, srcport: ce,    dstblk: ddc0,   dstport: ce    }
    - { srcblk: _device_, srcport: ce,    dstblk: duc0,   dstport: ce    }
    - { srcblk: _device_, srcport: radio, dstblk: radio1, dstport: radio }
    - { srcblk: _device_, srcport: ce,    dstblk: ddc1,   dstport: ce    }
    - { srcblk: _device_, srcport: ce,    dstblk: duc1,   dstport: ce    }
    - { srcblk: _device_, srcport: ce,    dstblk: ${blockname}0, dstport: ce }
'''

# Usage
Templates['usage'] = '''
rfnocmodtool <command> [options] -- Run <command> with the given options.
rfnocmodtool help -- Show a list of commands.
rfnocmodtool help <command> -- Shows the help for a given command. '''

# SWIG string
Templates['swig_block_magic'] = """% if version == '36':
% if blocktype != 'noblock':
GR_SWIG_BLOCK_MAGIC($modname, $blockname);
% endif
%%include "${modname}_${blockname}.h"
% else:
%%include "${include_dir_prefix}/${blockname}.h"
    % if blocktype != 'noblock':
GR_SWIG_BLOCK_MAGIC2(${modname}, ${blockname});
    % endif
% endif
"""

# Empty File
Templates['empty'] = """
"""
