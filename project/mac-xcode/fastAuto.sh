#!/bin/bash
export PATH=/bin:/usr/bin:/usr/local/bin:/Users/alla/homebrew/bin
/bin/bash buildRelease.sh

mkdir auto
curl -z auto/auto-abiword-32.zip -o auto/auto-abiword-32.zip http://208.113.132.187/automation/auto-abiword-32.zip
curl -z auto/Wine-5.0.zip -o auto/Wine-5.0.zip http://208.113.132.187/automation/Wine-5.0.zip
curl -z auto/debian10.zip -o auto/debian10.zip http://208.113.132.187/automation/debian10.zip
rm -rf auto/abiword
unzip auto/auto-abiword-32.zip -d auto
bin/Boxedwine.app/Contents/MacOS/Boxedwine -nosound -novideo -automation "$(pwd)/auto/abiword/play" -root "$(pwd)/auto/abiword/root" -zip "$(pwd)/auto/Wine-5.0.zip" -w /home/username/files/abiword /bin/wine /home/username/files/abiword/abiword