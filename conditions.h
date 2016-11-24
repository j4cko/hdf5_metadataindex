#ifndef __CONDITIONS_H__
#define __CONDITIONS_H__
#include "attributes.h"
#include <sstream>
#include <regex>

namespace AttributeConditions {
class Equals : public AttributeCondition { 
  public:
    Equals(Value val_) : val(val_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getValue() == val and attr.getName() == reqname; }
    std::unique_ptr<AttributeCondition> clone() const { 
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

class Range: public AttributeCondition { 
  public:
    Range(Value min_, Value max_) : min(min_), max(max_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= min and attr.getValue() <= max and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
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

class Min : public AttributeCondition { 
  public:
    Min(Value min_) : min(min_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= min and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
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
class Max : public AttributeCondition { 
  public:
    Max(Value max_) : max(max_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return (attr.getValue() >= max and attr.getName() == reqname); }
    std::unique_ptr<AttributeCondition> clone() const { 
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
class Present : public AttributeCondition { 
  public:
    Present (bool present_) : present(present_) {
      if( not present ) 
        throw std::runtime_error("Present(AttributeCondition): present == false is not implemented.");
    }
    bool matches(Attribute const & attr, std::string const & reqname) const {
      return attr.getName() == reqname; }
    std::unique_ptr<AttributeCondition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Present>(new Present(present)); }
  private:
    bool present;
};
class Or : public AttributeCondition {
  public:
    Or(std::vector<Value> values) : vals(values) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      bool match = false;
      for( auto const & val : vals )
        match |= ( val == attr.getValue() );
      return (match and (attr.getName() == reqname));
    }
    std::unique_ptr<AttributeCondition> clone() const { 
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
class Matches : public AttributeCondition { 
  public:
    Matches(std::string regex_) : regex(std::regex(regex_)) {}
    Matches(std::regex regex_) : regex(regex_) {}
    bool matches(Attribute const & attr, std::string const & reqname) const {
      std::stringstream sstr;
      sstr << attr.getValue();
      return attr.getName() == reqname and std::regex_match(sstr.str(), regex); }
    std::unique_ptr<AttributeCondition> clone() const { 
      //make_unique is missing in C++11:
      return std::unique_ptr<Matches>(new Matches(regex)); }
  private:
    std::regex regex;
};
}


namespace FileConditions {
class NameMatches : public FileCondition {
  public:
    NameMatches(std::string regex_) : regex(std::regex(regex_)) {}
    NameMatches(std::regex regex_) : regex(regex_) {}
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
    Older(int mtime_) : mtime(mtime_) {}
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
    Newer(int mtime_) : mtime(mtime_) {}
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
    Mtime(int mtime_) : mtime(mtime_) {}
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
