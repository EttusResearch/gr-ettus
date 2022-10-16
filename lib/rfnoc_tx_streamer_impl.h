/* -*- c++ -*- */
/*
 * Copyright 2020 Ettus Research, A National Instruments Brand.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef INCLUDED_ETTUS_RFNOC_TX_STREAMER_IMPL_H
#define INCLUDED_ETTUS_RFNOC_TX_STREAMER_IMPL_H

#include <gnuradio/ettus/rfnoc_tx_streamer.h>

namespace gr {
namespace ettus {

class rfnoc_tx_streamer_impl : public rfnoc_tx_streamer
{
public:
    rfnoc_tx_streamer_impl(rfnoc_graph::sptr graph,
                           const size_t num_chans,
                           const ::uhd::stream_args_t& stream_args,
                           const size_t vlen);
    ~rfnoc_tx_streamer_impl();

    /***** API ***************************************************************/
    std::string get_unique_id() const override { return d_unique_id; }

    /***** GNU Radio API *****************************************************/
    bool check_topology(int, int) override;
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

private:
    //! Number of streaming channels
    const size_t d_num_chans;
    //! Bytes per item (e.g. 4 for sc16)
    const size_t d_itemsize;
    //! Input vector length
    const size_t d_vlen;
    //! Reference to the underlying graph
    rfnoc_graph::sptr d_graph;
    //! Stream args
    ::uhd::stream_args_t d_stream_args;
    //! Reference to the actual streamer
    ::uhd::tx_streamer::sptr d_streamer;
    //! Copy of the streamer's block ID
    const std::string d_unique_id;
    //! Stash for the TX metadata
    ::uhd::tx_metadata_t d_metadata;
    //! TX timeout value
    double d_timeout = 1.0;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_TX_STREAMER_IMPL_H */
