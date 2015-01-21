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
    ) : gr::block("rfnoc_fir_cci",
              gr::io_signature::make(0, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 1, sizeof(gr_complex)))
    {
      ::uhd::stream_args_t stream_args("fc32", "sc16");
      /***** Set up block control ********************************/
      d_rfnoccer = new rfnoc::rfnoc_common(
          dev, rfnoc::rfnoc_common::make_block_id("FIR", block_select, device_select),
          stream_args, stream_args,
          boost::bind(&gr::block::consume,      this, _1, _2),
          boost::bind(&gr::block::consume_each, this, _1),
          boost::bind(&gr::block::produce,      this, _1, _2)
      );

      /***** Finalize I/O signatures and configure GR block ******/
      set_input_signature(d_rfnoccer->get_input_signature());
      set_output_signature(d_rfnoccer->get_output_signature());
      set_tag_propagation_policy(TPP_ALL_TO_ALL);
    }

    rfnoc_fir_cci_impl::~rfnoc_fir_cci_impl()
    {
      delete d_rfnoccer;
    }

    void rfnoc_fir_cci_impl::set_taps(const std::vector<int> &taps)
    {
      d_rfnoccer->get_block_ctrl_throw< ::uhd::rfnoc::fir_block_ctrl >()->set_taps(taps);
    }

    int
    rfnoc_fir_cci_impl::general_work (
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items
    ) {
      return d_rfnoccer->general_work(
          noutput_items,
          ninput_items,
          input_items,
          output_items
      );
    }

    bool rfnoc_fir_cci_impl::check_topology(int ninputs, int noutputs)
    {
      return d_rfnoccer->check_topology(ninputs, noutputs);
    }

    bool rfnoc_fir_cci_impl::start()
    {
      return d_rfnoccer->start(detail()->ninputs(), detail()->noutputs());
    }

    bool rfnoc_fir_cci_impl::stop()
    {
      return d_rfnoccer->stop();
    }

    std::string
    rfnoc_fir_cci_impl::get_block_id()
    {
      return d_rfnoccer->get_block_ctrl()->get_block_id().get();
    }

  } /* namespace ettus */
} /* namespace gr */

