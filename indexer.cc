#include "attributes.h"
#include "indexHdf5.h"
#include "sqliteHelpers.h"

#include <iostream>
int main(int argc, char** argv) {
  //TODO: make tests out of this:
  Value bla(0);
  std::cout << bla << std::endl;
  bla = 5;
  std::cout << bla << std::endl;
  bla = (int)bla + 3;
  std::cout << bla << std::endl;

  if( bla == 8 ) std::cout << " bla == 8 (ok) " << std::endl;
  else           std::cout << " bla != 8 (?) " << std::endl;

  Attribute attr("exampleattr", 5);
  std::cout << typeToString(attr.getType()) << ": " << attr.getValue() << std::endl;

  Attribute attr2("exampleattr2", 6.432);
  std::cout << typeToString(attr2.getType()) << ": " << attr2.getValue() << std::endl;

  std::cout << typeToString(attr2.getType()) << std::endl;
  Index testind;
  testind.push_back({{attr2}, "testpath"});
  printIndex(testind, std::cout);


  if( argc != 3 ) { std::cerr << "wrong number of arguments." << std::endl;
                    return -1; }

  const std::string h5file(argv[1]);
  const std::string sqlfile(argv[2]);

  sqlite3 *db;
  sqlite3_open(sqlfile.c_str(), &db);
  prepareSqliteFile(db);

  Index idx = indexFile(h5file);

  insertDataset(db, idx);

  sqlite3_close(db);
  
 // printIndex(indexFile(argv[1]), std::cout);
  return 0;
}
