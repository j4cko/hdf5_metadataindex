#ifndef __H5HELPERS_H__
#define __H5HELPERS_H__
#include <sys/stat.h>
#include "hdf5.h"
#include <string>
/*
 * DISCLAIMER:
 * This file is highly non-portable and probably works only on POSIX / *nix
 * systems.
 */
namespace H5DataHelpers{
bool h5_file_exists(std::string filename);
bool h5_file_is_hdf5(std::string filename);
int getFileModificationTime(std::string filename);
}
#endif
