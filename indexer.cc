#include "attributes.h"
#include "indexHdf5.h"
#include "sqliteHelpers.h"

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


  if( argc != 3 ) { std::cerr << "wrong number of arguments." << std::endl;
                    return -1; }

  const std::string h5file(argv[1]);
  const std::string sqlfile(argv[2]);

  sqlite3 *db;
  sqlite3_open(sqlfile.c_str(), &db);
  prepareSqliteFile(&db);
  sqlite3_close(db);

  Index idx = indexFile(h5file);
  
 // printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
