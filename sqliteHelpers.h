#ifndef __SQLITEHELPERS_H__
#define __SQLITEHELPERS_H__
#include <sqlite3.h>
#include <sstream>
#include "attributes.h"

void prepareSqliteFile(sqlite3 * db);
void insertDataset(sqlite3 *db, 
     Index const & idx);
#endif
