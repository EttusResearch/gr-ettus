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

#ifndef INCLUDED_ETTUS_RFNOC_FOSPHOR_C_H
#define INCLUDED_ETTUS_RFNOC_FOSPHOR_C_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <ettus/rfnoc_block.h>

namespace gr {
  namespace ettus {

    /*!
     * \brief RFNoC fosphor block
     * \ingroup ettus
     */
    class ETTUS_API rfnoc_fosphor_c : virtual public rfnoc_block
    {
     public:
      typedef boost::shared_ptr<rfnoc_fosphor_c> sptr;

      /*!
       * \param fft_size The size of the FFT (num of bins)
       * \param dev device3 instance
       * \param block_select Block select
       * \param device_select Device select
       */
      static sptr make(
          const int fft_size,
          const device3::sptr &dev, const int block_select=-1, const int device_select=-1
      );
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FOSPHOR_C_H */

