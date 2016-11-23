#ifndef __CONDITIONS_H__
#define __CONDITIONS_H__
#include "attributes.h"
#include <sstream>

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
      sstr << valentryname << " = \"" << val << "\" ";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      sstr << keyentryname << " = \"" << name << "\" ";
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
      sstr << valentryname << " between \"" << min << "\" and \"" << max << "\"";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      sstr << keyentryname << " = \"" << name << "\" ";
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
      sstr << valentryname << " >= \"" << min << "\"";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      sstr << keyentryname << " = \"" << name << "\" ";
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
      sstr << valentryname << " <= \"" << max << "\"";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      sstr << keyentryname << " = \"" << name << "\" ";
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
    std::string getSqlValueDescription(std::string const & valentryname) const { 
      std::stringstream sstr;
      sstr << valentryname << " is not null";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      if( present )
        sstr << keyentryname << " = \"" << name << "\" ";
      else
        sstr << keyentryname << " != \"" << name << "\" ";
      return sstr.str();
    }
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
      sstr << "( ";
      for( auto i = 0u; i < vals.size() - 1; i++ )
        sstr << valentryname << " = \"" << vals[i] << "\" or ";
      sstr << valentryname << " = \"" << vals.back() << "\" )";
      return sstr.str();
    }
    std::string getSqlKeyDescription(std::string const& keyentryname, std::string const & name) const {
      std::stringstream sstr;
      sstr << keyentryname << "= \"" << name << "\" ";
      return sstr.str();
    }
  private:
    std::vector<Value> vals;
};
}
#endif
