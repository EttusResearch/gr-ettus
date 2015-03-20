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
#include "rfnoc_vector_iir_cc_impl.h"
#include <uhd/usrp/rfnoc/vector_iir_block_ctrl.hpp>

namespace gr {
  namespace ettus {

    rfnoc_vector_iir_cc::sptr
    rfnoc_vector_iir_cc::make(int vlen, double alpha, double beta, const device3::sptr &dev, const int block_select, const int device_select)
    {
      return gnuradio::get_initial_sptr
        (new rfnoc_vector_iir_cc_impl(vlen, alpha, beta, dev, block_select, device_select));
    }

    static ::uhd::stream_args_t _make_vectoriir_stream_args(size_t vlen, double alpha, double beta)
    {
      ::uhd::stream_args_t stream_args("fc32", "sc16");
      stream_args.args["vector_len"] = str(boost::format("%s") % vlen);
      stream_args.args["alpha"]      = str(boost::format("%s") % alpha);
      stream_args.args["beta"]       = str(boost::format("%s") % beta);
      return stream_args;
    }

    rfnoc_vector_iir_cc_impl::rfnoc_vector_iir_cc_impl(int vlen, double alpha, double beta, const device3::sptr &dev, const int block_select, const int device_select)
      : rfnoc_block("rfnoc_vector_iir_cc"),
        rfnoc_block_impl(
            dev,
            rfnoc_block_impl::make_block_id("VectorIIR", block_select, device_select),
            _make_vectoriir_stream_args(vlen, alpha, beta),
            _make_vectoriir_stream_args(vlen, alpha, beta)
        )
    {
    }

    rfnoc_vector_iir_cc_impl::~rfnoc_vector_iir_cc_impl()
    {
    }

    void rfnoc_vector_iir_cc_impl::set_vector_iir_alpha(const double alpha)
    {
      get_block_ctrl_throw< ::uhd::rfnoc::vector_iir_block_ctrl >()->set_alpha(alpha);
    }

    void rfnoc_vector_iir_cc_impl::set_vector_iir_beta(const double beta)
    {
      get_block_ctrl_throw< ::uhd::rfnoc::vector_iir_block_ctrl >()->set_beta(beta);
    }


  } /* namespace ettus */
} /* namespace gr */

