/* -*- c++ -*- */
/*
 * Copyright 2015 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INCLUDED_UHD_RFNOC_COMMON_H
#define INCLUDED_UHD_RFNOC_COMMON_H

#include <ettus/device3.h>
#include <gnuradio/types.h>
#include <gnuradio/logger.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <uhd/device3.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/atomic.hpp>
#include <uhd/usrp/rfnoc/block_ctrl_base.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <map>

// Add this to the end of your class declaration:
#define GR_RFNOC_BLOCK_IMPL_H() \
     public: \
      int general_work( \
          int noutput_items, \
          gr_vector_int &ninput_items, \
          gr_vector_const_void_star &input_items, \
          gr_vector_void_star &output_items \
      ) { \
        return d_rfnoccer->general_work( \
            noutput_items, \
            ninput_items, \
            input_items, \
            output_items \
        ); \
      } \
      bool check_topology(int ninputs, int noutputs) { return d_rfnoccer->check_topology(ninputs, noutputs); } \
      bool start() { return d_rfnoccer->start(detail()->ninputs(), detail()->noutputs()); } \
      bool stop() { return d_rfnoccer->stop(); } \
      void set_register(size_t reg, boost::uint32_t value) { d_rfnoccer->get_block_ctrl()->sr_write(reg, value); } \
      std::string get_block_id() { return d_rfnoccer->get_block_ctrl()->get_block_id().get(); } \
      ::uhd::usrp::multi_usrp::sptr get_device() const { return d_rfnoccer->get_device(); }; \
      boost::shared_ptr< ::uhd::rfnoc::block_ctrl_base > get_block_ctrl() const { return d_rfnoccer->get_block_ctrl(); }; \
      template <typename T> boost::shared_ptr<T> get_block_ctrl() const { return d_rfnoccer->get_block_ctrl<T>(); } \
      template <typename T> boost::shared_ptr<T> get_block_ctrl_throw() const { return d_rfnoccer->get_block_ctrl_throw<T>(); } \
     private: \
      rfnoc::rfnoc_common::sptr d_rfnoccer;

// Have your ctor call this ctor macro:
#define GR_RFNOC_BLOCK_SUPER_CTOR(name) \
    gr::block(name, \
          /* Default IO signatures: These will be overridden in the constructor */ \
          gr::io_signature::make(0, 0, 1), \
          gr::io_signature::make(0, 0, 1) \
       )

// Plonk this into your constructor:
#define GR_RFNOC_BLOCK_INIT(dev, block_id, tx_stream_args, rx_stream_args) \
      /***** Set up block control ********************************/ \
      d_rfnoccer = rfnoc::rfnoc_common::sptr(new rfnoc::rfnoc_common( \
          dev, block_id, \
          tx_stream_args, rx_stream_args, \
          boost::bind(&gr::block::consume,      this, _1, _2), \
          boost::bind(&gr::block::consume_each, this, _1), \
          boost::bind(&gr::block::produce,      this, _1, _2) \
      )); \
      /***** Finalize I/O signatures and configure GR block ******/ \
      set_input_signature(d_rfnoccer->get_input_signature()); \
      set_output_signature(d_rfnoccer->get_output_signature()); \
      set_tag_propagation_policy(TPP_DONT); \

#define GR_RFNOC_BLOCK_INIT_CFG(dev, block_id, tx_stream_args, rx_stream_args, cfg_command) \
      /***** Set up block control ********************************/ \
      d_rfnoccer = rfnoc::rfnoc_common::sptr(new rfnoc::rfnoc_common( \
          dev, block_id, \
          tx_stream_args, rx_stream_args, \
          boost::bind(&gr::block::consume,      this, _1, _2), \
          boost::bind(&gr::block::consume_each, this, _1), \
          boost::bind(&gr::block::produce,      this, _1, _2) \
      )); \
      /***** Run configuration command ***************************/ \
      cfg_command ; \
      d_rfnoccer->update_io_signature(); \
      /***** Finalize I/O signatures and configure GR block ******/ \
      set_input_signature(d_rfnoccer->get_input_signature()); \
      set_output_signature(d_rfnoccer->get_output_signature()); \
      set_tag_propagation_policy(TPP_DONT); \

namespace gr {
  //namespace uhd {
  namespace ettus {
    namespace rfnoc {
      class ETTUS_API rfnoc_common {
        public:
          /*********************************************************************
           * Types
           *********************************************************************/
          typedef boost::shared_ptr<rfnoc_common> sptr;
          typedef boost::function<void(int, int)> block_func_t;
          typedef boost::function<void(int)> block_func1_t;

          /*********************************************************************
           * Statics
           *********************************************************************/
          static std::string make_block_id(
              const std::string &block_name,
              const int block_select=-1,
              const int device_select=-1
          );

          /*********************************************************************
           * Structors
           *********************************************************************/
          rfnoc_common(
              const device3::sptr &dev,
              const std::string &block_id,
              const ::uhd::stream_args_t &tx_stream_args,
              const ::uhd::stream_args_t &rx_stream_args,
              //gr::logger_ptr logger,
              block_func_t consumer, block_func1_t consumer_each,
              block_func_t producer
          );
          ~rfnoc_common();

          /*********************************************************************
           * GR Block functions
           *********************************************************************/
          bool check_topology(int ninputs, int noutputs);
          bool start(size_t ninputs, size_t noutputs);
          bool stop();

          int general_work(
              int noutput_items,
              gr_vector_int &ninput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
          );

          gr::io_signature::sptr get_input_signature();
          gr::io_signature::sptr get_output_signature();

          //! Call this after a command you called changes the I/O signature
          void update_io_signature();

          /*********************************************************************
           * RFNoC block related functions.
           *********************************************************************/
          ::uhd::usrp::multi_usrp::sptr get_device() const { return _dev; };

          //! \returns Block ID in string format.
          std::string get_block_id() const { return _blk_ctrl->get_block_id(); };

          //! Returns a shared pointer to the block control
          boost::shared_ptr< ::uhd::rfnoc::block_ctrl_base > get_block_ctrl() const { return _blk_ctrl; };

          //! Returns a shared pointer to the block control (templated version)
          //
          // Use this to get access to derived block control types.
          template <typename T>
          boost::shared_ptr<T> get_block_ctrl() const
          {
            return boost::dynamic_pointer_cast<T>(get_block_ctrl());
          }

          //! Like get_block_ctrl(), but throws a std::runtime_error if the conversion
          // fails.
          template <typename T>
          boost::shared_ptr<T> get_block_ctrl_throw() const
          {
            boost::shared_ptr<T> the_sptr = boost::dynamic_pointer_cast<T>(get_block_ctrl());
            if (!the_sptr) {
              throw std::runtime_error("Block control is not of requested type.");
            }
            return the_sptr;
          }

        private:

          /*********************************************************************
           * Workers and Helpers
           *********************************************************************/
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

          void flush(size_t streamer_index);

          /*********************************************************************
           * Private attributes
           *********************************************************************/
          /*** Device and block controls ***********************/
          ::uhd::usrp::multi_usrp::sptr       _dev;
          ::uhd::rfnoc::block_ctrl_base::sptr _blk_ctrl;
          ::uhd::device_addr_t                _merged_args;

          template <typename T_streamer, typename T_md>
          struct stream_info {
            ::uhd::stream_args_t stream_args;
            size_t nchans;
            //! Used by GNU Radio, not necessarily RFNoC
            size_t vlen;
            //! Size of one OTW item, regardless of vlen
            size_t itemsize;
            bool align;
            T_md metadata;
            //! For aligned streaming this vector has length 1. Otherwise,
            // it holds one streamer per port.
            std::vector<T_streamer> streamers;

            stream_info(const ::uhd::stream_args_t &stream_args_) :
              stream_args(stream_args_),
              nchans(stream_args_.channels.size()),
              vlen(1),
              //align(stream_args_.channels.size() > 1 && stream_args_.args.cast<bool>("align", false))
              align(stream_args_.args.cast<bool>("align", false))
            {}
          };

          /*** TX **********************************************/
          stream_info< ::uhd::tx_streamer::sptr, ::uhd::tx_metadata_t > _tx;
          block_func_t   _consume;
          block_func1_t  _consume_each;

          /*** RX **********************************************/
          stream_info< ::uhd::rx_streamer::sptr, ::uhd::rx_metadata_t > _rx;
          block_func_t   _produce;

          /*** Multi-Streamer Sync and concurrency stuff ********/
          boost::recursive_mutex d_mutex;
          //! Counts the number of instantiations of this block
          // that actually have their own streamers
          static std::map<std::string, bool> _active_streamers;
          static ::uhd::reusable_barrier _tx_barrier;
          static ::uhd::reusable_barrier _rx_barrier;
          static boost::recursive_mutex s_setup_mutex;

      }; // class rfnoc_common

    } // namespace rfnoc
  } // namespace uhd
} // namespace gr

#endif /* INCLUDED_UHD_RFNOC_COMMON_H */
