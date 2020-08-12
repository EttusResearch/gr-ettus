/* -*- c++ -*- */

#define RFNOC_EXAMPLE_API
#define ETTUS_API

%include "gnuradio.i"/*			*/// the common stuff

//load generated python docstrings
%include "rfnoc_example_swig_doc.i"
//Header from gr-ettus

%ignore gr::ettus::rfnoc_graph::create_rx_streamer;
%ignore gr::ettus::rfnoc_graph::create_tx_streamer;
%ignore gr::ettus::rfnoc_graph::get_block_ref;
%ignore gr::ettus::rfnoc_block::get_block_ref;
%ignore gr::ettus::rfnoc_block::make_block_ref;

%include <ettus/rfnoc_graph.h>
%include <ettus/rfnoc_block.h>
%include <ettus/rfnoc_block_generic.h>

%template(set_int_property) gr::ettus::rfnoc_block::set_property<int>;
%template(set_bool_property) gr::ettus::rfnoc_block::set_property<bool>;
%template(set_string_property) gr::ettus::rfnoc_block::set_property<std::string>;
%template(set_double_property) gr::ettus::rfnoc_block::set_property<double>;

%template(get_int_property) gr::ettus::rfnoc_block::get_property<int>;
%template(get_bool_property) gr::ettus::rfnoc_block::get_property<bool>;
%template(get_string_property) gr::ettus::rfnoc_block::get_property<std::string>;
%template(get_double_property) gr::ettus::rfnoc_block::get_property<double>;

%{
#include <uhd/types/time_spec.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <ettus/rfnoc_graph.h>
#include <ettus/rfnoc_block.h>
#include <ettus/rfnoc_block_generic.h>
%}

