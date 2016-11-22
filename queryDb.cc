#include "attributes.h"
#include "sqliteHelpers.h"
#include <iostream>

Request queryToRequest(std::string const & query) {
  Request req;
  req.push_back(Attribute("px", 0));
  req.push_back(Attribute("py", 0));
  req.push_back(Attribute("pz", 0));
  req.push_back(Attribute("hpe", 1));
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

  sqlite3 *db;
  sqlite3_open(dbfile.c_str(), &db);

  Index idx = queryDb(db, req);

  sqlite3_close(db);

  printIndex(idx, std::cout);

  return 0;
}
