# csread

C++ library for reading cross sections generated using the [cstool project](https://github.com/eScatter/cstool) from HDF5 files. It is maintained as a separate project because it is used by more than one simulator.

## Building

Dependencies:
* C++11 compiler, gcc 4.8.4 and later are known to work
* The C++ component of the [HDF5 libraries](https://www.hdfgroup.org/downloads/hdf5/), version 1.10 or greater.

This project is not supposed to be built on its own. It creates a `csread` library meant to be used by CMake projects.

If CMake cannot find HDF5 libraries or complains about an old version, please get the latest version [here](https://www.hdfgroup.org/downloads/hdf5/). Be sure to compile with C++ enabled (`--enable-cxx`). If CMake still cannot find the libraries, their path can be provided with the `-DHDF5_ROOT=/your/path/` command-line option. Be sure to clear the cache!

