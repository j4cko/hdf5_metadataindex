#include <sstream>
#include "parseJson.h"
namespace rqcd_file_index {
std::string typeToString(Type const & type) {
  switch(type) {
    case Type::NUMERIC: return "numeric";
    case Type::BOOLEAN: return "bool";
    case Type::STRING:  return "string";
    case Type::ARRAY:   return "array";
    default: throw std::runtime_error("typeToString: unknown / unimplemented type. (may be used uninitialized?)");
  }
}
Type typeFromTypeid(std::type_info const & tinfo) {
  if( tinfo == typeid(double) or tinfo == typeid(float) or tinfo == typeid(int)
   or tinfo == typeid(char)   or tinfo == typeid(unsigned int) 
   or tinfo == typeid(unsigned char) )      return Type::NUMERIC;
  else if( tinfo == typeid(std::string) )   return Type::STRING;
  else if( tinfo == typeid(bool) )          return Type::BOOLEAN;
  else throw std::runtime_error("typeFromTypeid: unknown type (or array type)");
}
Type typeFromString(std::string const & typestr) {
  if( typestr == "numeric" )                return Type::NUMERIC;
  else if( typestr == "bool" )              return Type::BOOLEAN;
  else if( typestr == "string" )            return Type::STRING;
  else if( typestr == "array" )             return Type::ARRAY;
  else throw std::runtime_error("typeFromString: unknown type.");
}
bool isArrayString(std::string const & str) {
  return ( str[0] == '{' and str.back() == '}' );
}
Value valueFromString(std::string const & str){
  //array:
  if( isArrayString(str) ){
    std::stringstream sstr;
    sstr << str;
    Json::Value root;
    sstr >> root;
    return jsonValueToValue(root);
  }
  // bool:
  else if( str == "true" ) return Value(true);
  else if( str == "false" ) return Value(false);
  else {
    //numeric
    try {
      std::size_t pos;
      double d = std::stod(str, &pos);
      // if not everything could be converted, it is in fact a string:
      if( pos != str.size() ) return Value(str);
      return Value(d);
    } catch(...){}
    // string:
    return Value(str);
  }
}
}
