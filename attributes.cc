#include "attributes.h"
std::string typeToString(Type const & type) {
  switch(type) {
    case Type::INTEGER: return "integer";
    case Type::REAL:    return "real";
    case Type::BOOLEAN: return "bool";
    case Type::STRING:  return "string";
    default: throw std::runtime_error("typeToString: unknown / unimplemented type.");
  }
}
Type typeFromTypeid(std::type_info const & tinfo) {
  if( typeid(double) == tinfo )             return Type::REAL;
  else if( tinfo == typeid(float) )         return Type::REAL;
  else if( tinfo == typeid(int) )           return Type::INTEGER;
  else if( tinfo == typeid(char) )          return Type::INTEGER;
  else if( tinfo == typeid(unsigned int) )  return Type::INTEGER;
  else if( tinfo == typeid(unsigned char) ) return Type::INTEGER;
  else if( tinfo == typeid(std::string) )   return Type::STRING;
  else if( tinfo == typeid(bool) )          return Type::BOOLEAN;
  else throw std::runtime_error("typeFromTypeid: unknown type.");
}
