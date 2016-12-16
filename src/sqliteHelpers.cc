/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "sqliteHelpers.h"
#include <sstream>
#include <set>
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
    else if( std::string(azColName[i]) == "row" )   dsetspec->location.row = std::stoi(std::string(argv[i]));
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
      "create table if not exists files("
        "fileid integer primary key asc,"
        "fname text unique,"
        "mtime int);" 
      "create table if not exists attributes("
        "attrid integer primary key asc,"
        "attrname text unique,"
        "type   text);"
      "create table if not exists filelocations("
        "locid  integer primary key asc,"
        "locname text,"
        "row integer,"
        "fileid integer references files(fileid));"
      "create table if not exists attrvalues("
        "valueid  integer primary key asc,"
        "attrid int references attributes(attrid),"
        "value  blob);"
      "create table if not exists locattrjunction("
        "attrvalid integer references attrvalues(valueid),"
        "locid integer references filelocations(locid));");

  int rc = sqlite3_exec( db,
      request.c_str(),
      printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << request;
    throw std::runtime_error(errstr.str());
  } else { std::cout << "database created." << std::endl;}
}
std::string createFilelocInsertionString( 
    std::tuple<std::string, std::string, int> const & fileloc ) {
  std::stringstream sstr;
  sstr << 
    "insert or ignore into filelocations(locname,row,fileid) select '" << 
      std::get<1>(fileloc) << //locname
      "'," << std::get<2>(fileloc) << ",(select fileid from files where fname = '" 
      << std::get<0>(fileloc) <<  //filename
    "') where not exists (select 1 from filelocations where locname = '"
    << std::get<1>(fileloc) << "' and row = " << std::get<2>(fileloc) << 
    " and fileid = (select fileid from files where fname = '" << std::get<0>(fileloc) 
    << "'));";
  return sstr.str();
}
std::string createAttributesInsertionString( std::pair<std::string, std::string> const & attr ) {
  std::stringstream sstr;
  sstr << 
    "insert or ignore into attributes(attrname,type) values('" <<
    attr.first << "', '" << attr.second << "');";
  return sstr.str();
}
std::string createAttributeValuesInsertionString( 
    std::tuple<std::string, std::string, std::string> const & attrval ) {
  std::stringstream sstr;
  sstr << 
    "insert into attrvalues(attrid, value) select " <<
    "(select attrid from attributes where attrname = '" << std::get<0>(attrval) <<
    "' and type = '" <<  std::get<1>(attrval) << "'), ";
    if( std::get<1>(attrval) == "array" or std::get<1>(attrval) == "string") {
      sstr << "'" << std::get<2>(attrval) << "'";
    } else {
      sstr << std::get<2>(attrval) << "";
    }
  sstr << " where not exists ( select 1 from attrvalues where attrid = "
    "(select attrid from attributes where attrname = '" << std::get<0>(attrval) <<
    "' and type = '" << std::get<1>(attrval) << "') and value = ";
    if( std::get<1>(attrval) == "array" or std::get<1>(attrval) == "string") {
      sstr << "'" << std::get<2>(attrval) << "');";
    } else {
      sstr << std::get<2>(attrval) << ");";
    }
  return sstr.str();
}
std::string createJunctionInsertionString(
  std::pair<std::tuple<std::string, std::string, int>, std::tuple<std::string, std::string, std::string>> const & junc ) {
  std::stringstream sstr;

  sstr <<
    "insert into locattrjunction(attrvalid, locid) select ( "
    "select valueid from attrvalues where attrid = ( "
    "select attrid from attributes where attrname = '"
    << std::get<0>(junc.second) << "' and type = '" << std::get<1>(junc.second) << "') and value = ";
  if( std::get<1>(junc.second) == "string" or std::get<1>(junc.second) == "array" ) {
    sstr << "'" << std::get<2>(junc.second) << "'";
  } else {
    sstr << std::get<2>(junc.second);
  }
  sstr << "), (select locid from filelocations where locname = '" 
       << std::get<1>(junc.first) << "' and row = " << std::get<2>(junc.first) 
       << " and fileid = ( select fileid from files where fname = '" 
       << std::get<0>(junc.first) << "')) where not exists ( select 1 from "
       "locattrjunction where attrvalid = (select valueid from attrvalues where attrid = ( "
       "select attrid from attributes where attrname = '"
       << std::get<0>(junc.second) << "' and type = '" << std::get<1>(junc.second) << "' and value = ";
  if( std::get<1>(junc.second) == "string" or std::get<1>(junc.second) == "array" ) {
    sstr << "'" << std::get<2>(junc.second) << "'";
  } else {
    sstr << std::get<2>(junc.second);
  }
  sstr << ")));";
  return sstr.str();
}
void removeFile(sqlite3 *db, File const & file) {
  // removes file from database, i.e. removes all filelocations and
  // locattrjunctions and files pointing to this file. doesn't remove attributes
  // and attrvalues because these might potentially be used from other files.
  std::stringstream sstr;
  sstr << "begin transaction;"; 
  // delete locattrjunctions:
  sstr << "delete from locattrjunction where locid in "
    "(select locid from filelocations where fileid = "
       "(select fileid from files where fname = '" << file.filename << "' and "
       "mtime = " << file.mtime << "));";
  // delete filelocations:
  sstr << "delete from filelocations where fileid = "
       "(select fileid from files where fname = '" << file.filename << "' and "
       "mtime = " << file.mtime << ");";
  // delete file itself:
  sstr << "delete from files where fname = '"  << file.filename << "' and "
       "mtime = " << file.mtime << ";";
  sstr << "commit transaction;";
  char *zErrMsg = nullptr;
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
}
void insertDataset(sqlite3 *db, Index const & idx ) {
  char *zErrMsg = nullptr;
  std::stringstream sstr;
  //insert all files:
  auto filelist = getUniqueFiles(idx);
  for( auto file : filelist ){
    /*sstr << "insert or replace into files(fname, mtime) values('" 
      << file.filename << "', " << file.mtime << ");";
      */
    sstr << "insert into files(fname, mtime) select '" << file.filename << "', " << file.mtime << " where not exists (select 1 from files where fname = '" << file.filename << "');";
  }
  int rc = sqlite3_exec( db,
      sstr.str().c_str(), printCallback, 0, &zErrMsg );
  if( rc != SQLITE_OK ) {
    std::stringstream errstr;
    errstr << "SQL error: " << zErrMsg << "\nfailed request was: " << sstr.str();
    throw std::runtime_error(errstr.str());
  }
  sstr.clear(); sstr.str("");
  
  //insert attributes and filelocations:
  // pre-compile a unique list of insertions:
  {
    std::set<std::pair<std::string, std::string>> attributes; //attribute name, type
    std::set<std::tuple<std::string, std::string, std::string>> attrvalues; //attribute name, type, value
    std::set<std::tuple<std::string, std::string, int>> filelocations; //file, dsetname, row
    // this seems to be a rather complicated construct, but is just a pair of
    // the filelocation and the attrvalue:
    std::set<std::pair<std::tuple<std::string, std::string, int>, std::tuple<std::string, std::string, std::string>>> junctions;
    std::stringstream valsstr;
    for( auto const & dset : idx ) {
      filelocations.insert(std::make_tuple(dset.file.filename, dset.datasetname, dset.location.row));
      for( auto const & attr : dset.attributes ) {
        valsstr.clear(); valsstr.str("");
        valsstr << attr.getValue();
        attributes.insert(std::make_pair(attr.getName(), typeToString(attr.getType())));
        attrvalues.insert(std::make_tuple(attr.getName(), typeToString(attr.getType()), valsstr.str()));
        junctions.insert(std::make_pair(
          std::make_tuple(dset.file.filename, dset.datasetname, dset.location.row),
          std::make_tuple(attr.getName(), typeToString(attr.getType()), valsstr.str())));
      }
    }
    sstr << "begin transaction;";
    for( auto const & fileloc : filelocations ){
      sstr << createFilelocInsertionString(fileloc);
    }
    sstr << "commit transaction; begin transaction;";
    for( auto const & attr : attributes ){
      sstr << createAttributesInsertionString(attr);
    }
    sstr << "commit transaction; begin transaction;";
    for( auto const & attr : attrvalues ){
      sstr << createAttributeValuesInsertionString(attr);
    }
    sstr << "commit transaction; begin transaction;";
  //TODO: fileattrjunction
    for( auto const & junc : junctions ) {
      sstr << createJunctionInsertionString(junc);
    }
    sstr << "commit transaction;";
  }
  
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
    for( auto i = 0u; i < req.attrrequests.size(); ++i ) {
      sstr << "(select locid from locattrjunction where "
        "attrvalid = (select valueid from attrvalues where "
        << req.attrrequests[i].getSqlValueDescription("value") 
        << " and attrid = (select attrid from attributes where " 
        << req.attrrequests[i].getSqlKeyDescription("attrname") << ")))";
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
  sstr << "select locname,row,fname,mtime from (select * from filelocations inner join files on filelocations.fileid = files.fileid) where locid=" << locid << ";";
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
  sstr << "select attrname,value,type from (select * from attributes inner join attrvalues on attributes.attrid = attrvalues.attrid) where valueid in (select attrvalid from locattrjunction where locid = " << locid << ");";
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
