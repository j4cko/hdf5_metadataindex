#include "attributes.h"
#include <sstream>
std::string typeToString(Type const & type) {
  switch(type) {
    case Type::NUMERIC: return "numeric";
    case Type::BOOLEAN: return "bool";
    case Type::STRING:  return "string";
    default: throw std::runtime_error("typeToString: unknown / unimplemented type.");
  }
}
Type typeFromTypeid(std::type_info const & tinfo) {
  if( tinfo == typeid(double) or tinfo == typeid(float) or tinfo == typeid(int)
   or tinfo == typeid(char)   or tinfo == typeid(unsigned int) 
   or tinfo == typeid(unsigned char) )      return Type::NUMERIC;
  else if( tinfo == typeid(std::string) )   return Type::STRING;
  else if( tinfo == typeid(bool) )          return Type::BOOLEAN;
  else throw std::runtime_error("typeFromTypeid: unknown type.");
}
Type typeFromString(std::string const & typestr) {
  if( typestr == "numeric" )                return Type::NUMERIC;
  else if( typestr == "bool" )              return Type::BOOLEAN;
  else if( typestr == "string" )            return Type::STRING;
  else throw std::runtime_error("typeFromString: unknow type.");
}
Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr) {
  const Type typespec = typeFromString(typestr);
  std::stringstream sstr;
  sstr << valstr;
  double doublebuf; bool boolbuf;
  switch ( typespec ) {
    case Type::NUMERIC:
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
