#!/usr/bin/env sh

cd `dirname $0`
root=`pwd`
cd -

set -e

if [ -z "$CMAKE_BUILD_TYPE" ]; then
    export CMAKE_BUILD_TYPE=Debug
fi

if [ -z "$BUILD_SHARED_LIBS" ]; then
    export BUILD_SHARED_LIBS=1
fi

$root/build.sh
cd $root/tests/install
./test.sh
