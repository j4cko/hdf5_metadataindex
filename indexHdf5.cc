#include "indexHdf5.h"
#include "h5helpers.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include <iostream>
#include <sstream>
void printIndex(Index const & idx, std::ostream& os) {
  for ( auto dataset : idx ) {
    os << "dataset: " << dataset.second << 
      " has the following attributes:" << std::endl;
    for ( auto attr : dataset.first ) {
      os << "  - " << attr.getName() 
        << " (" << typeToString(attr.getType()) << ")" << std::endl;
    }
  }
}

void traverseFile(hid_t const & obj_id, Index& idx) {
  // list members:
  //
  // for each group: traverse(...)
  //
  // if member == dataset:
  // add to index.
}
Index indexFile(std::string filename) {
  Index res;
  if( not H5DataHelpers::h5_file_exists(filename) ) {
    std::stringstream sstr;
    sstr << "File \"" << filename << "\" does not exist.";
    throw std::runtime_error(sstr.str());
  }
  if( not H5DataHelpers::h5_file_is_hdf5(filename) ) {
    std::stringstream sstr;
    sstr << "File \"" << filename << "\" is not a hdf5 file.";
    throw std::runtime_error(sstr.str());
  }
  hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if( file_id < 0 ) throw std::runtime_error("could not open file.");
  try {
    traverseFile(file_id, res);
  } catch (...) {
    H5Fclose(file_id);
    throw;
  }
  H5Fclose(file_id);
  return res;
}
int main(int argc, char** argv) {
  //TODO: make tests out of this:
  Value bla(0);
  std::cout << (int)bla << std::endl;
  bla = 5;
  std::cout << (int)bla << std::endl;
  bla = (int)bla + 3;
  std::cout << (int)bla << std::endl;

  Attribute attr("exampleattr", 5);
  std::cout << typeToString(attr.getType()) << std::endl;
  Attribute attr2("exampleattr2", 6.432);
  std::cout << typeToString(attr2.getType()) << (double) attr2.getValue() << std::endl;
  Index testind;
  testind.push_back({{attr, attr2}, "testpath"});
  printIndex(testind, std::cout);


  if( argc != 2 ) {
    std::cerr << "wrong number of arguments." << std::endl;
    std::abort();
  }
  
  printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
