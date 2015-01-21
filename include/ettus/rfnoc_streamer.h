/* -*- c++ -*- */
/* 
 * Copyright 2014-2015 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_UHD_RFNOC_STREAMER_H
#define INCLUDED_UHD_RFNOC_STREAMER_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <gnuradio/block.h>
#include <uhd/stream.hpp>

namespace gr {
  //namespace uhd {
  namespace ettus {

    /*!
     * \brief Base class for RFNoC blocks
     * \ingroup uhd
     *
     * RFNoC is method provided by UHD to have multiple processing blocks
     * on an FPGA (typically on a USRP). These blocks can be connected
     * dynamically, similar to GNU Radio blocks.
     *
     * This is an interface to represent a UHD RFNoC block in GNU Radio.
     * It can also be used to connect GNU Radio blocks with RFNoC blocks;
     * in this case, a streamer will internally be created for the transport
     * between the FPGA and the GNU Radio flow graph.
     */
    //class UHD_API rfnoc_streamer : virtual public gr::block
    class ETTUS_API rfnoc_streamer : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<rfnoc_streamer> sptr;

      static sptr make(
          const device3::sptr &dev,
          const std::string &block_id,
          const ::uhd::stream_args_t &stream_args,
          const int ninputs=1,
          const int noutputs=1,
          bool align_inputs=false,
          bool align_outputs=false
      );

      virtual void set_option(const std::string &key, const std::string &val) = 0;

      // Add RFNoC-relevant definitions:
      virtual void set_register(size_t reg, boost::uint32_t value) = 0;
      virtual std::string get_block_id() = 0;
    };

  } // namespace uhd
} // namespace gr

#endif /* INCLUDED_UHD_RFNOC_STREAMER_H */
