/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#ifndef __SQLITEHELPERS_H__
#define __SQLITEHELPERS_H__
#include <sqlite3.h>
#include <string>
#include "attributes.h"
#include "indexHdf5.h"

namespace rqcd_file_index {
namespace sqlite_helpers {
void prepareSqliteFile(sqlite3 * db);
File getFile(sqlite3 *db, std::string const & file);
void removeFile(sqlite3 *db, std::string const & file);
std::vector<File> listFiles(sqlite3* db);
std::vector<std::pair<std::string, std::string>> listAttributes(sqlite3* db);
void insertDataset(sqlite3 *db, Index const & idx);
DatasetSpec idsToDatasetSpec(sqlite3 *db, int locid);
std::vector<std::string> idsToDsetnames(sqlite3 *db, std::vector<int> const & locids);
std::vector<std::string> idsToFilenames(sqlite3 *db, std::vector<int> const & locids);
std::vector<int> getLocIdsMatchingPreSelection(sqlite3 *db, Request const & req);
Index idsToIndex(sqlite3 *db, std::vector<int> locids);
int getFileModificationTime(sqlite3 *db, std::string const & filename);
}}
#endif
