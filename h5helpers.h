#include <sys/stat.h>
#include "hdf5.h"
namespace H5DataHelpers{
bool h5_file_exists(std::string filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}
bool h5_file_is_hdf5(std::string filename) {
  return (H5Fis_hdf5(filename.c_str()) > 0);
}
}
