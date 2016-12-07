#include "jsonToValue.h"
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
  else if( json.isArray() ){
    std::cout << "here, we should parse the json array and return an array with keys \"0\", \"1\",..." << std::endl;
    throw std::runtime_error("json arrays are not yet implemented.");
  }
  else if( json.isObject() ){
    auto names = json.getMemberNames();
    std::map<std::string, Value> mmap;
    for( auto const & name : names ) {
      mmap.insert({name, jsonValueToValue(json[name])});
    }
    return Value(mmap);
  }
  else {
    throw std::runtime_error("json value is not representable as Value.");
  }
}
AttributeRequest parseAttributeRequest(Json::Value const & root, std::string const & name) {
  if( root[name].isArray() ) {
    throw std::runtime_error("array requests are not implemented.");
  }
  if( isRepresentableAsValue(root[name]) ) {
    return AttributeRequest(name, AttributeConditions::Equals(jsonValueToValue(root[name])));
  }
  else if( root[name].isMember("not") ) {
    return AttributeRequest(name, AttributeConditions::NotEquals(
            jsonValueToValue(root[name]["not"])));
  }
  else if( root[name].isMember("min") and root[name].isMember("max") ){
    return AttributeRequest(name, AttributeConditions::Range(
            jsonValueToValue(root[name]["min"]),
            jsonValueToValue(root[name]["max"])));
  }
  else if( root[name].isMember("min") ){
    return AttributeRequest(name, AttributeConditions::Min(
            jsonValueToValue(root[name]["min"])));
  }
  else if( root[name].isMember("max") ){
    return AttributeRequest(name, AttributeConditions::Max(
            jsonValueToValue(root[name]["max"])));
  }
  else if( root[name].isMember("present") and root[name]["present"].isBool() ){
    return AttributeRequest(name, AttributeConditions::Present(root[name]["present"].asBool()));
  }
  else if( root[name].isMember("or") and root[name]["or"].isArray() ) {
    std::vector<Value> vec;
    for( auto const & elem : root[name]["or"] ) {
      vec.push_back(jsonValueToValue(elem));
    }
    return AttributeRequest(name, AttributeConditions::Or(vec));
  }
  else if( root[name].isMember("matches") and root[name]["matches"].isString() ) {
    return AttributeRequest(name, AttributeConditions::Matches(root[name]["matches"].asString()));
  }
  else {
    throw std::runtime_error("json value is not parsable to Request.");
  }
}
FileRequest parseFileRequest(Json::Value const & root, std::string const & name) {
  assert(root.isMember(name));
  if( name == "matches" and root[name].isString() ) {
    return std::unique_ptr<FileConditions::NameMatches>(
        new FileConditions::NameMatches(root["matches"].asString()));
  } else if( name == "newer" and root[name].isInt() ) {
    return std::unique_ptr<FileConditions::Newer>(
        new FileConditions::Newer(root["newer"].asInt()));
  } else if( name == "older" and root[name].isInt() ) {
    return std::unique_ptr<FileConditions::Older>(
        new FileConditions::Older(root["older"].asInt()));
  } else if( name == "mtime" and root[name].isInt() ) {
    return std::unique_ptr<FileConditions::Mtime>(
        new FileConditions::Mtime(root["mtime"].asInt()));
  } else {
    throw std::runtime_error("unknown FileRequest (or arguments could not be handled)");
  }
}
Request queryToRequest(std::string const & query) {
  Request req;
  std::stringstream sstr;
  sstr << query;
  Json::Value root;
  sstr >> root;
  auto names = root.getMemberNames();
  for( auto name : names ) {
    if( name == std::string("attributes") ) {
      //attribute request:
      for( auto attrname : root["attributes"].getMemberNames())
        req.attrrequests.push_back(parseAttributeRequest(root["attributes"], attrname));
    } else if ( name == std::string("file") ) {
      for( auto condname : root["file"].getMemberNames()) {
        req.filerequests.push_back(parseFileRequest(root["file"], condname));
      }
    } else if ( name == std::string("luacode") ) {
      throw std::runtime_error("lua postprocessing is not yet implemented.");
    } else {
      std::stringstream sstr;
      sstr << "unknown request type: " << name << std::endl;
      throw std::runtime_error(sstr.str());
    }
  }

  return req;
}
