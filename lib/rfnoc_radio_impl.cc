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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "rfnoc_radio_impl.h"

namespace gr {
  namespace ettus {

    rfnoc_radio::sptr
    rfnoc_radio::make(
          const device3::sptr &dev,
          const ::uhd::stream_args_t &tx_stream_args,
          const ::uhd::stream_args_t &rx_stream_args,
          const int radio_select,
          const int device_select
    ) {
      return gnuradio::get_initial_sptr(
          new rfnoc_radio_impl(dev, tx_stream_args, rx_stream_args, radio_select, device_select)
      );
    }

    rfnoc_radio_impl::rfnoc_radio_impl(
        const device3::sptr &dev,
        const ::uhd::stream_args_t &tx_stream_args,
        const ::uhd::stream_args_t &rx_stream_args,
        const int radio_select,
        const int device_select
    ) : rfnoc_block("rfnoc_radio"),
        rfnoc_block_impl(
            dev,
            rfnoc_block_impl::make_block_id("Radio", radio_select, device_select),
            tx_stream_args, rx_stream_args
        )
    {
      d_chan = get_block_ctrl()->get_block_id().get_block_count();
    }

    rfnoc_radio_impl::~rfnoc_radio_impl()
    {
      /* nop */
    }

    void rfnoc_radio_impl::set_tx_freq(double freq)
    {
      get_device()->set_tx_freq(freq, d_chan);
    }

    void rfnoc_radio_impl::set_rx_freq(double freq)
    {
      get_device()->set_rx_freq(freq, d_chan);
    }

    void rfnoc_radio_impl::set_tx_rate(double rate)
    {
      get_device()->set_tx_rate(rate, d_chan);
    }

    void rfnoc_radio_impl::set_rx_rate(double rate)
    {
      get_device()->set_rx_rate(rate, d_chan);
    }

    void rfnoc_radio_impl::set_tx_gain(double gain)
    {
      get_device()->set_tx_gain(gain, d_chan);
    }

    void rfnoc_radio_impl::set_rx_gain(double gain)
    {
      get_device()->set_rx_gain(gain, d_chan);
    }

    void rfnoc_radio_impl::set_tx_antenna(const std::string &ant)
    {
      get_device()->set_tx_antenna(ant, d_chan);
    }

    void rfnoc_radio_impl::set_rx_antenna(const std::string &ant)
    {
      get_device()->set_rx_antenna(ant, d_chan);
    }


  } /* namespace ettus */
} /* namespace gr */

