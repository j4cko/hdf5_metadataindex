cmake_minimum_required (VERSION 2.6)
project (Hdf5MetadataIndex)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(LibFindMacros)

message("-- try to find jsoncpp --")
find_package(Jsoncpp)

if(Jsoncpp_FOUND)
  message(" --> found jsoncpp")
else(Jsoncpp_FOUND)
  message(" --> couldn't find.")
endif(Jsoncpp_FOUND)
set(Jsoncpp_PROCESS_INCLUDES Jsoncpp_INCLUDE_DIR)
set(Jsoncpp_PROCESS_LIBS Jsoncpp_LIBRARY)


add_executable(indexHdf5 "${PROJECT_SOURCE_DIR}/indexHdf5.cc")
add_executable(queryDb "${PROJECT_SOURCE_DIR}/queryDb.cc")
