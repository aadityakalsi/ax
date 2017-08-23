#!/usr/bin/env sh

root=`dirname $0`

cd $root

root=`pwd`

set -e

if [ -z "$CMAKE_BUILD_TYPE" ]; then
    export CMAKE_BUILD_TYPE=Debug
fi

if [ -z "$BUILD_SHARED_LIBS" ]; then
    export BUILD_SHARED_LIBS=1
fi

if [ -z "$GENERATOR" ]; then
    export GENERATOR_ARG=
else
    export GENERATOR_ARG="-G'$GENERATOR'"
fi

cmd="cmake ${GENERATOR_ARG} -H$root -B$root/_build -DCMAKE_INSTALL_PREFIX=$root/_install -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} -Wno-dev"

printf "Running CMake\n-------------\n\$ $cmd\n\n"

bash -c "$cmd"

env VERBOSE=1 cmake --build $root/_build --target tests.run --config $CMAKE_BUILD_TYPE
env VERBOSE=1 cmake --build $root/_build --target install --config $CMAKE_BUILD_TYPE
