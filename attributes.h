#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__
#include <vector>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <memory>

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
  Value(Value const &rr) : content(rr.content->clone()) {}
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
private:
  struct Concept {
    virtual ~Concept() {}
    Concept() = delete;
    Concept(std::type_info const &ti_) : ti(ti_) {}
    virtual Concept *clone() const = 0;
    virtual void print(std::ostream& os) const = 0;
    std::type_info const &ti;
  };

  template <typename T> struct Model : Concept {
    Model(T const &value) : Concept(typeid(T)), object(value) {}
    void print(std::ostream& os) const { os << object; }
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

class Condition {
  public:
    virtual bool matches(Attribute const & attr, std::string const & reqname) const = 0;
    virtual std::unique_ptr<Condition> clone() const = 0;
};

class AttributeRequest {
  public:
    AttributeRequest(std::string const & name, Condition const & in) :
      reqname(name), cond(std::move(in.clone())) {}
    bool matches(Attribute const & attr) const { return cond->matches(attr, reqname); }
    std::string getName() const { return reqname; }
  private:
    std::string reqname;
    std::unique_ptr<Condition> cond;
};

Attribute attributeFromStrings(std::string const & name, std::string const & valstr, 
        std::string const & typestr);

typedef std::vector<std::pair<std::vector<Attribute>,std::string>> Index;

Index indexFile(std::string filename);
#endif
