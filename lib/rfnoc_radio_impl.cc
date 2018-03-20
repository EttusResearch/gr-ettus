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
      _radio_ctrl = get_block_ctrl_throw< ::uhd::rfnoc::radio_ctrl >();
    }

    rfnoc_radio_impl::~rfnoc_radio_impl()
    {
      /* nop */
    }

    double rfnoc_radio_impl::get_rate()
    {
      return _radio_ctrl->get_rate();
    }

    void rfnoc_radio_impl::set_rate(const double rate)
    {
      _radio_ctrl->set_rate(rate);
    }

    double rfnoc_radio_impl::get_tx_freq(const size_t chan)
    {
      return _radio_ctrl->get_tx_frequency(chan);
    }

    void rfnoc_radio_impl::set_tx_freq(const double freq, const size_t chan)
    {
      _radio_ctrl->set_tx_frequency(freq, chan);
    }

    double rfnoc_radio_impl::get_rx_freq(const size_t chan)
    {
      return _radio_ctrl->get_rx_frequency(chan);
    }

    void rfnoc_radio_impl::set_rx_freq(const double freq, const size_t chan)
    {
      _radio_ctrl->set_rx_frequency(freq, chan);
    }

    double rfnoc_radio_impl::get_tx_gain(const size_t chan)
    {
      return _radio_ctrl->get_tx_gain(chan);
    }

    void rfnoc_radio_impl::set_tx_gain(const double gain, const size_t chan)
    {
      _radio_ctrl->set_tx_gain(gain, chan);
    }

    double rfnoc_radio_impl::get_rx_gain(const size_t chan)
    {
      return _radio_ctrl->get_rx_gain(chan);
    }

    void rfnoc_radio_impl::set_rx_gain(const double gain, const size_t chan)
    {
      _radio_ctrl->set_rx_gain(gain, chan);
    }

    double rfnoc_radio_impl::get_rx_bandwidth(const size_t chan)
    {
      return _radio_ctrl->get_rx_bandwidth(chan);
    }

    void rfnoc_radio_impl::set_rx_bandwidth(const double bandwidth, const size_t chan)
    {
      _radio_ctrl->set_rx_bandwidth(bandwidth, chan);
    }

    std::string rfnoc_radio_impl::get_tx_antenna(const size_t chan)
    {
      return _radio_ctrl->get_tx_antenna(chan);
    }

    void rfnoc_radio_impl::set_tx_antenna(const std::string &ant, const size_t chan)
    {
      _radio_ctrl->set_tx_antenna(ant, chan);
    }

    std::string rfnoc_radio_impl::get_rx_antenna(const size_t chan)
    {
      return _radio_ctrl->get_rx_antenna(chan);
    }

    void rfnoc_radio_impl::set_rx_antenna(const std::string &ant, const size_t chan)
    {
      _radio_ctrl->set_rx_antenna(ant, chan);
    }

    // FIXME everything down from here needs to be mapped on to the block API
    void rfnoc_radio_impl::set_tx_dc_offset(bool enable, const size_t chan)
    {
      //get_device()->set_tx_dc_offset(enable, chan);
    }

    void rfnoc_radio_impl::set_tx_dc_offset(const std::complex< double > &offset, const size_t chan)
    {
      //get_device()->set_tx_dc_offset(offset, chan);
    }

    void rfnoc_radio_impl::set_rx_dc_offset(bool enable, const size_t chan)
    {
      //get_device()->set_rx_dc_offset(enable, chan);
    }

    void rfnoc_radio_impl::set_rx_dc_offset(const std::complex< double > &offset, const size_t chan)
    {
      //get_device()->set_rx_dc_offset(offset, chan);
    }

    std::vector<std::string> rfnoc_radio_impl::get_rx_lo_names(const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_names(chan);
    }

    std::vector<std::string> rfnoc_radio_impl::get_rx_lo_sources(const std::string &name, const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_sources(name, chan);
    }

    uhd::freq_range_t rfnoc_radio_impl::get_rx_lo_freq_range(const std::string &name, const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_freq_range(name, chan);
    }

    void rfnoc_radio_impl::set_rx_lo_source(const std::string &src, const std::string &name, const size_t chan)
    {
        _radio_ctrl->set_rx_lo_source(src, name, chan);
    }

    const std::string rfnoc_radio_impl::get_rx_lo_source(const std::string &name, const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_source(name, chan);
    }

    void rfnoc_radio_impl::set_rx_lo_export_enabled(bool enabled, const std::string &name, const size_t chan)
    {
        _radio_ctrl->set_rx_lo_export_enabled(enabled, "all", chan);
    }

    bool rfnoc_radio_impl::get_rx_lo_export_enabled(const std::string &name, const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_export_enabled(name, chan);
    }

    double rfnoc_radio_impl::set_rx_lo_freq(double freq, const std::string &name, const size_t chan)
    {
        return _radio_ctrl->set_rx_lo_freq(freq, name, chan);
    }

    double rfnoc_radio_impl::get_rx_lo_freq(const std::string &name, const size_t chan)
    {
        return _radio_ctrl->get_rx_lo_freq(name, chan);
    }

    void rfnoc_radio_impl::set_clock_source(const std::string &source)
    {
      _radio_ctrl->set_clock_source(source);
    }
    
    std::string rfnoc_radio_impl::get_clock_source()
    {
      return _radio_ctrl->get_clock_source();
    }

    std::vector<std::string> rfnoc_radio_impl::get_gpio_banks() const
    {
      return _radio_ctrl->get_gpio_banks();
    }

    void rfnoc_radio_impl::set_gpio_attr(
        const std::string &bank,
        const std::string &attr,
        const uint32_t value,
        const uint32_t mask
    ) {
      _radio_ctrl->set_gpio_attr(bank, attr, value, mask);
    }

    void rfnoc_radio_impl::set_rx_port_chan_map(const size_t port, const size_t chan)
    {
      _radio_ctrl->set_rx_port_chan_map(port, chan);
    }

    void rfnoc_radio_impl::set_tx_port_chan_map(const size_t port, const size_t chan)
    {
      _radio_ctrl->set_tx_port_chan_map(port, chan);
    }

    uint32_t rfnoc_radio_impl::get_gpio_attr(const std::string &bank, const std::string &attr)
    {
      return _radio_ctrl->get_gpio_attr(bank, attr);
    }

    void rfnoc_radio_impl::set_time_next_pps(const uhd::time_spec_t &spec)
    {
      _radio_ctrl->set_time_next_pps(spec);
    }

    uhd::time_spec_t rfnoc_radio_impl::get_time_now(void)
    {
      return _radio_ctrl->get_time_now();
    }

    uhd::time_spec_t rfnoc_radio_impl::get_time_last_pps(void)
    {
      return _radio_ctrl->get_time_last_pps();
    }

    uhd::time_spec_t rfnoc_radio_impl::get_command_time(const size_t chan)
    {
      return _radio_ctrl->get_command_time(chan);
    }

    void rfnoc_radio_impl::set_command_time(const uhd::time_spec_t &time, const size_t chan)
    {
      _radio_ctrl->set_command_time(time, chan);
    }

    void rfnoc_radio_impl::clear_command_time(const size_t chan)
    {
      _radio_ctrl->clear_command_time(chan);
    }

    void rfnoc_radio_impl::set_command_tick_rate(const double tick_rate, const size_t chan)
    {
      _radio_ctrl->set_command_tick_rate(chan);
    }

  } /* namespace ettus */
} /* namespace gr */

