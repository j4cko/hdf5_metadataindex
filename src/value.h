#ifndef __VALUE_H__
#define __VALUE_H__
#include <string>
#include <stdexcept>
#include <map>
#include <vector>

namespace rqcd_hdf5_index {
enum class Type {
  NUMERIC,
  BOOLEAN,
  STRING,
  ARRAY
};

class Value {
public:
  ~Value() { delete content; }
  Value(double val) : content(new NumericModel(val)), type(Type::NUMERIC) {}
  Value(int val) : content(new NumericModel((double)val)), type(Type::NUMERIC) {}
  Value(std::size_t val) : content(new NumericModel((double)val)), type(Type::NUMERIC) {}
  Value(bool val) : content(new BooleanModel(val)), type(Type::BOOLEAN) {}
  Value(std::string val) : content(new StringModel(val)), type(Type::STRING) {}
  Value(char const * val) : content(new StringModel(std::string(val))), type(Type::STRING) {}
  Value(std::map<std::string, Value> const & val) : content(new ArrayModel(val)), type(Type::ARRAY) {}
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
  std::map<std::string, Value> getMap() const {
    if( getType() != Type::ARRAY)
      throw std::runtime_error("Value must be ARRAY to get a value from it.");
    return dynamic_cast<ArrayModel const *>(content)->object;
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
  std::map<std::string, Value> & getMap() {
    if( getType() != Type::ARRAY)
      throw std::runtime_error("Value must be ARRAY to get a value from it.");
    return dynamic_cast<ArrayModel *>(content)->object;
  }
  friend std::ostream& operator<<(std::ostream& os, Value const & val) {
    val.content->print(os);
    return os;
  }
  /*
  friend std::istream& operator>>(std::istream& is, Value & val) {
    val.content->read(is);
    return os;
  }*/
  friend bool operator==(Value const & a, Value const & b) {
    if( a.getType() != b.getType() ) return false;
    bool equal = true;
    switch( a.getType() ) {
      case Type::NUMERIC:
        return a.getNumeric() == b.getNumeric();
      case Type::STRING:
        return a.getString() == b.getString();
      case Type::BOOLEAN:
        return a.getBool() == b.getBool();
      case Type::ARRAY:
        //compare keys:
        for( auto const & key : a.keys() )
          equal &= (b.getMap().count(key) == 1);
        if( not equal ) return false;
        // has all keys.
        for( auto const & key : a.keys() )
          equal &= (a[key] == b[key]);
        // all keys are equal?
        return equal;
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
  std::vector<std::string> keys() const {
    if( getType() != Type::ARRAY )
      throw std::runtime_error("type must be ARRAY to get the keys");
    std::vector<std::string> keys;
    auto const & map = dynamic_cast<ArrayModel *>(content)->object;
    for( const auto & elem : map)
      keys.push_back(elem.first);
    return keys;
  }
  void insert(std::string const & key, Value const & val ) {
    if( getType() != Type::ARRAY )
      throw std::runtime_error("type must be ARRAY to insert values.");
    auto & map = dynamic_cast<ArrayModel *>(content)->object;
    if( map.count(key) != 0 )
      map.at(key) = val;
    else
      map.insert({key, val});
  }
  Value & operator[](std::string const & key) {
    if( getType() != Type::ARRAY )
      throw std::runtime_error("type must be ARRAY to use operator[].");
    auto & map = dynamic_cast<ArrayModel *>(content)->object;
    if( map.count(key) != 1 )
      throw std::runtime_error("key not found.");
    return map.at(key);
  }
  Value operator[](std::string const & key) const {
    if( getType() != Type::ARRAY )
      throw std::runtime_error("type must be ARRAY to use operator[].");
    auto const & map = dynamic_cast<ArrayModel *>(content)->object;
    if( map.count(key) != 1 )
      throw std::runtime_error("key not found.");
    return map.at(key);
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
    void print(std::ostream& os) const { os << (object ? "true" : "false") ; }
    BooleanModel(bool const & value) : Concept(), object(value) {}
    bool object;
  };
  struct NumericModel : Concept {
    Concept *clone() const { return new NumericModel(object); }
    void print(std::ostream& os) const { os << object; }
    NumericModel(double const &value) : Concept(), object(value) {}
    double object;
  };
  struct ArrayModel : Concept {
    Concept *clone() const { return new ArrayModel(object); }
    void print(std::ostream& os) const {
      os << "{";
      auto it = object.cbegin();
      auto end = object.cend();
      --end;
      for( ; it != end; ++it )
        os << "\"" << it->first << "\": " << it->second << ",";
      if( not object.empty() ) 
        os << "\"" << it->first << "\": " << it->second;
      os << "}";
    }
    ArrayModel(std::map<std::string, Value> const & val) : Concept(), object(val) {}
    std::map<std::string, Value> object;
  };
  Concept *content;
  Type type;
};

std::string typeToString(Type const & type);
Type typeFromTypeid(std::type_info const & tinfo);
Value valueFromString(std::string const & str);
Type typeFromString(std::string const & typestr);
}
#endif
