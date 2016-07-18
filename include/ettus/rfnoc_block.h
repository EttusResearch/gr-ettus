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

#ifndef INCLUDED_UHD_RFNOC_BLOCK_H
#define INCLUDED_UHD_RFNOC_BLOCK_H

#include <ettus/api.h>
#include <ettus/device3.h>
#include <gnuradio/block.h>
#include <uhd/stream.hpp>
#include <uhd/types/time_spec.hpp>
#include <uhd/rfnoc/constants.hpp>

namespace gr {
  namespace ettus {

    /*!
     * \brief Base class for RFNoC blocks.
     *
     * Any GNU Radio block that is meant to control an RFNoC block
     * should be derived from this class. It will do the following:
     *
     * - Instantiate a UHD block controller class internally. This
     *   exposes access to settings registers and other block configuration
     *   methods.
     * - Map the RFNoC block stream signature to GNU Radio I/O signatures.
     *   If the derived block changes the RFNoC stream signature (e.g., an
     *   FFT block might set a different FFT size than the default), you
     *   must call update_gr_io_signature() to make this change visible to
     *   GNU Radio.
     * - Set the tag propagation to TPP_DONT. This can of course be overridden
     *   in the derived block's constructor.
     */
    class ETTUS_API rfnoc_block : public gr::block
    {
     protected:
      rfnoc_block(const std::string &name); // Defined in rfnoc_block_impl.cc
      rfnoc_block() {} // For the virtual subclassing

     public:
      // Add RFNoC-relevant definitions:
      //! Allows setting a register on the settings bus
      virtual void set_register(const size_t reg, const uint32_t value, const size_t port=0) = 0;
      //! Allows setting a register on the settings bus (named version)
      virtual void set_register(const std::string &reg, const uint32_t value, const size_t port=0) = 0;
      //! Allows reading a readback register on the settings bus
      virtual uint64_t get_register(const uint32_t reg, const size_t port=0) = 0;
      //! Allows reading a readback register on the settings bus (named register version)
      virtual uint64_t get_register(const std::string &reg, const size_t port=0) = 0;
      //! Return the full actual block ID of this block (e.g. 0/FFT_0)
      virtual std::string get_block_id() const = 0;

      virtual void set_arg(const std::string &key, const int val,          const size_t port = 0) = 0;
      virtual void set_arg(const std::string &key, const double val,       const size_t port = 0) = 0;
      virtual void set_arg(const std::string &key, const std::string &val, const size_t port = 0) = 0;

      virtual void set_command_time(const uhd::time_spec_t &time_spec, const size_t port = ::uhd::rfnoc::ANY_PORT) = 0;
      virtual uhd::time_spec_t get_command_time(const size_t port = 0) = 0;
      virtual void clear_command_time(const size_t port = ::uhd::rfnoc::ANY_PORT) = 0;

      /*! Specify a time stamp at which to start streaming
       *
       * This is valid for the next run only.
       */
      virtual void set_start_time(const uhd::time_spec_t &spec) = 0;

      // GNU Radio-specific overrides (defined in rfnoc_block_impl.cc)
      virtual int general_work(
          int noutput_items,
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items,
          gr_vector_void_star &output_items
       ) = 0;

      virtual bool check_topology(int ninputs, int noutputs) = 0;
      virtual bool start() = 0;
      virtual bool stop() = 0;
    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_UHD_RFNOC_BLOCK_H */
