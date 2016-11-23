#ifndef __SQLITEHELPERS_H__
#define __SQLITEHELPERS_H__
#include <sqlite3.h>
#include <sstream>
#include "attributes.h"
#include "indexHdf5.h"

void prepareSqliteFile(sqlite3 * db);
void insertDataset(sqlite3 *db, 
                   Index const & idx,
                   std::string const & file, unsigned int mtime);
DatasetSpec idsToDatasetSpec(sqlite3 *db, int locid);
std::vector<std::string> idsToDsetnames(sqlite3 *db, std::vector<int> const & locids);
std::vector<std::string> idsToFilenames(sqlite3 *db, std::vector<int> const & locids);
std::vector<int> getLocIds(sqlite3 *db, Request const & req);
Index idsToIndex(sqlite3 *db, std::vector<int> locids);
int getFileModificationTime(sqlite3 *db, std::string filename);
#endif
