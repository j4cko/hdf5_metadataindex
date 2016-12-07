#include "attributes.h"
#include "indexHdf5.h"
#include "readTable.h"
#include "postselection.h"
#include "conditions.h"
#include "indexHdf5.h"
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
  std::string testdatadir(".");
  if( argc == 2 ) testdatadir = argv[1];
  else if( argc > 2 ){ std::cout << "unknown arguments." << std::endl; return 1; }

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
    arr.insert("another", Value(123.4));
    SIMPLETEST( "adding values after construction: ", , (arr["another"] == Value(123.4) && arr["test"] == Value(3) && arr["othertest"] == Value("testval")) );
    SIMPLETEST( "adding values after construction: ", auto list=arr.keys(), list.size() == 3 );
    SIMPLETEST( "changing values after insertion: ", arr["othertest"] = Value("anotherstring");, arr["othertest"] == "anotherstring" );
  }
  {
    std::map<std::string, Value> valmap;
    Value arr1(valmap), arr2(valmap);
    arr1.insert("test", Value(2)); arr2.insert("test", Value(3));
    SIMPLETEST( "comparison of two arrays works: inequal?", , not ( arr1 == arr2 ) );
    arr2["test"] = Value(2);
    SIMPLETEST( "comparison of two arrays works: equal?", , ( arr1 == arr2 ) );
  }

  std::cout << "=================================================" << std::endl;
  std::cout << "|| Attribute class                             ||"<< std::endl;
  std::cout << "=================================================" << std::endl;
  SIMPLETEST( "attribute holds name ", Attribute attr("exampleattr", 5);, attr.getName() == "exampleattr");
  SIMPLETEST( "attribute holds value", Attribute attr("exampleattr", 5);, attr.getValue() == 5);
  SIMPLETEST( "attribute returns type", Attribute attr("exampleattr", 5);, attr.getType() == Type::NUMERIC);

  std::cout << "=================================================" << std::endl;
  std::cout << "|| Read table                                  ||"<< std::endl;
  std::cout << "=================================================" << std::endl;
  {
    Index tblidx;
    try {
       tblidx = indexFile(testdatadir + "/table_testdata.h5");
    } catch (std::exception const & exc) {
      std::cout << "could not index test file \"" << testdatadir + "/table_testdata.h5" << "\": " << exc.what();
      return -1;
    }
    std::cout << "size: " << tblidx.size() << std::endl;
    SIMPLETEST("Size of index is correct (the only dset was split in components)?", , tblidx.size() == 2025);
    SIMPLETEST("Dataset was correctly recognized?", , tblidx.front().datasetname == "/rqcd/stoch_discon/stochsolve0/solve_0/data");
    Request request;
    filterIndexByPostselectionRules(tblidx, request);
    SIMPLETEST("Empty request leaves index size unchanged?", , tblidx.size() == 2025);
    request.attrrequests.push_back(AttributeRequest("interpolator", AttributeConditions::Equals(7)));
    filterIndexByPostselectionRules(tblidx, request);
    SIMPLETEST("request returns all interp=7 values?", , tblidx.size() == 54);
    request.attrrequests.push_back(AttributeRequest("hpe", AttributeConditions::Equals(4)));
    filterIndexByPostselectionRules(tblidx, request);
    SIMPLETEST("request returns all interp=7 AND hpe=4 values?", , tblidx.size() == 27);
    std::map<std::string, Value> map;
    map.insert({"0", 1}); map.insert({"1", 1}); map.insert({"2", 1});
    request.attrrequests.push_back(AttributeRequest("mom", AttributeConditions::Equals(map)));
    filterIndexByPostselectionRules(tblidx, request);
    SIMPLETEST("request returns all interp=7 AND hpe=4 values at mom=1,1,1?", , tblidx.size() == 1);
  }
  return 0;
}
