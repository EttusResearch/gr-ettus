# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT rfnoc_example_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT rfnoc_example_sources)

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

list(APPEND rfnoc_example_python_files
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(rfnoc_example
   ../../..
   gr::rfnoc_example
   "${rfnoc_example_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(TARGET rfnoc_example_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rfnoc_example_python>
    ${PROJECT_BINARY_DIR}/test_modules/gnuradio/rfnoc_example/
)

install(TARGETS rfnoc_example_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/rfnoc_example COMPONENT pythonapi)
