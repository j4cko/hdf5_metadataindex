cmake_minimum_required (VERSION 2.8)
project (rqcd_mdi)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_library( fileindex src/filehelpers.cc src/value.cc src/attributes.cc src/postselection.cc src/parseJson.cc )
add_library( hdf5index src/filehelpers.cc src/h5helpers.cc src/indexHdf5.cc src/hdf5ReaderGeneric.cc )
add_library( sqliteindex src/sqliteHelpers.cc )

add_executable(mdi src/mdi.cc)
add_executable(tests src/tests.cc )

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Werror -Wshadow -Wnon-virtual-dtor -pedantic")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package (JsonCpp REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JSONCPP_LIBRARIES})

find_package (HDF5 REQUIRED COMPONENTS C HL)
include_directories(${HDF5_INCLUDE_DIRS})
set(LIBS ${LIBS} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

find_package (SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})

target_link_libraries( mdi sqliteindex hdf5index fileindex ${LIBS} )
target_link_libraries( tests sqliteindex hdf5index fileindex ${LIBS} )

install(TARGETS mdi fileindex hdf5index sqliteindex
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/rqcd_mdi 
        FILES_MATCHING PATTERN "*.h")
