#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__
#include <vector>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <memory>
#include <list>

enum class Type {
  NUMERIC,
  BOOLEAN,
  STRING
};

std::string typeToString(Type const & type);
Type typeFromTypeid(std::type_info const & tinfo);

class Value {
public:
  template <typename T>
  Value(T const &content_) : content(new Model<T>(content_)), 
                             type(typeFromTypeid(typeid(T))) {}
  ~Value() { delete content; }
  Value(Value const &rr) : content(rr.content->clone()), type(rr.getType()) {}
  Value &operator=(Value rr) {
    if (content->ti.hash_code() != rr.content->ti.hash_code())
      throw std::runtime_error("Assigning Values of different types!");
    std::swap(content, rr.content);
    return *this;
  }
  template <typename T>
  operator T() const { return this->get<T>(); }
  Type getType() const { return type; }
  bool typeMatches(std::type_info const &tinfo) const {
    return content->ti.hash_code() == tinfo.hash_code();
  }
  template <typename Tout> Tout get() const {
    if (typeid(Tout).hash_code() != content->ti.hash_code())
      throw std::runtime_error("Value.get(): types do not match.");
    return (Tout)((Model<Tout> *)content)->object;
  }
  friend std::ostream& operator<<(std::ostream& os, Value const & val) {
    val.content->print(os);
    return os;
  }
  friend bool operator==(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) return false;
    return a.content->equals(b.content);
  }
  friend bool operator<(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<: comparing Values of different type.");
    return a.content->less(b.content);
  }
  friend bool operator<=(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<: comparing Values of different type.");
    return a.content->lessEqual(b.content);
  }
  friend bool operator>(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<: comparing Values of different type.");
    return a.content->greater(b.content);
  }
  friend bool operator>=(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<: comparing Values of different type.");
    return a.content->greaterEqual(b.content);
  }
private:
  struct Concept {
    virtual ~Concept() {}
    Concept() = delete;
    Concept(std::type_info const &ti_) : ti(ti_) {}
    virtual Concept *clone() const = 0;
    virtual void print(std::ostream& os) const = 0;
    virtual bool equals(Concept const * other) const = 0;
    virtual bool less(Concept const * other) const = 0;
    virtual bool lessEqual(Concept const * other) const = 0;
    virtual bool greater(Concept const * other) const = 0;
    virtual bool greaterEqual(Concept const * other) const = 0;
    std::type_info const &ti;
  };

  template <typename T> struct Model : Concept {
    Model(T const &value) : Concept(typeid(T)), object(value) {}
    void print(std::ostream& os) const { os << object; }
    bool equals(Concept const * other) const {
      if( other->ti.hash_code() != ti.hash_code() ) return false;
      //now, a cast is safe:
      return (object == static_cast<Model<T> const*>(other)->object);
    }
    bool less(Concept const * other) const {
      if( other->ti.hash_code() != ti.hash_code() ) return false;
      //now, a cast is safe:
      return (object < static_cast<Model<T> const*>(other)->object);
    }
    bool lessEqual(Concept const * other) const {
      if( other->ti.hash_code() != ti.hash_code() ) return false;
      //now, a cast is safe:
      return (object <= static_cast<Model<T> const*>(other)->object);
    }
    bool greater(Concept const * other) const {
      if( other->ti.hash_code() != ti.hash_code() ) return false;
      //now, a cast is safe:
      return (object > static_cast<Model<T> const*>(other)->object);
    }
    bool greaterEqual(Concept const * other) const {
      if( other->ti.hash_code() != ti.hash_code() ) return false;
      //now, a cast is safe:
      return (object >= static_cast<Model<T> const*>(other)->object);
    }
    Concept *clone() const { return new Model(object); }
    T object;
  };
  Concept *content;
  Type type;
};

class Attribute {
  public:
    template<typename T>
    Attribute(std::string const & name, T const & in) : 
      attrname(name), val(in) {
      assert(val.typeMatches(typeid(T)));
    }
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
};
typedef std::unique_ptr<FileCondition> FileRequest;
struct DatasetSpec {
  std::vector<Attribute> attributes;
  std::string datasetname;
  File file;
};

typedef std::vector<DatasetSpec> Index;
struct Request {
  std::vector<AttributeRequest> attrrequests;
  std::vector<FileRequest>      filerequests;
  //TODO later:
  //std::vector<LuaRequest>       luarequests;
};

std::list<File> getUniqueFiles(Index const & idx);

Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr);
#endif
