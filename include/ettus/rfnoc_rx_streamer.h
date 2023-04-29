/* -*- c++ -*- */
/*
 * Copyright 2019 Ettus Research, a National Instruments Brand.
 * Copyright 2020 Ettus Research, A National Instruments Brand.
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

#ifndef INCLUDED_ETTUS_RFNOC_RX_STREAMER_H
#define INCLUDED_ETTUS_RFNOC_RX_STREAMER_H

#include <gnuradio/sync_block.h>
#include <ettus/api.h>
#include <ettus/rfnoc_graph.h>
#include <uhd/stream.hpp>

namespace gr {
namespace ettus {

/*! RFNoC Rx Streamer: Block to handle data flow from an RFNoC flow graph into
 * a GNU Radio flow graph.
 *
 * Use this block for ingress into a GNU Radio flow graph. "Rx" is from the
 * viewpoint of the GNU Radio flow graph. For example, if the GNU Radio flow
 * graph is receiving samples from a radio, use this block to transport the
 * samples into GNU Radio.
 *
 * Note: The input ports of this block can only connect to other RFNoC blocks.
 *
 * \ingroup uhd_blk
 */
class ETTUS_API rfnoc_rx_streamer : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<rfnoc_rx_streamer> sptr;

    /*!
     * \param graph Reference to the graph this block is connected to
     * \param num_chans Number of input- and output ports
     * \param stream_args These will be passed on to
     *                    rfnoc_graph::create_rx_streamer, see that for details.
     *                    The cpu_format and otw_format parts of these args will
     *                    be used to determine the in- and output signatures of
     *                    this block.
     * \param vlen Vector length
     * \param issue_stream_cmd_on_start If true, the streamer sends a stream
     *                                  command upstream.
     */
    static sptr make(rfnoc_graph::sptr graph,
                     const size_t num_chans,
                     const ::uhd::stream_args_t& stream_args,
                     const size_t vlen = 1,
                     const bool issue_stream_cmd_on_start = true);

    //! Return the unique ID associated with the underlying RFNoC streamer
    virtual std::string get_unique_id() const = 0;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RX_STREAMER_H */
