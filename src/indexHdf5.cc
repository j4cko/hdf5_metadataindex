#include "indexHdf5.h"
#include "h5helpers.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include <vector>
#include <iostream>
#include <sstream>
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata) {
  std::vector<Attribute>* attrs = (std::vector<Attribute>*)opdata;
  herr_t res = 0;
  hid_t attr_id = H5Aopen(o_id, name, H5P_DEFAULT);
  hid_t dtype = H5Aget_type(attr_id);
  hid_t dspace = H5Aget_space(attr_id); //NEED TO CLOSE!
  int rank = H5Sget_simple_extent_ndims(dspace);
  int npts = H5Sget_simple_extent_npoints(dspace);
  res = H5Sclose(dspace);
  if( rank > 1 ) return -1; // no multidimensional arrays implemented...
  if( res < 0 ) { H5Tclose(dtype); H5Aclose(attr_id); return res; }
  H5T_class_t typeclass = H5Tget_class(dtype);
  std::stringstream sstr;
  if( typeclass == H5T_INTEGER ) {
    int* intbuf = new int[npts];
    res |= H5Aread(attr_id, dtype, intbuf);
    if( res != 0 ) { 
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else {
      for( auto i = 0; i < npts; i++) {
        sstr.clear(); sstr.str("");
        if( npts > 1 )
          sstr << name << "[" << i << "]";
        else sstr << name;
        attrs->push_back(Attribute(sstr.str(), (double)intbuf[i]));
      }
    }
  } else if ( typeclass == H5T_FLOAT ) {
    double* floatbuf = new double[npts];
    res |= H5Aread(attr_id, dtype, floatbuf);
    if( res != 0 ) { 
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else
      for( auto i = 0; i < npts; i++) {
        sstr.clear(); sstr.str("");
        if( npts > 1 )
          sstr << name << "[" << i << "]";
        else sstr << name;
        attrs->push_back(Attribute(sstr.str(), floatbuf[i]));
      }
  } else if ( typeclass == H5T_STRING ) {
    if( rank == 1 ) return -4; //no string array implemented.
    auto size = H5Tget_size(dtype);
    char* buf = new char[size];
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
    // avoid double slashes at the beginning (first / root elem of idxstack is
    // the file itself
    if( not elem.datasetname.empty() ) fullpath += "/" + elem.datasetname;
    std::cout << "elem: " << elem.datasetname << " from file " << elem.file.filename << std::endl;
    for( auto const & attr : elem.attributes ) {
      thisspec.attributes.push_back(attr);
    }
  }
  // give it the name of the current node:
  thisspec.datasetname = fullpath;
  thisspec.file        = idxstack.back().file;
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
  assert(not stackidx.empty());

  stackidx.push_back({attrdata, std::string(name), stackidx.back().file});

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
  int mtime = H5DataHelpers::getFileModificationTime(filename);

  //open file:
  hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if( file_id < 0 ) throw std::runtime_error("could not open file.");

  //iterate over the file, filling two index objects, the first is the
  // current stack, i.e. all elements that lead from the top (root) node to the
  // current one.
  // the second is the collection of all datasets that will be gathered into the
  // database.
  std::pair<Index,Index> data;
  data.first.push_back({{}, "", filename, mtime});
  herr_t visitres = H5Literate(file_id, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, h5_link_iterate, &data);
  H5Fclose(file_id);

  //turning some c into c++ errors:
  if( visitres == -1 ) throw std::runtime_error("no multidimensional array implemented.");
  else if( visitres == -2 ) throw std::runtime_error("unimplemented datatype.");
  else if( visitres == -3 ) throw std::runtime_error("cannot open object.");
  else if( visitres == -4 ) throw std::runtime_error("no string array implemented.");
  else if( visitres < 0 )throw std::runtime_error("other unknown error during"
                                                  "traversal of the file.");
  //returning only the datasets and not the stack (which should be empty anyway)
  return data.second;
}
void printIndex(Index const & idx, std::ostream& os) {
  for ( auto dataset : idx ) {
    os << "dataset \"" << dataset.datasetname << "\" (from file \""
      << dataset.file.filename << "\")" <<
      " has the following attributes:" << std::endl;
    for ( auto attr : dataset.attributes ) {
      os << "  - " << attr.getName() 
        << " (" << typeToString(attr.getType()) << ") = "
        << attr.getValue() << std::endl;
    }
  }
}