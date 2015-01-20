/* -*- c++ -*- */
/* 
 * Copyright 2014 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_UHD_RFNOC_STREAMER_IMPL_H
#define INCLUDED_UHD_RFNOC_STREAMER_IMPL_H

#include <ettus/rfnoc_streamer.h>
#include <ettus/rfnoc_common.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/atomic.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/rfnoc/block_ctrl_base.hpp>
#include <boost/thread/mutex.hpp>
#include <map>

namespace gr {
  //namespace uhd {
  namespace ettus {

    class rfnoc_streamer_impl : public rfnoc_streamer
    {
     public:
      rfnoc_streamer_impl(
          const device3::sptr &dev,
          const std::string &block_id,
          const ::uhd::stream_args_t &stream_args,
          const int ninputs,
          const int noutputs,
          bool align_inputs,
          bool align_outputs
      );
      ~rfnoc_streamer_impl();

      void set_register(size_t reg, boost::uint32_t value);
      void set_option(const std::string &key, const std::string &val);
      std::string get_block_id();

      void set_taps(const std::vector<int> &taps);
      void set_window(const std::vector<int> &coeff);

      int general_work(
          int noutput_items,
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items,
          gr_vector_void_star &output_items
      );

      bool check_topology(int ninputs, int noutputs);
      bool start();
      bool stop();

      void flush(size_t streamer_index);

     private:
      void work_tx_a(
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items
      );
      void work_tx_u(
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items
      );

      int work_rx_a(
          int noutput_items,
          gr_vector_void_star &output_items
      );
      void work_rx_u(
          int noutput_items,
          gr_vector_void_star &output_items
      );

      boost::recursive_mutex d_mutex;

      rfnoc::rfnoc_common *d_rfnoccer;

    };
  } // namespace uhd
} // namespace gr

#endif /* INCLUDED_UHD_RFNOC_STREAMER_IMPL_H */
