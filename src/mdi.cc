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
