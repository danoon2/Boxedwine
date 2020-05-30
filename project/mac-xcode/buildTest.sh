#!/bin/sh
mkdir bin
rm -rf bin/BoxedwineTest.app
xcodebuild build -workspace "Boxedwine.xcworkspace" -config Release -scheme BoxedwineTest -archivePath archiveTest archive
#xcodebuild -archivePath archiveX64 archive.xcarchive -exportArchive -export Boxedwine.app -exportFormat App
mv archiveTest.xcarchive/Products/Applications/BoxedwineTest.app ./bin/
rm -rf archiveTest.xcarchive
