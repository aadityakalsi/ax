#!/usr/bin/env sh

cd `dirname $0`
currdir=`pwd`
cd -

parent=`dirname $currdir`
root=`dirname $parent`

set -e

cmake -H. -B$currdir/_build -DCMAKE_INSTALL_PREFIX=$root/_install -DTESTS_DIR=$root/tests
env VERBOSE=1 cmake --build $currdir/_build --target tests_run > $currdir/testLog.txt
rm -fr $currdir/_build

echo '----------------------------------------------------------'
echo '# TEST LOG                                               #'
echo '----------------------------------------------------------'
cat $currdir/testLog.txt
echo '----------------------------------------------------------'
