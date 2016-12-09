#ifndef __INDEX_HDF5_H__
#define __INDEX_HDF5_H__
#include "attributes.h"
#include <hdf5.h>
#include <vector>
#include <string>

namespace rqcd_hdf5_index {
Index indexFile(std::string filename);
void mergeIndex( Index & res, Index const & idxToMerge );
std::string getFullpath( Index const & idxstack );
void printIndex(Index const & idx, std::ostream& os);
Value readTable(hid_t link, const char* name);
Value readArray( hid_t type, std::size_t idx, void const * buf, std::size_t const & size);
std::vector<Attribute> tableFieldsToAttributes(Value const & table);
Index splitDsetSpecWithTable( DatasetSpec const & dset, Value const & table );
Index expandIndex(Index const & idx, std::map<std::string, Value> const & tables);
bool isTable( hid_t link, const char* name );
herr_t h5_attr_iterate( hid_t o_id, const char *name, const H5A_info_t *attrinfo, void *opdata);
DatasetSpec processvector( Index const & idxstack );
herr_t h5_link_iterate( hid_t thislink, const char *name, const H5L_info_t *info, void *opdata);
} //rqcd_hdf5_index
#endif
