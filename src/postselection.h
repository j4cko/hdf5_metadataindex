/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "attributes.h"
#include "conditions.h"

namespace rqcd_file_index {
void filterIndexByAttributeRequests(Index& idx, std::vector<AttributeRequest> const & req);
void filterIndexByFileRequests(Index& idx, std::vector<FileRequest> const & req);
void filterIndexByPostselectionRules(Index& idx, Request const & req);
}
