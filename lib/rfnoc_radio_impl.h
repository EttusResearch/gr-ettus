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
#include <uhd/types/ranges.hpp>

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

      double get_rate();
      double get_tx_freq(const size_t chan);
      double get_rx_freq(const size_t chan);
      double get_tx_gain(const size_t chan);
      double get_rx_gain(const size_t chan);
      double get_rx_bandwidth(const size_t chan);
      std::string get_tx_antenna(const size_t chan);
      std::string get_rx_antenna(const size_t chan);

      void set_rate(const double rate);
      void set_tx_freq(const double freq, const size_t chan);
      void set_rx_freq(const double freq, const size_t chan);
      void set_tx_gain(const double gain, const size_t chan);
      void set_rx_gain(const double gain, const size_t chan);
      void set_rx_bandwidth(const double bandwidth, const size_t chan);
      void set_tx_antenna(const std::string &ant, const size_t chan);
      void set_rx_antenna(const std::string &ant, const size_t chan);
      void set_tx_dc_offset(bool enable, const size_t chan);
      void set_tx_dc_offset(const std::complex< double > &offset, const size_t chan);
      void set_rx_dc_offset(bool enable, const size_t chan);
      void set_rx_dc_offset(const std::complex< double > &offset, const size_t chan);

      std::vector<std::string> get_rx_lo_names(const size_t chan);
      std::vector<std::string> get_rx_lo_sources(const std::string &name, const size_t chan);
      uhd::freq_range_t get_rx_lo_freq_range(const std::string &name, const size_t chan);

      void set_rx_lo_source(const std::string &src, const std::string &name, const size_t chan);
      const std::string get_rx_lo_source(const std::string &name, const size_t chan);

      void set_rx_lo_export_enabled(bool enabled, const std::string &name, const size_t chan);
      bool get_rx_lo_export_enabled(const std::string &name, const size_t chan);

      double set_rx_lo_freq(double freq, const std::string &name, const size_t chan);
      double get_rx_lo_freq(const std::string &name, const size_t chan);

      void set_clock_source(const std::string &source);
      std::string get_clock_source();
      
      std::vector<std::string> get_gpio_banks() const;
      void set_gpio_attr(
          const std::string &bank,
          const std::string &attr,
          const uint32_t value,
          const uint32_t mask
      );
      uint32_t get_gpio_attr(const std::string &bank, const std::string &attr);

      void set_command_tick_rate(const double tick_rate, const size_t chan);

      uhd::time_spec_t get_time_now(void);
      uhd::time_spec_t get_time_last_pps(void);
      void set_time_next_pps(const uhd::time_spec_t &spec);

      uhd::time_spec_t get_command_time(const size_t chan);
      void set_command_time(const uhd::time_spec_t &time, const size_t chan);
      void clear_command_time(const size_t chan);

      void set_rx_port_chan_map(const size_t port, const size_t chan);
      void set_tx_port_chan_map(const size_t port, const size_t chan);
     private:
      ::uhd::rfnoc::radio_ctrl::sptr _radio_ctrl;

      bool _start_time_set = false;
      uhd::time_spec_t _start_time;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_RADIO_IMPL_H */

