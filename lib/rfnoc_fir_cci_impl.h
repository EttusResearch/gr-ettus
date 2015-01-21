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

#ifndef INCLUDED_ETTUS_RFNOC_FIR_CCI_IMPL_H
#define INCLUDED_ETTUS_RFNOC_FIR_CCI_IMPL_H

#include <ettus/rfnoc_fir_cci.h>
#include <ettus/rfnoc_common.h>

namespace gr {
  namespace ettus {

    class rfnoc_fir_cci_impl : public rfnoc_fir_cci
    {
     private:
      rfnoc::rfnoc_common *d_rfnoccer;

     public:
      rfnoc_fir_cci_impl(
          const std::vector<int> &taps,
          const device3::sptr &dev,
          const int block_select,
          const int device_select
      );
      ~rfnoc_fir_cci_impl();

      void set_taps(const std::vector<int> &taps);

      bool check_topology(int ninputs, int noutputs);
      bool start();
      bool stop();

      std::string get_block_id();

      int general_work(
          int noutput_items,
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items,
          gr_vector_void_star &output_items
      );
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FIR_CCI_IMPL_H */

