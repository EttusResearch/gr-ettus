/* -*- c++ -*- */
/* Copyright 2015 Ettus Research
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * gr-ettus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with gr-ettus; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "rfnoc_generic_impl.h"
#include <boost/format.hpp>

namespace gr {
  namespace ettus {

    rfnoc_generic::sptr
      rfnoc_generic::make(
          const device3::sptr &dev,
          const ::uhd::stream_args_t &tx_stream_args,
          const ::uhd::stream_args_t &rx_stream_args,
          const std::string &block_name,
          const int block_select,
          const int device_select
    ) {
      return gnuradio::get_initial_sptr(
          new rfnoc_generic_impl(dev, tx_stream_args, rx_stream_args, block_name, block_select, device_select)
      );
    }

    rfnoc_generic_impl::rfnoc_generic_impl(
        const device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const std::string &block_name,
        const int block_select,
        const int device_select
    ) : rfnoc_block(str(boost::format("uhd_rfnoc_%s") % block_name)),
        rfnoc_block_impl(
            dev,
            rfnoc_block_impl::make_block_id(block_name, block_select, device_select),
            tx_stream_args, rx_stream_args
        )
    {
      /* nop */
    }

    rfnoc_generic_impl::~rfnoc_generic_impl()
    {
      /* nop */
    }

  } /* namespace ettus */
} /* namespace gr */

