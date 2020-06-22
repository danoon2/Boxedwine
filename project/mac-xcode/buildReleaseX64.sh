#!/bin/sh
mkdir bin
rm -rf bin/BoxedwineX64.app
xcodebuild build -workspace "Boxedwine.xcworkspace" -config Release -scheme BoxedwineX64 -archivePath archiveX64 archive
#xcodebuild -archivePath archiveX64 archive.xcarchive -exportArchive -export Boxedwine.app -exportFormat App
mv archiveX64.xcarchive/Products/Applications/BoxedwineX64.app ./bin/
rm -rf archiveX64.xcarchive
