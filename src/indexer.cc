/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include <iostream>
#include "attributes.h"
#include "indexHdf5.h"
#include "sqliteHelpers.h"
#include "h5helpers.h"

using namespace rqcd_file_index;

int main(int argc, char** argv) {
  if( argc != 3 ) { std::cerr << "wrong number of arguments." << std::endl;
                    return -1; }

  const std::string h5file(argv[1]);
  const std::string sqlfile(argv[2]);

  sqlite3 *db;
  sqlite3_open(sqlfile.c_str(), &db);
  sqlite_helpers::prepareSqliteFile(db);

  Index idx = indexHdf5File(h5file);

  sqlite_helpers::insertDataset(db, idx);

  sqlite3_close(db);
  
 // printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
