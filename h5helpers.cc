#include "h5helpers.h"

bool H5DataHelpers::h5_file_exists(std::string filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}
bool H5DataHelpers::h5_file_is_hdf5(std::string filename) {
  return (H5Fis_hdf5(filename.c_str()) > 0);
}
int H5DataHelpers::getFileModificationTime(std::string filename) {
  struct stat buffer ;
  stat(filename.c_str(), &buffer);
  return buffer.st_mtime;
}
