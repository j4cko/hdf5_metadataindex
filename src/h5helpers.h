/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#ifndef __H5HELPERS_H__
#define __H5HELPERS_H__
#include "hdf5.h"
#include <string>
namespace H5DataHelpers{
bool h5_file_exists(std::string filename);
bool h5_file_is_hdf5(std::string filename);
int getFileModificationTime(std::string filename);
}
#endif
