#include "sqliteHelpers.h"
#include <sstream>
#include <iostream>
namespace rqcd_file_index {
namespace sqlite_helpers {
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
static int getIntCallback(void *intvar, int argc, char** argv, char** azColName) {
  if( argc != 1) { return -1;}
  std::stringstream sstr(argv[0]);
  sstr >> *((int*)intvar);
  return 0;
}
static int dsetspecFileinfoCallback(void *var, int argc, char** argv, char** azColName) {
  DatasetSpec* dsetspec = static_cast<DatasetSpec*>(var);
  if( argc != 4 ) return -1;
  for(int i = 0; i < argc; i++) {
    if( std::string(azColName[i]) == "locname" ) dsetspec->datasetname = std::string(argv[i]);
    else if( std::string(azColName[i]) == "fname" ) dsetspec->file.filename = std::string(argv[i]);
    else if( std::string(azColName[i]) == "row" )   dsetspec->location.begin = std::stoi(std::string(argv[i]));
    else if( std::string(azColName[i]) == "mtime" ) dsetspec->file.mtime = std::stoi(std::string(argv[i]));
  }
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
  std::string request(
      "create table attributes("
        "attrid integer primary key asc,"
        "attrname text unique,"
        "type   text);"
      "create table filelocations("
        "locid  integer primary key asc,"
        "locname text,"
        "row integer,"
        "fileid integer);"
      "create table attrvalues("
        "valueid  integer primary key asc,"
        "attrid int,"
        "locid  int,"
        "value  blob);"
      "create table files("
        "fileid integer primary key asc,"
        "fname text,"
        "mtime int);" );
  int rc = sqlite3_exec( db,
      request.c_str(),
      printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << request;
    throw std::runtime_error(errstr.str());
  } else { std::cout << "database created." << std::endl;}
}
std::string createAttributesInsertionString( Attribute const & attr) {
  std::stringstream sstr;
  // TODO: should find another sol than ignore..
  sstr <<
    "insert or ignore into attributes(attrname,type) values(\"" <<
    attr.getName() << "\", \"" << typeToString(attr.getType()) << "\");";
  return sstr.str();
}
std::string createFilelocInsertionString( DatasetSpec const & dset ) {
  std::stringstream sstr;
  sstr << 
    "insert into filelocations(locname,row,fileid) values('" << dset.datasetname << 
    "',"<<dset.location.begin << ",(select fileid from files where fname = '" << dset.file.filename << 
    "' and mtime = " << dset.file.mtime << "));";
  return sstr.str();
}
std::string createAttrValueInsertionString( DatasetSpec const & dset, 
                                            Attribute const & attr ) {
  std::stringstream sstr;
  sstr <<
  "insert into attrvalues(attrid,locid,value) values(( select attrid from attributes where attrname=\'" <<
  attr.getName() << "\'), (select locid from filelocations where locname=\'" << dset.datasetname << "\' and row=" << dset.location.begin << "),";
  if(attr.getType() == Type::STRING or attr.getType() == Type::ARRAY)
    //TODO: need to "escape" single quotes? single quotes are escaped by
    //single quotes..
    sstr << "\'" << attr.getValue() << "\');";
  else
    sstr << attr.getValue() << ");";
  return sstr.str();
}
void insertDataset(sqlite3 *db, Index const & idx ) {
  char *zErrMsg = nullptr;
  std::stringstream sstr;
  //insert all files:
  auto filelist = getUniqueFiles(idx);
  for( auto file : filelist )
    sstr << "insert into files(fname, mtime) values(\"" << file.filename << "\", "
                                                        << file.mtime << ");";
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  sstr.clear(); sstr.str("");
  
  //insert attributes and filelocations:
  sstr << "begin transaction;";
  for( auto dset : idx ){
    sstr << createFilelocInsertionString(dset);
    for( auto const & attr : dset.attributes )
      sstr << createAttributesInsertionString(attr);
  }
  sstr << "commit transaction;";
  rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  //connect attributes with locations:
  sstr.clear(); sstr.str("");
  sstr << "begin transaction;";
  for( auto const & dset : idx )
    for( auto const & attr : dset.attributes )
      sstr << createAttrValueInsertionString( dset, attr );
  sstr << "commit transaction;";
  rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
}
// find all with 250 smearing iterations:
// select locname from filelocations where locid in (select locid from attrvalues where value="250" and attrid=(select attrid from attributes where attrname="smeariter"));
//
// find all with 250 smeariter and 3 hpe:
// select locname from filelocations where locid in (select locid from attrvalues where value="3" and attrid=(select attrid from attributes where attrname="hpe")) and locid in (select locid from attrvalues where value="250" and attrid=(select attrid from attributes where attrname="smeariter"))

std::vector<int> getLocIdsMatchingPreSelection(sqlite3 *db, Request const & req) {
  std::vector<int> res;
  std::string query;
  //for empty requests, return everything:
  if( req.attrrequests.empty() )
    query = "select locid from filelocations;";
  else {
    //build sql query:
    std::stringstream sstr;
    sstr << "select locid from filelocations where locid in ";
    for( auto i = 0u; i < req.attrrequests.size(); ++i ){
      sstr << "(select locid from attrvalues where " << 
        req.attrrequests[i].getSqlValueDescription("value") << 
        " and attrid=(select attrid from attributes where " << 
        req.attrrequests[i].getSqlKeyDescription("attrname") << "))";
      if( i < req.attrrequests.size() - 1 ) sstr << " and locid in ";
    }
    sstr << ";";
    query = sstr.str();
  }
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db,
      query.c_str(), insertIntCallback, &res, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << query;
    throw std::runtime_error(errstr.str());
  }
  return res;
}
std::vector<std::string> idsToDsetnames(sqlite3 *db,
    std::vector<int> const & locids) {
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
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  return res;
}
int getFileModificationTime(sqlite3 *db, std::string const & filename) {
  int mtimeDb = 0;
  std::stringstream sstr;
  sstr << "select mtime from files where fname=\"" << filename << "\";";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db, 
      sstr.str().c_str(), getIntCallback, &mtimeDb, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  return mtimeDb;
}
std::vector<std::string> idsToFilenames(sqlite3 *db,
    std::vector<int> const & locids) {
  std::vector<std::string> res;
  std::stringstream sstr;
  //select * from files where fileid=(select fileid from filelocations where locid=5);
  if( locids.size() < 1 ) return res;
  for( auto const & locid : locids ) {
    sstr << "select fname from files where fileid=(select fileid from filelocations where locid=" << locid << ");";
  }
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db, 
    sstr.str().c_str(), insertStringCallback, &res, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  return res;
}
//typedef std::pair<std::vector<Attribute>,std::string> DatasetSpec;
DatasetSpec idsToDatasetSpec(sqlite3 *db, int locid) {
  DatasetSpec res;
  std::stringstream sstr;
  // first get the locname (datasetname)
  // select locname,fname,mtime from (select * from filelocations inner join files on filelocations.fileid = files.fileid);
  sstr << "select locname,row,fname,mtime from (select * from filelocations inner join files on filelocations.fileid = files.fileid) where locid=\"" << locid << "\";";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db, 
      sstr.str().c_str(), dsetspecFileinfoCallback, &res, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  // then get the attributes:
  sstr.clear(); sstr.str("");
  sstr << "select attrname,value,type from (select * from attributes inner join attrvalues on attributes.attrid = attrvalues.attrid) where locid=\"" << locid << "\";";
  rc = sqlite3_exec( db, 
      sstr.str().c_str(), insertAttributeCallback, &(res.attributes), &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
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
} // sqlite_helpers
} // rqcd_file_index
