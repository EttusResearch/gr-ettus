/* -*- c++ -*- */
/*
 * Copyright 2022 Ettus Research, A National Instruments Brand.
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

#ifndef INCLUDED_ETTUS_RFNOC_SIGGEN_IMPL_H
#define INCLUDED_ETTUS_RFNOC_SIGGEN_IMPL_H

#include <gnuradio/ettus/rfnoc_siggen.h>
#include <uhd/rfnoc/siggen_block_control.hpp>

namespace gr {
namespace ettus {

class rfnoc_siggen_impl : public rfnoc_siggen
{
public:
    rfnoc_siggen_impl(::uhd::rfnoc::noc_block_base::sptr block_ref);
    ~rfnoc_siggen_impl();

    /*** API *****************************************************************/
    void set_amplitude(const double amplitude, const size_t chan);
    void set_constant(const std::complex<double> constant, const size_t chan);
    void set_enable(const bool enable, const size_t chan);
    void set_sine_phase_increment(const double phase_inc, const size_t chan);
    void set_sine_frequency(const double frequency, const double sample_rate, const size_t chan);
    void set_waveform(const ::uhd::rfnoc::siggen_waveform type, const size_t chan);
    void set_waveform(const std::string &type, const size_t chan);
    void set_samples_per_packet(const size_t spp, const size_t chan);

    double get_amplitude(const size_t chan);
    std::complex<double> get_constant(const size_t chan);
    bool get_enable(const size_t chan);
    double get_sine_phase_increment(const size_t chan);
    ::uhd::rfnoc::siggen_waveform get_waveform(const size_t chan);
    std::string get_waveform_string(const size_t chan);
    size_t get_samples_per_packet(const size_t chan);

private:
    ::uhd::rfnoc::siggen_block_control::sptr d_siggen_ref;
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_SIGGEN_IMPL_H */
