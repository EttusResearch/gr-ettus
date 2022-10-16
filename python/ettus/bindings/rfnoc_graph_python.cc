/*
 * Copyright 2022 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(rfnoc_graph.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(6b049a0573efa7f36c2f45ddbaa40b07)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ettus/rfnoc_graph.h>
// pydoc.h is automatically generated in the build directory
#include <rfnoc_graph_pydoc.h>

void bind_rfnoc_graph(py::module& m)
{

    using rfnoc_graph = ::gr::ettus::rfnoc_graph;


    py::class_<rfnoc_graph, std::shared_ptr<rfnoc_graph>>(
        m, "rfnoc_graph", D(rfnoc_graph))

        .def(py::init(&rfnoc_graph::make), py::arg("dev_addr"), D(rfnoc_graph, make))


        .def("connect",
             (void(rfnoc_graph::*)(std::string const&,
                                   size_t const,
                                   std::string const&,
                                   size_t const,
                                   bool const)) &
                 rfnoc_graph::connect,
             py::arg("src_block"),
             py::arg("src_block_port"),
             py::arg("dst_block"),
             py::arg("dst_block_port"),
             py::arg("skip_property_propagation") = false,
             D(rfnoc_graph, connect, 0))


        .def("connect",
             (void(rfnoc_graph::*)(std::string const&, std::string const&, bool const)) &
                 rfnoc_graph::connect,
             py::arg("src_block"),
             py::arg("dst_block"),
             py::arg("skip_property_propagation") = false,
             D(rfnoc_graph, connect, 1))


        .def("create_rx_streamer",
             &rfnoc_graph::create_rx_streamer,
             py::arg("num_ports"),
             py::arg("args"),
             D(rfnoc_graph, create_rx_streamer))


        .def("create_tx_streamer",
             &rfnoc_graph::create_tx_streamer,
             py::arg("num_ports"),
             py::arg("args"),
             D(rfnoc_graph, create_tx_streamer))


        .def("commit", &rfnoc_graph::commit, D(rfnoc_graph, commit))


        .def("get_block_id",
             &rfnoc_graph::get_block_id,
             py::arg("block_name"),
             py::arg("device_select"),
             py::arg("block_select"),
             D(rfnoc_graph, get_block_id))


        .def("set_time_source",
             &rfnoc_graph::set_time_source,
             py::arg("source"),
             py::arg("mb_index"),
             D(rfnoc_graph, set_time_source))


        .def("set_clock_source",
             &rfnoc_graph::set_clock_source,
             py::arg("source"),
             py::arg("mb_index"),
             D(rfnoc_graph, set_clock_source))


        .def("get_block_ref",
             &rfnoc_graph::get_block_ref,
             py::arg("block_id"),
             py::arg("max_ref_count"),
             D(rfnoc_graph, get_block_ref))

        ;
}