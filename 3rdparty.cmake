# 3rdparty.cmake

function(add_3p_deps_pvt deps)
  set(ENV{3p_deps_pvt} $ENV{3p_deps_pvt} "${deps}")
endfunction()

function(add_3p_deps_pub deps)
  set(ENV{3p_deps_pub} $ENV{3p_deps_pub} "${deps}")
endfunction()

if (BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

add_subdirectory(3p/libuv)

# OS deps
find_package(Threads)
msg("3p deps: $ENV{3p_deps}")
