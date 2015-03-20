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

#ifndef INCLUDED_ETTUS_RFNOC_FIR_CCI_H
#define INCLUDED_ETTUS_RFNOC_FIR_CCI_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <ettus/rfnoc_block.h>

namespace gr {
  namespace ettus {

    /*!
     * \brief RFNoC FIR block
     * \ingroup ettus
     */
    class ETTUS_API rfnoc_fir_cci : virtual public rfnoc_block
    {
     public:
      typedef boost::shared_ptr<rfnoc_fir_cci> sptr;

      /*!
       * \param taps Taps
       * \param dev device3 instance
       * \param block_select Block select
       * \param device_select Device select
       */
      static sptr make(
          const std::vector<int> &taps,
          const device3::sptr &dev,
          const int block_select=-1,
          const int device_select=-1
      );

      //! Update the filter taps
      virtual void set_taps(const std::vector<int> &taps) = 0;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FIR_CCI_H */

