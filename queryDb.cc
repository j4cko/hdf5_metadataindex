#include "attributes.h"
#include "conditions.h"
#include "sqliteHelpers.h"
#include <iostream>
#include <sstream>
#include "jsonToValue.h"
#include "h5helpers.h"

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
  auto files = idsToFilenames(db, ids);

  for( auto file : files ) std::cout << file << std::endl;
  printIndex(idx, std::cout);
  assert(res.size() == files.size());
  for( auto i = 0u; i < res.size(); ++i) {
    std::cout << res[i] << " " << files[i] << std::endl; }

  if( not H5DataHelpers::h5_file_exists(files.back()) )
    std::cerr << "original file no longer exists at this place: " << files.back() << std::endl;
  else {
    //check one of the files for mtime:
    auto mtime = H5DataHelpers::getFileModificationTime(files.back());
    auto mtimeDb = getFileModificationTime(db, files.back());
    if( mtime - mtimeDb > 0 )
      std::cerr << "file \"" << files.back() << "\" has changed since db creation." << std::endl;
    else
      std::cout << "file \"" << files.back() << "\" probably in sync with db." << std::endl;
  }

  sqlite3_close(db);

  return 0;
}
