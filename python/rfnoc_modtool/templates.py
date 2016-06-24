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
#if $blocktype == 'rfnoc'
\#include <ettus/rfnoc_block_impl.h>
#end if

namespace gr {
  namespace ${modname} {

#if $blocktype == 'rfnoc'
    class ${blockname}_impl : public ${blockname}, public gr::ettus::rfnoc_block_impl
#else
    class ${blockname}_impl : public ${blockname}
#end if
    {
     private:
      // Nothing to declare in this block.

#if $blocktype == 'tagged_stream'
     protected:
      int calculate_output_stream_length(const gr_vector_int &ninput_items);

#end if
     public:
      ${blockname}_impl(
#if $blocktype == 'rfnoc'
#if $arglist:
        ${strip_default_values($arglist)},
#end if
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const int block_select,
        const int device_select
#else
        ${strip_default_values($arglist)}
#end if
      );
      ~${blockname}_impl();

      // Where all the action really happens
#if $blocktype == 'general'
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
#else if $blocktype == 'tagged_stream'
      int work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
#else if $blocktype == 'hier'
#silent pass
#else if $blocktype == 'rfnoc'
#silent pass
#else
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
#end if
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
#if $blocktype == 'noblock'
\#include <${include_dir_prefix}/${blockname}.h>
#else
\#include "${blockname}_impl.h"
#end if
#if $blocktype == 'rfnoc'
\#include <${include_dir_prefix}/${blockname}_block_ctrl.hpp>
#end if
namespace gr {
  namespace ${modname} {

#if $blocktype == 'noblock'
    $blockname::${blockname}(${strip_default_values($arglist)})
    {
    }

    $blockname::~${blockname}()
    {
    }
#else
    ${blockname}::sptr
    ${blockname}::make(
#if $blocktype == 'rfnoc'
#if $arglist:
        ${strip_default_values($arglist)},
#end if
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const int block_select,
        const int device_select
#else
      ${strip_default_values($arglist)}
#end if
    )
    {
      return gnuradio::get_initial_sptr(
        new ${blockname}_impl(
#if $blocktype == 'rfnoc'
#if $arglist:
            ${strip_arg_types($arglist)},
#end if
            dev,
            tx_stream_args,
            rx_stream_args,
            block_select,
            device_select
#else
            ${strip_arg_types($arglist)}
#end if
        )
      );
    }

#if $blocktype == 'decimator'
#set $decimation = ', <+decimation+>'
#else if $blocktype == 'interpolator'
#set $decimation = ', <+interpolation+>'
#else if $blocktype == 'tagged_stream'
#set $decimation = ', <+len_tag_key+>'
#else
#set $decimation = ''
#end if
#if $blocktype == 'source'
#set $inputsig = '0, 0, 0'
#else
#set $inputsig = '<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)'
#end if
#if $blocktype == 'sink'
#set $outputsig = '0, 0, 0'
#else
#set $outputsig = '<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)'
#end if
    /*
     * The private constructor
     */
    ${blockname}_impl::${blockname}_impl(
#if $blocktype == 'rfnoc'
#if $arglist
         ${strip_default_values($arglist)},
#end if
         const gr::ettus::device3::sptr &dev,
         const ::uhd::stream_args_t &tx_stream_args,
         const ::uhd::stream_args_t &rx_stream_args,
         const int block_select,
         const int device_select
#else
         ${strip_default_values($arglist)}
#end if
    )
#if $blocktype == 'rfnoc'
      : gr::${grblocktype}("${blockname}"),
        gr::${grblocktype}_impl(
            dev,
            gr::${grblocktype}_impl::make_block_id("${blockname}",  block_select, device_select),
            tx_stream_args, rx_stream_args
            )
#else
    ${blockname}_impl::${blockname}_impl(${strip_default_values($arglist)})
      : gr::${grblocktype}("${blockname}",
              gr::io_signature::make($inputsig),
              gr::io_signature::make($outputsig)$decimation)
#end if

#if $blocktype == 'hier'
    {
      connect(self(), 0, d_firstblock, 0);
      // connect other blocks
      connect(d_lastblock, 0, self(), 0);
    }
#else
    {}
#end if

    /*
     * Our virtual destructor.
     */
    ${blockname}_impl::~${blockname}_impl()
    {
    }

#if $blocktype == 'general'
    void
    ${blockname}_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    ${blockname}_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }
#else if $blocktype == 'tagged_stream'
    int
    ${blockname}_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = /* <+set this+> */;
      return noutput_items ;
    }

    int
    ${blockname}_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }
#else if $blocktype == 'hier'
#silent pass
#else if $blocktype == 'rfnoc'
#silent pass
#else
    int
    ${blockname}_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
#if $blocktype == 'source'
#silent pass
#else
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
#end if
#if $blocktype == 'sink'
#silent pass
#else
      <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];
#end if

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }
#end if
#end if

  } /* namespace ${modname} */
} /* namespace gr */

'''

# Block definition header file (for include/)
Templates['block_def_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#ifndef INCLUDED_${modname.upper()}_${blockname.upper()}_H
\#define INCLUDED_${modname.upper()}_${blockname.upper()}_H

\#include <${include_dir_prefix}/api.h>
#if $blocktype != 'noblock' and $blocktype!='rfnoc'
\#include <gnuradio/${grblocktype}.h>
#end if
#if $blocktype == 'rfnoc'
\#include <ettus/device3.h>
\#include <ettus/rfnoc_block.h>
\#include <uhd/stream.hpp>
#end if

namespace gr {
  namespace ${modname} {

#if $blocktype == 'noblock'
    /*!
     * \\brief <+description+>
     *
     */
    class ${modname.upper()}_API $blockname
    {
    public:
      ${blockname}(${arglist});
      ~${blockname}();
    private:
    };
#else
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
#if $blocktype == 'rfnoc'
#if $arglist
        $arglist,
#end if
        const gr::ettus::device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const int block_select=-1,
        const int device_select=-1
#else
         $arglist
#end if
        );
    };
#end if
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

# C++ file for QA TODO check this
Templates['qa_cpp'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#include <gnuradio/attributes.h>
\#include <cppunit/TestAssert.h>
\#include "qa_${blockname}.h"
\#include <${include_dir_prefix}/${blockname}.h>

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

# Header file for QA  TODO check this
Templates['qa_h'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#ifndef _QA_${blockname.upper()}_H_
\#define _QA_${blockname.upper()}_H_

\#include <cppunit/extensions/HelperMacros.h>
\#include <cppunit/TestCase.h>

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

\#endif /* _QA_${blockname.upper()}_H_ */

'''

# Python QA code TODO check this
Templates['qa_python'] = '''\#!/usr/bin/env python
# -*- coding: utf-8 -*-
${str_to_python_comment($license)}
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
#if $lang == 'cpp'
import ${modname}_swig as ${modname}
#else
from ${blockname} import ${blockname}
#end if

class qa_$blockname (gr_unittest.TestCase):

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

Templates['grc_xml'] = '''<?xml version="1.0"?>
<block>
#if $blocktype == 'rfnoc'
  <name>RFNoC: $blockname</name>
#else
  <name>$blockname</name>
#end if
  <key>${modname}_$blockname</key>
  <category>$modname</category>
  <import>import $modname</import>
  <make>${modname}.${blockname}(
#if $blocktype == 'rfnoc'
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
#else
          ${strip_arg_types_grc($arglist)}
#end if
  )</make>
  <!-- Make one 'param' node for every Parameter you want settable from the GUI.
       Sub-nodes:
       * name
       * key (makes the value accessible as \$keyname, e.g. in the make node)
       * type -->


#if $blocktype == 'rfnoc'
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
#end if
  <param>
    <name>...</name>
    <key>...</key>
    <type>...</type>
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
// Copyright 2015 Ettus Research
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

## Old stuff
# C++ file of a GR block
Templates['block_cpp36'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}
\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

#if $blocktype != 'noblock'
\#include <gr_io_signature.h>
#end if
\#include "${modname}_${blockname}.h"

#if $blocktype == 'noblock'
${modname}_${blockname}::${modname}_${blockname}(${strip_default_values($arglist)})
{
}

${modname}_${blockname}::~${modname}_${blockname}()
{
}
#else
${modname}_${blockname}_sptr
${modname}_make_${blockname} (${strip_default_values($arglist)})
{
  return gnuradio::get_initial_sptr (new ${modname}_${blockname}(${strip_arg_types($arglist)}));
}

#if $blocktype == 'decimator'
#set $decimation = ', <+decimation+>'
#else if $blocktype == 'interpolator'
#set $decimation = ', <+interpolation+>'
#else
#set $decimation = ''
#end if
#if $blocktype == 'sink'
#set $inputsig = '0, 0, 0'
#else
#set $inputsig = '<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)'
#end if
#if $blocktype == 'source'
#set $outputsig = '0, 0, 0'
#else
#set $outputsig = '<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)'
#end if

/*
 * The private constructor
 */
${modname}_${blockname}::${modname}_${blockname} (${strip_default_values($arglist)})
  : gr_${grblocktype} ("${blockname}",
       gr_make_io_signature($inputsig),
       gr_make_io_signature($outputsig)$decimation)
{
#if $blocktype == 'hier'
  connect(self(), 0, d_firstblock, 0);
  // <+connect other blocks+>
  connect(d_lastblock, 0, self(), 0);
#else
  // Put in <+constructor stuff+> here
#end if
}


/*
 * Our virtual destructor.
 */
${modname}_${blockname}::~${modname}_${blockname}()
{
  // Put in <+destructor stuff+> here
}
#end if


#if $blocktype == 'general'
void
${modname}_${blockname}::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

int
${modname}_${blockname}::general_work (int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items)
{
  const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
  <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

  // Do <+signal processing+>
  // Tell runtime system how many input items we consumed on
  // each input stream.
  consume_each (noutput_items);

  // Tell runtime system how many output items we produced.
  return noutput_items;
}
#else if $blocktype == 'hier' or $blocktype == 'noblock'
#pass
#else
int
${modname}_${blockname}::work(int noutput_items,
      gr_vector_const_void_star &input_items,
      gr_vector_void_star &output_items)
{
  const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
  <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

  // Do <+signal processing+>

  // Tell runtime system how many output items we produced.
  return noutput_items;
}
#end if

'''

# Block definition header file (for include/)
Templates['block_h36'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#ifndef INCLUDED_${modname.upper()}_${blockname.upper()}_H
\#define INCLUDED_${modname.upper()}_${blockname.upper()}_H

\#include <${modname}_api.h>
#if $blocktype == 'noblock'
class ${modname.upper()}_API $blockname
{
  ${blockname}(${arglist});
  ~${blockname}();
 private:
};

#else
\#include <gr_${grblocktype}.h>

class ${modname}_${blockname};

typedef boost::shared_ptr<${modname}_${blockname}> ${modname}_${blockname}_sptr;

${modname.upper()}_API ${modname}_${blockname}_sptr ${modname}_make_${blockname} ($arglist);

/*!
 * \\brief <+description+>
 * \ingroup ${modname}
 *
 */
class ${modname.upper()}_API ${modname}_${blockname} : public gr_$grblocktype
{
 private:
  friend ${modname.upper()}_API ${modname}_${blockname}_sptr ${modname}_make_${blockname} (${strip_default_values($arglist)});

  ${modname}_${blockname}(${strip_default_values($arglist)});

 public:
  ~${modname}_${blockname}();

#if $blocktype == 'general'
  void forecast (int noutput_items, gr_vector_int &ninput_items_required);

  // Where all the action really happens
  int general_work (int noutput_items,
      gr_vector_int &ninput_items,
      gr_vector_const_void_star &input_items,
      gr_vector_void_star &output_items);
#else if $blocktype == 'hier'
#pass
#else
  // Where all the action really happens
  int work (int noutput_items,
      gr_vector_const_void_star &input_items,
      gr_vector_void_star &output_items);
#end if
};
#end if

\#endif /* INCLUDED_${modname.upper()}_${blockname.upper()}_H */

'''

# C++ file for QA
Templates['qa_cpp36'] = '''/* -*- c++ -*- */
${str_to_fancyc_comment($license)}

\#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(qa_${modname}_${blockname}_t1){
    BOOST_CHECK_EQUAL(2 + 2, 4);
    // TODO BOOST_* test macros here
}

BOOST_AUTO_TEST_CASE(qa_${modname}_${blockname}_t2){
    BOOST_CHECK_EQUAL(2 + 2, 4);
    // TODO BOOST_* test macros here
}

'''

# Header file for QA
Templates['qa_cmakeentry36'] = """
add_executable($basename $filename)
target_link_libraries($basename gnuradio-$modname \${Boost_LIBRARIES})
GR_ADD_TEST($basename $basename)
"""

