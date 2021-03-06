/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "indexHdf5.h"
#include "h5helpers.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>

//defines maximum char length for field names in tables.
#define CHARMAX 1024

namespace rqcd_file_index {
Value readArray( hid_t type, std::size_t idx, void const * buf, std::size_t const & size){
  hid_t arrtype = H5Tget_member_type(type, idx);
  auto ndims = H5Tget_array_ndims(arrtype);
  hsize_t * dims = new hsize_t[ndims];
  H5Tget_array_dims(arrtype, dims);
  hid_t super = H5Tget_super( arrtype );
  std::stringstream sstr;
  std::map<std::string, Value> arrmap;
  if( ndims != 1 ) throw std::runtime_error("not implemented / not tested for multidim arrays!");
  for( auto idim = 0; idim < ndims; ++idim ) {
    if( H5Tget_class( super ) == H5T_INTEGER ) {
      for( auto i = 0u; i < dims[idim]; ++i ) {
        sstr.str(""); sstr.clear(); sstr << i;
        arrmap.insert({sstr.str(), Value( (double)((int*)buf)[i] )});
      }
      if( size != sizeof(int)*dims[idim] ) {
        delete[] dims;
        throw std::runtime_error("invalid read size.");
      }
    } else if ( H5Tget_class( super ) == H5T_FLOAT ) {
      for( auto i = 0u; i < dims[idim]; ++i ) {
        sstr.str(""); sstr.clear(); sstr << i;
        arrmap.insert({sstr.str(), Value( ((double*)buf)[i] )});
      }
      if( size != sizeof(double)*dims[idim] ){
        delete[] dims;
        throw std::runtime_error("invalid read size.");
      }
        /*
         * TODO: add Float, Bool, String... 
         */
    } else {
      delete[] dims;
      throw std::runtime_error("not implemented array type.");
    }
  }
  delete[] dims;
  H5Tclose(super);
  H5Tclose(arrtype);
  return Value(arrmap);
}
Value readTable(hid_t link, const char* name) {
  /*
   * returns "a" value, containing the complete table as arrays:
   *
   * top level: "numeric" list of entries
   *   ["0"]["row"]: (row number)
   *        ["fields"]["somefieldname"]: (value)
   *                  ["otherfieldname"]: (othervalue)
   *   ["1"]["row"]: ...
   *        ["fields"]...
   */
  std::map<std::string, Value> outerTable, rowTable, innerTable;
  hsize_t nfields, nrecords;
  H5TBget_table_info( link, name, &nfields, &nrecords);
  char** fieldnames = new char*[nfields]; size_t* fieldsizes = new size_t[nfields];
  for( auto i =00u ; i < nfields; i++) { fieldnames[i] = new char[CHARMAX]; }
  size_t* fieldoffsets = new size_t[nfields]; size_t typesize;
  H5TBget_field_info( link, name, fieldnames, fieldsizes, fieldoffsets, &typesize );
  char* buffer = new char[typesize*nrecords];
  H5TBread_table(link, name, typesize, fieldoffsets, fieldsizes, buffer);
  
  // interpretation:
  // read type:
  hid_t dset_id = H5Dopen(link, name, H5P_DEFAULT);
  hid_t type_id = H5Dget_type(dset_id);
  if( H5Tget_class(type_id) != H5T_COMPOUND )
    throw std::runtime_error("table type is not compound.");
  int nmembers = H5Tget_nmembers(type_id);
  std::stringstream sstr;
  for( auto irecord = 0u; irecord < nrecords; ++irecord ) {
    rowTable.clear();
    innerTable.clear();
    for( auto i = 0; i < nmembers; i++ ) {
      if( H5Tget_member_class( type_id, i ) == H5T_INTEGER ) {
        Value val((double)*((int const *)(&buffer[typesize*irecord+fieldoffsets[i]])));
        innerTable.insert({fieldnames[i], val});
      } else if (H5Tget_member_class( type_id, i ) == H5T_ARRAY ) {
        Value val = readArray(type_id, i, &(buffer[typesize*irecord+fieldoffsets[i]]), fieldsizes[i]);
        innerTable.insert({fieldnames[i], val});
        /*
         * TODO: add Float, Bool, String... 
         */
      } else {
        throw std::runtime_error("not implemented.");
      }
    }
    rowTable.insert({"fields", innerTable});
    rowTable.insert({"row", Value((double)irecord)});
    sstr.clear(); sstr.str(""); sstr << irecord;
    outerTable.insert({sstr.str(), Value(rowTable)});
  }
  H5Tclose(type_id);
  H5Dclose(dset_id);
  delete[] buffer;
  for( auto i = 0u; i < nfields; i++) { delete[] fieldnames[i]; }
  delete[] fieldsizes; delete[] fieldoffsets; delete[] fieldnames; 
  return Value(outerTable);
}

bool isTable( hid_t link, const char* name ) {
  hid_t oid = H5Oopen(link, name, H5P_DEFAULT);
  if( not H5Aexists_by_name(link, name, "CLASS", H5P_DEFAULT) ) {
    H5Oclose(oid);
    return false;
  }
  hid_t attrid =  H5Aopen_by_name(link, name, "CLASS", H5P_DEFAULT, H5P_DEFAULT);
  hid_t dtype = H5Aget_type(attrid);
  auto size = H5Tget_size(dtype);
  char* buf = new char[size];
  auto res = H5Aread(attrid, dtype, buf);
  if( res != 0 ) { 
    delete[] buf;
    H5Tclose(dtype);
    H5Aclose(attrid);
    return false;
  }
  std::string strbuf(buf);
  delete[] buf;
  H5Tclose(dtype);
  H5Aclose(attrid);
  H5Oclose(oid);
  return strbuf == "TABLE";
}
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata) {
  /*
   * arrays are implemented as maps with string keys (see attributes.h)
   * in hdf5, attributes can be simply arrays. in this case,
   * we generate string keys with simply the integer as string
   */
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
      delete[] intbuf;
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else {
      std::map<std::string, Value> arr;
      for( auto i = 0; i < npts; i++) {
        sstr.clear(); sstr.str("");
        sstr << i;
        arr.insert({sstr.str(), (double)intbuf[i]});
      }
      if( npts == 1 ) attrs->push_back(Attribute(name, arr.at("0")));
      else            attrs->push_back(Attribute(name, Value(arr)));
      delete[] intbuf;
    }
  } else if ( typeclass == H5T_FLOAT ) {
    double* floatbuf = new double[npts];
    res |= H5Aread(attr_id, dtype, floatbuf);
    if( res != 0 ) { 
      delete[] floatbuf;
      H5Tclose(dtype);
      H5Aclose(attr_id);
      return res; }
    else {
      std::map<std::string, Value> arr;
      for( auto i = 0; i < npts; i++) {
        sstr.clear(); sstr.str("");
        sstr << i;
        arr.insert({sstr.str(), floatbuf[i]});
      }
      if( npts == 1 ) attrs->push_back(Attribute(name, arr.at("0")));
      else            attrs->push_back(Attribute(name, Value(arr)));
      delete[] floatbuf;
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
  for( auto const & elem : idxstack) {
    for( auto const & attr : elem.attributes ) {
      thisspec.attributes.push_back(attr);
    }
  }
  // give it the name of the current node:
  thisspec.datasetname = getFullpath(idxstack);
  thisspec.file        = idxstack.back().file;
  return thisspec;
}
herr_t h5_link_iterate( hid_t thislink, const char *name, const H5L_info_t *info, void *opdata) {
  //make the data easier accessible:
  std::tuple<Index,Index,std::map<std::string, Value>>* data = (std::tuple<Index,Index,std::map<std::string, Value>>*) opdata;
  Index & stackidx = std::get<0>(*data);
  Index & dsetidx = std::get<1>(*data);
  std::map<std::string, Value> & tables = std::get<2>(*data);
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

  stackidx.push_back(DatasetSpec(attrdata, std::string(name), stackidx.back().file, DatasetChunkSpec(0)));

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
    // IF it isn't a table:
    if( isTable( thislink, name ) ) {
      //read and store table... 
      stackidx.pop_back();
      tables.insert({ getFullpath(stackidx), readTable(thislink, name)});
    } else {
      dsetidx.push_back(processvector(stackidx));
      stackidx.pop_back();
    }
  }
  else return -2; //unimplemented datatype
  return 0;
}
Index indexHdf5File(std::string filename) {
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
  std::tuple<Index,Index,std::map<std::string,Value>> data;
  std::get<0>(data).push_back(DatasetSpec({}, "", File(filename, mtime), DatasetChunkSpec(0)));
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
  return expandIndex(std::get<1>(data), std::get<2>(data));
}
}
