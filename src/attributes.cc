#include "attributes.h"
namespace rqcd_hdf5_index {
Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr) {
  const Type typespec = typeFromString(typestr);
  auto resval = valueFromString(valstr);
  assert(resval.getType() == typespec);
  return Attribute(name, resval);
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
}
