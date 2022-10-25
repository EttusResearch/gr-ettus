# gr-ettus: Experimental UHD and USRP features for GNU Radio

This is an out-of-tree module with experimental and other features
extending gr-uhd.

For regular `multi_usrp` operations, this OOT module is *not* required.

## RFNoC

This branch (`maint-3.7`) is required to use advanced RFNoC features
from UHD 3.x together with GNU Radio 3.7. Refer to the following table to
identify the correct software packages for other versions of GNU Radio and/or
UHD.

| GNU Radio Version | UHD Version | Required Branch                          |
|-------------------|-------------|------------------------------------------|
| 3.8.x             | 4.x         | [`maint-3.8-uhd4.0`](https://github.com/EttusResearch/gr-ettus/tree/maint-3.8-uhd4.0) (this branch) |
| 3.10.x            | 4.x         | gr-ettus not required -- use gr-uhd directly!
| 3.8.x             | 3.x         | [`maint-3.8`](https://github.com/EttusResearch/gr-ettus/tree/maint-3.8) |
| 3.7.x             | 3.x         | [`maint-3.7`](https://github.com/EttusResearch/gr-ettus/tree/maint-3.7) |


Notes:
- GNU Radio 3.9 is no longer supported and therefore has no RFNoC support
- GNU Radio 3.7 does not have UHD 4 support, and there are no plans to
  support it.

## Dependencies

This OOT requires GNU Radio version 3.7.x with gr-uhd enabled.
It also requires UHD 3.14 (or a later UHD 3.x version) to be installed.
UHD 3.15 is recommended.

Support for the Qt-based Fosphor display block requires that Qt5 be
installed and must be explicitly enabled in order to be built. To
enable this support, set the ENABLE\_QT flag to ON when running
cmake.

## License

All code in this repository is licensed under the GPLv3 (see file
COPYING). Unless stated otherwise, copyright belongs to Ettus Research.

In some cases, copyright may already have been transferred to the FSF
for blocks that are planned for inclusion in gr-uhd.
