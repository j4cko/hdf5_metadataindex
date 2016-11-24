#ifndef __CONDITIONS_H__
#define __CONDITIONS_H__
#include "attributes.h"
#include <sstream>
#include <regex>

namespace Conditions {
class Equals : public Condition { 
  public:
    Equals(Value val_) : val(val_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getValue() == val and attr.getName() == reqname; }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Equals>(new Equals(val)); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      if( val.getType() == Type::STRING )
        sstr << valentryname << " = '" << val << "' ";
      else
        sstr << valentryname << " = " << val << " ";
      return sstr.str();
    }
  private:
    Value val;
};

class Range: public Condition { 
  public:
    Range(Value min_, Value max_) : min(min_), max(max_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= min and attr.getValue() <= max and attr.getName() == reqname); }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Range>(new Range(min, max)); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      assert(min.getType() == max.getType());
      if( min.getType() == Type::STRING )
        sstr << valentryname << " between '" << min << "' and '" << max << "' ";
      else
        sstr << valentryname << " between " << min << " and " << max << " ";
      return sstr.str();
    }
  private:
    Value min, max;
};

class Min : public Condition { 
  public:
    Min(Value min_) : min(min_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= min and attr.getName() == reqname); }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Min>(new Min(min)); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      if( min.getType() == Type::STRING )
        sstr << valentryname << " >= '" << min << "' ";
      else
        sstr << valentryname << " >= " << min << " ";
      return sstr.str();
    }
  private:
    Value min;
};
class Max : public Condition { 
  public:
    Max(Value max_) : max(max_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= max and attr.getName() == reqname); }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Max>(new Max(max)); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      if( max.getType() == Type::STRING )
        sstr << valentryname << " <= '" << max << "' ";
      else
        sstr << valentryname << " <= " << max << " ";
      return sstr.str();
    }
  private:
    Value max;
};
class Present : public Condition { 
  public:
    Present (bool present_) : present(present_) {
      if( not present ) 
        throw std::runtime_error("Present(Condition): present == false is not implemented.");
    }
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getName() == reqname; }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Present>(new Present(present)); }
  private:
    bool present;
};
class Or : public Condition {
  public:
    Or(std::vector<Value> values) : vals(values) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      bool match = false;
      for( auto const & val : vals )
        match |= ( val == attr.getValue() );
      return (match and (attr.getName() == reqname));
    }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Or>(new Or(vals)); }
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      assert(vals.size() > 0);
      Type typecheck = vals.front();
      sstr << "( ";
      for( auto i = 0u; i < vals.size() - 1; i++ ){
        assert(vals[i].getType() == typecheck);
        if( typecheck == Type::STRING )
          sstr << valentryname << " = '" << vals[i] << "' or ";
        else
          sstr << valentryname << " = " << vals[i] << " or ";
      }
      if( typecheck == Type::STRING )
        sstr << valentryname << " = '" << vals.back() << "' or ";
      else
        sstr << valentryname << " = " << vals.back() << " )";
      return sstr.str();
    }
  private:
    std::vector<Value> vals;
};
class Matches : public Condition { 
  public:
    Matches(std::string regex_) : regex(std::regex(regex_)) {}
    Matches(std::regex regex_) : regex(regex_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      std::stringstream sstr;
      sstr << attr.getValue();
      return attr.getName() == reqname and std::regex_match(sstr.str(), regex); }
    std::unique_ptr<Condition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Matches>(new Matches(regex)); }
  private:
    std::regex regex;
};}
#endif
