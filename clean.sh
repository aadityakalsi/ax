#!/usr/bin/env sh
exec=`realpath $0`
root=`dirname $exec`

rm -fr $root/_build $root/_install tests/install/_source tests/install/testLog.txt
find $root -name '*~' | xargs rm -f
