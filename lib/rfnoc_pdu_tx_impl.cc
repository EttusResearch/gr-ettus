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

#include "rfnoc_pdu_tx_impl.h"
#include <gnuradio/gr_complex.h>
#include <gnuradio/io_signature.h>
#include <pmt/pmt.h>

namespace gr {
namespace ettus {

rfnoc_pdu_tx::sptr rfnoc_pdu_tx::make(const device3::sptr& dev,
                                      const ::uhd::stream_args_t& tx_stream_args,
                                      const ::uhd::stream_args_t& rx_stream_args,
                                      const std::string& block_name,
                                      const int block_select,
                                      const int device_select)
{
    return gnuradio::get_initial_sptr(new rfnoc_pdu_tx_impl(
        dev, tx_stream_args, rx_stream_args, block_name, block_select, device_select));
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


rfnoc_pdu_tx_impl::rfnoc_pdu_tx_impl(const device3::sptr& dev,
                                     const ::uhd::stream_args_t& tx_stream_args,
                                     const ::uhd::stream_args_t& rx_stream_args,
                                     const std::string& block_name,
                                     const int block_select,
                                     const int device_select)
    : rfnoc_block("rfnoc_pdu_tx"),
      rfnoc_block_impl(
          dev,
          rfnoc_block_impl::make_block_id(block_name, block_select, device_select),
          tx_stream_args,
          rx_stream_args)
{
    message_port_register_in(pmt::mp("data"));
    set_msg_handler(pmt::mp("data"),
                    boost::bind(&rfnoc_pdu_tx_impl::handle_data_message, this, _1));

    set_input_signature(io_signature::make(0, 0, 0));
    // set_output_signature(io_signature::make(0, 0, 0));
}

rfnoc_pdu_tx_impl::~rfnoc_pdu_tx_impl() { /* nop */ }


size_t itemsize(vector_type type)
{
    switch (type) {
    case byte_t:
        return sizeof(char);
    case float_t:
        return sizeof(float);
    case complex_t:
        return sizeof(gr_complex);
    default:
        throw std::runtime_error("bad PDU type");
    }
}

vector_type type_from_pmt(pmt::pmt_t vector)
{
    if (pmt::is_u8vector(vector))
        return byte_t;
    if (pmt::is_f32vector(vector))
        return float_t;
    if (pmt::is_c32vector(vector))
        return complex_t;
    throw std::runtime_error("bad PDU type");
}

void rfnoc_pdu_tx_impl::handle_data_message(pmt::pmt_t msg)
{
    pmt::pmt_t vector = pmt::cdr(msg);

    size_t offset(0);
    size_t isize(itemsize(type_from_pmt(vector)));
    int len(pmt::length(vector) * isize);

    // printf("pdu impl data message %lu, %lu, %d\n", offset, isize, len);
    // std::cout << boost::format("pdu impl data message %lu, %lu, %d") % offset % isize %
    // len << std::endl;

    const size_t num_sent = _tx.streamers[0]->send(
        pmt::uniform_vector_elements(vector, offset), len * _tx.vlen, _tx.metadata, 1.0);
    // std::cout << boost::format("Send %lu bytes") % num_sent << std::endl;
}

bool rfnoc_pdu_tx_impl::start()
{
    boost::recursive_mutex::scoped_lock lock(d_mutex);
    size_t ninputs = 1;
    size_t noutputs = 0;
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

    //////////////////// TX
    //////////////////////////////////////////////////////////////////
    // Setup TX streamer.
    if (ninputs && _tx.streamers.empty()) {
        // Get a block control for the tx side:
        ::uhd::rfnoc::sink_block_ctrl_base::sptr tx_blk_ctrl =
            boost::dynamic_pointer_cast<::uhd::rfnoc::sink_block_ctrl_base>(_blk_ctrl);
        if (!tx_blk_ctrl) {
            GR_LOG_FATAL(d_logger,
                         str(boost::format("Not a sink_block_ctrl_base: %s") %
                             _blk_ctrl->unique_id()));
            return false;
        }
        if (_tx.align) { // Aligned streamers:
            GR_LOG_DEBUG(
                d_debug_logger,
                str(boost::format("Creating one aligned tx streamer for %d inputs.") %
                    ninputs));
            GR_LOG_DEBUG(
                d_debug_logger,
                str(boost::format("cpu: %s  otw: %s  args: %s channels.size: %d ") %
                    _tx.stream_args.cpu_format % _tx.stream_args.otw_format %
                    _tx.stream_args.args.to_string() % _tx.stream_args.channels.size()));
            assert(ninputs == _tx.stream_args.channels.size());
            ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_tx.stream_args);
            if (tx_stream) {
                _tx.streamers.push_back(tx_stream);
            } else {
                GR_LOG_FATAL(d_logger,
                             str(boost::format("Can't create tx streamer(s) to: %s") %
                                 _blk_ctrl->get_block_id().get()));
                return false;
            }
        } else { // Unaligned streamers:
            for (size_t i = 0; i < size_t(ninputs); i++) {
                _tx.stream_args.channels = std::vector<size_t>(1, i);
                _tx.stream_args.args["block_port"] = str(boost::format("%d") % i);
                GR_LOG_DEBUG(d_debug_logger,
                             str(boost::format("creating tx streamer with: %s") %
                                 _tx.stream_args.args.to_string()));
                ::uhd::tx_streamer::sptr tx_stream = _dev->get_tx_stream(_tx.stream_args);
                if (tx_stream) {
                    _tx.streamers.push_back(tx_stream);
                }
            }
            if (_tx.streamers.size() != size_t(ninputs)) {
                GR_LOG_FATAL(d_logger,
                             str(boost::format("Can't create tx streamer(s) to: %s") %
                                 _blk_ctrl->get_block_id().get()));
                return false;
            }
        }
    }

    _tx.metadata.start_of_burst = false;
    _tx.metadata.end_of_burst = false;
    _tx.metadata.has_time_spec = false;

    return true;
}

} /* namespace ettus */
} /* namespace gr */
