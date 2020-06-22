#!/bin/sh
mkdir bin
rm -rf bin/BoxedwineAutomationX64.app
xcodebuild build -workspace "Boxedwine.xcworkspace" -config Release -scheme BoxedwineAutomationX64 -archivePath archive archive
#xcodebuild -archivePath archiveX64 archive.xcarchive -exportArchive -export Boxedwine.app -exportFormat App
mv archive.xcarchive/Products/Applications/BoxedwineAutomationX64.app ./bin/
rm -rf archive.xcarchive
