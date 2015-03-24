# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND ettus_sources
    device3.cc
    rfnoc_block_impl.cc
    rfnoc_fir_cci_impl.cc
    rfnoc_fosphor_c_impl.cc
    rfnoc_window_cci_impl.cc
    rfnoc_radio_impl.cc
    rfnoc_generic_impl.cc
    rfnoc_vector_iir_cc_impl.cc
)

set(ettus_sources "${ettus_sources}" PARENT_SCOPE)
if(NOT ettus_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT ettus_sources)

add_library(gnuradio-ettus SHARED ${ettus_sources})
target_link_libraries(gnuradio-ettus
    ${Boost_LIBRARIES}
    ${GNURADIO_ALL_LIBRARIES}
    ${UHD_LIBRARIES}
)
set_target_properties(gnuradio-ettus PROPERTIES DEFINE_SYMBOL "gnuradio_ettus_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-ettus PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-ettus
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_ettus_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ettus.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_ettus.cc
)

add_executable(test-ettus ${test_ettus_sources})

target_link_libraries(
  test-ettus
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-ettus
)

GR_ADD_TEST(test_ettus test-ettus)
