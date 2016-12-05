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
  ~Value() { delete content; }
  Value(double val) : content(new NumericModel(val)), type(Type::NUMERIC) {}
  Value(int val) : content(new NumericModel((double)val)), type(Type::NUMERIC) {}
  Value(std::size_t val) : content(new NumericModel((double)val)), type(Type::NUMERIC) {}
  Value(bool val) : content(new BooleanModel(val)), type(Type::BOOLEAN) {}
  Value(std::string val) : content(new StringModel(val)), type(Type::STRING) {}
  Value(char const * val) : content(new StringModel(std::string(val))), type(Type::STRING) {}
  Value(Value const &rr) : content(rr.content->clone()), type(rr.getType()) {}
  Value &operator=(Value rr) {
    if( rr.getType() != getType() )
      throw std::runtime_error("Type-changing assignment.");
    std::swap(content, rr.content);
    return *this;
  }
  /* These type conversion operators could be useful, but might turn the code
   * less explicit... 
  operator double() const { 
    if( type != Type::NUMERIC ) 
      throw std::runtime_error("Value is not numeric.");
    return getNumeric();
  }
  operator bool() const { 
    if( type != Type::BOOLEAN ) 
      throw std::runtime_error("Value is not boolean.");
    return getBool();
  }
  operator std::string() const { 
    if( type != Type::STRING ) 
      throw std::runtime_error("Value is not string.");
    return getString();
  }
  */
  Type getType() const { return type; }
  /*template <typename Tout> Tout get() const {
    return (Tout)((Model<Tout> *)content)->object;
  }*/
  double getNumeric() const { 
    if( getType() != Type::NUMERIC ) 
      throw std::runtime_error("Value must be NUMERIC to get a numeric value.");
    return dynamic_cast<NumericModel const *>(content)->object;
  }
  bool getBool() const { 
    if( getType() != Type::BOOLEAN) 
      throw std::runtime_error("Value must be BOOLEAN to get a bool value.");
    return dynamic_cast<BooleanModel const *>(content)->object;
  }
  std::string getString() const { 
    if( getType() != Type::STRING) 
      throw std::runtime_error("Value must be STRING to get a string value.");
    return dynamic_cast<StringModel const *>(content)->object;
  }
  double & getNumeric() { 
    if( getType() != Type::NUMERIC ) 
      throw std::runtime_error("Value must be NUMERIC to get a numeric value.");
    return dynamic_cast<NumericModel *>(content)->object;
  }
  bool & getBool() { 
    if( getType() != Type::BOOLEAN) 
      throw std::runtime_error("Value must be BOOLEAN to get a bool value.");
    return dynamic_cast<BooleanModel *>(content)->object;
  }
  std::string & getString() { 
    if( getType() != Type::STRING) 
      throw std::runtime_error("Value must be STRING to get a string value.");
    return dynamic_cast<StringModel *>(content)->object;
  }
  friend std::ostream& operator<<(std::ostream& os, Value const & val) {
    val.content->print(os);
    return os;
  }
  friend bool operator==(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) return false;
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() == b.getNumeric();
      case Type::STRING:
        return a.getString() == b.getString();
      case Type::BOOLEAN:
        return a.getBool() == b.getBool();
      default:
        throw std::runtime_error("unsupported type for operator==");
    }
  }
  friend bool operator<(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<: comparing Values of different type.");
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() < b.getNumeric();
      case Type::STRING:
        return a.getString() < b.getString();
      default:
        throw std::runtime_error("unsupported type for operator<");
    }
  }
  friend bool operator<=(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator<=: comparing Values of different type.");
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() <= b.getNumeric();
      case Type::STRING:
        return a.getString() <= b.getString();
      default:
        throw std::runtime_error("unsupported type for operator<=");
    }
  }
  friend bool operator>(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator>: comparing Values of different type.");
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() > b.getNumeric();
      case Type::STRING:
        return a.getString() > b.getString();
      default:
        throw std::runtime_error("unsupported type for operator>");
    }
  }
  friend bool operator>=(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) 
      throw std::runtime_error("operator>: comparing Values of different type.");
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() >= b.getNumeric();
      case Type::STRING:
        return a.getString() >= b.getString();
      default:
        throw std::runtime_error("unsupported type for operator>=");
    }
  }
  Value& operator+=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator+=: cannot add different types.");
    switch( getType() ) {
      case Type::NUMERIC:
        getNumeric() += other.getNumeric();
        break;
      case Type::STRING:
        getString() += other.getString();
        break;
      default:
        throw std::runtime_error("operator+=: only defined for NUMERIC or STRING.");
    }
    return *this;
  }
  friend Value operator+(Value const & a, Value const & b) {
    Value res(a);
    res += b;
    return res;
  }
  Value& operator-=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator-=: cannot subtract different types.");
    switch( getType() ) {
      case Type::NUMERIC:
        getNumeric() -= other.getNumeric();
        break;
      default:
        throw std::runtime_error("operator-=: only defined for NUMERIC.");
    }
    return *this;
  }
  friend Value operator-(Value const & a, Value const & b) {
    Value res(a);
    res -= b;
    return res;
  }
  Value& operator*=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator*=: cannot multiply different types.");
    switch( getType() ) {
      case Type::NUMERIC:
        getNumeric() *= other.getNumeric();
        break;
      default:
        throw std::runtime_error("operator*=: only defined for NUMERIC.");
    }
    return *this;
  }
  friend Value operator*(Value const & a, Value const & b) {
    Value res(a);
    res *= b;
    return res;
  }
  Value& operator/=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator/=: cannot divide different types.");
    switch( getType() ) {
      case Type::NUMERIC:
        getNumeric() /= other.getNumeric();
        break;
      default:
        throw std::runtime_error("operator/=: only defined for NUMERIC.");
    }
    return *this;
  }
  friend Value operator/(Value const & a, Value const & b) {
    Value res(a);
    res /= b;
    return res;
  }
  
  /*
  Value& operator-=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator-=: cannot subtract different types.");
    if( getType() != Type::NUMERIC )
      throw std::runtime_error("operator-=: only defined for NUMERIC.");
    content->subtract(other.content);
    return *this;
  }
  friend Value operator-(Value const & a, Value const & b) {
    Value res(a);
    res -= b;
    return res;
  }
  Value& operator*=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator*=: cannot multiply different types.");
    if( getType() != Type::NUMERIC )
      throw std::runtime_error("operator*=: only defined for NUMERIC.");
    content->multiply(other.content);
    return *this;
  }
  friend Value operator*(Value const & a, Value const & b) {
    Value res(a);
    res *= b;
    return res;
  }
  Value& operator/=(Value const & other) {
    if( getType() != other.getType() )
      throw std::runtime_error("operator/=: cannot multiply different types.");
    if( getType() != Type::NUMERIC )
      throw std::runtime_error("operator/=: only defined for NUMERIC.");
    content->divide(other.content);
    return *this;
  }
  friend Value operator/(Value const & a, Value const & b) {
    Value res(a);
    res /= b;
    return res;
  }
  */
private:
  struct Concept {
    virtual ~Concept() {}
    Concept() = default;
    virtual Concept *clone() const = 0;
    virtual void print(std::ostream& os) const = 0;
  };

  struct StringModel : Concept {
    StringModel(std::string const & value) : Concept(), object(value) {}
    Concept *clone() const { return new StringModel(object); }
    void print(std::ostream& os) const { os << object; }
    std::string object;
  };
  struct BooleanModel : Concept {
    Concept *clone() const { return new BooleanModel(object); }
    void print(std::ostream& os) const { os << object; }
    BooleanModel(bool const & value) : Concept(), object(value) {}
    bool object;
  };
  struct NumericModel : Concept {
    Concept *clone() const { return new NumericModel(object); }
    void print(std::ostream& os) const { os << object; }
    NumericModel(double const &value) : Concept(), object(value) {}
    double object;
  };
  Concept *content;
  Type type;
};

class Attribute {
  public:
    template<typename T>
    Attribute(std::string const & name, T const & in) : 
      attrname(name), val(in) { }
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