#!/bin/sh
cd ../../lib
rm -rf mac
mkdir mac
cd mac
curl -z maclib.zip http://boxedwine.org/v/depend/mac/1/maclib.zip --output maclib.zip
unzip maclib.zip
