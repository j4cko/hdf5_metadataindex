/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "filehelpers.h"
#include "h5helpers.h"

bool H5DataHelpers::h5_file_exists(std::string filename) {
  return FileHelpers::file_exists(filename);
}
bool H5DataHelpers::h5_file_is_hdf5(std::string filename) {
  return (H5Fis_hdf5(filename.c_str()) > 0);
}
int H5DataHelpers::getFileModificationTime(std::string filename) {
  return FileHelpers::getFileModificationTime(filename);
}
