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


int main( int argc, char** argv ) {
  std::cout << "instantiating values from basic types:" << std::endl;
  SIMPLETEST( "Is Value(0) == 0?", Value bla(0); , bla == 0 );

  SIMPLETEST( "Assigning another value to existing?", Value bla(0); bla = 5;, bla == 5 );
  SIMPLETEST( "Algebra with values and base..", Value bla(3); Value blub = bla + 5;, blub == 8 );
  SIMPLETEST( "Algebra with values..", Value bla(3); Value blu(5); Value blub = bla + 5;, blub == 8 );
  SIMPLETEST( "Comparisons: Val < int: ", Value bla(3);, bla < 5 );
  SIMPLETEST( "Comparisons: int > Val: ", Value bla(3);, 5 > bla );
  SIMPLETEST( "double and int are the same val?", Value bla(3); Value blu(3.4), blu > bla );
  SIMPLETEST( "type of Value(3) == numeric?", Value bla(3), bla.getType() == Type::NUMERIC );
  SIMPLETEST( "type of Value(\"test\") == string?", Value bla("test"), bla.getType() == Type::STRING );

  //bla = (int)bla + 3;
  //std::cout << bla << std::endl;

  //if( bla == 8 ) std::cout << " bla == 8 (ok) " << std::endl;
  //else           std::cout << " bla != 8 (?) " << std::endl;
/* attribute stuff:
  Attribute attr("exampleattr", 5);
  std::cout << typeToString(attr.getType()) << ": " << attr.getValue() << std::endl;

  Attribute attr2("exampleattr2", 6.432);
  std::cout << typeToString(attr2.getType()) << ": " << attr2.getValue() << std::endl;

  std::cout << typeToString(attr2.getType()) << std::endl;
  Index testind;
  testind.push_back({{attr2}, "testpath"});
  //printIndex(testind, std::cout);


  Value val(2);
  */
  return 0;
}
