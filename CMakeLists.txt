cmake_minimum_required (VERSION 3.0)
project (Hdf5MetadataIndex)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_executable(indexHdf5 "${CMAKE_SOURCE_DIR}/src/indexer.cc" src/attributes.cc src/h5helpers.cc src/indexHdf5.cc src/sqliteHelpers.cc )
add_executable(queryDb "${CMAKE_SOURCE_DIR}/src/queryDb.cc" src/indexHdf5.cc src/attributes.cc src/h5helpers.cc src/sqliteHelpers.cc src/postselection.cc )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package (JsonCpp REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JSONCPP_LIBRARIES})

find_package (HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})
set(LIBS ${LIBS} ${HDF5_LIBRARIES})

find_package (SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})

target_link_libraries( indexHdf5 ${LIBS} )
target_link_libraries( queryDb ${LIBS} )
