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

#ifndef INCLUDED_ETTUS_RFNOC_DUC_H
#define INCLUDED_ETTUS_RFNOC_DUC_H

#include <gnuradio/ettus/api.h>
#include <gnuradio/ettus/rfnoc_block.h>
#include <gnuradio/ettus/rfnoc_graph.h>

namespace gr {
namespace ettus {

/*! RFNoC Digital Upconverter Block
 *
 * \ingroup uhd_blk
 */
class ETTUS_API rfnoc_duc : virtual public rfnoc_block
{
public:
    typedef std::shared_ptr<rfnoc_duc> sptr;

    /*!
     * \param graph Reference to the rfnoc_graph object this block is attached to
     * \param block_args Additional block arguments
     * \param device_select Device Selection
     * \param instance Instance Selection
     */
    static sptr make(rfnoc_graph::sptr graph,
                     const ::uhd::device_addr_t& block_args,
                     const int device_select,
                     const int instance);

    /*! Set DDS frequency
     */
    virtual double set_freq(const double freq,
                            const size_t chan,
                            const ::uhd::time_spec_t time = ::uhd::time_spec_t::ASAP) = 0;

    /*! Set the sample rate at the input
     *
     * When the DUC block is connected to a TX streamer block, then this will
     * help with the property propagation.
     *
     * \param rate Input Sampling Rate (Hz)
     * \param chan Channel Index
     */
    virtual double set_input_rate(const double rate, const size_t chan) = 0;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_DUC_H */
