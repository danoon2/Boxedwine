#!/bin/sh
cd ../../lib
libVersion=$(< mac/version.txt)
if [ $libVersion -eq 4 ]; then
  echo "Mac Lib up to date."
else 
  echo "Update Mac Lib"
  rm -rf mac
  mkdir mac
  cd mac
  curl -z maclib.zip http://boxedwine.org/v/depend/mac/4/maclib.zip --output maclib.zip
  unzip maclib.zip
fi
