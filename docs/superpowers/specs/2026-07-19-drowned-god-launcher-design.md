# Drowned God Launcher Design

## Goal

Create `C:\Users\james\Desktop\drowned\drowngod.bat` so the Drowned God demo runs under Wine's Windows 95 compatibility setting.

## Launcher Behavior

The batch file will:

1. Disable command echoing.
2. Change the working directory to the directory containing the batch file so `DROWNGOD.EXE` can resolve the adjacent DLL and `ASSETS` directory.
3. Run `winecfg -v win95`, the command supported by the bundled Wine 11 build for setting the global Windows version and exiting.
4. Exit without launching the demo if `winecfg` reports an error.
5. Run `DROWNGOD.EXE` after the compatibility setting succeeds.

The exact content will be:

```bat
@echo off
cd /d "%~dp0"
winecfg -v win95
if errorlevel 1 exit /b %errorlevel%
DROWNGOD.EXE
```

## Scope and Side Effects

The launcher changes the Wine container's global Windows-version setting to Windows 95 each time it runs. It will not restore the previous version afterward. No game files or Boxedwine source files will be changed.

## Verification

After creation, verify that `drowngod.bat` exists in the demo directory and that its content exactly matches the approved commands. Do not execute the launcher automatically, because doing so would change the active Wine container and open the game.
