#!/bin/bash
/bin/bash build64.sh

mkdir auto64
curl -z auto64/auto-abiword-64.zip -o auto64/auto-abiword-64.zip http://208.113.132.187/automation/auto-abiword-64.zip
curl -z auto64/Wine-5.0.zip -o auto64/Wine-5.0.zip http://208.113.132.187/automation/Wine-5.0.zip
curl -z auto64/debian10.zip -o auto64/debian10.zip http://208.113.132.187/automation/debian10.zip
rm -rf auto64/abiword
unzip auto64/auto-abiword-64.zip -d auto64
bin/boxedwine -nosound -novideo -automation "$(pwd)/auto64/abiword/play" -root "$(pwd)/auto64/abiword/root" -zip "$(pwd)/auto64/Wine-5.0.zip" -w /home/username/files/abiword /bin/wine /home/username/files/abiword/abiword
