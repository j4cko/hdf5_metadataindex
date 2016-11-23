#include "attributes.h"
#include "conditions.h"
#include "sqliteHelpers.h"
#include <iostream>
#include <sstream>
#include "jsonToValue.h"

int main(int argc, char** argv) {
  if( argc != 3 ) {
    std::cerr << "wrong number of args." << std::endl; 
    return -1; 
  }

  const std::string dbfile(argv[1]);
  const std::string query(argv[2]);

  Request req = queryToRequest(query);

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
