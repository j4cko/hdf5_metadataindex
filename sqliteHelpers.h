#ifndef __SQLITEHELPERS_H__
#define __SQLITEHELPERS_H__
#include <sqlite3.h>
#include <sstream>
#include "attributes.h"
#include "indexHdf5.h"

void prepareSqliteFile(sqlite3 * db);
void insertDataset(sqlite3 *db, 
     Index const & idx);
Index queryDb(sqlite3 *db, Request const & req);
#endif
