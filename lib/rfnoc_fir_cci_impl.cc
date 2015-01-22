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
#include "rfnoc_fir_cci_impl.h"
#include <uhd/usrp/rfnoc/fir_block_ctrl.hpp>

namespace gr {
  namespace ettus {

    rfnoc_fir_cci::sptr
    rfnoc_fir_cci::make(
        const std::vector<int> &taps,
        const device3::sptr &dev,
        const int block_select,
        const int device_select
    ) {
      return gnuradio::get_initial_sptr(
          new rfnoc_fir_cci_impl(
            taps, dev, block_select, device_select
          )
      );
    }


    rfnoc_fir_cci_impl::rfnoc_fir_cci_impl(
        const std::vector<int> &taps,
        const device3::sptr &dev,
        const int block_select,
        const int device_select
    ) : GR_RFNOC_BLOCK_SUPER_CTOR("rfnoc_fir_cci")
    {
      ::uhd::stream_args_t stream_args("fc32", "sc16");
      GR_RFNOC_BLOCK_INIT(
          dev, rfnoc::rfnoc_common::make_block_id("FIR", block_select, device_select),
          stream_args, stream_args
      );
    }

    rfnoc_fir_cci_impl::~rfnoc_fir_cci_impl()
    {
      /* nop */
    }

    void rfnoc_fir_cci_impl::set_taps(const std::vector<int> &taps)
    {
      get_block_ctrl_throw< ::uhd::rfnoc::fir_block_ctrl >()->set_taps(taps);
    }

  } /* namespace ettus */
} /* namespace gr */

