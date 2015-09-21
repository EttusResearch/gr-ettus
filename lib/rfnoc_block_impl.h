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

#ifndef INCLUDED_ETTUS_RFNOC_BLOCK_IMPL_H
#define INCLUDED_ETTUS_RFNOC_BLOCK_IMPL_H

#include <ettus/rfnoc_block.h>
#include <uhd/device3.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/atomic.hpp>
#include <uhd/rfnoc/block_ctrl_base.hpp>
#include <uhd/rfnoc/sink_block_ctrl_base.hpp>
#include <uhd/rfnoc/source_block_ctrl_base.hpp>
#include <uhd/convert.hpp>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <map>

static const pmt::pmt_t EOB_KEY = pmt::string_to_symbol("rx_eob");

namespace gr {
  namespace ettus {

    class rfnoc_block_impl : virtual public rfnoc_block
    {
     public:
      /*********************************************************************
       * Statics
       *********************************************************************/
      static std::string make_block_id(
          const std::string &block_name,
          const int block_select=-1,
          const int device_select=-1
      );

      /*********************************************************************
       * GR Block functions
       *********************************************************************/
      int general_work(
          int noutput_items,
          gr_vector_int &ninput_items,
          gr_vector_const_void_star &input_items,
          gr_vector_void_star &output_items
       );

      bool check_topology(int ninputs, int noutputs);
      bool start();
      bool stop();

      /**********************************************************************
       * Structors
       *********************************************************************/
      virtual ~rfnoc_block_impl();
     protected:
      rfnoc_block_impl(
              const device3::sptr &dev,
              const std::string &block_id,
              const ::uhd::stream_args_t &tx_stream_args,
              const ::uhd::stream_args_t &rx_stream_args
      );

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
      // The return pointer will be null if the block was not of type T.
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

      void set_register(size_t reg, boost::uint32_t value) { _blk_ctrl->sr_write(reg, value); }

      void set_arg(const std::string &key, const int val, const size_t port = 0)
      {
          _blk_ctrl->set_arg<int>(key, val, port);
      }

      void set_arg(const std::string &key, const double val, const size_t port = 0)
      {
          _blk_ctrl->set_arg<double>(key, val, port);
      }

      void set_arg(const std::string &key, const std::string &val, const size_t port = 0)
      {
          _blk_ctrl->set_arg<std::string>(key, val, port);
      }

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
       * I/O Signatures
       ********************************************************************/
      //! Queries the RFNoC block for its current state and updates the
      //  cached I/O signatures. This will update _tx and _rx.
      void update_rfnoc_io_signature();

      //! Runs set_input_signature() and set_output_signature() using the current
      //  RFNoC stream signatures. This method can *only* be called inside the constructor.
      //  Calls update_rfnoc_io_signature() internally.
      void update_gr_io_signature();

      //! Translate the RFNoC block's input signature into a GNU Radio input signature
      gr::io_signature::sptr get_rfnoc_input_signature();
      //! Translate the RFNoC block's output signature into a GNU Radio output signature
      gr::io_signature::sptr get_rfnoc_output_signature();

     private:
      /*********************************************************************
       * Private attributes
       ********************************************************************/
      /*** Device and block controls ***********************/
      ::uhd::usrp::multi_usrp::sptr       _dev;
      ::uhd::rfnoc::block_ctrl_base::sptr _blk_ctrl;
      ::uhd::device_addr_t                _merged_args;

      /*** Stream info type ********************************/
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

      /*** RX **********************************************/
      stream_info< ::uhd::rx_streamer::sptr, ::uhd::rx_metadata_t > _rx;

      /*** Multi-Streamer Sync and concurrency stuff ********/
      boost::recursive_mutex d_mutex;
      //! Counts the number of instantiations of this block
      // that actually have their own streamers
      static std::map<std::string, bool> _active_streamers;
      static ::uhd::reusable_barrier _tx_barrier;
      static ::uhd::reusable_barrier _rx_barrier;
      static boost::recursive_mutex s_setup_mutex;

    };

  } // namespace ettus
} // namespace gr

#endif /* INCLUDED_ETTUS_RFNOC_BLOCK_IMPL_H */
