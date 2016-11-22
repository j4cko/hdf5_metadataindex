#include "sqliteHelpers.h"
#include <iostream>
static int insertStringCallback(void *idx, int argc, char** argv, char** azColName){
  std::vector<std::string>* vec = (std::vector<std::string>*)idx;
  if( argc != 1 ) { return -1; }
  vec->push_back(std::string(argv[0]));
  return 0;
}
static int insertIntCallback(void *idx, int argc, char** argv, char** azColName){
  std::vector<int>* vec = (std::vector<int>*)idx;
  if( argc != 1 ) { return -1; }
  std::stringstream sstr(argv[0]);
  int res;
  sstr >> res;
  vec->push_back(res);
  return 0;
}
static int getStringCallback(void *str, int argc, char** argv, char** azColName) {
  if( argc != 1) { return -1;}
  *((std::string*)str) = std::string(argv[0]);
  return 0;
}
static int printCallback(void *NotUsed, int argc, char** argv, char** azColName){
  for(int i = 0; i < argc; i++){
    std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
  }
  return 0;
}
static int insertAttributeCallback(void *vec, int argc, char** argv, char** azColName) {
  std::vector<Attribute>* attrvec = (std::vector<Attribute>*)vec;
  if( argc != 3 ) return -1;
  std::string name, val, type;
  for ( auto i = 0; i < argc; i++ ) {
    if( std::string(azColName[i]) == "attrname" )         name = argv[i];
    else if( std::string(azColName[i]) == "type" )        type = argv[i];
    else if( std::string(azColName[i]) == "value" )       val= argv[i];
    else return -2;
  }
  attrvec->push_back(attributeFromStrings(name, val, type));
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
      printCallback, 0, &zErrMsg );
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
    }
  }
  sstr << "commit transaction;";
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
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
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
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

std::vector<int> getLocIds(sqlite3 *db, Request const & req) {
  std::vector<int> res;
  //build sql query:
  std::stringstream sstr;
  sstr << "select locid from filelocations where locid in ";
  for( auto i = 0u; i < req.size(); ++i ){
    sstr << "(select locid from attrvalues where value=\"" << req[i].getValue() << "\" and attrid=(select attrid from attributes where attrname=\"" << req[i].getName() << "\"))";
    if( i < req.size() - 1 ) sstr << " and locid in ";
  }
  sstr << ";";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), insertIntCallback, &res, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  return res;
}
std::vector<std::string> idsToDsetnames(sqlite3 *db, std::vector<int> locids) {
  std::vector<std::string> res;
  std::stringstream sstr;
  if( locids.size() < 1 ) return res;
  sstr << "select locname from filelocations where locid in (";
  for( auto i = 0u; i < locids.size()-1; i++ ) { sstr << locids[i] << ","; }
  sstr << locids.back() << ");";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db, 
      sstr.str().c_str(), insertStringCallback, &res, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  return res;
}
//typedef std::pair<std::vector<Attribute>,std::string> DatasetSpec;
DatasetSpec idsToDatasetSpec(sqlite3 *db, int locid) {
  DatasetSpec res;
  std::stringstream sstr;
  // first get the locname:
  sstr << "select locname from filelocations where locid=\"" << locid << "\";";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db, 
      sstr.str().c_str(), getStringCallback, &(res.second), &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  // then get the attributes:
  sstr.clear(); sstr.str("");
  sstr << "select attrname,value,type from (select * from attributes inner join attrvalues on attributes.attrid = attrvalues.attrid) where locid=\"" << locid << "\";";
  rc = sqlite3_exec( db, 
      sstr.str().c_str(), insertAttributeCallback, &(res.first), &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg;
    throw std::runtime_error(errstr.str());
  }
  return res;
}
Index idsToIndex(sqlite3 *db, std::vector<int> locids) {
  Index res;
  //run over all locids:
  for( auto locid : locids ) res.push_back(idsToDatasetSpec(db, locid));
  return res;
}
