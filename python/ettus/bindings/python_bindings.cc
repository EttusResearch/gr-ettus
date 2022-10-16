/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

namespace py = pybind11;

// Headers for binding functions
/**************************************/
// The following comment block is used for
// gr_modtool to insert function prototypes
// Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
#ifdef ENABLE_QT
void bind_fosphor_display(py::module& m);
#endif
void bind_rfnoc_block(py::module& m);
void bind_rfnoc_block_generic(py::module& m);
void bind_rfnoc_ddc(py::module& m);
void bind_rfnoc_duc(py::module& m);
void bind_rfnoc_fir_filter(py::module& m);
void bind_rfnoc_graph(py::module& m);
void bind_rfnoc_rx_radio(py::module& m);
void bind_rfnoc_rx_streamer(py::module& m);
void bind_rfnoc_siggen(py::module& m);
void bind_rfnoc_tx_radio(py::module& m);
void bind_rfnoc_tx_streamer(py::module& m);
void bind_rfnoc_window(py::module& m);
// ) END BINDING_FUNCTION_PROTOTYPES


// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(ettus_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");

    /**************************************/
    // The following comment block is used for
    // gr_modtool to insert binding function calls
    // Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    #ifdef ENABLE_QT
    bind_fosphor_display(m);
    #endif
    bind_rfnoc_block(m);
    bind_rfnoc_block_generic(m);
    bind_rfnoc_ddc(m);
    bind_rfnoc_duc(m);
    bind_rfnoc_fir_filter(m);
    bind_rfnoc_graph(m);
    bind_rfnoc_rx_radio(m);
    bind_rfnoc_rx_streamer(m);
    bind_rfnoc_siggen(m);
    bind_rfnoc_tx_radio(m);
    bind_rfnoc_tx_streamer(m);
    bind_rfnoc_window(m);
    // ) END BINDING_FUNCTION_CALLS
}
