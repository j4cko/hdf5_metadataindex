#include "attributes.h"
#include "conditions.h"
#include "sqliteHelpers.h"
#include <iostream>
#include <sstream>
#include "parseJson.h"
#include "h5helpers.h"
#include "postselection.h"
#include <list>

using namespace rqcd_hdf5_index; 
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

  auto files = getUniqueFiles(idx);
  for( auto file : files ) {
    if( not H5DataHelpers::h5_file_exists(file.filename) )
      std::cerr << "original file no longer exists at this place: \"" << file.filename << "\"" << std::endl;
    else {
      //check one of the files for mtime:
      auto mtime = H5DataHelpers::getFileModificationTime(file.filename);
      if( mtime - file.mtime > 0 )
        std::cerr << "file \"" << files.back().filename << "\" has changed since db creation." << std::endl;
      else
        std::cout << "file \"" << files.back().filename << "\" probably in sync with db." << std::endl;
    }
  }

  sqlite3_close(db);

  return 0;
}
