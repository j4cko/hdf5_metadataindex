#include "indexHdf5.h"
#include "h5helpers.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include <vector>
#include <iostream>
#include <sstream>
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata) {
  std::vector<Attribute>* attrs = (std::vector<Attribute>*)opdata;
  hid_t attr_id = H5Aopen(o_id, name, H5P_DEFAULT);
  hid_t dtype = H5Aget_type(attr_id);
  H5T_class_t typeclass = H5Tget_class(dtype);
  hid_t memtype = -1;
  int intbuf; double floatbuf;
  herr_t res = 0;
  if( typeclass == H5T_INTEGER ) {
    memtype = H5Tcopy(H5T_NATIVE_INT);
    res |= H5Aread(attr_id, memtype, &intbuf);
    res |= H5Tclose(memtype);
    if( res != 0 ) { 
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else
      attrs->push_back(Attribute(name, (double)intbuf));
  } else if ( typeclass == H5T_FLOAT ) {
    memtype = H5Tcopy(/*H5T_IEEE_F64LE*/ H5T_NATIVE_DOUBLE);
    res |= H5Aread(attr_id, memtype, &floatbuf);
    res |= H5Tclose(memtype);
    if( res != 0 ) { 
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else
    attrs->push_back(Attribute(name, floatbuf));
  } else if ( typeclass == H5T_STRING ) {
    auto size = H5Tget_size(dtype);
    char* buf = new char[size*sizeof(char)];
    res = H5Aread(attr_id, dtype, buf);
    if( res != 0 ) { 
      delete[] buf;
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else {
      attrs->push_back(Attribute(name, std::string(buf)));
      delete[] buf;
    }
  } else {
    H5Tclose(dtype);
    H5Aclose(attr_id);
    return -2;
  }
  res = H5Tclose(dtype);
  res &= H5Aclose(attr_id);
  return 0;
}
DatasetSpec processvector( Index const & idxstack ) {
  //copy all attributes along the hierarchical way from the root node to the
  //current node into thisspec:
  DatasetSpec thisspec;
  std::string fullpath("");
  for( auto const & elem : idxstack) {
    fullpath += "/" + elem.second;
    for( auto const & attr : elem.first ) {
      thisspec.first.push_back(attr);
    }
  }
  // give it the name of the current node:
  thisspec.second = fullpath;
  return thisspec;
}
herr_t h5_link_iterate( hid_t thislink, const char *name, const H5L_info_t *info, void *opdata) {
  //make the data easier accessible:
  std::pair<Index,Index>* data = (std::pair<Index,Index>*) opdata;
  Index & stackidx = data->first;
  Index & dsetidx = data->second;
  //
  //get all attributes of the current link:
  std::vector<Attribute> attrdata;
  hid_t obj_open_id = H5Oopen(thislink, name, H5P_DEFAULT);
  if( obj_open_id <= 0 ) return -3; //cannot open object
  herr_t res = H5Aiterate(obj_open_id, H5_INDEX_NAME, H5_ITER_NATIVE, 
                           NULL, h5_attr_iterate, &attrdata);
  if( res < 0 ) return res;
  res = H5Oclose(obj_open_id);
  if( res < 0 ) return res;
  //
  //save name and attributes on the stack index:
  stackidx.push_back({attrdata, std::string(name)});

  //find out the type of the object:
  H5O_info_t objinfo;
  res = H5Oget_info_by_name(thislink, name, &objinfo, H5P_DEFAULT);
  if( res < 0 ) return res;
  if( objinfo.type == H5O_TYPE_GROUP ){
    //if it is a group: further recursion into subgroup:
    hid_t groupId = H5Gopen(thislink, name, H5P_DEFAULT);
    res = H5Literate(groupId, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, h5_link_iterate, opdata);
    H5Gclose(groupId);
    if( res < 0 ) return res;
    stackidx.pop_back();
  }
  else if( objinfo.type == H5O_TYPE_DATASET ) {
    // if it is a dataset: end of recursion: 
    // work back through the vector... and store in index:
    dsetidx.push_back(processvector(stackidx));
    stackidx.pop_back();
  }
  else return -2; //unimplemented datatype
  return 0;
}
Index indexFile(std::string filename) {
  //checks:
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

  //open file:
  hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if( file_id < 0 ) throw std::runtime_error("could not open file.");

  //iterate over the file, filling two index objects, the first is the
  // current stack, i.e. all elements that lead from the top (root) node to the
  // current one.
  // the second is the collection of all datasets that will be gathered into the
  // database.
  std::pair<Index,Index> data;
  herr_t visitres = H5Literate(file_id, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, h5_link_iterate, &data);
  H5Fclose(file_id);

  //turning some c into c++ errors:
  if( visitres == -2 ) throw std::runtime_error("unimplemented datatype.");
  else if( visitres == -3 ) throw std::runtime_error("cannot open object.");
  else if( visitres < 0 )throw std::runtime_error("other unknown error during"
                                                  "traversal of the file.");
  //returning only the datasets and not the stack (which should be empty anyway)
  return data.second;
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
