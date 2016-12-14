/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#ifndef __HDF5READERGENERIC_H__
#define __HDF5READERGENERIC_H__
#include "hdf5.h"
#include "h5helpers.h"
#include "attributes.h" // for dsetspec / file
#include <vector>
#include <complex>
#include <string>
#include <sstream>
#include <iostream>
namespace rqcd_hdf5_reader_generic {
using namespace rqcd_file_index;
class H5ReaderGeneric{
  public:
  H5ReaderGeneric(File const & file);
  H5ReaderGeneric(std::string const & file);
  H5ReaderGeneric(DatasetSpec const & dsetspec);
  ~H5ReaderGeneric();
  H5ReaderGeneric(H5ReaderGeneric const &) = delete; // no copy,
  H5ReaderGeneric(H5ReaderGeneric && other); // just move!
  std::vector<std::complex<double>> read(DatasetSpec const & dsetspec);
  private:
  hid_t file_id;
  bool checkMtime(File const & file);
};
}
#endif
