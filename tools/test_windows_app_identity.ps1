# Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
param([ValidateSet('x64','Win32','ARM64')][string]$Platform = 'x64', [switch]$CompileOnly)
$ErrorActionPreference = 'Stop'
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $installation) { throw 'Visual Studio C++ tools are required.' }
$vcvars = Join-Path $installation 'VC\Auxiliary\Build\vcvarsall.bat'
$target = switch ($Platform) { 'Win32' { 'x64_x86' } 'ARM64' { 'x64_arm64' } default { 'x64' } }
$library = if ($Platform -eq 'Win32') { 'project\msvc\BoxedWine\Release\SDL2.lib' } else { "project\msvc\BoxedWine\$Platform\Release\SDL2.lib" }
if (-not (Test-Path (Join-Path $repository $library))) { throw "Build Release SDL2 for $Platform first." }
$output = Join-Path $repository "tmp\windows-app-identity-checks\$Platform"
New-Item -ItemType Directory -Force -Path $output | Out-Null
$batch = Join-Path $output 'build.cmd'
$commands = @(
    '@echo off', "call `"$vcvars`" $target >nul", 'if errorlevel 1 exit /b 1',
    "cd /d `"$output`"",
    "cl /nologo /EHsc /std:c++17 /MT /I`"$repository\lib\sdl2\include`" `"$repository\tools\test_windows_app_identity.cpp`" `"$repository\platform\windows\windowsAppIdentity.cpp`" /Feidentity.exe /link `"$repository\$library`" crypt32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib setupapi.lib version.lib advapi32.lib shell32.lib uuid.lib",
    'exit /b %errorlevel%'
)
[IO.File]::WriteAllLines($batch, $commands, [Text.Encoding]::Default)
& $env:ComSpec /d /c $batch
if ($LASTEXITCODE) { throw 'App identity check compilation failed.' }
if (-not $CompileOnly) {
    if ($Platform -eq 'ARM64' -and $env:PROCESSOR_ARCHITECTURE -ne 'ARM64') { throw 'Run ARM64 checks on an ARM64 machine or pass -CompileOnly.' }
    foreach ($mode in @('valid', 'absent', 'invalid', 'oversize')) {
        & (Join-Path $output 'identity.exe') $mode
        if ($LASTEXITCODE) { throw "App identity check failed: $mode" }
    }
}
