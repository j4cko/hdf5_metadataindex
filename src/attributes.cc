#include "attributes.h"
#include "parseJson.h"

namespace rqcd_file_index {
Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr) {
  const Type typespec = typeFromString(typestr);
  auto resval = valueFromString(valstr);
  assert(resval.getType() == typespec);
  return Attribute(name, resval);
}
DatasetSpec dsetSpecFromString(std::string const & str) {
  std::stringstream sstr;
  sstr << str;
  Json::Value root;
  sstr >> root;
  return parseDsetspec(root);
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
void printIndex(Index const & idx, std::ostream& os) {
  for ( auto const & dataset : idx ) {
    os << "dataset \"" << dataset.datasetname << "\" (from file \""
      << dataset.file.filename << "\" and row " << dataset.location.row << ")" <<
      " has the following attributes:" << std::endl;
    for ( auto attr : dataset.attributes ) {
      os << "  - " << attr.getName() 
        << " (" << typeToString(attr.getType()) << ") = "
        << attr.getValue() << std::endl;
    }
  }
}
std::string getFullpath( Index const & idxstack ) {
  std::string fullpath("");
  for( auto const & elem : idxstack) {
    // avoid double slashes at the beginning (first / root elem of idxstack is
    // the file itself
    if( not elem.datasetname.empty() ) fullpath += "/" + elem.datasetname;
  }
  return fullpath;
}
void mergeIndex( Index & res, Index const & idxToMerge ) {
  for ( auto const & from : idxToMerge )
    res.push_back(from);
}
std::vector<Attribute> tableFieldsToAttributes(Value const & table) {
  std::vector<Attribute> res;
  for( auto const & key : table["fields"].keys() ) {
    res.push_back(Attribute(key, Value(table["fields"][key])));
  }
  return res;
}
Index splitDsetSpecWithTable( DatasetSpec const & dset, Value const & table ) {
  /*
   * splits a dataset specifier that is described by a table into several
   * DatasetSpecs, each holding the attributes from the table and the correct
   * location within the dataset.
   */
  Index res;
  for( auto const & req : table.getMap() ) {
    auto attrs = tableFieldsToAttributes(req.second);
    DatasetSpec newdset(dset);
    for( auto && attr : attrs )
      newdset.attributes.push_back(std::move(attr));
    newdset.location.row = (int)(req.second["row"].getNumeric());
    res.push_back(std::move(newdset));
  }
  return res;
}
Index expandIndex(Index const & idx, std::map<std::string, Value> const & tables) {
  Index res;
  for( auto const & elem : idx ) {
    // for each DatasetSpec in the Index, check if there is a table describing
    // the data contents
    std::string tablename = elem.datasetname.substr(0, elem.datasetname.rfind("/"));
    if( tables.count( tablename ) > 0) {
      // dset has table:
      mergeIndex( res, splitDsetSpecWithTable(elem, tables.at(tablename)) );
    } else {
      // doesn't have table, add to result index:
      res.push_back(elem);
    }
  }
  return res;
}
SearchMode searchModeFromString(std::string str) {
  std::transform(str.begin(), str.end(),str.begin(), ::toupper);
  if( str == "FIRST" )             return SearchMode::FIRST;
  else if ( str == "AVERAGE" )     return SearchMode::AVERAGE;
  else if ( str == "CONCATENATE" ) return SearchMode::CONCATENATE;
  else if ( str == "ALL" )         return SearchMode::ALL;
  else
    throw std::runtime_error("unsupported searchmode! Only FIRST, AVERAGE and "
                             "CONCATENATE are supported");
}
std::ostream& operator<<(std::ostream& os, Index const & idx) {
  os << "[";
  for( auto it = idx.cbegin(); it != --(idx.cend()); ++it)
    os << *it << ", ";
  if( not idx.empty() )
    os << idx.back();
  os << "]";
  return os;
}
}
