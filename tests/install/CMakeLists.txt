# CMakeLists.txt for install

cmake_minimum_required(VERSION 3.0)

set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

file(GLOB src_files ${TESTS_DIR}/*.c ${TESTS_DIR}/*.cpp)
set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/_source)
file(COPY ${src_files} DESTINATION ${src_dir})

find_package(webserv)

file(GLOB src_files ${src_dir}/*.c ${src_dir}/*.cpp)

add_executable(tests_install ${src_files})

target_include_directories(
  tests_install
  PRIVATE ${TESTS_DIR})

target_link_libraries(tests_install webserv)

add_custom_target(
  tests_run
  COMMAND tests_install
  DEPENDS tests_install)
