#!/bin/sh
set -e
sh buildjs.sh
mkdir -p build
rm -f build/*
cp boxedwine.html build/
cp boxedwine.css build/
cp boxedwine-shell.js build/
cp boxedwine.js build/
cp jszip.min.js build/
cp browserfs.boxedwine.js build/
cp boxedwine.wasm build/
cp buildfiles/* build/
cd build
DATE=`date +%Y-%m-%d`
zip emscripten$DATE.zip *
