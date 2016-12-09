#include <iostream>
#include <sstream>
#include <list>
#include "attributes.h"
#include "conditions.h"
#include "sqliteHelpers.h"
#include "parseJson.h"
#include "h5helpers.h"
#include "postselection.h"
#include "hdf5ReaderGeneric.h"

using namespace rqcd_hdf5_index;
using namespace rqcd_hdf5_reader_generic;

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
  sqlite3_close(db);


  auto files = getUniqueFiles(idx);
  std::vector<std::complex<double>> result;
  for( auto file : files ) {
    // process only this file:
    auto idxcp = idx;
    Request onlyThisFileReq;
    onlyThisFileReq.filerequests.push_back(std::unique_ptr<FileCondition>(
          new FileConditions::NameMatches(file.filename)));
    filterIndexByPostselectionRules(idxcp, onlyThisFileReq);
    try {
      H5ReaderGeneric reader(file);
      for( auto const & dsetspec : idxcp ) {
        auto thisdset = reader.read(dsetspec);
        for( auto && number : thisdset ) {
          result.push_back(std::move(number));
        }
      }
    } catch (std::exception const & exc) {
      std::cerr << "ERROR while reading file: " << exc.what() << std::endl;
      return 1;
    }
  }
  // output:
  for( auto const & number : result ) {
    std::cout << std::real(number) << " " << std::imag(number) << std::endl;
  }
  return 0;
}
