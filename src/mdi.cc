/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include <iostream>
#include "attributes.h"
#include "indexHdf5.h"
#include "sqliteHelpers.h"
#include "filehelpers.h"

using namespace rqcd_file_index;

void version(int argc, char** argv) {
  std::cerr << "TODO: take version info from cmake" << std::endl;
}
void help(int argc, char** argv) {
  std::cout << argv[0] << " [subcommand] [arguments]" << std::endl;
  std::cout << "\n" <<
    "available subcommands:" << std::endl <<
    "  create <idxfile> <hdf5 file>  indexes hdf5 file" << std::endl <<
    "  update <idxfile> <hdf5 file>  updates hdf5 file in the index" << std::endl <<
    "  rm <idxfile> <hdf5 file>      removes hdf5 file from index" << std::endl <<
    "  files <idxfile>               lists file contained in index" << std::endl <<
    "  attributes <idxfile>          lists attributes in index" << std::endl <<
    "  get <idxfile> <query>         outputs all data matching the query" << std::endl <<
    "  query <idxfile> <query>       shows all hits matching the query" << std::endl <<
    "                                (without reading from the hdf5 file)" << std::endl <<
    "  help                          outputs this help" << std::endl <<
    "  version                       outputs version information" << std::endl;
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

    sqlite_helpers::removeFile(db, rqcd_file_index::File(h5file));

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
    sqlite3_open(sqlfile.c_str(), &db);
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
    sqlite3_open(sqlfile.c_str(), &db);

    sqlite_helpers::removeFile(db, rqcd_file_index::File(h5file));

    Index idx = indexHdf5File(h5file);
    sqlite_helpers::insertDataset(db, idx);

    sqlite3_close(db);
  } catch ( std::exception const & exc ) {
    std::cerr << "ERROR " << exc.what() << std::endl;
    return 1;
  }
  
  return 0;
}

int main(int argc, char** argv) {

  if( argc < 2 ) {
    std::cerr << "no command given." << std::endl;
    help(argc, argv);
    return 1;
  }

  std::string command(argv[1]);

  if( command == "index" ) {
    return indexFile(argc, argv);
  } else if ( command == "rm" or command == "remove" ) {
    return removeFile(argc, argv);
  } else if ( command == "update" ) {
    return updateFile(argc, argv);
  } else if ( command == "help" ) {
    help(argc, argv);
    return 0;
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
