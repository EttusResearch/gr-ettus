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

#ifndef INCLUDED_ETTUS_RFNOC_PDU_RX_IMPL_H
#define INCLUDED_ETTUS_RFNOC_PDU_RX_IMPL_H

#include <ettus/rfnoc_block_impl.h>
#include <ettus/rfnoc_pdu_rx.h>

namespace gr {
namespace ettus {

enum vector_type { byte_t, float_t, complex_t };

class rfnoc_pdu_rx_impl : public rfnoc_pdu_rx, public rfnoc_block_impl
{
public:
    rfnoc_pdu_rx_impl(const device3::sptr& dev,
                      const ::uhd::stream_args_t& tx_stream_args,
                      const ::uhd::stream_args_t& rx_stream_args,
                      const std::string& block_name,
                      const int block_select,
                      const int device_select = -1,
                      const int mtu = 2048);
    bool start();
    ~rfnoc_pdu_rx_impl();


private:
    bool d_started;
    bool d_finished;
    std::vector<uint8_t> d_rxbuf;
    gr::thread::thread d_thread;

    pmt::pmt_t d_port;
    basic_block* d_blk;

    void run();
    void start_rxthread(basic_block* blk, pmt::pmt_t rxport);
    void stop_rxthread();
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_PDU_IMPL_H */
