#include "indexHdf5.h"
#include "h5helpers.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include <iostream>
#include <sstream>
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata) {
  std::vector<Attribute>* attrs = (std::vector<Attribute>*)opdata;
  hid_t attr_id = H5Aopen(o_id, name, H5P_DEFAULT);
  hid_t dtype = H5Aget_type(attr_id);
  H5T_class_t typeclass = H5Tget_class(dtype);
  hid_t memtype = -1;
  int intbuf; double floatbuf;
  switch( typeclass ) {
    case H5T_INTEGER:
      memtype = H5Tcopy(H5T_NATIVE_INT);
      H5Aread(attr_id, memtype, &intbuf);
      H5Tclose(memtype);
      attrs->push_back(Attribute(name, intbuf));
      break;
    case H5T_FLOAT:
      memtype = H5Tcopy(/*H5T_IEEE_F64LE*/ H5T_NATIVE_DOUBLE);
      H5Aread(attr_id, memtype, &floatbuf);
      H5Tclose(memtype);
      attrs->push_back(Attribute(name, floatbuf));
      break;
    case H5T_STRING:
      //should not throw from here.... therefore:
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return -1;
      break;
    default:
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return -2;
  }
  herr_t res = H5Tclose(dtype);
  res &= H5Aclose(attr_id);
  return 0;
}
herr_t h5_obj_iterate( hid_t o_id, const char *name, const H5O_info_t *objinfo, void *opdata) {
  Index* idx = (Index*)opdata;

  if( std::string(name) == ".") return 0; // ignore the root node

  std::vector<Attribute> attrs;
  hid_t obj_open_id = H5Oopen(o_id, name, H5P_DEFAULT);
  if( obj_open_id <= 0 ) throw std::runtime_error("cannot open object.");
  herr_t res = H5Aiterate2(obj_open_id, H5_INDEX_NAME, H5_ITER_NATIVE, 
                           NULL, h5_attr_iterate, &attrs);
  if( res < 0 ) return res;
  res = H5Oclose(obj_open_id);
  if( res < 0 ) return res;
  idx->push_back({attrs, std::string(name)});
  return 0;
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
  herr_t visitres = H5Ovisit(file_id, H5_INDEX_NAME, H5_ITER_NATIVE, 
                        h5_obj_iterate, &res);
  H5Fclose(file_id);
  if( visitres == -1 ) throw std::runtime_error("strings are not implemented.");
  else if( visitres == -2 ) throw std::runtime_error("unimplemented datatype.");
  else if( visitres < 0 )throw std::runtime_error("other unknown error during"
                                                  "traversal of the file.");
  return res;
}
void printIndex(Index const & idx, std::ostream& os) {
  for ( auto dataset : idx ) {
    os << "dataset: " << dataset.second << 
      " has the following attributes:" << std::endl;
    for ( auto attr : dataset.first ) {
      os << "  - " << attr.getName() 
        << " (" << typeToString(attr.getType()) << ") = "
        << attr.getValue() << std::endl;
    }
  }
}
