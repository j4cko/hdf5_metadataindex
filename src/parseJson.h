/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#ifndef __PARSE_JSON_H__
#define __PARSE_JSON_H__
#include <json/json.h>
#include "conditions.h"
#include "value.h"
#include "attributes.h"

namespace rqcd_file_index {
bool isRepresentableAsValue(Json::Value const & json);
Value jsonValueToValue(Json::Value const & json);
Attribute parseAttribute( Json::Value const & root);
AttributeRequest parseAttributeRequest(Json::Value const & root, std::string const & name);
DatasetSpec parseDsetspec( Json::Value const & root );
FileRequest parseFileRequest(Json::Value const & root, std::string const & name);
Request queryToRequest(std::string const & query);
std::vector<Request> queryToRequestList(std::string const query);
}
#endif
