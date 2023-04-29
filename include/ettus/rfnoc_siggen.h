/* -*- c++ -*- */
/*
 * Copyright 2022 Ettus Research, A National Instruments Brand.
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

#ifndef INCLUDED_ETTUS_RFNOC_SIGGEN_H
#define INCLUDED_ETTUS_RFNOC_SIGGEN_H

#include <ettus/api.h>
#include <ettus/rfnoc_block.h>
#include <uhd/rfnoc/siggen_block_control.hpp>

namespace gr {
namespace ettus {

/*! RFNoC Signal Generator Block
 *
 * \ingroup uhd_blk
 */
class ETTUS_API rfnoc_siggen : virtual public rfnoc_block
{
public:
    typedef std::shared_ptr<rfnoc_siggen> sptr;

    static const ::uhd::rfnoc::siggen_waveform CONSTANT ;
    static const ::uhd::rfnoc::siggen_waveform SINE_WAVE ;
    static const ::uhd::rfnoc::siggen_waveform NOISE ;

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

    /*! Set the amplitude of the signal
     *
     * \param amplitude Amplitude of the signal (double)
     * \param chan      Channel index
     */
    virtual void set_amplitude(const double amplitude, const size_t chan) = 0 ;

    /*! Get the amplitude of the signal
     *
     * \param chan  Channel index
     * \returns     Signal amplitude
     */
    virtual double get_amplitude(const size_t chan) = 0 ;

    /*! Set a complex constant of the signal
     *
     * \param constant  Constant for the signal (complex double)
     * \param chan      Channel index
     */
    virtual void set_constant(const std::complex<double> constant, const size_t chan) = 0 ;

    /*! Get the complex constant of the signal
     *
     * \param chan  Channel index
     * \returns     Complex constant of signal
     */
    virtual std::complex<double> get_constant(const size_t chan) = 0 ;

    /*! Enable the channel
     *
     * \param enable    Boolean to enable or disable output
     * \param chan      Channel index
     */
    virtual void set_enable(const bool enable, const size_t chan) = 0 ;

    /*! Get the channel enable state
     *
     * \param chan  Channel index
     * \returns     Enable State
     */
    virtual bool get_enable(const size_t chan) = 0 ;

    /*! Set the sine frequency in terms of the sample_rate
     *
     * \param frequency     The frequency of the tone being set
     * \param sample_rate   The sample rate of the block
     * \param chan          Channel index
     */
    virtual void set_sine_frequency(const double frequency, const double sample_rate, const size_t chan) = 0 ;

    /*! Set the sine frequency phase increment
     *
     * \param phase_inc The normalized phase increment per sample
     * \param chan      Channel index
     */
    virtual void set_sine_phase_increment(const double phase_inc, const size_t chan) = 0 ;

    /*! Get the sine frequency phase increment
     *
     * \param chan  Channel index
     * \returns     The normalized phase increment per sample
     */
    virtual double get_sine_phase_increment(const size_t chan) = 0 ;

    /*! Set the type of waveform using the siggen_waveform class
     *
     * \param type The waveform to choose
     * \param chan Channel index
     */
    virtual void set_waveform(const ::uhd::rfnoc::siggen_waveform type, const size_t chan) = 0 ;

    /*! Set the type of waveform using a string
     *
     * \param type  The waveform to choose ["CONSTANT", "SINE_WAVE", "NOISE"]
     * \param chan  Channel index
     */
    virtual void set_waveform(const std::string &type, const size_t chan) = 0 ;

    /*! Get the type of waveform as a string
     *
     * \param chan  Channel index
     * \returns     The current waveform as a string
     */
    virtual std::string get_waveform_string(const size_t chan) = 0 ;

    /*! Get the waveform
     *
     * \param chan  Channel index
     * \returns     The current waveform
     */
    virtual ::uhd::rfnoc::siggen_waveform get_waveform(const size_t chan) = 0 ;

    /*! Set the number of samples per packet
     *
     * \param spp   The number of samples per packet
     * \param chan  Channel index
     */
    virtual void set_samples_per_packet(const size_t spp, const size_t chan) = 0 ;

    /*! Get the number of samples per packet
     *
     * \param chan  Channel index
     * \returns     The number of samples per packet
     */
    virtual size_t get_samples_per_packet(const size_t chan) = 0 ;

};

// Convenience waveform types for Python
const ::uhd::rfnoc::siggen_waveform rfnoc_siggen::CONSTANT  = ::uhd::rfnoc::siggen_waveform::CONSTANT ;
const ::uhd::rfnoc::siggen_waveform rfnoc_siggen::SINE_WAVE = ::uhd::rfnoc::siggen_waveform::SINE_WAVE ;
const ::uhd::rfnoc::siggen_waveform rfnoc_siggen::NOISE     = ::uhd::rfnoc::siggen_waveform::NOISE ;

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_SIGGEN_H */
