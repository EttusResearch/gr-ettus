/* -*- c++ -*- */
/* 
 * Copyright 2014 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rfnoc_streamer_impl.h"
#include <gnuradio/io_signature.h>
#include <uhd/usrp/rfnoc/source_block_ctrl_base.hpp>
#include <uhd/usrp/rfnoc/sink_block_ctrl_base.hpp>
#include <uhd/usrp/rfnoc/fir_block_ctrl.hpp>
#include <uhd/usrp/rfnoc/window_block_ctrl.hpp>
#include <uhd/convert.hpp>
#include <iostream>
#include <cassert>

namespace gr {
  //namespace uhd {
  namespace ettus {

    rfnoc_streamer::sptr
    rfnoc_streamer::make(
        const device3::sptr &dev,
        const std::string &block_id,
        const ::uhd::stream_args_t &stream_args,
        const int ninputs,
        const int noutputs,
        bool align_inputs,
        bool align_outputs
    ) {
      return gnuradio::get_initial_sptr(
          new rfnoc_streamer_impl(
            dev, block_id, stream_args, ninputs, noutputs, align_inputs, align_outputs
          )
      );
    }


    rfnoc_streamer_impl::rfnoc_streamer_impl(
        const device3::sptr &dev,
        const std::string &block_id,
        const ::uhd::stream_args_t &stream_args,
        const int ninputs,
        const int noutputs,
        bool align_inputs,
        bool align_outputs
    ) : GR_RFNOC_BLOCK_SUPER_CTOR("uhd_rfnoc_streamer")
    {
      GR_RFNOC_BLOCK_INIT(dev, block_id, stream_args, stream_args);
    }


    rfnoc_streamer_impl::~rfnoc_streamer_impl()
    {
      /* nop */
    }

  } /* namespace uhd */
} /* namespace gr */

