@echo off
setlocal
set "BOXEDWINE_EXE=%~1"
if not defined BOXEDWINE_EXE set "BOXEDWINE_EXE=Boxedwine.exe"
java -cp "%~dp0bin\BoxedWineRunner.jar" boxedwine.org.PrepareFilesystem "%~dp0filesystem.properties" "%~dp0..\automation-filesystems" "%~dp0fs"
if errorlevel 1 exit /b %errorlevel%
java -jar "%~dp0bin\BoxedWineRunner.jar" -user-reg "%~dp0fs\user.reg" "%~dp0fs\fs.zip" "%~dp0scripts" "%BOXEDWINE_EXE%" -nosound -novideo
exit /b %errorlevel%
