#ifndef __INDEX_HDF5_H__
#define __INDEX_HDF5_H__
#include <vector>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <cassert>
class Value {
public:
  template <typename T>
  Value(T const &content_) : content(new Model<T>(content_)) {}
  ~Value() { delete content; }
  Value(Value const &rr) : content(rr.content->clone()) {}
  Value &operator=(Value rr) {
    if (content->ti.hash_code() != rr.content->ti.hash_code())
      throw std::runtime_error("Assigning Values of different types!");
    std::swap(content, rr.content);
    return *this;
  }
  template <typename T>
  operator T() const { return this->get<T>(); }
  std::string typeName() const { return std::string(content->ti.name()); }
  bool typeMatches(std::type_info const &tinfo) const {
    return content->ti.hash_code() == tinfo.hash_code();
  }
  template <typename Tout> Tout get() const {
    if (typeid(Tout).hash_code() != content->ti.hash_code())
      throw std::runtime_error("Value.get(): types do not match.");
    return (Tout)((Model<Tout> *)content)->object;
  }
private:
  struct Concept {
    virtual ~Concept() {}
    Concept() = delete;
    Concept(std::type_info const &ti_) : ti(ti_) {}
    virtual Concept *clone() const = 0;
    std::type_info const &ti;
  };
  template <typename T> struct Model : Concept {
    Model(T const &value) : Concept(typeid(T)), object(value) {}
    Concept *clone() const { return new Model(object); }
    T object;
  };
  Concept *content;
};
enum class Type {
  FLOAT_DOUBLE,
  FLOAT_SINGLE,
  INT_SHORT,
  INT_LONG,
  UINT_SHORT,
  UINT_LONG,
  STRING
};
std::string typeToString(Type const & type) {
  switch(type) {
    case Type::FLOAT_DOUBLE: return "double";
    case Type::FLOAT_SINGLE: return "float";
    case Type::INT_SHORT:    return "short int";
    case Type::INT_LONG:     return "long int";
    case Type::UINT_SHORT:   return "unsigned long int";
    case Type::UINT_LONG:    return "unsigned short int";
    case Type::STRING:       return "string";
    default: throw std::runtime_error("typeToString: unknown / unimplemented type.");
  }
}
Type typeFromTypeid(std::type_info const & tinfo) {
  if( typeid(double) == tinfo )             return Type::FLOAT_DOUBLE;
  else if( tinfo == typeid(float) )         return Type::FLOAT_SINGLE;
  else if( tinfo == typeid(int) )           return Type::INT_LONG;
  else if( tinfo == typeid(char) )          return Type::INT_SHORT;
  else if( tinfo == typeid(unsigned int) )  return Type::UINT_LONG;
  else if( tinfo == typeid(unsigned char) ) return Type::UINT_SHORT;
  else if( tinfo == typeid(std::string) )   return Type::STRING;
  else throw std::runtime_error("typeFromTypeid: unknown type.");
}
class Attribute {
  public:
    template<typename T>
    Attribute(std::string const & name, T const & in) : 
      attrname(name), val(in), type(typeFromTypeid(typeid(T))) {
      assert(val.typeMatches(typeid(T)));
    }
    Type getType() const { return type; }
    Value getValue() const { return val; };
    std::string getName() const { return attrname; };
  private:
    std::string attrname;
    Value val;
    Type type;
};
typedef std::vector<std::pair<std::vector<Attribute>,std::string>> Index;

Index indexFile(std::string filename);

void printIndex(Index const & idx, std::ostream& os);
#endif
