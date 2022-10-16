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
/* BINDTOOL_HEADER_FILE(rfnoc_siggen.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(b94b4d1c3e58e4f2804598a5509e9767)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ettus/rfnoc_siggen.h>
// pydoc.h is automatically generated in the build directory
#include <rfnoc_siggen_pydoc.h>

void bind_rfnoc_siggen(py::module& m)
{

    using rfnoc_siggen = ::gr::ettus::rfnoc_siggen;


    py::class_<rfnoc_siggen, gr::ettus::rfnoc_block, std::shared_ptr<rfnoc_siggen>>(
        m, "rfnoc_siggen", D(rfnoc_siggen))

        .def(py::init(&rfnoc_siggen::make),
             py::arg("graph"),
             py::arg("block_args"),
             py::arg("device_select"),
             py::arg("instance"),
             D(rfnoc_siggen, make))


        .def("set_amplitude",
             &rfnoc_siggen::set_amplitude,
             py::arg("amplitude"),
             py::arg("chan"),
             D(rfnoc_siggen, set_amplitude))


        .def("get_amplitude",
             &rfnoc_siggen::get_amplitude,
             py::arg("chan"),
             D(rfnoc_siggen, get_amplitude))


        .def("set_constant",
             &rfnoc_siggen::set_constant,
             py::arg("constant"),
             py::arg("chan"),
             D(rfnoc_siggen, set_constant))


        .def("get_constant",
             &rfnoc_siggen::get_constant,
             py::arg("chan"),
             D(rfnoc_siggen, get_constant))


        .def("set_enable",
             &rfnoc_siggen::set_enable,
             py::arg("enable"),
             py::arg("chan"),
             D(rfnoc_siggen, set_enable))


        .def("get_enable",
             &rfnoc_siggen::get_enable,
             py::arg("chan"),
             D(rfnoc_siggen, get_enable))


        .def("set_sine_frequency",
             &rfnoc_siggen::set_sine_frequency,
             py::arg("frequency"),
             py::arg("sample_rate"),
             py::arg("chan"),
             D(rfnoc_siggen, set_sine_frequency))


        .def("set_sine_phase_increment",
             &rfnoc_siggen::set_sine_phase_increment,
             py::arg("phase_inc"),
             py::arg("chan"),
             D(rfnoc_siggen, set_sine_phase_increment))


        .def("get_sine_phase_increment",
             &rfnoc_siggen::get_sine_phase_increment,
             py::arg("chan"),
             D(rfnoc_siggen, get_sine_phase_increment))


        .def("set_waveform",
             (void(rfnoc_siggen::*)(uhd::rfnoc::siggen_waveform const, size_t const)) &
                 rfnoc_siggen::set_waveform,
             py::arg("type"),
             py::arg("chan"),
             D(rfnoc_siggen, set_waveform, 0))


        .def("set_waveform",
             (void(rfnoc_siggen::*)(std::string const&, size_t const)) &
                 rfnoc_siggen::set_waveform,
             py::arg("type"),
             py::arg("chan"),
             D(rfnoc_siggen, set_waveform, 1))


        .def("get_waveform_string",
             &rfnoc_siggen::get_waveform_string,
             py::arg("chan"),
             D(rfnoc_siggen, get_waveform_string))


        .def("get_waveform",
             &rfnoc_siggen::get_waveform,
             py::arg("chan"),
             D(rfnoc_siggen, get_waveform))


        .def("set_samples_per_packet",
             &rfnoc_siggen::set_samples_per_packet,
             py::arg("spp"),
             py::arg("chan"),
             D(rfnoc_siggen, set_samples_per_packet))


        .def("get_samples_per_packet",
             &rfnoc_siggen::get_samples_per_packet,
             py::arg("chan"),
             D(rfnoc_siggen, get_samples_per_packet))

        ;
}