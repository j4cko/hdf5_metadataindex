#ifndef __INDEX_HDF5_H__
#define __INDEX_HDF5_H__
#include "attributes.h"
#include <hdf5.h>
#include <vector>
#include <string>

namespace rqcd_file_index {
Index indexHdf5File(std::string filename);
Value readTable(hid_t link, const char* name);
Value readArray( hid_t type, std::size_t idx, void const * buf, std::size_t const & size);
bool isTable( hid_t link, const char* name );
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata);
DatasetSpec processvector( Index const & idxstack );
herr_t h5_link_iterate( hid_t thislink, const char *name, const H5L_info_t *info, void *opdata);
} //rqcd_file_index
#endif
