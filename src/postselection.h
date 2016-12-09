#include "attributes.h"
#include "conditions.h"
#include "indexHdf5.h"

namespace rqcd_hdf5_index {
void filterIndexByAttributeRequests(Index& idx, std::vector<AttributeRequest> const & req);
void filterIndexByFileRequests(Index& idx, std::vector<FileRequest> const & req);
void filterIndexByPostselectionRules(Index& idx, Request const & req);
}
