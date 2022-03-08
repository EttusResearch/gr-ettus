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

#include "rfnoc_siggen_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace ettus {

rfnoc_siggen::sptr rfnoc_siggen::make(rfnoc_graph::sptr graph,
                                const ::uhd::device_addr_t& block_args,
                                const int device_select,
                                const int instance)
{
    return gnuradio::get_initial_sptr(new rfnoc_siggen_impl(
        rfnoc_block::make_block_ref(graph, block_args, "SigGen", device_select, instance)));
}


rfnoc_siggen_impl::rfnoc_siggen_impl(::uhd::rfnoc::noc_block_base::sptr block_ref)
    : rfnoc_block(block_ref), d_siggen_ref(get_block_ref<::uhd::rfnoc::siggen_block_control>())
{
}

rfnoc_siggen_impl::~rfnoc_siggen_impl() {
    //set_enable(false, 0) ;
}

/******************************************************************************
 * rfnoc_siggen API
 *****************************************************************************/
void rfnoc_siggen_impl::set_amplitude(const double amplitude, const size_t chan=0) {
    d_siggen_ref->set_amplitude(amplitude, chan);
}

double rfnoc_siggen_impl::get_amplitude(const size_t chan=0) {
    return d_siggen_ref->get_amplitude(chan);
}

void rfnoc_siggen_impl::set_constant(const std::complex<double> constant, const size_t chan=0) {
    d_siggen_ref->set_constant(constant, chan);
}

std::complex<double> rfnoc_siggen_impl::get_constant(const size_t chan=0) {
    return d_siggen_ref->get_constant(chan);
}

void rfnoc_siggen_impl::set_enable(const bool enable, const size_t chan=0) {
    d_siggen_ref->set_enable(enable, chan);
}

bool rfnoc_siggen_impl::get_enable(const size_t chan=0) {
    return d_siggen_ref->get_enable(chan);
}

void rfnoc_siggen_impl::set_sine_phase_increment(const double phase_inc, const size_t chan=0) {
    d_siggen_ref->set_sine_phase_increment(phase_inc, chan);
}

double rfnoc_siggen_impl::get_sine_phase_increment(const size_t chan=0) {
    return d_siggen_ref->get_sine_phase_increment(chan);
}

void rfnoc_siggen_impl::set_sine_frequency(const double frequency, const double sample_rate, const size_t chan=0) {
    d_siggen_ref->set_sine_frequency(frequency, sample_rate, chan);
}

void rfnoc_siggen_impl::set_waveform(const ::uhd::rfnoc::siggen_waveform type, const size_t chan=0) {
    d_siggen_ref->set_waveform(type, chan);
}

void rfnoc_siggen_impl::set_waveform(const std::string &type, const size_t chan=0) {
    if(type == "NOISE") {
        d_siggen_ref->set_waveform(NOISE, chan);
    } else if(type == "CONSTANT") {
        d_siggen_ref->set_waveform(CONSTANT, chan);
    } else if(type == "SINE_WAVE") {
        d_siggen_ref->set_waveform(SINE_WAVE, chan);
    } else {
        throw std::runtime_error("SigGen waveform type (" + type + ") is not [NOISE, CONSTANT, SINE_WAVE]");
    }
}

::uhd::rfnoc::siggen_waveform rfnoc_siggen_impl::get_waveform(const size_t chan) {
    return d_siggen_ref->get_waveform(chan);
}

std::string rfnoc_siggen_impl::get_waveform_string(const size_t chan) {
    ::uhd::rfnoc::siggen_waveform type = get_waveform(chan);
    switch(type) {
        case NOISE      : return "NOISE";
        case CONSTANT   : return "CONSTANT";
        case SINE_WAVE  : return "SINE_WAVE";
        default         : throw std::runtime_error("SigGen waveform type enumeration (" +std::to_string(static_cast<uint32_t>(type)) + ") is not [NOISE, CONSTANT, SINE_WAVE]");
    }
}

void rfnoc_siggen_impl::set_samples_per_packet(const size_t spp, const size_t chan=0) {
    d_siggen_ref->set_samples_per_packet(spp, chan);
}

size_t rfnoc_siggen_impl::get_samples_per_packet(const size_t chan=0) {
    return d_siggen_ref->get_samples_per_packet(chan);
}

} /* namespace ettus */
} /* namespace gr */
