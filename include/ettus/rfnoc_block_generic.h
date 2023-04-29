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

#ifndef INCLUDED_ETTUS_RFNOC_BLOCK_GENERIC_H
#define INCLUDED_ETTUS_RFNOC_BLOCK_GENERIC_H

#include <gnuradio/block.h>
#include <ettus/api.h>
#include <ettus/rfnoc_block.h>
#include <ettus/rfnoc_graph.h>

namespace gr {
namespace ettus {

/*! Generic RFNoC block holder
 *
 * This block can represent any RFNoC block
 *
 * \ingroup uhd_blk
 */
class ETTUS_API rfnoc_block_generic : virtual public rfnoc_block
{
public:
    typedef std::shared_ptr<rfnoc_block_generic> sptr;

    /*!
     * \param graph Reference to the underlying rfnoc_graph object
     * \param block_args Arguments that get passed into the block
     * \param block_name Block name. This argument, along with \p device_select
     *                   and \p block_select, are used to identify which block
     *                   is instantiated.
     * \param device_select Optional: Device count.
     * \param block_select Optional: Block select.
     */
    static sptr make(rfnoc_graph::sptr graph,
                     const ::uhd::device_addr_t& block_args,
                     const std::string& block_name,
                     const int device_select = -1,
                     const int block_select = -1);
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_BLOCK_GENERIC_H */
