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

#ifndef INCLUDED_ETTUS_RFNOC_RADIO_IMPL_H
#define INCLUDED_ETTUS_RFNOC_RADIO_IMPL_H

#include <ettus/rfnoc_radio.h>
#include <ettus/rfnoc_block_impl.h>
#include <uhd/rfnoc/radio_ctrl.hpp>

namespace gr {
  namespace ettus {

    class rfnoc_radio_impl : public rfnoc_radio, public rfnoc_block_impl
    {
     public:
      rfnoc_radio_impl(
            const device3::sptr &dev,
            const ::uhd::stream_args_t &tx_stream_args,
            const ::uhd::stream_args_t &rx_stream_args,
            const int radio_select,
            const int device_select
      );
      ~rfnoc_radio_impl();

      void set_rate(const double rate);
      void set_tx_freq(const double freq, const size_t chan);
      void set_rx_freq(const double freq, const size_t chan);
      void set_tx_gain(const double gain, const size_t chan);
      void set_rx_gain(const double gain, const size_t chan);
      void set_tx_antenna(const std::string &ant, const size_t chan);
      void set_rx_antenna(const std::string &ant, const size_t chan);
      void set_tx_dc_offset(bool enable, const size_t chan);
      void set_tx_dc_offset(const std::complex< double > &offset, const size_t chan);
      void set_rx_dc_offset(bool enable, const size_t chan);
      void set_rx_dc_offset(const std::complex< double > &offset, const size_t chan);

      uhd::time_spec_t get_time_now(void);
      void set_command_time(const uhd::time_spec_t &time);
      void clear_command_time(void);
     private:
      ::uhd::rfnoc::radio_ctrl::sptr _radio_ctrl;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RADIO_IMPL_H */

