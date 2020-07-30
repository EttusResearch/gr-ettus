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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rfnoc_block_generic_impl.h"
#include <ettus/rfnoc_graph.h>

namespace gr {
namespace ettus {

/******************************************************************************
 * Factory and Structors
 *****************************************************************************/
rfnoc_block_generic::sptr
rfnoc_block_generic::make(rfnoc_graph::sptr graph,
                          const ::uhd::device_addr_t& block_args,
                          const std::string& block_name,
                          const int device_select,
                          const int block_select)
{
    return gnuradio::get_initial_sptr(
        new rfnoc_block_generic_impl(rfnoc_block::make_block_ref(
            graph, block_args, block_name, device_select, block_select)));
}

rfnoc_block_generic_impl::rfnoc_block_generic_impl(
    ::uhd::rfnoc::noc_block_base::sptr block_ref)
    : rfnoc_block(block_ref)
{
}

rfnoc_block_generic_impl::~rfnoc_block_generic_impl() {}

} /* namespace ettus */
} /* namespace gr */
