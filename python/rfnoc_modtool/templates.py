#!/usr/bin/env python
###RFNoC Modtool

''' All the templates for skeleton files (needed by ModToolAdd) '''

from datetime import datetime

Templates = {}


# Default licence
Templates['defaultlicense'] = '''
Copyright %d ${copyrightholder}.

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
''' % datetime.now().year

Templates['grlicense'] = '''
Copyright %d Free Software Foundation, Inc.

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
''' % datetime.now().year

# Header file of a sync/decimator/interpolator block
Templates['block_impl_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}
\#ifndef INCLUDED_${modname.upper()}_${blockname.upper()}_IMPL_H
\#define INCLUDED_${modname.upper()}_${blockname.upper()}_IMPL_H

\#include <${include_dir_prefix}/${blockname}.h>
\#include <${include_dir_prefix}/${blockname}_block_ctrl.hpp>
\#include <ettus/rfnoc_block_impl.h>

namespace gr {
  namespace ${modname} {

    class ${blockname}_impl : public ${blockname}, public gr::ettus::rfnoc_block_impl
    {
     private:
      // Nothing to declare in this block.

     public:
      ${blockname}_impl(
#if $arglist:
        ${strip_default_values($arglist)},
#end if
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const int block_select,
        const int device_select
      );
      ~${blockname}_impl();

      // Where all the action really happens
    };

  } // namespace ${modname}
} // namespace gr

\#endif /* INCLUDED_${modname.upper()}_${blockname.upper()}_IMPL_H */

'''

# C++ file of a GR block
Templates['block_impl_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}
\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include <gnuradio/io_signature.h>
\#include "${blockname}_impl.h"
namespace gr {
  namespace ${modname} {
    ${blockname}::sptr
    ${blockname}::make(
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
#if $arglist:
        ${strip_default_values($arglist)},
#end if
        const int block_select,
        const int device_select
    )
    {
      return gnuradio::get_initial_sptr(
        new ${blockname}_impl(
#if $arglist:
            ${strip_arg_types($arglist)},
#end if
            dev,
            tx_stream_args,
            rx_stream_args,
            block_select,
            device_select
        )
      );
    }

    /*
     * The private constructor
     */
    ${blockname}_impl::${blockname}_impl(
#if $arglist
         ${strip_default_values($arglist)},
#end if
         const gr::ettus::device3::sptr &dev,
         const ::uhd::stream_args_t &tx_stream_args,
         const ::uhd::stream_args_t &rx_stream_args,
         const int block_select,
         const int device_select
    )
      : gr::${grblocktype}("${blockname}"),
        gr::${grblocktype}_impl(
            dev,
            gr::${grblocktype}_impl::make_block_id("${blockname}",  block_select, device_select),
            tx_stream_args, rx_stream_args
            )
    {}

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
${str_to_fancyc_comment($license)}

\#ifndef INCLUDED_${modname.upper()}_${blockname.upper()}_H
\#define INCLUDED_${modname.upper()}_${blockname.upper()}_H

\#include <${include_dir_prefix}/api.h>
\#include <ettus/device3.h>
\#include <ettus/rfnoc_block.h>
\#include <uhd/stream.hpp>

namespace gr {
  namespace ${modname} {

    /*!
     * \\brief <+description of block+>
     * \ingroup ${modname}
     *
     */
    class ${modname.upper()}_API ${blockname} : virtual public gr::$grblocktype
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
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
#if $arglist
        $arglist,
#end if
        const int block_select=-1,
        const int device_select=-1
        );
    };
  } // namespace ${modname}
} // namespace gr

\#endif /* INCLUDED_${modname.upper()}_${blockname.upper()}_H */

'''
# Header for RFNoC Block Controller (UHD Host-part)
Templates['block_ctrl_hpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#ifndef INCLUDED_LIBUHD_RFNOC_${modname.upper()}_${blockname.upper()}_HPP
\#define INCLUDED_LIBUHD_RFNOC_${modname.upper()}_${blockname.upper()}_HPP

\#include <uhd/rfnoc/source_block_ctrl_base.hpp>
\#include <uhd/rfnoc/sink_block_ctrl_base.hpp>

namespace uhd {
    namespace rfnoc {

/*! \\brief Block controller for the standard copy RFNoC block.
 *
 */
class UHD_API ${blockname}_block_ctrl : public source_block_ctrl_base, public sink_block_ctrl_base
{
public:
    UHD_RFNOC_BLOCK_OBJECT(${blockname}_block_ctrl)

    /*!
     * Your block configuration here
    */
}; /* class ${blockname}_block_ctrl*/

}} /* namespace uhd::rfnoc */

#endif /* INCLUDED_LIBUHD_RFNOC_${modname.upper()}_${blockname.upper()}_BLOCK_CTRL_HPP */
'''

# RFNoC Block Controller (UHD Host-part)
Templates['block_ctrl_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#include <${include_dir_prefix}/${blockname}_block_ctrl.hpp>
\#include <uhd/convert.hpp>
\#include <uhd/utils/msg.hpp>

using namespace uhd::rfnoc;

class ${blockname}_block_ctrl_impl : public ${blockname}_block_ctrl
{
public:

    UHD_RFNOC_BLOCK_CONSTRUCTOR(${blockname}_block_ctrl)
    {

    }
private:

};

UHD_RFNOC_BLOCK_REGISTER(${blockname}_block_ctrl,"${blockname}");
'''

Templates['grc_xml'] = '''<?xml version="1.0"?>
<block>
  <name>RFNoC: $blockname</name>
  <key>${modname}_$blockname</key>
  <category>$modname</category>
  <import>import $modname</import>
  <make>${modname}.${blockname}(
#if $arglist
          ${strip_arg_types_grc($arglist)},
#end if
          self.device3,
          uhd.stream_args( \# TX Stream Args
                cpu_format="\$type",
                otw_format="\$otw",
                args="gr_vlen={0},{1}".format(\${grvlen}, "" if \$grvlen == 1 else "spp={0}".format(\$grvlen)),
          ),
          uhd.stream_args( \# RX Stream Args
                cpu_format="\$type",
                otw_format="\$otw",
                args="gr_vlen={0},{1}".format(\${grvlen}, "" if \$grvlen == 1 else "spp={0}".format(\$grvlen)),
          ),
          \$block_index,
          \$device_index
  )</make>
  <!-- Make one 'param' node for every Parameter you want settable from the GUI.
       Sub-nodes:
       * name
       * key (makes the value accessible as \$keyname, e.g. in the make node)
       * type -->

  <param>
    <name>Host Data Type</name>
    <key>type</key>
    <type>enum</type>
    <option>
      <name>Complex float32</name>
      <key>fc32</key>
      <opt>type:complex</opt>
    </option>
    <option>
      <name>Complex int16</name>
      <key>sc16</key>
      <opt>type:sc16</opt>
    </option>
    <option>
      <name>Byte</name>
      <key>u8</key>
      <opt>type:byte</opt>
    </option>
    <option>
      <name>VITA word32</name>
      <key>item32</key>
      <opt>type:s32</opt>
    </option>
  </param>
  <!--RFNoC basic block configuration -->
  <param>
    <name>Device Select</name>
    <key>device_index</key>
    <value>-1</value>
    <type>int</type>
    <hide>\#if int(\$device_index()) &lt; 0 then 'part' else 'none'\#</hide>
    <tab>RFNoC Config</tab>
  </param>

  <param>
    <name>${blockname.upper()} Select</name>
    <key>block_index</key>
    <value>-1</value>
    <type>int</type>
    <hide>\#if int(\$block_index()) &lt; 0 then 'part' else 'none'\#</hide>
    <tab>RFNoC Config</tab>
  </param>

  <param>
    <name>FPGA Module Name</name>
    <key>fpga_module_name</key>
    <value>noc_block_${blockname}</value>
    <type>string</type>
    <hide>all</hide>
    <tab>RFNoC Config</tab>
  </param>

  <param>
    <name>Force Vector Length</name>
    <key>grvlen</key>
    <value>1</value>
    <type>int</type>
  </param>

  <param>
    <name>Device Format</name>
    <key>otw</key>
    <type>enum</type>
    <option>
      <name>Complex int16</name>
      <key>sc16</key>
    </option>
    <option>
      <name>Complex int8</name>
      <key>sc8</key>
    </option>
    <option>
      <name>Byte</name>
      <key>u8</key>
    </option>
  </param>

  <!-- Make one 'sink' node per input. Sub-nodes:
       * name (an identifier for the GUI)
       * type
       * vlen
       * optional (set to 1 for optional inputs) -->
  <sink>
    <name>in</name>
    <type>\$type.type</type>
    <vlen>\$grvlen</vlen>
    <domain>rfnoc</domain>
  </sink>

  <!-- Make one 'source' node per output. Sub-nodes:
       * name (an identifier for the GUI)
       * type
       * vlen
       * optional (set to 1 for optional inputs) -->
  <source>
    <name>out</name>
    <type>\$type.type</type>
    <vlen>\$grvlen</vlen>
    <domain>rfnoc</domain>
  </source>
</block>
'''

# Block Controller xml at rfnoc/
Templates['rfnoc_xml'] = '''<?xml version="1.0"?>
<!--Default XML file-->
<nocblock>
  <name>${blockname}</name>
  <blockname>${blockname}</blockname>
  <ids>
    <id revision="0">${noc_id}</id>
  </ids>
  <!--One input, one output. If this is used, better have all the info the C++ file.-->
  <ports>
    <sink>
      <name>in</name>
    </sink>
    <source>
      <name>out</name>
    </source>
  </ports>
</nocblock>
'''

# RFNoC Verilog file
Templates['rfnoc_v'] = '''
//
${str_to_fancyc_comment($license)}
//
module noc_block_$blockname \#(
  parameter NOC_ID = 64'h${noc_id},
  parameter STR_SINK_FIFOSIZE = 11)
(
  input bus_clk, input bus_rst,
  input ce_clk, input ce_rst,
  input  [63:0] i_tdata, input  i_tlast, input  i_tvalid, output i_tready,
  output [63:0] o_tdata, output o_tlast, output o_tvalid, input  o_tready,
  output [63:0] debug
);

  ////////////////////////////////////////////////////////////
  //
  // RFNoC Shell
  //
  ////////////////////////////////////////////////////////////
  wire [31:0] set_data;
  wire [7:0]  set_addr;
  wire        set_stb;
  reg  [63:0] rb_data;
  wire [7:0]  rb_addr;

  wire [63:0] cmdout_tdata, ackin_tdata;
  wire        cmdout_tlast, cmdout_tvalid, cmdout_tready, ackin_tlast, ackin_tvalid, ackin_tready;

  wire [63:0] str_sink_tdata, str_src_tdata;
  wire        str_sink_tlast, str_sink_tvalid, str_sink_tready, str_src_tlast, str_src_tvalid, str_src_tready;

  wire [15:0] src_sid;
  wire [15:0] next_dst_sid, resp_out_dst_sid;
  wire [15:0] resp_in_dst_sid;

  wire        clear_tx_seqnum;

  noc_shell #(
    .NOC_ID(NOC_ID),
    .STR_SINK_FIFOSIZE(STR_SINK_FIFOSIZE))
  noc_shell (
    .bus_clk(bus_clk), .bus_rst(bus_rst),
    .i_tdata(i_tdata), .i_tlast(i_tlast), .i_tvalid(i_tvalid), .i_tready(i_tready),
    .o_tdata(o_tdata), .o_tlast(o_tlast), .o_tvalid(o_tvalid), .o_tready(o_tready),
    // Computer Engine Clock Domain
    .clk(ce_clk), .reset(ce_rst),
    // Control Sink
    .set_data(set_data), .set_addr(set_addr), .set_stb(set_stb),
    .rb_stb(1'b1), .rb_data(rb_data), .rb_addr(rb_addr),
    // Control Source
    .cmdout_tdata(cmdout_tdata), .cmdout_tlast(cmdout_tlast), .cmdout_tvalid(cmdout_tvalid), .cmdout_tready(cmdout_tready),
    .ackin_tdata(ackin_tdata), .ackin_tlast(ackin_tlast), .ackin_tvalid(ackin_tvalid), .ackin_tready(ackin_tready),
    // Stream Sink
    .str_sink_tdata(str_sink_tdata), .str_sink_tlast(str_sink_tlast), .str_sink_tvalid(str_sink_tvalid), .str_sink_tready(str_sink_tready),
    // Stream Source
    .str_src_tdata(str_src_tdata), .str_src_tlast(str_src_tlast), .str_src_tvalid(str_src_tvalid), .str_src_tready(str_src_tready),
    // Stream IDs set by host
    .src_sid(src_sid),                   // SID of this block
    .next_dst_sid(next_dst_sid),         // Next destination SID
    .resp_in_dst_sid(resp_in_dst_sid),   // Response destination SID for input stream responses / errors
    .resp_out_dst_sid(resp_out_dst_sid), // Response destination SID for output stream responses / errors
    // Misc
    .vita_time('d0), .clear_tx_seqnum(clear_tx_seqnum),
    .debug(debug));

  ////////////////////////////////////////////////////////////
  //
  // AXI Wrapper
  // Convert RFNoC Shell interface into AXI stream interface
  //
  ////////////////////////////////////////////////////////////
  wire [31:0] m_axis_data_tdata;
  wire        m_axis_data_tlast;
  wire        m_axis_data_tvalid;
  wire        m_axis_data_tready;

  wire [31:0] s_axis_data_tdata;
  wire        s_axis_data_tlast;
  wire        s_axis_data_tvalid;
  wire        s_axis_data_tready;

  axi_wrapper #(
    .SIMPLE_MODE(1))
  axi_wrapper (
    .clk(ce_clk), .reset(ce_rst),
    .clear_tx_seqnum(clear_tx_seqnum),
    .next_dst(next_dst_sid),
    .set_stb(set_stb), .set_addr(set_addr), .set_data(set_data),
    .i_tdata(str_sink_tdata), .i_tlast(str_sink_tlast), .i_tvalid(str_sink_tvalid), .i_tready(str_sink_tready),
    .o_tdata(str_src_tdata), .o_tlast(str_src_tlast), .o_tvalid(str_src_tvalid), .o_tready(str_src_tready),
    .m_axis_data_tdata(m_axis_data_tdata),
    .m_axis_data_tlast(m_axis_data_tlast),
    .m_axis_data_tvalid(m_axis_data_tvalid),
    .m_axis_data_tready(m_axis_data_tready),
    .m_axis_data_tuser(),
    .s_axis_data_tdata(s_axis_data_tdata),
    .s_axis_data_tlast(s_axis_data_tlast),
    .s_axis_data_tvalid(s_axis_data_tvalid),
    .s_axis_data_tready(s_axis_data_tready),
    .s_axis_data_tuser(),
    .m_axis_config_tdata(),
    .m_axis_config_tlast(),
    .m_axis_config_tvalid(),
    .m_axis_config_tready(),
    .m_axis_pkt_len_tdata(),
    .m_axis_pkt_len_tvalid(),
    .m_axis_pkt_len_tready());

  ////////////////////////////////////////////////////////////
  //
  // User code
  //
  ////////////////////////////////////////////////////////////
  // NoC Shell registers 0 - 127,
  // User register address space starts at 128
  localparam SR_USER_REG_BASE = 128;

  // Control Source Unused
  assign cmdout_tdata  = 64'd0;
  assign cmdout_tlast  = 1'b0;
  assign cmdout_tvalid = 1'b0;
  assign ackin_tready  = 1'b1;

  // Settings registers
  //
  // - The settings register bus is a simple strobed interface.
  // - Transactions include both a write and a readback.
  // - The write occurs when set_stb is asserted.
  //   The settings register with the address matching set_addr will
  //   be loaded with the data on set_data.
  // - Readback occurs when rb_stb is asserted. The read back strobe
  //   must assert at least one clock cycle after set_stb asserts /
  //   rb_stb is ignored if asserted on the same clock cycle of set_stb.
  //   Example valid and invalid timing:
  //              __    __    __    __
  //   clk     __|  |__|  |__|  |__|  |__
  //               _____
  //   set_stb ___|     |________________
  //                     _____
  //   rb_stb  _________|     |__________     (Valid)
  //                           _____
  //   rb_stb  _______________|     |____     (Valid)
  //           __________________________
  //   rb_stb                                 (Valid if readback data is a constant)
  //               _____
  //   rb_stb  ___|     |________________     (Invalid / ignored, same cycle as set_stb)
  //
  localparam [7:0] SR_TEST_REG_0 = SR_USER_REG_BASE;
  localparam [7:0] SR_TEST_REG_1 = SR_USER_REG_BASE + 8'd1;

  wire [31:0] test_reg_0;
  setting_reg #(
    .my_addr(SR_TEST_REG_0), .awidth(8), .width(32))
  sr_test_reg_0 (
    .clk(ce_clk), .rst(ce_rst),
    .strobe(set_stb), .addr(set_addr), .in(set_data), .out(test_reg_0), .changed());

  wire [31:0] test_reg_1;
  setting_reg #(
    .my_addr(SR_TEST_REG_1), .awidth(8), .width(32))
  sr_test_reg_1 (
    .clk(ce_clk), .rst(ce_rst),
    .strobe(set_stb), .addr(set_addr), .in(set_data), .out(test_reg_1), .changed());

  // Readback registers
  // rb_stb set to 1'b1 on NoC Shell
  always @(posedge ce_clk) begin
    case(rb_addr)
      8'd0 : rb_data <= {32'd0, test_reg_0};
      8'd1 : rb_data <= {32'd0, test_reg_1};
      default : rb_data <= 64'h0BADC0DE0BADC0DE;
    endcase
  end

  /* Simple Loopback */
  assign m_axis_data_tready = s_axis_data_tready;
  assign s_axis_data_tvalid = m_axis_data_tvalid;
  assign s_axis_data_tlast  = m_axis_data_tlast;
  assign s_axis_data_tdata  = m_axis_data_tdata;

endmodule
'''

# RFNoC Testbench
Templates['rfnoc_tb'] = '''${str_to_fancyc_comment($license)}
`timescale 1ns/1ps
`define NS_PER_TICK 1
`define NUM_TEST_CASES 5

`include "sim_exec_report.vh"
`include "sim_clks_rsts.vh"
`include "sim_rfnoc_lib.svh"

module noc_block_${blockname}_tb();
  `TEST_BENCH_INIT("noc_block_${blockname}",`NUM_TEST_CASES,`NS_PER_TICK);
  localparam BUS_CLK_PERIOD = \$ceil(1e9/166.67e6);
  localparam CE_CLK_PERIOD  = \$ceil(1e9/200e6);
  localparam NUM_CE         = 1;  // Number of Computation Engines / User RFNoC blocks to simulate
  localparam NUM_STREAMS    = 1;  // Number of test bench streams
  `RFNOC_SIM_INIT(NUM_CE, NUM_STREAMS, BUS_CLK_PERIOD, CE_CLK_PERIOD);
  `RFNOC_ADD_BLOCK(noc_block_${blockname}, 0);

  localparam SPP = 16; // Samples per packet

  /********************************************************
  ** Verification
  ********************************************************/
  initial begin : tb_main
    string s;
    logic [31:0] random_word;
    logic [63:0] readback;

    /********************************************************
    ** Test 1 -- Reset
    ********************************************************/
    `TEST_CASE_START("Wait for Reset");
    while (bus_rst) @(posedge bus_clk);
    while (ce_rst) @(posedge ce_clk);
    `TEST_CASE_DONE(~bus_rst & ~ce_rst);

    /********************************************************
    ** Test 2 -- Check for correct NoC IDs
    ********************************************************/
    `TEST_CASE_START("Check NoC ID");
    // Read NOC IDs
    tb_streamer.read_reg(sid_noc_block_${blockname}, RB_NOC_ID, readback);
    \$display("Read ${blockname.upper()} NOC ID: %16x", readback);
    `ASSERT_ERROR(readback == noc_block_${blockname}.NOC_ID, "Incorrect NOC ID");
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 3 -- Connect RFNoC blocks
    ********************************************************/
    `TEST_CASE_START("Connect RFNoC blocks");
    `RFNOC_CONNECT(noc_block_tb,noc_block_${blockname},SC16,SPP);
    `RFNOC_CONNECT(noc_block_${blockname},noc_block_tb,SC16,SPP);
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 4 -- Write / readback user registers
    ********************************************************/
    `TEST_CASE_START("Write / readback user registers");
    random_word = \$random();
    tb_streamer.write_user_reg(sid_noc_block_${blockname}, noc_block_${blockname}.SR_TEST_REG_0, random_word);
    tb_streamer.read_user_reg(sid_noc_block_${blockname}, 0, readback);
    \$sformat(s, "User register 0 incorrect readback! Expected: %0d, Actual %0d", readback[31:0], random_word);
    `ASSERT_ERROR(readback[31:0] == random_word, s);
    random_word = \$random();
    tb_streamer.write_user_reg(sid_noc_block_${blockname}, noc_block_${blockname}.SR_TEST_REG_1, random_word);
    tb_streamer.read_user_reg(sid_noc_block_${blockname}, 1, readback);
    \$sformat(s, "User register 1 incorrect readback! Expected: %0d, Actual %0d", readback[31:0], random_word);
    `ASSERT_ERROR(readback[31:0] == random_word, s);
    `TEST_CASE_DONE(1);

    /********************************************************
    ** Test 5 -- Test sequence
    ********************************************************/
    // ${blockname}'s user code is a loopback, so we should receive
    // back exactly what we send
    `TEST_CASE_START("Test sequence");
    fork
      begin
        cvita_payload_t send_payload;
        for (int i = 0; i < SPP/2; i++) begin
          send_payload.push_back(64'(i));
        end
        tb_streamer.send(send_payload);
      end
      begin
        cvita_payload_t recv_payload;
        cvita_metadata_t md;
        logic [63:0] expected_value;
        tb_streamer.recv(recv_payload,md);
        for (int i = 0; i < SPP/2; i++) begin
          expected_value = i;
          \$sformat(s, "Incorrect value received! Expected: %0d, Received: %0d", expected_value, recv_payload[i]);
          `ASSERT_ERROR(recv_payload[i] == expected_value, s);
        end
      end
    join
    `TEST_CASE_DONE(1);
    `TEST_BENCH_DONE;

  end
endmodule
'''

# RFNoC Testbenches Makefile
Templates['tb_makefile'] = '''

${str_to_python_comment($license)}

\#-------------------------------------------------
\# Top-of-Makefile
\#-------------------------------------------------
\# Define BASE_DIR to point to the "top" dir
BASE_DIR = \$(FPGA_TOP_DIR)/usrp3/top
\# Include viv_sim_preample after defining BASE_DIR
include \$(BASE_DIR)/../tools/make/viv_sim_preamble.mak

\#-------------------------------------------------
\# Testbench Specific
\#-------------------------------------------------
\# Define only one toplevel module
SIM_TOP = noc_block_${blockname}_tb

\# Add test bench, user design under test, and
\# additional user created files
SIM_SRCS = \\
\$(abspath noc_block_${blockname}_tb.sv) \\
\$(abspath ../../fpga-src/noc_block_${blockname}.v)

MODELSIM_USER_DO =

\#-------------------------------------------------
\# Bottom-of-Makefile
\#-------------------------------------------------
\# Include all simulator specific makefiles here
\# Each should define a unique target to simulate
\# e.g. xsim, vsim, etc and a common "clean" target
include \$(BASE_DIR)/../tools/make/viv_simulator.mak
'''

# Usage
Templates['usage'] = '''
rfnocmodtool <command> [options] -- Run <command> with the given options.
rfnocmodtool help -- Show a list of commands.
rfnocmodtool help <command> -- Shows the help for a given command. '''

# SWIG string
Templates['swig_block_magic'] = """#if $version == '36'
#if $blocktype != 'noblock'
GR_SWIG_BLOCK_MAGIC($modname, $blockname);
#end if
%include "${modname}_${blockname}.h"
#else
%include "${include_dir_prefix}/${blockname}.h"
#if $blocktype != 'noblock'
GR_SWIG_BLOCK_MAGIC2($modname, $blockname);
#end if
#end if
"""

# Empty File
Templates['empty'] = """
"""
