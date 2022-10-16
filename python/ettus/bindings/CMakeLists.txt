# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT ettus_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT ettus_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND ettus_python_files
        rfnoc_block_generic_python.cc
        rfnoc_block_python.cc
        rfnoc_ddc_python.cc
        rfnoc_duc_python.cc
        rfnoc_fir_filter_python.cc
        rfnoc_graph_python.cc
        rfnoc_rx_radio_python.cc
        rfnoc_rx_streamer_python.cc
        rfnoc_siggen_python.cc
        rfnoc_tx_radio_python.cc
        rfnoc_tx_streamer_python.cc
        rfnoc_window_python.cc
    python_bindings.cc)

if(ENABLE_QT)
list(APPEND ettus_python_files
        fosphor_display_python.cc)
endif(ENABLE_QT)

GR_PYBIND_MAKE_OOT(ettus
   ../../..
   gr::ettus
   "${ettus_python_files}")

# copy bindings extension for use in QA test module
#add_custom_command(TARGET ettus_python POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ettus_python>
#    ${PROJECT_BINARY_DIR}/test_modules/gnuradio/ettus/
#)

install(TARGETS ettus_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/ettus COMPONENT pythonapi)
