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

#ifndef INCLUDED_ETTUS_RFNOC_DDC_IMPL_H
#define INCLUDED_ETTUS_RFNOC_DDC_IMPL_H

#include <gnuradio/ettus/rfnoc_ddc.h>
#include <uhd/rfnoc/ddc_block_control.hpp>

namespace gr {
namespace ettus {

class rfnoc_ddc_impl : public rfnoc_ddc
{
public:
    rfnoc_ddc_impl(::uhd::rfnoc::noc_block_base::sptr block_ref);
    ~rfnoc_ddc_impl();

    /*** API *****************************************************************/
    double set_freq(const double freq,
                    const size_t chan,
                    const ::uhd::time_spec_t time = ::uhd::time_spec_t::ASAP);
    double set_output_rate(const double rate, const size_t chan);

private:
    ::uhd::rfnoc::ddc_block_control::sptr d_ddc_ref;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_DDC_IMPL_H */
