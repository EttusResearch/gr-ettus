find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_ETTUS gnuradio-ettus)

FIND_PATH(
    GR_ETTUS_INCLUDE_DIRS
    NAMES gnuradio/ettus/api.h
    HINTS $ENV{ETTUS_DIR}/include
        ${PC_ETTUS_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_ETTUS_LIBRARIES
    NAMES gnuradio-ettus
    HINTS $ENV{ETTUS_DIR}/lib
        ${PC_ETTUS_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-ettusTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_ETTUS DEFAULT_MSG GR_ETTUS_LIBRARIES GR_ETTUS_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_ETTUS_LIBRARIES GR_ETTUS_INCLUDE_DIRS)
