#ifndef __CONDITIONS_H__
#define __CONDITIONS_H__
#include "attributes.h"
#include <sstream>
#include <iostream>
#include <regex>

namespace AttributeConditions {
class Equals : public AttributeCondition { 
  public:
    explicit Equals(Value val_) : val(val_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getValue() == val and attr.getName() == reqname; }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Equals>(val); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
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
class NotEquals : public AttributeCondition { 
  public:
    explicit NotEquals(Value val_) : val(val_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      if( attr.getType() == val.getType() and attr.getName() == reqname 
          and (not (attr.getValue() == val)) ) return true;
      else return false;
    }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<NotEquals>(val); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
      std::stringstream sstr;
      if( val.getType() == Type::STRING )
        sstr << valentryname << " != '" << val << "' ";
      else
        sstr << valentryname << " != " << val << " ";
      return sstr.str();
    }
  private:
    Value val;
};
class Range: public AttributeCondition { 
  public:
    Range(Value min_, Value max_) : min(min_), max(max_) {
      if (min.getType() != max.getType())
        throw std::runtime_error("Range condition: min and max must be of the same type.");
    }
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getType() == min.getType() && attr.getValue() >= min and attr.getValue() <= max and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Range>(min, max); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
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

class Min : public AttributeCondition { 
  public:
    explicit Min(Value min_) : min(min_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getType() == min.getType() && attr.getValue() >= min and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Min>(min); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
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
class Max : public AttributeCondition { 
  public:
    explicit Max(Value max_) : max(max_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getType() == max.getType() && attr.getValue() >= max and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Max>(max); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
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
class Present : public AttributeCondition { 
  public:
    explicit Present (bool present_) : present(present_) {
      if( not present ) 
        throw std::runtime_error("Present(AttributeCondition): present == false is not implemented.");
    }
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getName() == reqname; }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Present>(present); }
  private:
    bool present;
};
class Or : public AttributeCondition {
  public:
    explicit Or(std::vector<Value> const & values) : vals(values) {
      // is this even necessary??
      Type t = vals.front().getType();
      for( auto const & val : values ) {
        if( t != val.getType() )
          throw std::runtime_error("AttributeConditions::Or: All values must be of same type.");
      }
    }
    bool matches(Attribute const & attr, std::string const & reqname) const {
      bool match = false;
      for( auto const & val : vals )
        match |= ( val == attr.getValue() );
      return (match and (attr.getName() == reqname));
    }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Or>(vals); }
    std::string getSqlValueDescription(std::string const & valentryname) const override { 
      std::stringstream sstr;
      assert(vals.size() > 0);
      Type typecheck = vals.front().getType();
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
class Matches : public AttributeCondition { 
  public:
    explicit Matches(std::string const & regex_) : regex(std::regex(regex_)) {}
    explicit Matches(std::regex const & regex_) : regex(regex_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      std::stringstream sstr;
      sstr << attr.getValue();
      return attr.getName() == reqname and std::regex_match(sstr.str(), regex); }
    std::unique_ptr<AttributeCondition> clone() const { 
      return std::make_unique<Matches>(regex); }
  private:
    std::regex regex;
};
}


namespace FileConditions {
class NameMatches : public FileCondition {
  public:
    explicit NameMatches(std::string const & regex_) : regex(std::regex(regex_)) {}
    explicit NameMatches(std::regex const & regex_) : regex(regex_) {}
    bool matches(File const & file) const {
      return std::regex_match(file.filename, regex);
    }
    std::unique_ptr<FileCondition> clone() const { 
      return std::unique_ptr<NameMatches>(new NameMatches(regex));
    }
  private:
    std::regex regex;
};
class Older: public FileCondition {
  public:
    explicit Older(int mtime_) : mtime(mtime_) {}
    bool matches(File const & file) const {
      return file.mtime < mtime;
    }
    std::unique_ptr<FileCondition> clone() const { 
      return std::unique_ptr<Older>(new Older(mtime));
    }
  private:
    int mtime;
};
class Newer: public FileCondition {
  public:
    explicit Newer(int mtime_) : mtime(mtime_) {}
    bool matches(File const & file) const {
      return file.mtime > mtime;
    }
    std::unique_ptr<FileCondition> clone() const { 
      return std::unique_ptr<Newer>(new Newer(mtime));
    }
  private:
    int mtime;
};
class Mtime : public FileCondition {
  public:
    explicit Mtime(int mtime_) : mtime(mtime_) {}
    bool matches(File const & file) const {
      return file.mtime == mtime;
    }
    std::unique_ptr<FileCondition> clone() const { 
      return std::unique_ptr<Mtime>(new Mtime(mtime));
    }
  private:
    int mtime;
};
}
#endif
