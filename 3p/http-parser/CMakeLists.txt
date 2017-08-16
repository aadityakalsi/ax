# CMakeLists.txt

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DHTTP_PARSER_STRICT=1)
else()
  add_definitions(-DHTTP_PARSER_STRICT=0)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(http_parser OBJECT http_parser.c http_parser.h)
