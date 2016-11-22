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
// find all with 250 smearing iterations:
// select locname from filelocations where locid in (select locid from attrvalues where value="250" and attrid=(select attrid from attributes where attrname="smeariter"));
//
// find all with 250 smeariter and 3 hpe:
// select locname from filelocations where locid in (select locid from attrvalues where value="3" and attrid=(select attrid from attributes where attrname="hpe")) and locid in (select locid from attrvalues where value="250" and attrid=(select attrid from attributes where attrname="smeariter"))

Index queryDb(sqlite3 *db, Request const & req) {
  Index res;
  //build sql query:
  std::stringstream sstr;
  sstr << "select locname from filelocations where locid in ";
  for( auto i = 0u; i < req.size(); ++i ){
    sstr << "(select locid from attrvalues where value=\"" << req[i].getValue() << "\" and attrid=(select attrid from attributes where attrname=\"" << req[i].getName() << "\"))";
    if( i < req.size() - 1 ) sstr << " and locid in ";
  }
  sstr << ";";
  std::cout << "DEBUG REQUEST: " << sstr.str() << std::endl;
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), callback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  return res;
}
