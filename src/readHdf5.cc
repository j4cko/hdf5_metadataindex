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

using namespace rqcd_file_index;
using namespace rqcd_hdf5_reader_generic;

Index getMatchingDatasetSpecs(sqlite3 *db, Request const & req) {
  auto ids = sqlite_helpers::getLocIdsMatchingPreSelection(db, req);
  Index idx = sqlite_helpers::idsToIndex(db, ids);
  filterIndexByPostselectionRules(idx, req);
  return idx;
}

template<typename T>
void concatenate(std::vector<T> & res, std::vector<T> const & in) {
  res.reserve(res.size() + in.size());
  for( auto it = in.begin(); it != in.end(); ++it )
    res.push_back(*it);
}
template<typename T>
void elementalAdd(std::vector<T> & res, std::vector<T> const & in) {
  if( res.empty() ) {
    res = in;
    return;
  }
  if( res.size() != in.size() ){
    throw std::runtime_error("vector sizes differ!");
  }
  auto a = res.begin();
  for( auto b = in.begin(); 
       a != res.end() and b != in.end(); 
       ++a, ++b)
    *a += *b;
}
template<typename T>
void multiplyScalar(std::vector<T> & res, T const & in) {
  for( auto a = res.begin(); a != res.end(); ++a)
    *a *= in;
}
int main(int argc, char** argv) {
  if( argc != 3 ) {
    std::cerr << "wrong number of args." << std::endl; 
    return -1; 
  }

  const std::string dbfile(argv[1]);
  const std::string query(argv[2]);

  Request req;
  try {
    req = queryToRequest(query);
  } catch (std::exception const & exc ) {
    std::cerr << "ERROR while parsing request: " << exc.what() << std::endl;
    return 3;
  }

  sqlite3 *db;
  int rc = sqlite3_open(dbfile.c_str(), &db);
  if( rc ) {
    std::cerr << "ERROR opening sqlite file: " << sqlite3_errmsg(db);
    sqlite3_close(db);
    return 4;
  }
  Index idx;
  try {
    idx = getMatchingDatasetSpecs(db, req);
  } catch( std::exception & exc ) {
    sqlite3_close(db);
    std::cerr << "ERROR processing request: " << exc.what() << std::endl;
    return 5;
  }
  sqlite3_close(db);

  std::vector<std::complex<double>> result;
  std::size_t num_hits = 0;
  if( req.smode == SearchMode::FIRST ) {
    // read only the first match:
    try {
      if( idx.empty() )
        return 0;
      H5ReaderGeneric reader(idx.front().file);
      result = reader.read(idx.front());
      num_hits++;
    } catch (std::exception const & exc) {
      std::cerr << "ERROR while reading file: " << exc.what() << std::endl;
      return 1;
    }
  } else if ( req.smode == SearchMode::CONCATENATE or 
              req.smode == SearchMode::AVERAGE ){
    auto files = getUniqueFiles(idx);
    for( auto file : files ) {
      // process only this file:
      auto idxcp = idx;
      Request onlyThisFileReq;
      onlyThisFileReq.filerequests.push_back(std::unique_ptr<FileCondition>(
            new FileConditions::NameMatches(file.filename)));
      filterIndexByPostselectionRules(idxcp, onlyThisFileReq);
      try {
        H5ReaderGeneric reader(file);
        for (auto const & dsetspec : idxcp) {
          if( req.smode == SearchMode::CONCATENATE )
            concatenate( result, reader.read(dsetspec) );
          else if ( req.smode == SearchMode::AVERAGE )
            elementalAdd( result, reader.read(dsetspec) );
          num_hits++;
        }
      } catch (std::exception const & exc) {
        std::cerr << "ERROR while reading file: " << exc.what() << std::endl;
        return 1;
      }
    }
  } else {
    std::cerr << "ERROR: unknown search mode." << std::endl;
    return 2;
  }
  if( req.smode == SearchMode::AVERAGE ) {
    std::complex<double> r = num_hits;
    multiplyScalar(result, 1./r);
  }
  // output:
  for( auto const & number : result ) {
    std::cout << std::real(number) << " " << std::imag(number) << std::endl;
  }
  return 0;
}
