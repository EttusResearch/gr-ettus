/* -*- c++ -*- */
/* Copyright 2015 Ettus Research
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


#ifndef INCLUDED_ETTUS_RFNOC_RADIO_H
#define INCLUDED_ETTUS_RFNOC_RADIO_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <ettus/rfnoc_block.h>
#include <uhd/stream.hpp>

namespace gr {
  namespace ettus {

    /*!
     * \brief <+description of block+>
     * \ingroup ettus
     *
     */
    class ETTUS_API rfnoc_radio : virtual public rfnoc_block
    {
     public:
      typedef boost::shared_ptr<rfnoc_radio> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of ettus::rfnoc_radio.
       *
       * To avoid accidental use of raw pointers, ettus::rfnoc_radio's
       * constructor is in a private implementation
       * class. ettus::rfnoc_radio::make is the public interface for
       * creating new instances.
       */
      static sptr make(
          const device3::sptr &dev,
          const ::uhd::stream_args_t &tx_stream_args,
          const ::uhd::stream_args_t &rx_stream_args,
          const int radio_select,
          const int device_select=-1
      );

      virtual void set_rate(const double rate) = 0;
      virtual void set_tx_freq(const double freq, const size_t chan=0) = 0;
      virtual void set_rx_freq(const double freq, const size_t chan=0) = 0;
      virtual void set_tx_gain(const double gain, const size_t chan=0) = 0;
      virtual void set_rx_gain(const double gain, const size_t chan=0) = 0;
      virtual void set_tx_antenna(const std::string &ant, const size_t chan=0) = 0;
      virtual void set_rx_antenna(const std::string &ant, const size_t chan=0) = 0;
      virtual void set_tx_dc_offset(bool enable, const size_t chan=0) = 0;
      virtual void set_tx_dc_offset(const std::complex< double > &offset, const size_t chan=0) = 0;
      virtual void set_rx_dc_offset(bool enable, const size_t chan=0) = 0;
      virtual void set_rx_dc_offset(const std::complex< double > &offset, const size_t chan=0) = 0;
      virtual void set_bandwidth(const double bandwidth, const size_t chan=0) = 0;

      virtual std::vector<std::string> get_gpio_banks() const = 0;
      virtual void set_gpio_attr(
          const std::string &bank,
          const std::string &attr,
          const uint32_t value,
          const uint32_t mask
      ) = 0;
      virtual uint32_t get_gpio_attr(const std::string &bank, const std::string &attr) = 0;

      virtual uhd::time_spec_t get_time_now(void) = 0;
      virtual void set_command_time(const uhd::time_spec_t &time, const size_t chan=0) = 0;
      virtual void clear_command_time(const size_t chan=0) = 0;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RADIO_H */

