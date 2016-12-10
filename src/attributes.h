#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <list>
#include <ostream>
#include <cassert>
#include "value.h"

namespace rqcd_file_index {
class Attribute {
  public:
    Attribute(std::string const & name, Value const & val_) : 
      attrname(name), val(val_) { }
    Type getType() const { return val.getType(); }
    Value getValue() const { return val; };
    std::string getName() const { return attrname; };
  private:
    std::string attrname;
    Value val;
};

class AttributeCondition {
  public:
    virtual bool matches(Attribute const & attr, std::string const & reqname) const = 0;
    virtual std::unique_ptr<AttributeCondition> clone() const = 0;
    virtual ~AttributeCondition() {} // need to implement this
    //TODO: further abstraction for cases that do not rely on SQL?
    //these sql request are only supposed to be a prefiltering, the final decision if
    //an attribute matches the condition is done by the routine matches!
    //  (thus, returning "is not null" is a valid default here)
    virtual std::string getSqlValueDescription(std::string const & valentryname) const {
      return valentryname + std::string(" is not null");
    }
    virtual std::string getSqlKeyDescription(std::string const & keyentryname, std::string const & name) const {
      return keyentryname + std::string(" = '") + name + std::string("'");
    }
};

class AttributeRequest {
  public:
    AttributeRequest(std::string const & name, AttributeCondition const & in) :
      reqname(name), cond(std::move(in.clone())) {}
    bool matches(Attribute const & attr) const { return cond->matches(attr, reqname); }
    std::string getName() const { return reqname; }
    std::string getSqlKeyDescription(std::string const & keyentryname) const { 
      return cond->getSqlKeyDescription(keyentryname, reqname); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      return cond->getSqlValueDescription(valentryname); }
  private:
    std::string reqname;
    std::unique_ptr<AttributeCondition> cond;
};
struct File {
  File(std::string const & name, int time) : filename(name), mtime(time) {}
  std::string filename;
  int mtime;
  friend bool operator==(File const & a, File const & b) {
    return a.filename == b.filename and a.mtime == b.mtime;
  }
};
class FileCondition {
  public:
    virtual bool matches(File const & file) const = 0;
    virtual std::unique_ptr<FileCondition> clone() const = 0;
    virtual ~FileCondition() {}
};
struct DatasetChunkSpec {
  DatasetChunkSpec(int const & start) : begin(start) {}
  DatasetChunkSpec() : begin(-1) {}; //default: read complete dataset.
  int begin;
};
class Hdf5DatasetCondition {
  public:
    virtual bool matches(std::string datasetname, DatasetChunkSpec loc) const = 0;
    virtual std::unique_ptr<Hdf5DatasetCondition> clone() const = 0;
    virtual ~Hdf5DatasetCondition() {};
};
typedef std::unique_ptr<FileCondition> FileRequest;
typedef std::unique_ptr<Hdf5DatasetCondition> Hdf5DatasetRequest;
struct DatasetSpec {
  DatasetSpec(std::vector<Attribute> const & attr, std::string const & dsetname, File const & file_, DatasetChunkSpec const & loc) : attributes(attr), datasetname(dsetname), file(file_), location(loc) {};
  DatasetSpec() : attributes(), datasetname(""), file({"", 0}), location() {};
  std::vector<Attribute> attributes;
  std::string datasetname;
  File file;
  DatasetChunkSpec location; // location within the dataset
};

typedef std::vector<DatasetSpec> Index;

enum class SearchMode {
  FIRST,
  AVERAGE,
  CONCATENATE
};
SearchMode searchModeFromString(std::string str);
struct Request {
  std::vector<AttributeRequest> attrrequests;
  std::vector<Hdf5DatasetRequest> dsetrequests;
  std::vector<FileRequest>      filerequests;
  //TODO later:
  //std::vector<LuaRequest>       luarequests;
  SearchMode smode = SearchMode::FIRST;
};

std::list<File> getUniqueFiles(Index const & idx);

Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr);

Value valueFromString(std::string const & str);
void mergeIndex( Index & res, Index const & idxToMerge );
std::string getFullpath( Index const & idxstack );
void printIndex(Index const & idx, std::ostream& os);
std::vector<Attribute> tableFieldsToAttributes(Value const & table);
Index splitDsetSpecWithTable( DatasetSpec const & dset, Value const & table );
Index expandIndex(Index const & idx, std::map<std::string, Value> const & tables);
}
#endif
