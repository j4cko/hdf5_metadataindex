#include "attributes.h"
#include "conditions.h"
#include <json/json.h>

bool isRepresentableAsValue(Json::Value const & json) {
  return (json.isDouble() or json.isBool() or json.isString());
}
Value jsonValueToValue(Json::Value const & json) {
  if( json.isDouble() ) 
    return Value(json.asDouble());
  else if( json.isBool() )
    return Value(json.asBool());
  else if( json.isString() )
    return Value(json.asString());
  else {
    throw std::runtime_error("json value is not representable as Value.");
  }
}
AttributeRequest parseRequest(Json::Value const & root, std::string const & name) {
  if( isRepresentableAsValue(root[name]) )
    return AttributeRequest(name, Conditions::Equals(jsonValueToValue(root[name])));
  else if( root[name].isMember("min") and root[name].isMember("max") )
    return AttributeRequest(name, Conditions::Range(
            jsonValueToValue(root[name]["min"]),
            jsonValueToValue(root[name]["max"])));
  else if( root[name].isMember("min") )
    return AttributeRequest(name, Conditions::Min(
            jsonValueToValue(root[name]["min"])));
  else if( root[name].isMember("max") )
    return AttributeRequest(name, Conditions::Max(
            jsonValueToValue(root[name]["max"])));
  else if( root[name].isMember("present") and root[name]["present"].isBool() )
    return AttributeRequest(name, Conditions::Present(root[name]["present"].asBool()));
  else if( root[name].isMember("or") and root[name]["or"].isArray() ) {
    std::vector<Value> vec;
    for( auto const & elem : root[name]["or"] ) {
      vec.push_back(jsonValueToValue(elem));
    }
    return AttributeRequest(name, Conditions::Or(vec));
  }
  else
    throw std::runtime_error("json value is not parsable to Request.");
}
Request queryToRequest(std::string const & query) {
  Request req;
  std::stringstream sstr;
  sstr << query;
  Json::Value root;
  sstr >> root;

  auto names = root.getMemberNames();
  for( auto name : names )
    req.push_back(parseRequest(root, name));

  return req;
}

