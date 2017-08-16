#!/usr/bin/env sh
exec=`realpath $0`
root=`dirname $exec`

rm -fr $root/_build $root/_install tests/install/_source tests/install/testLog.txt
cd $root
find -name '*~' | xargs rm -f
