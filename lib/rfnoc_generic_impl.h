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

#ifndef INCLUDED_ETTUS_RFNOC_GENERIC_IMPL_H
#define INCLUDED_ETTUS_RFNOC_GENERIC_IMPL_H

#include <ettus/rfnoc_generic.h>
#include <ettus/rfnoc_block_impl.h>

namespace gr {
  namespace ettus {

    class rfnoc_generic_impl : public rfnoc_generic, public rfnoc_block_impl
    {
     public:
      rfnoc_generic_impl(
          const device3::sptr &dev,
          const ::uhd::stream_args_t &tx_stream_args,
          const ::uhd::stream_args_t &rx_stream_args,
          const std::string &block_name,
          const int block_select,
          const int device_select
      );
      ~rfnoc_generic_impl();
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_GENERIC_IMPL_H */

