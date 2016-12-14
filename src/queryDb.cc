/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "attributes.h"
#include "conditions.h"
#include "sqliteHelpers.h"
#include <iostream>
#include <sstream>
#include "parseJson.h"
#include "postselection.h"
#include <list>

using namespace rqcd_file_index; 
Index getMatchingDatasetSpecs(sqlite3 *db, Request const & req) {
  auto ids = sqlite_helpers::getLocIdsMatchingPreSelection(db, req);
  Index idx = sqlite_helpers::idsToIndex(db, ids);
  filterIndexByPostselectionRules(idx, req);
  return idx;
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

  auto idx = getMatchingDatasetSpecs(db, req);

  printIndex(idx, std::cout);

  sqlite3_close(db);

  return 0;
}
