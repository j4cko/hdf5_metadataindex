#include "attributes.h"
#include <iostream>
#include <sstream>
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
Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr) {
  const Type typespec = typeFromString(typestr);
  std::stringstream sstr;
  sstr << valstr;
  double doublebuf; bool boolbuf;
  switch ( typespec ) {
    case Type::NUMERIC:
      sstr >> doublebuf;
      return Attribute(name, Value(doublebuf));
    case Type::BOOLEAN:
      sstr >> boolbuf;
      return Attribute(name, Value(boolbuf));
    case Type::STRING:
      return Attribute(name, Value(valstr));
    case Type::ARRAY:
      throw std::runtime_error("attributeFromStrings: arrays from string not impl");
    default:
      throw std::runtime_error("attributeFromStrings: unimplemented type");
  }
}
std::list<File> getUniqueFiles(Index const & idx) {
  std::list<File> res;
  for(auto const & dsetspec : idx ) {
    res.push_back(dsetspec.file);
  }
  res.sort([](File const & a, File const & b){
      if( a.filename == b.filename ) return a.mtime < b.mtime;
      else return a.filename < b.filename;
      });
  res.unique();
  return res;
}
