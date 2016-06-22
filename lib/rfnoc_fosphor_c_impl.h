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

#ifndef INCLUDED_ETTUS_RFNOC_FOSPHOR_C_IMPL_H
#define INCLUDED_ETTUS_RFNOC_FOSPHOR_C_IMPL_H

#include <ettus/rfnoc_fosphor_c.h>
#include <ettus/rfnoc_block_impl.h>

namespace gr {
  namespace ettus {

    class rfnoc_fosphor_c_impl : public rfnoc_fosphor_c, public rfnoc_block_impl
    {
     public:
      rfnoc_fosphor_c_impl(
          const int fft_size,
          const device3::sptr &dev,
          const int block_select,
          const int device_select
      );
      ~rfnoc_fosphor_c_impl();


     private:
      void handle_cfg_message(pmt::pmt_t msg);
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FOSPHOR_C_IMPL_H */

