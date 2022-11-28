# gr-ettus: Experimental UHD and USRP features for GNU Radio

The master branch of gr-ettus currently does not hold any code!

For regular `multi_usrp` operations, this OOT module is *not* required.

## RFNoC Support

This branch (`master`) is not required for using any RFNoC features.
Refer to the following table to
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
