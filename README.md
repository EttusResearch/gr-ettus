# gr-ettus: Experimental UHD and USRP features for GNU Radio

This is an out-of-tree module with experimental and other features
extending gr-uhd.

For regular USRP operations, this OOT module is *not* required.

## Dependencies

This OOT requires GNU Radio version 3.8 with gr-uhd enabled.
It also requires UHD 4.0 to be installed.

Support for the Qt-based Fosphor display block requires that Qt5 be
installed and must be explicitly enabled in order to be built. To
enable this support, set the ENABLE\_QT flag to ON when running
cmake.

## RFNoC

Currently, this OOT is required to run RFNoC with GNU Radio.

## License

All code in this repository is licensed under the GPLv3 (see file
COPYING). Unless stated otherwise, copyright belongs to Ettus Research.

In some cases, copyright may already have been transferred to the FSF
for blocks that are planned for inclusion in gr-uhd.
