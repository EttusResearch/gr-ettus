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
#include <uhd/rfnoc/graph.hpp>

//using namespace gr::uhd;
using namespace gr::ettus;

class device3_impl : public device3
{
 public:

  device3_impl(const ::uhd::device_addr_t &device_addr)
  {
    _dev = boost::dynamic_pointer_cast< ::uhd::device3 >(::uhd::device::make(device_addr));
    if (not _dev) {
      throw std::runtime_error("Could not find a generation-3 device matching device_addr.");
    }
    _dev->clear();
    _graph = _dev->create_graph("GNU Radio");
  }

  ~device3_impl()
  {
    // nop
  }

  ::uhd::device3::sptr get_device(void) { return _dev; };

  void connect(
      const std::string &block1,
      size_t src_block_port,
      const std::string block2,
      size_t dst_block_port
  ) {
    // Translate multi_block_port to actual block port
    // FIXME this should come from the map contained in the rfnoc_block base class
    if (src_block_port >= 2) src_block_port -= 2;
    if (dst_block_port >= 2) dst_block_port -= 2;

    std::cout << "Connecting " << block1 << " port " << src_block_port << " to " << block2 << " port " << dst_block_port << std::endl;
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
  ::uhd::device3::sptr _dev;
  ::uhd::rfnoc::graph::sptr _graph;

};

device3::sptr
device3::make(const ::uhd::device_addr_t &device_addr)
{
  check_abi();
  return device3::sptr( new device3_impl(device_addr) );
}

