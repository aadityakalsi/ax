#!/bin/bash

if [ $# -ne 2 ];
then
  echo "Usage:"
  echo "  createhdr path_to_hdr_file include_guard"
  exit 1
fi

proj_dir=`cd $(dirname $0); pwd`
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
echo "#ifndef $2" >> $1
echo "#define $2" >> $1
echo "" >> $1
echo "#endif/*$2*/" >> $1
