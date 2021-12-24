#!/bin/sh
if [ ! -d "../../lib/mac" ]
then
   sh fetchDepends.sh
fi
mkdir bin
rm -rf bin/Boxedwine.app
xcodebuild build -workspace "Boxedwine.xcworkspace" -config Release -scheme Boxedwine -archivePath archive archive
#xcodebuild -archivePath archiveX64 archive.xcarchive -exportArchive -export Boxedwine.app -exportFormat App
mv archive.xcarchive/Products/Applications/Boxedwine.app ./bin/
rm -rf archive.xcarchive
