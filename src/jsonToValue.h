#ifndef __JSON_TO_VALUE_H__
#define __JSON_TO_VALUE_H__
#include "attributes.h"
#include "conditions.h"
#include <json/json.h>

bool isRepresentableAsValue(Json::Value const & json);
Value jsonValueToValue(Json::Value const & json);
AttributeRequest parseAttributeRequest(Json::Value const & root, std::string const & name);
FileRequest parseFileRequest(Json::Value const & root, std::string const & name);
Request queryToRequest(std::string const & query);
#endif
