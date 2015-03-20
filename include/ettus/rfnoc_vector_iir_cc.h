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


#ifndef INCLUDED_ETTUS_RFNOC_VECTOR_IIR_CC_H
#define INCLUDED_ETTUS_RFNOC_VECTOR_IIR_CC_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <ettus/rfnoc_block.h>

namespace gr {
  namespace ettus {

    /*!
     * \brief RFNoC: Vector IIR
     */
    class ETTUS_API rfnoc_vector_iir_cc : virtual public rfnoc_block
    {
     public:
      typedef boost::shared_ptr<rfnoc_vector_iir_cc> sptr;

      static sptr make(int vlen, double alpha, double beta, const device3::sptr &dev, const int block_select=-1, const int device_select=-1);

      virtual void set_vector_iir_alpha(const double alpha) = 0;
      virtual void set_vector_iir_beta(const double beta) = 0;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_VECTOR_IIR_CC_H */

