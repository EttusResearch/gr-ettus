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
#include <uhd/usrp/rfnoc/vector_iir_block_ctrl.hpp>
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
              // These IO signatures will be overwritten in the constructor
              gr::io_signature::make(0, ninputs, sizeof(gr_complex)),
              gr::io_signature::make(0, noutputs, sizeof(gr_complex))),
      _stream_args(stream_args),
      _ninputs(ninputs),
      _noutputs(noutputs),
      _align_inputs(align_inputs),
      _align_outputs(align_outputs)
    {
      _dev = dev->get_device();
      _blk_ctrl = dev->get_device()->get_device3()->find_block_ctrl(block_id);
      if (not _blk_ctrl) {
        throw std::runtime_error(str(boost::format("Cannot find a block for ID: %s") % block_id));
      }
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Setting args on %s (%s)") % _blk_ctrl->get_block_id() % _stream_args.args.to_string()));
      int gr_vlen = _stream_args.args.cast<int>("gr_vlen", 1);
      if (_stream_args.args.has_key("gr_vlen")) {
        _stream_args.args.pop("gr_vlen");
      }

      _blk_ctrl->set_args(stream_args.args);
      _stream_args.args["block_id"] = _blk_ctrl->get_block_id().get();

      int itemsize = sizeof(gr_complex);
      if (_stream_args.cpu_format == "sc16") {
        itemsize = 2 * 2;
      }
      ::uhd::rfnoc::stream_sig_t in_sig = boost::dynamic_pointer_cast< ::uhd::rfnoc::sink_block_ctrl_base >(_blk_ctrl)->get_input_signature(0);
      _in_vlen = (in_sig.vlen == 0) ? 1 : in_sig.vlen;
      ::uhd::rfnoc::stream_sig_t out_sig = boost::dynamic_pointer_cast< ::uhd::rfnoc::source_block_ctrl_base >(_blk_ctrl)->get_output_signature(0);
      _out_vlen = (out_sig.vlen == 0) ? 1 : out_sig.vlen;

      if (gr_vlen != 1) {
        if (_out_vlen != 1 || _in_vlen != 1) {
          throw std::runtime_error("Can't set gr_vlen if underlying RFNoC block already has a vector length.\n");
        }
        GR_LOG_DEBUG(d_debug_logger, str(boost::format("Found setting gr_vlen == %d") % gr_vlen));
        GR_LOG_DEBUG(d_debug_logger, str(boost::format("If stuff doesn't work, remove this setting again!")));
        _in_vlen = _out_vlen = gr_vlen;
      }
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("GR itemsize == %d") % (itemsize * _in_vlen)));

      set_input_signature(gr::io_signature::make(0, ninputs, itemsize * _in_vlen));
      set_output_signature(gr::io_signature::make(0, noutputs, itemsize * _out_vlen));
      set_tag_propagation_policy(TPP_DONT);
    }


    rfnoc_streamer_impl::~rfnoc_streamer_impl()
    {
      /* nop */
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
        if (_align_inputs) {
          work_tx_a(ninput_items, input_items);
        } else {
          work_tx_u(ninput_items, input_items);
        }
      }

      // These call produce()
      if (!output_items.empty()) {
        if (_align_outputs) {
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
      size_t num_vectors_to_send = _tx_stream[0]->get_max_num_samps() / _out_vlen;
      const size_t num_sent = _tx_stream[0]->send(
          input_items,
          num_vectors_to_send * _in_vlen,
          _tx_metadata,
          1.0
      );

      consume_each(num_sent / _in_vlen);
    }

    void
    rfnoc_streamer_impl::work_tx_u(
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items
    ) {
      assert(input_items.size() == _tx_stream.size());

      // In every loop iteration, this will point to the relevant buffer
      gr_vector_const_void_star buff_ptr(1);

      for (size_t i = 0; i < _tx_stream.size(); i++) {
        buff_ptr[0] = input_items[i];
        //size_t num_vectors_to_send = std::min(_tx_stream[i]->get_max_num_samps() / _out_vlen, size_t(ninput_items[i]));
        size_t num_vectors_to_send = ninput_items[i];
        const size_t num_sent = _tx_stream[i]->send(
            buff_ptr,
            num_vectors_to_send * _in_vlen,
            _tx_metadata,
            1.0
        );
        consume(i, num_sent / _in_vlen);
      }
    }

    int
    rfnoc_streamer_impl::work_rx_a(
        int noutput_items,
        gr_vector_void_star &output_items
    ) {
      size_t num_vectors_to_recv = noutput_items;
      size_t num_samps = _rx_stream[0]->recv(
          output_items,
          num_vectors_to_recv * _out_vlen,
          _rx_metadata, 0.1
      );

      switch(_rx_metadata.error_code) {
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
            % _rx_metadata.strerror() % _rx_metadata.error_code << std::endl;
      }

      // There's no 'produce_each()', unfortunately
      return num_samps / _out_vlen;
    }

    void
    rfnoc_streamer_impl::work_rx_u(
        int noutput_items,
        gr_vector_void_star &output_items
    ) {
      assert(_rx_stream.size() == output_items.size());

      // In every loop iteration, this will point to the relevant buffer
      gr_vector_void_star buff_ptr(1);

      for (size_t i = 0; i < _rx_stream.size(); i++) {
        buff_ptr[0] = output_items[i];
        //size_t num_vectors_to_recv = std::min(_rx_stream[i]->get_max_num_samps() / _out_vlen, size_t(noutput_items));
        size_t num_vectors_to_recv = noutput_items;
        size_t num_samps = _rx_stream[i]->recv(
            buff_ptr,
            num_vectors_to_recv * _out_vlen,
            _rx_metadata, 0.1
        );
        produce(i, num_samps / _out_vlen);

        switch(_rx_metadata.error_code) {
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
              % _rx_metadata.strerror() % _rx_metadata.error_code << std::endl;
        }
      }
    }

    bool rfnoc_streamer_impl::check_topology(int ninputs, int noutputs)
    {
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("check_topology()")));
      {
        boost::recursive_mutex::scoped_lock lock(s_setup_mutex);
        std::string blk_id = _blk_ctrl->get_block_id().get();
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
      if (_rx_stream.size() != noutputs) {
        _rx_stream.clear();
      }
      if (_tx_stream.size() != ninputs) {
        _tx_stream.clear();
      }

      //////////////////// TX ///////////////////////////////////////////////////////////////
      // Setup TX streamer.
      if (ninputs && _tx_stream.empty()) {
        // Get a block control for the tx side:
        ::uhd::rfnoc::sink_block_ctrl_base::sptr tx_blk_ctrl =
            boost::dynamic_pointer_cast< ::uhd::rfnoc::sink_block_ctrl_base >(_blk_ctrl);
        if (!tx_blk_ctrl) {
          GR_LOG_FATAL(d_logger, str(boost::format("Not a sink_block_ctrl_base: %s") % _blk_ctrl->unique_id()));
          return false;
        }
        if (_align_inputs) { // Aligned streamers:
          GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned tx streamer for %d inputs.") % ninputs));
          GR_LOG_DEBUG(d_debug_logger,
              str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % _stream_args.cpu_format % _stream_args.otw_format % _stream_args.args.to_string() % _stream_args.channels.size()));
          assert(ninputs == _stream_args.channels.size());
          ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_stream_args);
          if (tx_stream) {
            _tx_stream.push_back(tx_stream);
          } else {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
            return false;
          }
        } else { // Unaligned streamers:
          for (size_t i = 0; i < size_t(ninputs); i++) {
            _stream_args.channels = std::vector<size_t>(1, i);
            GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating tx streamer with: %s") % _stream_args.args.to_string()));
            ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_stream_args);
            if (tx_stream) {
              _tx_stream.push_back(tx_stream);
            }
          }
          if (_tx_stream.size() != size_t(ninputs)) {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create tx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
            return false;
          }
        }
      }

      _tx_metadata.start_of_burst = false;
      _tx_metadata.end_of_burst = false;
      _tx_metadata.has_time_spec = false;

      // Wait for all RFNoC streamers to have set up their tx streamers
      _tx_barrier.wait();

      //////////////////// RX ///////////////////////////////////////////////////////////////
      // Setup RX streamer
      if (noutputs && _rx_stream.empty()) {
        // Get a block control for the rx side:
        ::uhd::rfnoc::source_block_ctrl_base::sptr rx_blk_ctrl =
            boost::dynamic_pointer_cast< ::uhd::rfnoc::source_block_ctrl_base >(_blk_ctrl);
        if (!rx_blk_ctrl) {
          GR_LOG_FATAL(d_logger, str(boost::format("Not a source_block_ctrl_base: %s") % _blk_ctrl->unique_id()));
          return false;
        }
        if (_align_outputs) { // Aligned streamers:
          GR_LOG_DEBUG(d_debug_logger, str(boost::format("Creating one aligned rx streamer for %d outputs.") % noutputs));
          GR_LOG_DEBUG(d_debug_logger,
              str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") % _stream_args.cpu_format % _stream_args.otw_format % _stream_args.args.to_string() % _stream_args.channels.size()));
          assert(noutputs == _stream_args.channels.size());
          ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_stream_args);
          if (rx_stream) {
            _rx_stream.push_back(rx_stream);
          } else {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
            return false;
          }
        } else { // Unaligned streamers:
          for (size_t i = 0; i < size_t(noutputs); i++) {
            _stream_args.channels = std::vector<size_t>(1, i);
            GR_LOG_DEBUG(d_debug_logger, str(boost::format("creating rx streamer with: %s") % _stream_args.args.to_string()));
            ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_stream_args);
            if (rx_stream) {
              _rx_stream.push_back(rx_stream);
            }
          }
          if (_rx_stream.size() != size_t(noutputs)) {
            GR_LOG_FATAL(d_logger, str(boost::format("Can't create rx streamer(s) to: %s") % _blk_ctrl->get_block_id().get()));
            return false;
          }
        }
      }

      // Wait for all RFNoC streamers to have set up their rx streamers
      _rx_barrier.wait();

      // Start the streamers
      if (!_rx_stream.empty()) {
        ::uhd::stream_cmd_t stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
        stream_cmd.stream_now = true;
        for (size_t i = 0; i < _rx_stream.size(); i++) {
          _rx_stream[i]->issue_stream_cmd(stream_cmd);
        }
      }

      return true;
    }

    bool rfnoc_streamer_impl::stop()
    {
      boost::recursive_mutex::scoped_lock lock(d_mutex);

      // TX: Send EOB
      _tx_metadata.start_of_burst = false;
      _tx_metadata.end_of_burst = true;
      _tx_metadata.has_time_spec = false;
      if (_align_inputs) {
        _tx_stream[0]->send(gr_vector_const_void_star(_tx_stream[0]->get_num_channels()), 0, _tx_metadata, 1.0);
      } else {
        for (size_t i = 0; i < _tx_stream.size(); i++) {
          // Always 1 channel per streamer in this case
          _tx_stream[i]->send(gr_vector_const_void_star(1), 0, _tx_metadata, 1.0);
        }
      }

      _tx_barrier.wait();

      // RX: Stop streaming and empty the buffers
      for (size_t i = 0; i < _rx_stream.size(); i++) {
        _rx_stream[i]->issue_stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
        flush(i);
      }

      return true;
    }

    void rfnoc_streamer_impl::flush(size_t streamer_index)
    {
      const size_t nbytes = 4096;
      size_t _nchan = 1;
      if (_align_outputs) {
        _nchan = _rx_stream[0]->get_num_channels();
      }
      std::vector<std::vector<char> > buffs(_nchan, std::vector<char>(nbytes));
      gr_vector_void_star outputs;
      for(size_t i = 0; i < _nchan; i++) {
        outputs.push_back(&buffs[i].front());
      }
      while(true) {
        const size_t bpi = ::uhd::convert::get_bytes_per_item(_stream_args.cpu_format);
        _rx_stream[streamer_index]->recv(outputs, nbytes/bpi, _rx_metadata, 0.0);
        if(_rx_metadata.error_code != ::uhd::rx_metadata_t::ERROR_CODE_NONE) {
          break;
        }
      }
    }

    void
    rfnoc_streamer_impl::set_register(size_t reg, boost::uint32_t value)
    {
      _blk_ctrl->sr_write(reg, value);
    }

    void
    rfnoc_streamer_impl::set_option(const std::string &key, const std::string &val)
    {
      GR_LOG_DEBUG(d_debug_logger, str(boost::format("Setting rfnoc option on %s %s==%s") % _blk_ctrl->get_block_id() % key % val));
      // *throws up* this should not be a hardcoded list of options TODO
      if (_blk_ctrl->get_block_id().get_block_name() == "Radio") {
        size_t chan = _blk_ctrl->get_block_id().get_block_count();
        if (key == "rx_freq") {
          double freq = boost::lexical_cast<double>(val);
          _dev->set_rx_freq(freq, chan);
        }
        else if (key == "tx_freq") {
          double freq = boost::lexical_cast<double>(val);
          _dev->set_tx_freq(freq, chan);
        }
        else if (key == "tx_ant") {
          _dev->set_tx_antenna(val, chan);
        }
        else if (key == "rx_ant") {
          _dev->set_rx_antenna(val, chan);
        }
        else if (key == "rx_gain") {
          double gain = boost::lexical_cast<double>(val);
          _dev->set_rx_gain(gain, chan);
        }
        else if (key == "tx_gain") {
          double gain = boost::lexical_cast<double>(val);
          _dev->set_tx_gain(gain, chan);
        }
        else if (key == "rx_rate") {
          double rate = boost::lexical_cast<double>(val);
          _dev->set_rx_rate(rate, chan);
        }
        else if (key == "tx_rate") {
          double rate = boost::lexical_cast<double>(val);
          _dev->set_tx_rate(rate, chan);
        }
      }
    }

    void rfnoc_streamer_impl::set_taps(const std::vector<int> &taps)
    {
      if (_blk_ctrl->get_block_id().get_block_name() != "FIR") {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      ::uhd::rfnoc::fir_block_ctrl::sptr fir_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::fir_block_ctrl >(_blk_ctrl);
      if (not fir_ctrl) {
        GR_LOG_ALERT(d_debug_logger, str(boost::format("Calling set_taps() on a non-FIR block!")));
        return;
      }

      fir_ctrl->set_taps(taps);
    }

    void rfnoc_streamer_impl::set_window(const std::vector<int> &coeffs)
    {
      if (_blk_ctrl->get_block_id().get_block_name() != "Window") {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      ::uhd::rfnoc::window_block_ctrl::sptr window_ctrl =
              boost::dynamic_pointer_cast< ::uhd::rfnoc::window_block_ctrl >(_blk_ctrl);
      if (not window_ctrl) {
        std::cout << "[GR] Calling set_window() on a non-Window block!" << std::endl;
        return;
      }

      window_ctrl->set_window(coeffs);
    }

    void rfnoc_streamer_impl::set_vector_iir_alpha(const double alpha)
    {
        if (_blk_ctrl->get_block_id().get_block_name() != "VectorIIR") {
          std::cout << "[GR] Calling set_vector_iir_alpha() on a non-VectorIIR block!" << std::endl;
          return;
        }

        ::uhd::rfnoc::vector_iir_block_ctrl::sptr vector_iir_ctrl =
                boost::dynamic_pointer_cast< ::uhd::rfnoc::vector_iir_block_ctrl >(_blk_ctrl);
        if (not vector_iir_ctrl) {
          std::cout << "[GR] Calling set_vector_iir_alpha() on a non-VectorIIR block!" << std::endl;
          return;
        }

        vector_iir_ctrl->set_alpha(alpha);
    }

    void rfnoc_streamer_impl::set_vector_iir_beta(const double beta)
    {
        if (_blk_ctrl->get_block_id().get_block_name() != "VectorIIR") {
          std::cout << "[GR] Calling set_vector_iir_beta() on a non-VectorIIR block!" << std::endl;
          return;
        }

        ::uhd::rfnoc::vector_iir_block_ctrl::sptr vector_iir_ctrl =
                boost::dynamic_pointer_cast< ::uhd::rfnoc::vector_iir_block_ctrl >(_blk_ctrl);
        if (not vector_iir_ctrl) {
          std::cout << "[GR] Calling set_vector_iir_beta() on a non-VectorIIR block!" << std::endl;
          return;
        }

        vector_iir_ctrl->set_beta(beta);
    }

    std::string
    rfnoc_streamer_impl::get_block_id()
    {
      return _blk_ctrl->get_block_id().get();
    }

  } /* namespace uhd */
} /* namespace gr */

