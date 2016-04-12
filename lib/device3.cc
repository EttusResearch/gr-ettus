/* -*- c++ -*- */
/* 
 * Copyright 2014 Free Software Foundation, Inc.
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

#include <ettus/device3.h>
#include <gnuradio/io_signature.h>
#include "gr_uhd_common.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/rfnoc/graph.hpp>

//using namespace gr::uhd;
using namespace gr::ettus;

class device3_impl : public device3
{
 public:

  device3_impl(const ::uhd::device_addr_t &device_addr)
  {
    _dev = ::uhd::usrp::multi_usrp::make(device_addr);
    if (not _dev->is_device3()) {
      throw std::runtime_error("Device is not a generation-3 device.");
    }
    _dev->get_device3()->clear();
    _graph = _dev->get_device3()->create_graph("GNU Radio");
  }

  ~device3_impl()
  {
    // nop
  }

  ::uhd::usrp::multi_usrp::sptr get_device(void) { return _dev; };

  void connect(
      const std::string &block1,
      size_t src_block_port,
      const std::string block2,
      size_t dst_block_port
  ) {
    _graph->connect(
        ::uhd::rfnoc::block_id_t(block1),
        src_block_port,
        ::uhd::rfnoc::block_id_t(block2),
        dst_block_port
    );
  }

  void connect(
      const std::string &block1,
      const std::string block2
  ) {
    _graph->connect(
        ::uhd::rfnoc::block_id_t(block1),
        ::uhd::rfnoc::block_id_t(block2)
    );
  }

 private:
  ::uhd::usrp::multi_usrp::sptr _dev;
  ::uhd::rfnoc::graph::sptr _graph;

};

device3::sptr
device3::make(const ::uhd::device_addr_t &device_addr)
{
  check_abi();
  return device3::sptr( new device3_impl(device_addr) );
}

