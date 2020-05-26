mkdir auto
curl -z auto/abiword.zip -o auto/abiword.zip http://208.113.132.187/automation/auto-abiword-64.zip
curl -z auto/Wine-5.0.zip -o auto/Wine-5.0.zip http://208.113.132.187/automation/Wine-5.0.zip
curl -z auto/debian10.zip -o auto/debian10.zip http://208.113.132.187/automation/debian10.zip
rmdir auto\abiword /s /Q
unzip auto\abiword.zip -d auto
x64\Release\Boxedwine -nosound -novideo -automation "%CD%\auto\abiword\play" -root "%CD%\auto\abiword\root" -zip "%CD%\auto\Wine-5.0.zip" -w /home/username/files/abiword /bin/wine /home/username/files/abiword/abiword