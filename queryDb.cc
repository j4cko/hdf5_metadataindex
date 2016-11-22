#include "attributes.h"
#include "sqliteHelpers.h"
#include <iostream>
#include <sstream>
#include <json/json.h>

Request queryToRequest(std::string const & query) {
  Request req;
  std::stringstream sstr;
  sstr << query;
  Json::Value root;
  sstr >> root;

  auto names = root.getMemberNames();
  for( auto name : names ) {
    if( root[name].isDouble() ) 
      req.push_back(Attribute(name, root[name].asDouble()));
    else if( root[name].isInt() )
      req.push_back(Attribute(name, root[name].asInt()));
    else if( root[name].isBool() )
      req.push_back(Attribute(name, root[name].asBool()));
    else if( root[name].isString() )
      req.push_back(Attribute(name, root[name].asString()));
  }
  return req;
}
int main(int argc, char** argv) {
  if( argc != 3 ) {
    std::cerr << "wrong number of args." << std::endl; 
    return -1; 
  }

  const std::string dbfile(argv[1]);
  const std::string query(argv[2]);

  Request req = queryToRequest(query);

  for( auto & attr : req )
    std::cout << "- " << attr.getName() << ": " << attr.getValue() << std::endl;

  sqlite3 *db;
  sqlite3_open(dbfile.c_str(), &db);

  /*auto names = getFilelocations(db, req);
  for( auto name : names ) { std::cout << name << std::endl; }
  */
  auto ids = getLocIds(db, req);
  Index idx = idsToIndex(db, ids);

  auto res = idsToDsetnames(db, ids);
  sqlite3_close(db);

  printIndex(idx, std::cout);
  for( auto name : res ) { std::cout << name << std::endl; }

  return 0;
}
