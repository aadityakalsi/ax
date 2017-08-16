# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

project(ax LANGUAGES C)

set(ver_maj 0)
set(ver_min 1)
set(ver_patch 0)
set(ver_string ${ver_maj}.${ver_min}.${ver_patch})

macro(msg m)
  message("[ax] ${m}")
endmacro(msg)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(PROJECT_VERSION ${ver_string})

if (UNIX)
  if(APPLE)
    set(CMAKE_INSTALL_NAME_DIR "@rpath/")
  else()
    set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
  endif()
endif(UNIX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(3rdparty.cmake)

msg("Got project source dir: ${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE export_hdr ${PROJECT_SOURCE_DIR}/export/*.h ${PROJECT_SOURCE_DIR}/export/*.hpp)

file(GLOB_RECURSE src_files ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)

msg("Got export hdrs: ${export_hdr}")
msg("Got src files: ${src_files}")

add_library(ax ${src_files} $<TARGET_OBJECTS:uv>)

if (BUILD_SHARED_LIBS)
  set(build_flag BUILDING_AX)
else()
  set(build_flag )
endif()

target_compile_definitions(
  ax
  PRIVATE
    ${build_flag}
    AX_VER_MAJ=${ver_maj}
    AX_VER_MIN=${ver_min}
    AX_VER_PATCH=${ver_patch}
AX_VER_STRING=${ver_string})

target_include_directories(
  ax
  PRIVATE include 3p/libuv/include
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/export>
         $<INSTALL_INTERFACE:include>)

msg("public  deps: $ENV{3p_deps_pub}")
msg("private deps: $ENV{3p_deps_pvt}")
target_link_libraries(ax PUBLIC $ENV{3p_deps_pub} PRIVATE $ENV{3p_deps_pvt})

#target_compile_features(
#  ax
#  PUBLIC
#  cxx_nullptr
#  cxx_lambdas
#  cxx_auto_type
#  cxx_static_assert)

configure_file(
  cmake/axConfig.cmake.in
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/axConfig.cmake @ONLY)

# INSTALL

export(
  TARGETS ax
  FILE ${PROJECT_BINARY_DIR}/axTargets.cmake)

export(PACKAGE ax)

# install configs

install(
  TARGETS ax
  EXPORT axTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)

install(
  DIRECTORY export/
  DESTINATION include)

set(cmake_config_dir lib/cmake/ax)

install(
  FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/axConfig.cmake
  DESTINATION ${cmake_config_dir}
  COMPONENT dev)

install(
  EXPORT axTargets
  DESTINATION ${cmake_config_dir}
  COMPONENT dev)

install(
  FILES ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION share/ax
  RENAME LICENSE.${PROJECT_NAME})

install(
  FILES ${PROJECT_SOURCE_DIR}/3p/libuv/LICENSE
  DESTINATION share/ax
  RENAME LICENSE.libuv)

# TESTS

include(tests.cmake)

# Packaging

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGE_VENDOR "Aaditya Kalsi")
set(CPACK_PACKAGE_CONTACT "aadityakalsi@gmail.com")
set(CPACK_STRIP_FILES 1)
include(CPack)
