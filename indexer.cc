#include "attributes.h"
#include "indexHdf5.h"
#include <iostream>
int main(int argc, char** argv) {
  //TODO: make tests out of this:
  Value bla(0);
  std::cout << (int)bla << std::endl;
  bla = 5;
  std::cout << (int)bla << std::endl;
  bla = (int)bla + 3;
  std::cout << (int)bla << std::endl;

  Attribute attr("exampleattr", 5);
  std::cout << typeToString(attr.getType()) << std::endl;
  Attribute attr2("exampleattr2", 6.432);
  std::cout << typeToString(attr2.getType()) << (double) attr2.getValue() << std::endl;
  Index testind;
  testind.push_back({{attr, attr2}, "testpath"});
  printIndex(testind, std::cout);


  if( argc != 2 ) {
    std::cerr << "wrong number of arguments." << std::endl;
    std::abort();
  }
  
  printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
