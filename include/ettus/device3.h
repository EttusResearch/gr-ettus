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

#ifndef INCLUDED_UHD_DEVICE3_H
#define INCLUDED_UHD_DEVICE3_H

//#include <gnuradio/uhd/api.h>
#include <ettus/api.h>
#include <uhd/types/device_addr.hpp>

#include <uhd/device3.hpp>
//#ifndef INCLUDED_UHD_USRP_MULTI_USRP_HPP
// namespace uhd {
// namespace usrp {
// class multi_usrp;
//}
//}
//#endif

namespace gr {
// namespace uhd {
namespace ettus {

/*!
 * \brief A representation of a generation 3 USRP device.
 *
 * The typical use case for this is in RFNoC flow graphs. Unlike
 * flow graphs with a typical USRP source and/or sink, we may have
 * many blocks that all use the same UHD device instantiation. This
 * class allows this UHD device to be shared between blocks.
 */
// class UHD_API device3
class ETTUS_API device3
{
public:
    typedef boost::shared_ptr<device3> sptr;
    virtual ~device3(){};

    //! Return a pointer to the underlying multi_usrp device
    virtual boost::shared_ptr<::uhd::device3> get_device(void) = 0;

    virtual void connect(const std::string& block1,
                         size_t src_block_port,
                         const std::string block2,
                         size_t dst_block_port) = 0;
    virtual void connect(const std::string& block1, const std::string block2) = 0;

    static sptr make(const ::uhd::device_addr_t& device_addr);
};

} // namespace ettus
} // namespace gr

#endif /* INCLUDED_UHD_DEVICE3_H */
