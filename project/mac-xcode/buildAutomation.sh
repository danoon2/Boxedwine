#!/bin/sh
mkdir bin
rm -rf bin/BoxedwineAutomation.app
xcodebuild build -workspace "Boxedwine.xcworkspace" -config Release -scheme BoxedwineAutomation -archivePath archive archive
#xcodebuild -archivePath archiveX64 archive.xcarchive -exportArchive -export Boxedwine.app -exportFormat App
mv archive.xcarchive/Products/Applications/BoxedwineAutomation.app ./bin/
rm -rf archive.xcarchive
