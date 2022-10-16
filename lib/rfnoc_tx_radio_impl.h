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

#ifndef INCLUDED_ETTUS_RFNOC_TX_RADIO_IMPL_H
#define INCLUDED_ETTUS_RFNOC_TX_RADIO_IMPL_H

#include <gnuradio/ettus/rfnoc_tx_radio.h>
#include <uhd/rfnoc/radio_control.hpp>

namespace gr {
namespace ettus {

class rfnoc_tx_radio_impl : public rfnoc_tx_radio
{
public:
    rfnoc_tx_radio_impl(::uhd::rfnoc::noc_block_base::sptr block_ref);
    ~rfnoc_tx_radio_impl();

    /*** API *****************************************************************/
    double set_rate(const double rate);
    void set_antenna(const std::string& antenna, const size_t chan);
    double set_frequency(const double frequency, const size_t chan);
    void set_tune_args(const ::uhd::device_addr_t& args, const size_t chan);
    double set_gain(const double gain, const size_t chan);
    double set_gain(const double gain, const std::string& name, const size_t chan);
    void set_gain_profile(const std::string& profile, const size_t chan);
    double set_bandwidth(const double bandwidth, const size_t chan);
    void
    set_lo_source(const std::string& source, const std::string& name, const size_t chan);
    void
    set_lo_export_enabled(const bool enabled, const std::string& name, const size_t chan);
    double set_lo_freq(const double freq, const std::string& name, const size_t chan);
    void set_dc_offset(const std::complex<double>& offset, const size_t chan);
    void set_iq_balance(const std::complex<double>& correction, const size_t chan);

private:
    ::uhd::rfnoc::radio_control::sptr d_wrapped_ref;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_TX_RADIO_IMPL_H */
