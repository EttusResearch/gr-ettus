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

#ifndef INCLUDED_ETTUS_RFNOC_RX_STREAMER_IMPL_H
#define INCLUDED_ETTUS_RFNOC_RX_STREAMER_IMPL_H

#include <gnuradio/ettus/rfnoc_rx_streamer.h>

namespace gr {
namespace ettus {

class rfnoc_rx_streamer_impl : public rfnoc_rx_streamer
{
public:
    rfnoc_rx_streamer_impl(rfnoc_graph::sptr graph,
                           const size_t num_chans,
                           const ::uhd::stream_args_t& stream_args,
                           const size_t vlen,
                           const bool issue_stream_cmd_on_start);
    ~rfnoc_rx_streamer_impl();

    std::string get_unique_id() const override { return d_unique_id; }

    /***** API ***************************************************************/
    void set_start_time(const ::uhd::time_spec_t& time);

    /***** GNU Radio API *****************************************************/
    bool check_topology(int, int) override;
    bool start() override;
    bool stop() override;
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

private:
    void flush();

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
    ::uhd::rx_streamer::sptr d_streamer;
    //! Copy of the streamer's block ID
    const std::string d_unique_id;
    //! Stash for the TX metadata
    ::uhd::rx_metadata_t d_metadata;
    //! RX timeout value
    double d_timeout = 1.0;
    //! True if the stream should immediately start with the flow graph without
    // external prompting
    const bool d_issue_stream_cmd_on_start;
    //! True if d_start_time holds a value we need to process
    bool d_start_time_set = false;
    //! A start time for the next stream command
    ::uhd::time_spec_t d_start_time;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RX_STREAMER_IMPL_H */
