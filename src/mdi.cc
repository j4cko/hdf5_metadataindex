/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include <iostream>
#include <complex>
#include "attributes.h"
#include "indexHdf5.h"
#include "sqliteHelpers.h"
#include "filehelpers.h"
#include "postselection.h"
#include "hdf5ReaderGeneric.h"
#include "parseJson.h"

using namespace rqcd_file_index;

Index getMatchingDatasetSpecs(sqlite3 *db, Request const & req) {
  auto ids = sqlite_helpers::getLocIdsMatchingPreSelection(db, req);
  Index idx = sqlite_helpers::idsToIndex(db, ids);
  filterIndexByPostselectionRules(idx, req);
  return idx;
}
bool fileNeedsUpdate(File const & file) {
  return file.mtime < FileHelpers::getFileModificationTime(file.filename);
}
void version(int argc, char** argv) {
  std::cerr << "TODO: take version info from cmake" << std::endl;
}
void help(int argc, char** argv) {
  std::cout << argv[0] << " [subcommand] [arguments]" << std::endl;
  std::cout << "\n" <<
    "available subcommands:" << std::endl <<
    "  index <idxfile> <hdf5 file>   indexes hdf5 file" << std::endl <<
    "  update <idxfile> <hdf5 file>  updates hdf5 file in the index" << std::endl <<
    "  updateAll <idxfile>           updates all files in the index" << std::endl <<
    "  rm <idxfile> <hdf5 file>      removes hdf5 file from index" << std::endl <<
    "  files <idxfile>               lists file contained in index" << std::endl <<
    "  attributes <idxfile>          lists attributes in index" << std::endl <<
    "  get <idxfile> <query>         outputs all data matching the query" << std::endl <<
    "  query <idxfile> <query>       shows all hits matching the query" << std::endl <<
    "                                (without reading from the hdf5 file)" << std::endl <<
    "  help                          outputs this help" << std::endl <<
    "  version                       outputs version information" << std::endl;
}
int listAttributes(int argc, char** argv) {
  if( argc != 3 ) {
    std::cerr << "TODO give help for attributes." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);
  try {
    if( not FileHelpers::file_exists(sqlfile) ) {
      throw std::runtime_error("index file does not exist!");
    }
    sqlite3 *db;
    sqlite3_open(sqlfile.c_str(), &db);

    auto attributes = sqlite_helpers::listAttributes(db);

    sqlite3_close(db);

    for( auto const & attr : attributes ) { 
      std::cout << "  - " << attr.first << " (" << attr.second << ")" << std::endl;
    }
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  return 0;
}
int listFiles(int argc, char** argv) {
  if( argc != 3 ) {
    std::cerr << "TODO give help for files." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);
  try {
    if( not FileHelpers::file_exists(sqlfile) ) {
      throw std::runtime_error("index file does not exist!");
    }
    sqlite3 *db;
    sqlite3_open(sqlfile.c_str(), &db);

    auto files = sqlite_helpers::listFiles(db);

    sqlite3_close(db);

    for( auto const & file : files ) { 
      if( not FileHelpers::file_exists(file.filename) ) {
        std::cout << "[abs] ";
      } else if( fileNeedsUpdate(file) ) {
        std::cout << "[upd] ";
      } else {
        std::cout << " [ok] ";
      }
      std::cout << file.filename << std::endl;
    }
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  return 0;
}
int removeFile(int argc, char** argv) {
  if( argc != 4 ) {
    std::cerr << "TODO give help for rm." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);
  const std::string h5file(argv[3]);

  try {
    if( not FileHelpers::file_exists(sqlfile) ) {
      throw std::runtime_error("index file does not exist!");
    }
    sqlite3 *db;
    sqlite3_open(sqlfile.c_str(), &db);

    sqlite_helpers::removeFile(db, h5file);

    sqlite3_close(db);
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  return 0;
}
int indexFile(int argc, char** argv) {
  if( argc != 4 )
  {
    std::cerr << "TODO give help for indexFile." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);
  const std::string h5file(argv[3]);

  try {
    sqlite3 *db;
    char *zErrMsg = nullptr;
    sqlite3_open(sqlfile.c_str(), &db);
    // does this help to improve performance?
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
    sqlite_helpers::prepareSqliteFile(db);

    Index idx = indexHdf5File(h5file);

    sqlite_helpers::insertDataset(db, idx);

    sqlite3_close(db);
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  
 // printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
int updateAllFiles(int argc, char** argv) {
  //TODO: might do something better than first remove and then add file:
  if( argc != 3 )
  {
    std::cerr << "TODO give help for updateFiles." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);

  try {
    sqlite3 *db;
    char *zErrMsg = nullptr;
    sqlite3_open(sqlfile.c_str(), &db);
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);

    auto files = sqlite_helpers::listFiles(db);

    for( auto const & file : files ) {
      if( fileNeedsUpdate( file ) )
      {
        if( FileHelpers::file_exists(file.filename) ){
          std::cout << "updating file \"" << file.filename << "\"..." << std::endl;
          sqlite_helpers::removeFile(db, file.filename);
          Index idx = indexHdf5File(file.filename);
          sqlite_helpers::insertDataset(db, idx);
        } else {
          std::cout << "file \"" << file.filename << "\" no longer exists. removing." << std::endl;
          sqlite_helpers::removeFile(db, file.filename);
        }
      } else {
        std::cout << "file \"" << file.filename << "\" is up to date." << std::endl;
      }
    }

    sqlite3_close(db);
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  
  return 0;
}
int updateFile(int argc, char** argv) {
  //TODO: might do something better than first remove and then add file:
  if( argc != 4 )
  {
    std::cerr << "TODO give help for indexFile." << std::endl;
    return 1;
  }
  const std::string sqlfile(argv[2]);
  const std::string h5file(argv[3]);

  try {
    sqlite3 *db;
    char *zErrMsg = nullptr;
    sqlite3_open(sqlfile.c_str(), &db);
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);

    if( not fileNeedsUpdate( sqlite_helpers::getFile(db, h5file) ) )
    {
      std::cerr << "ERROR File \"" << h5file << "\" doesn't need update.. To force an update first remove (rm) the file and then add it again." << std::endl;
      return 1;
    }
    sqlite_helpers::removeFile(db, h5file);

    Index idx = indexHdf5File(h5file);
    sqlite_helpers::insertDataset(db, idx);

    sqlite3_close(db);
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  
  return 0;
}
int queryDb(int argc, char** argv) {
  if( argc != 4 ) {
    std::cerr << "wrong number of args." << std::endl; 
    return -1; 
  }

  const std::string dbfile(argv[2]);
  const std::string query(argv[3]);

  Request req = queryToRequest(query);

  sqlite3 *db;
  sqlite3_open(dbfile.c_str(), &db);

  auto idx = getMatchingDatasetSpecs(db, req);

  printIndex(idx, std::cout);

  sqlite3_close(db);

  return 0;
}

void outputData( std::vector<std::pair<DatasetSpec, std::vector<std::complex<double>>>> const & res ) {
  for( auto const & hit : res ) {
    std::cout << hit.first << std::endl;
    for( auto const & nmbr : hit.second ) {
      std::cout << "  " << std::real(nmbr) << " " << std::imag(nmbr) << std::endl;
    }
  }
}
int getData(int argc, char** argv) {
  if( argc != 4 ) {
    std::cerr << "wrong number of args." << std::endl; 
    return -1; 
  }

  const std::string dbfile(argv[2]);
  const std::string query(argv[3]);

  Request req = queryToRequest(query);

  sqlite3 *db;
  sqlite3_open(dbfile.c_str(), &db);

  auto idx = getMatchingDatasetSpecs(db, req);

  std::vector<std::pair<DatasetSpec, std::vector<std::complex<double>>>> res;
  if ( req.smode == SearchMode::FIRST ) 
  {
    try {
      rqcd_hdf5_reader_generic::H5ReaderGeneric reader(idx.front().file);
      res.push_back(make_pair(idx.front(), reader.read(idx.front())));
    } catch (std::exception const & exc) {
      std::cerr << "ERROR while reading file: " << exc.what() << std::endl;
      return 1;
    }
  } else if ( req.smode == SearchMode::ALL ) {
    auto files = getUniqueFiles(idx);
    for( auto file : files ) {
      // process only this file:
      auto idxcp = idx;
      Request onlyThisFileReq;
      onlyThisFileReq.filerequests.push_back(std::unique_ptr<FileCondition>(
            new FileConditions::NameMatches(file.filename)));
      filterIndexByPostselectionRules(idxcp, onlyThisFileReq);
      try {
        rqcd_hdf5_reader_generic::H5ReaderGeneric reader(file);
        for( auto const & dset : idxcp ) {
          res.push_back(std::make_pair(dset, reader.read(dset)));
        }
      } catch (std::exception const & exc) {
        std::cerr << "ERROR while reading file: " << exc.what() << std::endl;
        return 1;
      }
    }
  }

  sqlite3_close(db);

  outputData(res);

  return 0;
}

int main(int argc, char** argv) {

  if( argc < 2 ) {
    std::cerr << "no command given." << std::endl;
    help(argc, argv);
    return 1;
  }

  std::string command(argv[1]);

  if( command == "index" or command  == "add" ) {
    return indexFile(argc, argv);
  } else if ( command == "rm" or command == "remove" ) {
    return removeFile(argc, argv);
  } else if ( command == "update" ) {
    return updateFile(argc, argv);
  } else if ( command == "updateAll" ) {
    return updateAllFiles(argc, argv);
  } else if ( command == "help" ) {
    help(argc, argv);
    return 0;
  } else if ( command == "files" ) {
    return listFiles(argc, argv);
  } else if ( command == "attributes" or command == "attr" ) {
    return listAttributes(argc, argv);
  } else if ( command == "query" ) {
    return queryDb(argc, argv);
  } else if ( command == "get" ) {
    return getData(argc, argv);
  } else if ( command == "version" ) {
    version(argc, argv);
    return 0;
  } else {
    std::cerr << "unknown command." << std::endl;
    help(argc, argv);
    return 1;
  }

  return -1;
}
