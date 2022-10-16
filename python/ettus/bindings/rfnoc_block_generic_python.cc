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
/* BINDTOOL_HEADER_FILE(rfnoc_block_generic.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(aa6bfb5fb77c52405bea9ee5309ff76f)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ettus/rfnoc_block_generic.h>
// pydoc.h is automatically generated in the build directory
#include <rfnoc_block_generic_pydoc.h>

void bind_rfnoc_block_generic(py::module& m)
{

    using rfnoc_block_generic = ::gr::ettus::rfnoc_block_generic;


    py::class_<rfnoc_block_generic,
               gr::ettus::rfnoc_block,
               std::shared_ptr<rfnoc_block_generic>>(
        m, "rfnoc_block_generic", D(rfnoc_block_generic))

        .def(py::init(&rfnoc_block_generic::make),
             py::arg("graph"),
             py::arg("block_args"),
             py::arg("block_name"),
             py::arg("device_select") = -1,
             py::arg("block_select") = -1,
             D(rfnoc_block_generic, make))


        ;
}
