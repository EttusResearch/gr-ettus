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

#ifndef INCLUDED_ETTUS_RFNOC_PDU_RX_H
#define INCLUDED_ETTUS_RFNOC_PDU_RX_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <ettus/rfnoc_block.h>

namespace gr {
  namespace ettus {

    /*!
     * \brief RFNoC fosphor block
     * \ingroup ettus
     */
    class ETTUS_API rfnoc_pdu_rx : virtual public rfnoc_block
    {
     public:
      typedef boost::shared_ptr<rfnoc_pdu_rx> sptr;

      /*!
       * \param dev device3 instance
       * \param tx_stream_args Tx Stream Args
       * \param rx_stream_args Tx Stream Args
       * \param block_name Block name, e.g. "FFT"
       * \param block_select Block select
       * \param device_select Device select
       * \param mtu max packet size 
       */
      static sptr make(
          const device3::sptr &dev,
          const ::uhd::stream_args_t &tx_stream_args,
          const ::uhd::stream_args_t &rx_stream_args,
	  const std::string &block_name,
          const int block_select,
          const int device_select=-1,
	  const int mtu=2048
      );
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_FOSPHOR_C_H */

