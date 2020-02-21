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

#include "rfnoc_pdu_rx_impl.h"
#include <gnuradio/gr_complex.h>
#include <gnuradio/io_signature.h>
#include <pmt/pmt.h>

namespace gr {
namespace ettus {

rfnoc_pdu_rx::sptr rfnoc_pdu_rx::make(const device3::sptr& dev,
                                      const ::uhd::stream_args_t& tx_stream_args,
                                      const ::uhd::stream_args_t& rx_stream_args,
                                      const std::string& block_name,
                                      const int block_select,
                                      const int device_select,
                                      const int mtu)
{
    return gnuradio::get_initial_sptr(new rfnoc_pdu_rx_impl(dev,
                                                            tx_stream_args,
                                                            rx_stream_args,
                                                            block_name,
                                                            block_select,
                                                            device_select,
                                                            mtu));
}

static ::uhd::stream_args_t
_make_stream_args(const char* host_type, const char* otw_type, size_t spp, size_t len)
{
    ::uhd::stream_args_t stream_args(host_type, otw_type);
    stream_args.args["spp"] = str(boost::format("%s") % spp);
    if (len > 1) {
        stream_args.channels.clear();
        for (size_t i = 0; i < len; i++)
            stream_args.channels.push_back(i);
    }
    return stream_args;
}


rfnoc_pdu_rx_impl::rfnoc_pdu_rx_impl(const device3::sptr& dev,
                                     const ::uhd::stream_args_t& tx_stream_args,
                                     const ::uhd::stream_args_t& rx_stream_args,
                                     const std::string& block_name,
                                     const int block_select,
                                     const int device_select,
                                     const int mtu)
    : rfnoc_block("rfnoc_pdu_rx"),
      rfnoc_block_impl(
          dev,
          rfnoc_block_impl::make_block_id(block_name, block_select, device_select),
          tx_stream_args,
          rx_stream_args),
      d_started(false),
      d_finished(false)
{
    d_rxbuf.resize(mtu, 0);
    message_port_register_out(pmt::mp("data"));
    start_rxthread(this, pmt::mp("data"));
    set_output_signature(io_signature::make(0, 0, 0));
}

rfnoc_pdu_rx_impl::~rfnoc_pdu_rx_impl()
{
    /* nop */
    stop_rxthread();
}


bool rfnoc_pdu_rx_impl::start()
{
    boost::recursive_mutex::scoped_lock lock(d_mutex);
    size_t ninputs = 0;
    size_t noutputs = 1;
    GR_LOG_DEBUG(
        d_debug_logger,
        str(boost::format("start(): ninputs == %d noutputs == %d") % ninputs % noutputs));

    // If the topology changed, we need to clear the old streamers
    if (_rx.streamers.size() != noutputs) {
        _rx.streamers.clear();
    }
    if (_tx.streamers.size() != ninputs) {
        _tx.streamers.clear();
    }

    // Setup RX streamer
    if (noutputs && _rx.streamers.empty()) {
        // Get a block control for the rx side:
        ::uhd::rfnoc::source_block_ctrl_base::sptr rx_blk_ctrl =
            boost::dynamic_pointer_cast<::uhd::rfnoc::source_block_ctrl_base>(_blk_ctrl);
        if (!rx_blk_ctrl) {
            GR_LOG_FATAL(d_logger,
                         str(boost::format("Not a source_block_ctrl_base: %s") %
                             _blk_ctrl->unique_id()));
            return false;
        }
        if (_rx.align) { // Aligned streamers:
            GR_LOG_DEBUG(
                d_debug_logger,
                str(boost::format("Creating one aligned rx streamer for %d outputs.") %
                    noutputs));
            GR_LOG_DEBUG(
                d_debug_logger,
                str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") %
                    _rx.stream_args.cpu_format % _rx.stream_args.otw_format %
                    _rx.stream_args.args.to_string() % _rx.stream_args.channels.size()));
            assert(noutputs == _rx.stream_args.channels.size());
            ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_rx.stream_args);
            if (rx_stream) {
                _rx.streamers.push_back(rx_stream);
            } else {
                GR_LOG_FATAL(d_logger,
                             str(boost::format("Can't create rx streamer(s) to: %s") %
                                 _blk_ctrl->get_block_id().get()));
                return false;
            }
        } else { // Unaligned streamers:
            for (size_t i = 0; i < size_t(noutputs); i++) {
                _rx.stream_args.channels = std::vector<size_t>(1, i);
                std::string portkey = str(boost::format("block_port%d") % i);
                if (!_rx.stream_args.args.has_key(portkey)) {
                    _rx.stream_args.args["block_port"] = str(boost::format("%d") % i);
                }
                GR_LOG_DEBUG(d_debug_logger,
                             str(boost::format("creating rx streamer with: %s") %
                                 _rx.stream_args.args.to_string()));
                ::uhd::rx_streamer::sptr rx_stream = _dev->get_rx_stream(_rx.stream_args);
                if (rx_stream) {
                    _rx.streamers.push_back(rx_stream);
                }
            }
            if (_rx.streamers.size() != size_t(noutputs)) {
                GR_LOG_FATAL(d_logger,
                             str(boost::format("Can't create rx streamer(s) to: %s") %
                                 _blk_ctrl->get_block_id().get()));
                return false;
            }
        }
    }

    // Start the streamers
    if (!_rx.streamers.empty()) {
        ::uhd::stream_cmd_t stream_cmd(::uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
        stream_cmd.stream_now = true;
        for (size_t i = 0; i < _rx.streamers.size(); i++) {
            _rx.streamers[i]->issue_stream_cmd(stream_cmd);
        }
    }

    return true;
}

void rfnoc_pdu_rx_impl::run()
{
    while (!d_finished) {
        if (_rx.streamers.size() != 1) {
            std::cout << "Waiting for stream to become available." << std::endl;
            sleep(1);
            continue;
        }
        const int result =
            _rx.streamers[0]->recv(&d_rxbuf[0], d_rxbuf.size(), _rx.metadata, 0.1, true);

        if (result < 0)
            throw std::runtime_error("rfnoc_pdu_rx_impl, bad read!");

        if (result > 0) {
            pmt::pmt_t vector = pmt::init_u8vector(result, &d_rxbuf[0]);
            pmt::pmt_t pdu = pmt::cons(pmt::PMT_NIL, vector);
            d_blk->message_port_pub(d_port, pdu);
        }
    }
}

void rfnoc_pdu_rx_impl::stop_rxthread()
{
    d_finished = true;

    if (d_started) {
        d_thread.interrupt();
        d_thread.join();
    }
}


void rfnoc_pdu_rx_impl::start_rxthread(basic_block* blk, pmt::pmt_t port)
{
    d_blk = blk;
    d_port = port;
    d_thread = gr::thread::thread(boost::bind(&rfnoc_pdu_rx_impl::run, this));
    d_started = true;
}

} /* namespace ettus */
} /* namespace gr */
