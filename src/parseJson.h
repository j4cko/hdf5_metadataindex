#ifndef __PARSE_JSON_H__
#define __PARSE_JSON_H__
#include <json/json.h>
#include "conditions.h"
#include "value.h"
#include "attributes.h"

namespace rqcd_hdf5_index {
bool isRepresentableAsValue(Json::Value const & json);
Value jsonValueToValue(Json::Value const & json);
AttributeRequest parseAttributeRequest(Json::Value const & root, std::string const & name);
FileRequest parseFileRequest(Json::Value const & root, std::string const & name);
Request queryToRequest(std::string const & query);
}
#endif
