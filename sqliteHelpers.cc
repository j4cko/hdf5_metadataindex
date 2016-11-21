#include "sqliteHelpers.h"
#include <iostream>
static int callback(void *NotUsed, int argc, char** argv, char** azColName){
  for(int i = 0; i < argc; i++){
    std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
  }
  return 0;
}
void prepareSqliteFile(sqlite3 *db) {
  /* creates the following tables:
   *
   * attributes:
   * id | name | type
   *
   * filelocations:
   * id | name | attrIds (?)
   *
   * attrvalues:
   * id | attrId | value | locId
   */
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db,
      "create table attributes("
      "attrid integer primary key asc,"
      "attrname text unique,"
      "type   text);"
      "create table filelocations("
      "locid  integer primary key asc,"
      "locname text);"
      "create table attrvalues("
      "valueid  integer primary key asc,"
      "attrid int,"
      "locid  int,"
      "value  blob);",
      callback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  } else { std::cout << "DEBUG: created... ok." << std::endl;}
}
void insertDataset(sqlite3 *db, 
                   Index const & idx ){
  char *zErrMsg = nullptr;
  std::stringstream sstr;
  //insert attributes and filelocations:
  sstr << "begin transaction;";
  for( auto dset : idx ){
    sstr << 
      "insert into filelocations(locname) values(\"" << dset.second << "\");";
    for( auto attr : dset.first ){
      // TODO: should find another sol than ignore..
      sstr <<
        "insert or ignore into attributes(attrname,type) values(\"" <<
        attr.getName() << "\", \"" << typeToString(attr.getType()) << "\");";
      std::cout <<
        "insert or ignore into attributes(attrname,type) values(\"" <<
        attr.getName() << "\", \"" << typeToString(attr.getType()) << "\");" << std::endl;
    }
  }
  sstr << "commit transaction;";
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), callback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  //connect attributes with locations:
  sstr.clear(); sstr.str("");
  sstr << "begin transaction;";
  for( auto dset : idx ){
    for( auto attr : dset.first ){
      sstr <<
        "insert into attrvalues(attrid,locid,value) values(( select attrid from attributes where attrname=\"" <<
        attr.getName() << "\"), (select locid from filelocations where locname=\"" << dset.second << "\"),\"" <<
        attr.getValue() << "\");";
    }
  }
  sstr << "commit transaction;";
  rc = sqlite3_exec( db,
      sstr.str().c_str(), callback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
}
