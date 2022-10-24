/* -*- c++ -*- */
/*
 * Copyright 2022 Ettus Research, A National Instruments Brand.
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

#include "rfnoc_fir_filter_impl.h"
#include <volk/volk.h>
#include <gnuradio/io_signature.h>

namespace gr {
namespace ettus {

rfnoc_fir_filter::sptr rfnoc_fir_filter::make(
    rfnoc_graph::sptr graph,
    const ::uhd::device_addr_t& block_args,
    const int device_select,
    const int instance)
{
    return gnuradio::get_initial_sptr(new rfnoc_fir_filter_impl(
        rfnoc_block::make_block_ref(graph, block_args, "FIR", device_select, instance)));
}


rfnoc_fir_filter_impl::rfnoc_fir_filter_impl(::uhd::rfnoc::noc_block_base::sptr block_ref)
    : rfnoc_block(block_ref), d_fir_filter_ref(get_block_ref<::uhd::rfnoc::fir_filter_block_control>())
{
}

rfnoc_fir_filter_impl::~rfnoc_fir_filter_impl() {}

/******************************************************************************
 * rfnoc_fir_filter API
 *****************************************************************************/
void rfnoc_fir_filter_impl::set_coefficients(const std::vector<float>& coeffs)
{
    std::vector<int16_t> coeffs_int16(coeffs.size());

    const float* in = &coeffs.front();
    int16_t* out = &coeffs_int16.front();

    volk_32f_s32f_convert_16i(out, in, 32768.0, coeffs.size());
    set_coefficients(coeffs_int16);
}
void rfnoc_fir_filter_impl::set_coefficients(const std::vector<int16_t>& coeffs)
{
    d_fir_filter_ref->set_coefficients(coeffs);
}

size_t rfnoc_fir_filter_impl::get_max_num_coefficients()
{
    return d_fir_filter_ref->get_max_num_coefficients();
}

std::vector<int16_t> rfnoc_fir_filter_impl::get_coefficients()
{
    return d_fir_filter_ref->get_coefficients();
}


} /* namespace ettus */
} /* namespace gr */
