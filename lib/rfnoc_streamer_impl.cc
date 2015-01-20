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
#include <gnuradio/block_detail.h>
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
    ) : gr::block("uhd_rfnoc_streamer",
          // Default IO signatures: These will be overridden in the constructor
          gr::io_signature::make(0, 0, 1),
          gr::io_signature::make(0, 0, 1)
       )
    {
      /***** Set up block control ********************************/
      d_rfnoccer = new rfnoc::rfnoc_common(
          dev, block_id,
          stream_args, stream_args,
          boost::bind(&gr::block::consume,      this, _1, _2),
          boost::bind(&gr::block::consume_each, this, _1),
          boost::bind(&gr::block::produce,      this, _1, _2)
      );

      /***** Finalize I/O signatures and configure GR block ******/
      set_input_signature(d_rfnoccer->get_input_signature());
      set_output_signature(d_rfnoccer->get_output_signature());
      set_tag_propagation_policy(TPP_DONT);
    }


    rfnoc_streamer_impl::~rfnoc_streamer_impl()
    {
      // TODO make rfnoccer a sptr so we don't need this delete
      delete d_rfnoccer;
    }

    int
    rfnoc_streamer_impl::general_work (
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

    bool rfnoc_streamer_impl::check_topology(int ninputs, int noutputs)
    {
      return d_rfnoccer->check_topology(ninputs, noutputs);
    }

    bool rfnoc_streamer_impl::start()
    {
      return d_rfnoccer->start(detail()->ninputs(), detail()->noutputs());
    }

    bool rfnoc_streamer_impl::stop()
    {
      return d_rfnoccer->stop();
    }

    void
    rfnoc_streamer_impl::set_register(size_t reg, boost::uint32_t value)
    {
      d_rfnoccer->get_block_ctrl()->sr_write(reg, value);
    }

    void
    rfnoc_streamer_impl::set_option(const std::string &key, const std::string &val)
    {
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Setting rfnoc option on %s %s==%s") % d_rfnoccer->get_block_ctrl()->get_block_id() % key % val));
      // *throws up* this should not be a hardcoded list of options TODO
      if (d_rfnoccer->get_block_ctrl()->get_block_id().get_block_name() == "Radio") {
        size_t chan = d_rfnoccer->get_block_ctrl()->get_block_id().get_block_count();
        if (key == "rx_freq") {
          double freq = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_rx_freq(freq, chan);
        }
        else if (key == "tx_freq") {
          double freq = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_tx_freq(freq, chan);
        }
        else if (key == "tx_ant") {
          d_rfnoccer->get_device()->set_tx_antenna(val, chan);
        }
        else if (key == "rx_ant") {
          d_rfnoccer->get_device()->set_rx_antenna(val, chan);
        }
        else if (key == "rx_gain") {
          double gain = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_rx_gain(gain, chan);
        }
        else if (key == "tx_gain") {
          double gain = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_tx_gain(gain, chan);
        }
        else if (key == "rx_rate") {
          double rate = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_rx_rate(rate, chan);
        }
        else if (key == "tx_rate") {
          double rate = boost::lexical_cast<double>(val);
          d_rfnoccer->get_device()->set_tx_rate(rate, chan);
        }
      }
    }

    void rfnoc_streamer_impl::set_taps(const std::vector<int> &taps)
    {
      if (d_rfnoccer->get_block_ctrl()->get_block_id().get_block_name() != "FIR") {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      ::uhd::rfnoc::fir_block_ctrl::sptr fir_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::fir_block_ctrl >(d_rfnoccer->get_block_ctrl());
      if (not fir_ctrl) {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      fir_ctrl->set_taps(taps);
    }

    void rfnoc_streamer_impl::set_window(const std::vector<int> &coeffs)
    {
      if (d_rfnoccer->get_block_ctrl()->get_block_id().get_block_name() != "Window") {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      ::uhd::rfnoc::window_block_ctrl::sptr window_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::window_block_ctrl >(d_rfnoccer->get_block_ctrl());
      if (not window_ctrl) {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      window_ctrl->set_window(coeffs);
    }

    std::string
    rfnoc_streamer_impl::get_block_id()
    {
      return d_rfnoccer->get_block_ctrl()->get_block_id().get();
    }

  } /* namespace uhd */
} /* namespace gr */

