#include "attributes.h"
#include <sstream>
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
Type typeFromString(std::string const & typestr) {
  if( typestr == "integer" )                return Type::INTEGER;
  else if( typestr == "real" )              return Type::REAL;
  else if( typestr == "bool" )              return Type::BOOLEAN;
  else if( typestr == "string" )            return Type::STRING;
  else throw std::runtime_error("typeFromString: unknow type.");
}
Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr) {
  const Type typespec = typeFromString(typestr);
  std::stringstream sstr;
  sstr << valstr;
  int intbuf; double doublebuf; bool boolbuf;
  switch ( typespec ) {
    case Type::INTEGER:
      sstr >> intbuf;
      return Attribute(name, intbuf);
    case Type::REAL:
      sstr >> doublebuf;
      return Attribute(name, doublebuf);
    case Type::BOOLEAN:
      sstr >> boolbuf;
      return Attribute(name, boolbuf);
    case Type::STRING:
      return Attribute(name, valstr);
    default:
      throw std::runtime_error("attributeFromStrings: unimplemented type");
  }
}
