#include "attributes.h"
#include <iostream>
int itest = 0;
#define SIMPLETEST( msg, code, condition ) \
  {\
  std::cout << msg;\
  code;\
  itest++;\
  if( condition ) std::cout << " ok." << std::endl;\
  else { std::cout << " failed." << std::endl; return itest; }}
#define SHOULDTHROWTEST( msg, code ) \
  {\
  itest++;\
  std::cout << msg;\
  bool caught = false;\
  try{\
    code;\
  } catch (std::exception const & exc) {\
    std::cout << " ok (caught exception \"" << exc.what() << "\")." << std::endl;\
    caught = true;\
  }\
  if( not caught ) {\
    std::cout << " failed (no exception caught)." << std::endl;\
    return itest;\
  }}


int main( int argc, char** argv ) {
  std::cout << "=================================================" << std::endl;
  std::cout << "|| Value class                                 ||" << std::endl;
  std::cout << "=================================================" << std::endl;
  SIMPLETEST( "Is Value(0) == 0?", Value bla(0); , bla == Value(0.0) );
  SIMPLETEST( "Assigning another value to existing?", Value bla(0); bla = 5;, bla == Value(5) );
  SHOULDTHROWTEST( "Assigning another value of different type should throw:", Value bla(0); bla = Value("test"););
  SIMPLETEST( "operator+= works on numeric types?", Value bla(2); bla += Value(3);, bla == Value(5));
  SIMPLETEST( "operator+= works on string types?", Value bla("other"); bla += Value("test");, bla == Value("othertest"));
  SIMPLETEST( "operator-= works on numeric types?", Value bla(2); bla -= Value(3);, bla == Value(-1));
  SIMPLETEST( "operator*= works on numeric types?", Value bla(2); bla *= Value(3);, bla == Value(6));
  SIMPLETEST( "operator/= works on numeric types?", Value bla(3); bla /= Value(2);, bla == Value(1.5));
  SHOULDTHROWTEST( "operator-= doesn't work on string types?", Value bla("other"); bla -= Value("test"); )
  SIMPLETEST( "operator+ works on numeric types?", Value bla(3); Value blu(2);, bla + blu == Value(5));
  SIMPLETEST( "operator- works on numeric types?", Value bla(3); Value blu(2);, bla - blu == Value(1));
  SIMPLETEST( "operator* works on numeric types?", Value bla(3); Value blu(2);, bla * blu == Value(6));
  SIMPLETEST( "operator/ works on numeric types?", Value bla(3); Value blu(2);, bla / blu == Value(1.5));
  SIMPLETEST( "Comparisons: Val < int: ", Value bla(3);, bla < 5 ); // works because 5 is converted to Value
  SIMPLETEST( "Comparisons: int > Val: ", Value bla(3);, 5 > bla );
  SIMPLETEST( "double and int are the same val?", Value bla(3); Value blu(3.4), blu > bla );
  SIMPLETEST( "type of Value(3) == numeric?", Value bla(3), bla.getType() == Type::NUMERIC );
  SIMPLETEST( "type of Value(\"test\") == string?", Value bla("test"), bla.getType() == Type::STRING );
  SHOULDTHROWTEST( "Adding a number and a string should throw", Value bla(2); Value blub("test"); bla + blub; );
  {
    std::map<std::string, Value> valmap; 
    valmap.insert({"test", Value(3)});
    valmap.insert({"othertest", Value("testval")});
    Value arr(valmap);
    SIMPLETEST( "arrays of values, constructed in a simple way: ", , (arr["test"] == Value(3) && arr["othertest"] == Value("testval")) );
  }

  std::cout << "=================================================" << std::endl;
  std::cout << "|| Attribute class                             ||"<< std::endl;
  std::cout << "=================================================" << std::endl;
  SIMPLETEST( "attribute holds name ", Attribute attr("exampleattr", 5);, attr.getName() == "exampleattr");
  SIMPLETEST( "attribute holds value", Attribute attr("exampleattr", 5);, attr.getValue() == 5);
  SIMPLETEST( "attribute returns type", Attribute attr("exampleattr", 5);, attr.getType() == Type::NUMERIC);

  return 0;
}
