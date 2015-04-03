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
#include "rfnoc_block_impl.h"

namespace gr {
  namespace ettus {

    class rfnoc_radio_impl : public rfnoc_radio, public rfnoc_block_impl

    {
     private:
       size_t d_chan;

     public:
      rfnoc_radio_impl(
            const device3::sptr &dev,
            const ::uhd::stream_args_t &tx_stream_args,
            const ::uhd::stream_args_t &rx_stream_args,
            const int radio_select,
            const int device_select
      );
      ~rfnoc_radio_impl();

      void set_tx_freq(double freq);
      void set_rx_freq(double freq);
      void set_tx_rate(double rate);
      void set_rx_rate(double rate);
      void set_tx_gain(double gain);
      void set_rx_gain(double gain);
      void set_tx_antenna(const std::string &ant);
      void set_rx_antenna(const std::string &ant);
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RADIO_IMPL_H */

