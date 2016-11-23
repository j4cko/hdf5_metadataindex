#ifndef __INDEX_HDF5_H__
#define __INDEX_HDF5_H__
#include <vector>
#include <string>
#include "attributes.h"

typedef std::vector<AttributeRequest> Request;
typedef std::pair<std::vector<Attribute>,std::string> DatasetSpec;
typedef std::vector<DatasetSpec> Index;

Index indexFile(std::string filename);

void printIndex(Index const & idx, std::ostream& os);
#endif
