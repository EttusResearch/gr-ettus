/* -*- c++ -*- */
/*
 * Copyright 2021 Ettus Research, A National Instruments Brand.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *P
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

#ifndef INCLUDED_ETTUS_RFNOC_WINDOW_IMPL_H
#define INCLUDED_ETTUS_RFNOC_WINDOW_IMPL_H

#include <gnuradio/ettus/rfnoc_fir_filter.h>
#include <uhd/rfnoc/fir_filter_block_control.hpp>

namespace gr {
namespace ettus {

class rfnoc_fir_filter_impl : public rfnoc_fir_filter
{
public:
    rfnoc_fir_filter_impl(::uhd::rfnoc::noc_block_base::sptr block_ref);
    ~rfnoc_fir_filter_impl();

    /*** API *****************************************************************/
    void set_coefficients(const std::vector<float>& coeffs, const size_t chan);
    void set_coefficients(const std::vector<int16_t>& coeffs, const size_t chan);
    size_t get_max_num_coefficients(const size_t chan);
    std::vector<int16_t> get_coefficients(const size_t chan);

private:
    ::uhd::rfnoc::fir_filter_block_control::sptr d_fir_filter_ref;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FIR_FILTER_IMPL_H */
