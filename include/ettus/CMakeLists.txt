# Copyright 2011,2013 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install header files
########################################################################
install(FILES
    api.h
    rfnoc_block.h
    rfnoc_block_generic.h
    rfnoc_ddc.h
    rfnoc_duc.h
    rfnoc_graph.h
    rfnoc_rx_radio.h
    rfnoc_rx_streamer.h
    rfnoc_tx_radio.h
    rfnoc_tx_streamer.h
    DESTINATION include/ettus
)

if (ENABLE_FOSPHOR)
    install(FILES
        fosphor_display.h
        DESTINATION include/ettus
    )
endif (ENABLE_FOSPHOR)

