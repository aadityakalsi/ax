#!/bin/bash

if [ $# -ne 1 ];
then
  echo "Usage:"
  echo "  createsrc path_to_src"
  exit 1
fi

proj_dir=`dirname $0`
fulldir=`cd $(dirname $1); pwd`

proj_diresc=`echo $proj_dir | sed 's,\/,\\\/,g'`\\/
relpath=`echo $fulldir | sed "s/$proj_diresc//g" | sed 's/[^/]*/\.\./g'`

# lic=$(cat $proj_dir/LICENSE)
lic="For license details see $relpath/LICENSE"

if [ -e $1 ]
then
    rm $1
fi

touch $1

echo "/*" >> $1
echo "${lic}" >> $1
echo "*/" >> $1
echo "" >> $1
echo "/**" >> $1
echo " * \file `basename $1`" >> $1
echo " * \date `date \"+%b %d, %Y\"`" >> $1
echo " */" >> $1
echo "" >> $1
