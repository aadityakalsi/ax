#!/usr/bin/env sh
exec=`realpath $0`
root=`dirname $exec`
obj_dir="_build/CMakeFiles/ax.dir/src/ax"
cov_dir="_build/coverage"
cov_file="_build/cov.info"

cd $root
lcov --base-directory $obj_dir --directory $obj_dir --zerocounters -q
env CFLAGS='-ftest-coverage -fprofile-arcs --coverage' ./build.sh
lcov --base-directory $obj_dir --directory $obj_dir -c -o $cov_file
genhtml --prefix $root -o $cov_dir -t 'libax' $cov_file

printf "\n---\nResults in: $root/$cov_dir/index.html\n---\n"
