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

    std::map<std::string, bool> rfnoc_streamer_impl::_active_streamers;
    ::uhd::reusable_barrier rfnoc_streamer_impl::_tx_barrier;
    ::uhd::reusable_barrier rfnoc_streamer_impl::_rx_barrier;
    boost::recursive_mutex rfnoc_streamer_impl::s_setup_mutex;

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
          stream_args, stream_args
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
      boost::recursive_mutex::scoped_lock lock(d_mutex);

      // These call consume()
      if (!input_items.empty()) {
        if (d_rfnoccer->_tx.align) {
          work_tx_a(ninput_items, input_items);
        } else {
          work_tx_u(ninput_items, input_items);
        }
      }

      // These call produce()
      if (!output_items.empty()) {
        if (d_rfnoccer->_rx.align) {
          return work_rx_a(noutput_items, output_items);
        } else {
          work_rx_u(noutput_items, output_items);
        }
      }

      return WORK_CALLED_PRODUCE;
    }

    void
    rfnoc_streamer_impl::work_tx_a(
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items
    ) {
      // TODO Figure out why this doesn't work. It looks like the fragmentation logic
      //  in the tx_streamer is screwing up the packets, they're definitely wrong
      //  on the wire (checked with wireshark).
      //size_t num_vectors_to_send = ninput_items[0];
      size_t num_vectors_to_send = d_rfnoccer->_tx.streamers[0]->get_max_num_samps() / d_rfnoccer->_rx.vlen;
      const size_t num_sent = d_rfnoccer->_tx.streamers[0]->send(
          input_items,
          num_vectors_to_send * d_rfnoccer->_tx.vlen,
          d_rfnoccer->_tx.metadata,
          1.0
      );

      consume_each(num_sent / d_rfnoccer->_tx.vlen);
    }

    void
    rfnoc_streamer_impl::work_tx_u(
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items
    ) {
      assert(input_items.size() == d_rfnoccer->_tx.streamers.size());

      // In every loop iteration, this will point to the relevant buffer
      gr_vector_const_void_star buff_ptr(1);

      for (size_t i = 0; i < d_rfnoccer->_tx.streamers.size(); i++) {
        buff_ptr[0] = input_items[i];
        //size_t num_vectors_to_send = std::min(d_rfnoccer->_tx.streamers[i]->get_max_num_samps() / d_rfnoccer->_rx.vlen, size_t(ninput_items[i]));
        size_t num_vectors_to_send = ninput_items[i];
        const size_t num_sent = d_rfnoccer->_tx.streamers[i]->send(
            buff_ptr,
            num_vectors_to_send * d_rfnoccer->_tx.vlen,
            d_rfnoccer->_tx.metadata,
            1.0
        );
        consume(i, num_sent / d_rfnoccer->_tx.vlen);
      }
    }

    int
    rfnoc_streamer_impl::work_rx_a(
        int noutput_items,
        gr_vector_void_star &output_items
    ) {
      size_t num_vectors_to_recv = noutput_items;
      size_t num_samps = d_rfnoccer->_rx.streamers[0]->recv(
          output_items,
          num_vectors_to_recv * d_rfnoccer->_rx.vlen,
          d_rfnoccer->_rx.metadata, 0.1
      );

      switch(d_rfnoccer->_rx.metadata.error_code) {
        case ::uhd::rx_metadata_t::ERROR_CODE_NONE:
          break;

        case ::uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
          //its ok to timeout, perhaps the user is doing finite streaming
          std::cout << "timeout on chan 0" << std::endl;
          break;

        case ::uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
          // Not much we can do about overruns here
          std::cout << "overrun on chan 0" << std::endl;
          break;

        default:
          std::cout << boost::format("RFNoC Streamer block received error %s (Code: 0x%x)")
            % d_rfnoccer->_rx.metadata.strerror() % d_rfnoccer->_rx.metadata.error_code << std::endl;
      }

      // There's no 'produce_each()', unfortunately
      return num_samps / d_rfnoccer->_rx.vlen;
    }

    void
    rfnoc_streamer_impl::work_rx_u(
        int noutput_items,
        gr_vector_void_star &output_items
    ) {
      assert(d_rfnoccer->_rx.streamers.size() == output_items.size());

      // In every loop iteration, this will point to the relevant buffer
      gr_vector_void_star buff_ptr(1);

      for (size_t i = 0; i < d_rfnoccer->_rx.streamers.size(); i++) {
        buff_ptr[0] = output_items[i];
        //size_t num_vectors_to_recv = std::min(d_rfnoccer->_rx.streamers[i]->get_max_num_samps() / d_rfnoccer->_rx.vlen, size_t(noutput_items));
        size_t num_vectors_to_recv = noutput_items;
        size_t num_samps = d_rfnoccer->_rx.streamers[i]->recv(
            buff_ptr,
            num_vectors_to_recv * d_rfnoccer->_rx.vlen,
            d_rfnoccer->_rx.metadata, 0.1
        );
        produce(i, num_samps / d_rfnoccer->_rx.vlen);

        switch(d_rfnoccer->_rx.metadata.error_code) {
          case ::uhd::rx_metadata_t::ERROR_CODE_NONE:
            break;

          case ::uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
            //its ok to timeout, perhaps the user is doing finite streaming
            std::cout << "timeout on chan " << i << std::endl;
            break;

          case ::uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
            // Not much we can do about overruns here
            std::cout << "overrun on chan " << i << std::endl;
            break;

          default:
            std::cout << boost::format("RFNoC Streamer block received error %s (Code: 0x%x)")
              % d_rfnoccer->_rx.metadata.strerror() % d_rfnoccer->_rx.metadata.error_code << std::endl;
        }
      }
    }

    bool rfnoc_streamer_impl::check_topology(int ninputs, int noutputs)
    {
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("check_topology()")));
      {
        boost::recursive_mutex::scoped_lock lock(s_setup_mutex);
        std::string blk_id = d_rfnoccer->_blk_ctrl->get_block_id().get();
        if (ninputs || noutputs) {
          _active_streamers[blk_id] = true;
        } else if (_active_streamers.count(blk_id)) {
          _active_streamers.erase(blk_id);
        }
        GR_LOG_DEBUG(d_debug_logger, str(boost::format("RFNoC blocks with streaming ports: %d") % _active_streamers.size()));
        _tx_barrier.resize(_active_streamers.size());
        _rx_barrier.resize(_active_streamers.size());
      }

      // TODO: Check if ninputs and noutputs match the blocks io signatures.
      return true;
    }

    bool rfnoc_streamer_impl::start()
    {
      boost::recursive_mutex::scoped_lock lock(d_mutex);
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("start()")));

      size_t ninputs = detail()->ninputs();
      size_t noutputs = detail()->noutputs();
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("ninputs == %d noutputs == %d") % ninputs % noutputs));

      // If the topology changed, we need to clear the old streamers
      if (d_rfnoccer->_rx.streamers.size() != noutputs) {
        d_rfnoccer->_rx.streamers.clear();
      }
      if (d_rfnoccer->_tx.streamers.size() != ninputs) {
        d_rfnoccer->_tx.streamers.clear();
      }

      //////////////////// TX ///////////////////////////////////////////////////////////////
      // Setup TX streamer.
      if (ninputs && d_rfnoccer->_tx.streamers.empty()) {
        // Get a block control for the tx side:
        ::uhd::rfnoc::sink_block_ctrl_base::sptr tx_blk_ctrl =
            boost::dynamic_pointer_cast< ::uhd::rfnoc::sink_block_ctrl_base >(d_rfnoccer->_blk_ctrl);
        if (!tx_blk_ctrl) {
          GR_LOG_FATAL(d_logger, str(boost::format("Not a sink_block_ctrl_base: %s") % d_rfnoccer->_blk_ctrl->unique_id()));
          return false;
        }
        if (d_rfnoccer->_tx.align) { // Aligned streamers:
          GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned tx streamer for %d inputs.") % ninputs));
          GR_LOG_DEBUG(d_debug_logger,
              str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % d_rfnoccer->_tx.stream_args.cpu_format % d_rfnoccer->_tx.stream_args.otw_format % d_rfnoccer->_tx.stream_args.args.to_string() % d_rfnoccer->_tx.stream_args.channels.size()));
          assert(ninputs == d_rfnoccer->_tx.stream_args.channels.size());
          ::uhd::tx_streamer::sptr tx_stream = d_rfnoccer->_dev->get_tx_stream(d_rfnoccer->_tx.stream_args);
          if (tx_stream) {
            d_rfnoccer->_tx.streamers.push_back(tx_stream);
          } else {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % d_rfnoccer->_blk_ctrl->get_block_id().get()));
            return false;
          }
        } else { // Unaligned streamers:
          for (size_t i = 0; i < size_t(ninputs); i++) {
            d_rfnoccer->_tx.stream_args.channels = std::vector<size_t>(1, i);
            GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating tx streamer with: %s") % d_rfnoccer->_tx.stream_args.args.to_string()));
            ::uhd::tx_streamer::sptr tx_stream = d_rfnoccer->_dev->get_tx_stream(d_rfnoccer->_tx.stream_args);
            if (tx_stream) {
              d_rfnoccer->_tx.streamers.push_back(tx_stream);
            }
          }
          if (d_rfnoccer->_tx.streamers.size() != size_t(ninputs)) {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % d_rfnoccer->_blk_ctrl->get_block_id().get()));
            return false;
          }
        }
      }

      d_rfnoccer->_tx.metadata.start_of_burst = false;
      d_rfnoccer->_tx.metadata.end_of_burst = false;
      d_rfnoccer->_tx.metadata.has_time_spec = false;

      // Wait for all RFNoC streamers to have set up their tx streamers
      _tx_barrier.wait();

      //////////////////// RX ///////////////////////////////////////////////////////////////
      // Setup RX streamer
      if (noutputs && d_rfnoccer->_rx.streamers.empty()) {
        // Get a block control for the rx side:
        ::uhd::rfnoc::source_block_ctrl_base::sptr rx_blk_ctrl =
            boost::dynamic_pointer_cast< ::uhd::rfnoc::source_block_ctrl_base >(d_rfnoccer->_blk_ctrl);
        if (!rx_blk_ctrl) {
          GR_LOG_FATAL(d_logger, str(boost::format("Not a source_block_ctrl_base: %s") % d_rfnoccer->_blk_ctrl->unique_id()));
          return false;
        }
        if (d_rfnoccer->_rx.align) { // Aligned streamers:
          GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned rx streamer for %d outputs.") % noutputs));
          GR_LOG_DEBUG(d_debug_logger,
              str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % d_rfnoccer->_rx.stream_args.cpu_format % d_rfnoccer->_rx.stream_args.otw_format % d_rfnoccer->_rx.stream_args.args.to_string() % d_rfnoccer->_rx.stream_args.channels.size()));
          assert(noutputs == d_rfnoccer->_rx.stream_args.channels.size());
          ::uhd::rx_streamer::sptr rx_stream = d_rfnoccer->_dev->get_rx_stream(d_rfnoccer->_rx.stream_args);
          if (rx_stream) {
            d_rfnoccer->_rx.streamers.push_back(rx_stream);
          } else {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % d_rfnoccer->_blk_ctrl->get_block_id().get()));
            return false;
          }
        } else { // Unaligned streamers:
          for (size_t i = 0; i < size_t(noutputs); i++) {
            d_rfnoccer->_rx.stream_args.channels = std::vector<size_t>(1, i);
            GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating rx streamer with: %s") % d_rfnoccer->_rx.stream_args.args.to_string()));
            ::uhd::rx_streamer::sptr rx_stream = d_rfnoccer->_dev->get_rx_stream(d_rfnoccer->_rx.stream_args);
            if (rx_stream) {
              d_rfnoccer->_rx.streamers.push_back(rx_stream);
            }
          }
          if (d_rfnoccer->_rx.streamers.size() != size_t(noutputs)) {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % d_rfnoccer->_blk_ctrl->get_block_id().get()));
            return false;
          }
        }
      }

      // Wait for all RFNoC streamers to have set up their rx streamers
      _rx_barrier.wait();

      // Start the streamers
      if (!d_rfnoccer->_rx.streamers.empty()) {
        ::uhd::stream_cmd_t stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
        stream_cmd.stream_now = true;
        for (size_t i = 0; i < d_rfnoccer->_rx.streamers.size(); i++) {
          d_rfnoccer->_rx.streamers[i]->issue_stream_cmd(stream_cmd);
        }
      }

      return true;
    }

    bool rfnoc_streamer_impl::stop()
    {
      boost::recursive_mutex::scoped_lock lock(d_mutex);

      // TX: Send EOB
      d_rfnoccer->_tx.metadata.start_of_burst = false;
      d_rfnoccer->_tx.metadata.end_of_burst = true;
      d_rfnoccer->_tx.metadata.has_time_spec = false;
      if (d_rfnoccer->_tx.align) {
        d_rfnoccer->_tx.streamers[0]->send(gr_vector_const_void_star(d_rfnoccer->_tx.streamers[0]->get_num_channels()), 0, d_rfnoccer->_tx.metadata, 1.0);
      } else {
        for (size_t i = 0; i < d_rfnoccer->_tx.streamers.size(); i++) {
          // Always 1 channel per streamer in this case
          d_rfnoccer->_tx.streamers[i]->send(gr_vector_const_void_star(1), 0, d_rfnoccer->_tx.metadata, 1.0);
        }
      }

      _tx_barrier.wait();

      // RX: Stop streaming and empty the buffers
      for (size_t i = 0; i < d_rfnoccer->_rx.streamers.size(); i++) {
        d_rfnoccer->_rx.streamers[i]->issue_stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
        d_rfnoccer->flush(i);
      }

      return true;
    }

    void
    rfnoc_streamer_impl::set_register(size_t reg, boost::uint32_t value)
    {
      d_rfnoccer->_blk_ctrl->sr_write(reg, value);
    }

    void
    rfnoc_streamer_impl::set_option(const std::string &key, const std::string &val)
    {
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Setting rfnoc option on %s %s==%s") % d_rfnoccer->_blk_ctrl->get_block_id() % key % val));
      // *throws up* this should not be a hardcoded list of options TODO
      if (d_rfnoccer->_blk_ctrl->get_block_id().get_block_name() == "Radio") {
        size_t chan = d_rfnoccer->_blk_ctrl->get_block_id().get_block_count();
        if (key == "rx_freq") {
          double freq = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_rx_freq(freq, chan);
        }
        else if (key == "tx_freq") {
          double freq = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_tx_freq(freq, chan);
        }
        else if (key == "tx_ant") {
          d_rfnoccer->_dev->set_tx_antenna(val, chan);
        }
        else if (key == "rx_ant") {
          d_rfnoccer->_dev->set_rx_antenna(val, chan);
        }
        else if (key == "rx_gain") {
          double gain = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_rx_gain(gain, chan);
        }
        else if (key == "tx_gain") {
          double gain = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_tx_gain(gain, chan);
        }
        else if (key == "rx_rate") {
          double rate = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_rx_rate(rate, chan);
        }
        else if (key == "tx_rate") {
          double rate = boost::lexical_cast<double>(val);
          d_rfnoccer->_dev->set_tx_rate(rate, chan);
        }
      }
    }

    void rfnoc_streamer_impl::set_taps(const std::vector<int> &taps)
    {
      if (d_rfnoccer->_blk_ctrl->get_block_id().get_block_name() != "FIR") {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      ::uhd::rfnoc::fir_block_ctrl::sptr fir_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::fir_block_ctrl >(d_rfnoccer->_blk_ctrl);
      if (not fir_ctrl) {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      fir_ctrl->set_taps(taps);
    }

    void rfnoc_streamer_impl::set_window(const std::vector<int> &coeffs)
    {
      if (d_rfnoccer->_blk_ctrl->get_block_id().get_block_name() != "Window") {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      ::uhd::rfnoc::window_block_ctrl::sptr window_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::window_block_ctrl >(d_rfnoccer->_blk_ctrl);
      if (not window_ctrl) {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      window_ctrl->set_window(coeffs);
    }

    std::string
    rfnoc_streamer_impl::get_block_id()
    {
      return d_rfnoccer->_blk_ctrl->get_block_id().get();
    }

  } /* namespace uhd */
} /* namespace gr */

