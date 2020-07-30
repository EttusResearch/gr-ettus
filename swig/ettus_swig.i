/* -*- c++ -*- */
/*
 * Copyright 2010-2014 Free Software Foundation, Inc.
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

// Defined during configure; avoids trying to locate
// header files if UHD was not installed.
//#ifdef GR_HAVE_UHD

#define ETTUS_API

#include <uhd/version.hpp>

//suppress 319. No access specifier given for base class name (ignored).
#pragma SWIG nowarn=319

////////////////////////////////////////////////////////////////////////
// standard includes
////////////////////////////////////////////////////////////////////////

%include <std_vector.i>
%include "gnuradio.i"

//load generated python docstrings
%include "ettus_swig_doc.i"

////////////////////////////////////////////////////////////////////////
// block headers
////////////////////////////////////////////////////////////////////////
%ignore gr::uhd::device3::get_device;
%rename("set_arg_int") set_arg(const std::string&, const int, const size_t);
%rename("set_arg_double") set_arg(const std::string&, const double, const size_t);
%rename("set_arg_str") set_arg(const std::string&, const std::string&, const size_t);

%ignore gr::ettus::rfnoc_graph::create_rx_streamer;
%ignore gr::ettus::rfnoc_graph::create_tx_streamer;
%ignore gr::ettus::rfnoc_graph::get_block_ref;
%ignore gr::ettus::rfnoc_block::get_block_ref;
%ignore gr::ettus::rfnoc_block::make_block_ref;

%{
#include <uhd/types/time_spec.hpp>
#include <uhd/usrp/multi_usrp.hpp> // This conveniently includes all the things, we don't actually need multi_usrp
#include <ettus/rfnoc_graph.h>
#include <ettus/rfnoc_block_generic.h>
#include <ettus/rfnoc_block.h>
#include <ettus/rfnoc_ddc.h>
#include <ettus/rfnoc_duc.h>
#include <ettus/rfnoc_rx_radio.h>
#include <ettus/rfnoc_rx_streamer.h>
#include <ettus/rfnoc_tx_radio.h>
#include <ettus/rfnoc_tx_streamer.h>
%}

#ifdef ENABLE_FOSPHOR
%{
#include <ettus/fosphor_display.h>
%}
#endif

////////////////////////////////////////////////////////////////////////
// used types
////////////////////////////////////////////////////////////////////////

%template(uhd_string_vector_t) std::vector<std::string>;

%template(uhd_size_vector_t) std::vector<size_t>;

%include <uhd/config.hpp>

%include <uhd/utils/pimpl.hpp>

%ignore uhd::dict::operator[]; //ignore warnings about %extend
%include <uhd/types/dict.hpp>
%template(string_string_dict_t) uhd::dict<std::string, std::string>; //define after dict

%extend uhd::dict<std::string, std::string>{
    std::string __getitem__(std::string key) {return (*self)[key];}
    void __setitem__(std::string key, std::string val) {(*self)[key] = val;}
};

%include <uhd/types/device_addr.hpp>

%template(range_vector_t) std::vector<uhd::range_t>; //define before range
%include <uhd/types/ranges.hpp>

%include <uhd/types/tune_request.hpp>

%include <uhd/types/tune_result.hpp>

%include <uhd/types/time_spec.hpp>

%extend uhd::time_spec_t{
    uhd::time_spec_t __add__(const uhd::time_spec_t &what)
    {
        uhd::time_spec_t temp = *self;
        temp += what;
        return temp;
    }
    uhd::time_spec_t __sub__(const uhd::time_spec_t &what)
    {
        uhd::time_spec_t temp = *self;
        temp -= what;
        return temp;
    }
};

%template(device_addr_vector_t) std::vector<uhd::device_addr_t>;

////////////////////////////////////////////////////////////////////////
// block magic
////////////////////////////////////////////////////////////////////////
%include <ettus/rfnoc_graph.h>
%include <ettus/rfnoc_block.h>
%include <ettus/rfnoc_tx_streamer.h>
%include <ettus/rfnoc_rx_streamer.h>
%include <ettus/rfnoc_block_generic.h>
%include <ettus/rfnoc_ddc.h>
%include <ettus/rfnoc_duc.h>
%include <ettus/rfnoc_rx_radio.h>
%include <ettus/rfnoc_tx_radio.h>

%template(set_int_property) gr::ettus::rfnoc_block::set_property<int>;
%template(set_bool_property) gr::ettus::rfnoc_block::set_property<bool>;
%template(set_string_property) gr::ettus::rfnoc_block::set_property<std::string>;
%template(set_double_property) gr::ettus::rfnoc_block::set_property<double>;

%template(get_int_property) gr::ettus::rfnoc_block::get_property<int>;
%template(get_bool_property) gr::ettus::rfnoc_block::get_property<bool>;
%template(get_string_property) gr::ettus::rfnoc_block::get_property<std::string>;
%template(get_double_property) gr::ettus::rfnoc_block::get_property<double>;

GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_graph)
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_tx_streamer);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_rx_streamer);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_block_generic);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_ddc);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_duc);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_rx_radio);
GR_SWIG_BLOCK_MAGIC2(ettus, rfnoc_tx_radio);

#ifdef ENABLE_FOSPHOR
%include <ettus/fosphor_display.h>
GR_SWIG_BLOCK_MAGIC2(ettus, fosphor_display);
#endif
