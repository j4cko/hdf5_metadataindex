#include "sqliteBindings.h"
#include <iostream>
#include <string>
#include "indexHdf5.h"

void prepareSqliteFile(sqlite3 ** db) {

}

int main(int argc, char** argv) {
  if( argc != 3 ) { std::cerr << "wrong number of arguments." << std::endl;
                    return -1; }
  const std::string h5file(argv[1]);
  const std::string sqlfile(argv[2]);

  sqlite3 *db;
  sqlite3_open(sqlfile.c_str(), &db);
  prepareSqliteFile(&db);
  sqlite3_close(db);

  Index idx = indexFile(h5file);
  return 0;
}
