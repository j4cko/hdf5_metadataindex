#include "hdf5ReaderGeneric.h"
H5ReaderGeneric::H5ReaderGeneric(File const & file) : H5ReaderGeneric(file.filename) {
  if( not checkMtime(file) )
    std::cerr << "WARNING: file is newer than the requested." << std::endl;
}
H5ReaderGeneric::H5ReaderGeneric(std::string const & file) {
  if( not H5DataHelpers::h5_file_exists(file) ) {
    std::stringstream sstr;
    sstr << "File \"" << file << "\" does not exist!";
    throw std::runtime_error(sstr.str());
  }
  if( not H5DataHelpers::h5_file_is_hdf5(file) ) {
    std::stringstream sstr;
    sstr << "File \"" << file << "\" is not a HDF5 file!";
    throw std::runtime_error(sstr.str());
  }
  file_id = H5Fopen(file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if( file_id < 0 ) {
    std::stringstream sstr;
    sstr << "File \"" << file << "\" could not be opened!";
    throw std::runtime_error(sstr.str());
  }
}
H5ReaderGeneric::H5ReaderGeneric(DatasetSpec const & dsetspec) : 
  H5ReaderGeneric(dsetspec.file) { }
H5ReaderGeneric::~H5ReaderGeneric() {
  auto res = H5Fclose(file_id);
  if( res < 0 ) {
    std::cerr << "WARNING: could not close file. maybe it has already been closed?" << std::endl;
  }
}
H5ReaderGeneric::H5ReaderGeneric(H5ReaderGeneric && other) {
  std::swap(file_id, other.file_id);
}
std::vector<std::complex<double>> 
H5ReaderGeneric::read(DatasetSpec const & dsetspec) {
  std::vector<std::complex<double>> res;
  /* dummy data: */
  res.push_back(std::complex<double>(1.0, 2.0));

  auto dsetid = H5Dopen(file_id, dsetspec.datasetname.c_str(), H5P_DEFAULT);
  auto dtype  = H5Dget_type(dsetid);
  auto dclass = H5Tget_class(dtype);

  if( dclass != H5T_FLOAT) {
    H5Tclose(dtype);
    H5Dclose(dsetid);
    throw std::runtime_error("dataset does not have float type.");
  }

  auto size = H5Tget_size(dtype);
  if( size != sizeof(double) ) {
    H5Tclose(dtype);
    H5Dclose(dsetid);
    throw std::runtime_error("dataset is not in double precision.");
  }

  auto dspace = H5Dget_space(dsetid);
  auto ndims  = H5Sget_simple_extent_ndims(dspace);
  hsize_t * dims = new hsize_t[ndims];
  auto status = H5Sget_simple_extent_dims(dspace, dims, NULL);

  hsize_t numberOfDoubles = 0;
  hid_t file_space_id = -1;
  hid_t mem_space_id  = -1;
  if( ndims == 1 ) {
    // read one-dim:
    if( dsetspec.location.begin >= 0 ) {
      H5Sclose(dspace);
      H5Tclose(dtype);
      H5Dclose(dsetid);
      throw std::runtime_error("row data requested, but dataset has only one dimension.");
    }
    numberOfDoubles = dims[0];
    file_space_id = H5S_ALL;
  } else if( ndims == 2 ) {
    if( dsetspec.location.begin < 0 ) {
      // read full dataset, serialize
      numberOfDoubles = dims[0]*dims[1];
      file_space_id   = H5S_ALL;
    } else if ( dsetspec.location.begin >= dims[0] ) {
      H5Sclose(dspace);
      H5Tclose(dtype);
      H5Dclose(dsetid);
      throw std::runtime_error("requested row is larger than the available rows in the dataset.");
    } else {
      // read only one column of the dataset:
      numberOfDoubles = dims[1];
      const hsize_t dspacedims[2] = {1u, numberOfDoubles};
      const hssize_t offsets[2] = {dsetspec.location.begin, 0u};
      file_space_id   = H5Screate_simple(2,dspacedims,NULL);
      status = H5Soffset_simple(file_space_id, offsets);
    }
  }
  
  if( numberOfDoubles % 2 != 0 ) {
    H5Sclose(dspace);
    H5Tclose(dtype);
    H5Dclose(dsetid);
    throw std::runtime_error("read size is odd: cannot be complex numbers.");
  }
  double * buf = new double[numberOfDoubles];
  mem_space_id = H5Screate_simple(1, &numberOfDoubles, NULL);
  status = H5Dread(dsetid, H5T_NATIVE_DOUBLE, H5S_ALL, file_space_id, H5P_DEFAULT, buf);

  res.resize(numberOfDoubles/2);
  for( auto i = 0u; i < numberOfDoubles / 2; ++i) {
    res[i] = std::complex<double>(buf[2*i], buf[2*i+1]);
  }
  delete[] dims;
  delete[] buf;

  H5Sclose(dspace);
  H5Tclose(dtype);
  H5Dclose(dsetid);
  return res;
}
bool H5ReaderGeneric::checkMtime(File const & file) {
  return (file.mtime <= H5DataHelpers::getFileModificationTime(file.filename));
}
