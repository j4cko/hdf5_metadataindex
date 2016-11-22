#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__
#include <vector>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <cassert>
#include <ostream>
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
  friend std::ostream& operator<<(std::ostream& os, Value const & val) {
    val.content->print(os);
    return os;
  }
private:
  struct Concept {
    virtual ~Concept() {}
    Concept() = delete;
    Concept(std::type_info const &ti_) : ti(ti_) {}
    virtual Concept *clone() const = 0;
    virtual void print(std::ostream& os) = 0;
    std::type_info const &ti;
  };
  template <typename T> struct Model : Concept {
    Model(T const &value) : Concept(typeid(T)), object(value) {}
    void print(std::ostream& os){ os << object; }
    Concept *clone() const { return new Model(object); }
    T object;
  };
  Concept *content;
};
enum class Type {
  NUMERIC,
  BOOLEAN,
  STRING
};
std::string typeToString(Type const & type);
Type typeFromTypeid(std::type_info const & tinfo);
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

Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr);

typedef std::vector<std::pair<std::vector<Attribute>,std::string>> Index;

Index indexFile(std::string filename);
#endif
