#include "readTable.h"
#include <sstream>

Value readArray( hid_t type, std::size_t idx, void const * buf, std::size_t const & size){
  hid_t arrtype = H5Tget_member_type(type, idx);
  auto ndims = H5Tget_array_ndims(arrtype);
  hsize_t * dims = new hsize_t[ndims];
  H5Tget_array_dims(arrtype, dims);
  hid_t super = H5Tget_super( arrtype );
  std::stringstream sstr;
  std::map<std::string, Value> arrmap;
  if( ndims != 1 ) throw std::runtime_error("not implemented / not tested for multidim arrays!");
  for( auto idim = 0u; idim < ndims; ++idim ) {
    if( H5Tget_class( super ) == H5T_INTEGER ) {
      for( auto idx = 0u; idx < dims[idim]; ++idx ) {
        sstr.str(""); sstr.clear(); sstr << idx;
        arrmap.insert({sstr.str(), Value( (double)((int*)buf)[idx] )});
      }
      if( size != sizeof(int)*dims[idim] ) {
        delete[] dims;
        throw std::runtime_error("invalid read size.");
      }
    } else if ( H5Tget_class( super ) == H5T_FLOAT ) {
      for( auto idx = 0u; idx < dims[idim]; ++idx ) {
        sstr.str(""); sstr.clear(); sstr << idx;
        arrmap.insert({sstr.str(), Value( ((double*)buf)[idx] )});
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
  for( int i=0; i < nfields; i++) { fieldnames[i] = new char[1024]; }
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
    for( auto i = 0u; i < nmembers; i++ ) {
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
  for( int i = 0; i < nfields; i++) { delete[] fieldnames[i]; }
  delete[] fieldsizes; delete[] fieldoffsets; delete[] fieldnames; 
  return Value(outerTable);
}
