# Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
param([ValidateSet('x64','Win32','ARM64')][string]$Platform = 'x64', [switch]$CompileOnly)
$ErrorActionPreference = 'Stop'
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $installation) { throw 'Visual Studio C++ tools are required.' }
$vcvars = Join-Path $installation 'VC\Auxiliary\Build\vcvarsall.bat'
$target = switch ($Platform) { 'Win32' { 'x64_x86' } 'ARM64' { 'x64_arm64' } default { 'x64' } }
$tlogs = Join-Path $repository "project\msvc\BoxedwineUI\artifacts\runtime-obj\$Platform\Release\BoxedWine.tlog"
if (-not (Test-Path (Join-Path $tlogs 'link.command.1.tlog'))) { throw "Build the Release native runtime for $Platform first." }
$output = Join-Path $repository "tmp\windows-embedded-resize\$Platform"
New-Item -ItemType Directory -Force -Path $output | Out-Null
# Reuse the engine's actual compiler settings and objects, replacing its entry
# point so the checks exercise production embedding and resizing code.
$flags = Get-Content (Join-Path $tlogs 'CL.command.1.tlog') -Encoding Unicode | Where-Object { $_.StartsWith('/c ') } | Select-Object -First 1
$flags = $flags -replace '/F[od]"[^"]*"', '' -replace ' /FC .*$', ''
$flags += ' /UNDEBUG /DSDL_MAIN_HANDLED'
foreach ($include in @('platform/sdl', 'lib/simde', 'lib/sdl2/include', 'lib/glew/include', 'lib/asmjit', 'lib/imgui', 'lib/zlib', 'lib/zlib/contrib/minizip', 'lib/pugixml/src')) {
    $flags += " /I`"$repository\$include`""
}
[IO.File]::WriteAllText((Join-Path $output 'system.rsp'), "$flags /Dmain=boxedwineHarnessOriginalMain /Fo`"$output\system.obj`" /Fd`"$output\test.pdb`" `"$repository\platform\sdl\knativesystem.cpp`"")
[IO.File]::WriteAllText((Join-Path $output 'test.rsp'), "$flags /Fo`"$output\test.obj`" /Fd`"$output\test.pdb`" `"$repository\tools\test_windows_embedded_resize.cpp`"")
$link = (Get-Content (Join-Path $tlogs 'link.command.1.tlog') -Encoding Unicode | Select-Object -Skip 1) -join "`n"
$link = $link -replace '/OUT:"[^"]*"', "/OUT:`"$output\resize.exe`"" -replace '/PDB:"[^"]*"', "/PDB:`"$output\resize.pdb`""
$link = $link -replace '/IMPLIB:"[^"]*"', "/IMPLIB:`"$output\resize.lib`"" -replace '/LTCGOUT:"[^"]*"', "/LTCGOUT:`"$output\resize.iobj`""
$link = $link -replace '/SUBSYSTEM:WINDOWS', '/SUBSYSTEM:CONSOLE' -replace '"[^"\n]*[\\/]KNATIVESYSTEM\.OBJ"', "`"$output\system.obj`""
$link += "`n`"$output\test.obj`"`n"
[IO.File]::WriteAllText((Join-Path $output 'link.rsp'), $link)
$batch = Join-Path $output 'build.cmd'
$commands = @(
    '@echo off', "call `"$vcvars`" $target >nul", 'if errorlevel 1 exit /b 1',
    "cd /d `"$repository\project\msvc\BoxedWine\BoxedWine`"",
    "cl @`"$output\system.rsp`"", 'if errorlevel 1 exit /b 1',
    "cl @`"$output\test.rsp`"", 'if errorlevel 1 exit /b 1',
    "link @`"$output\link.rsp`"",
    'exit /b %errorlevel%'
)
[IO.File]::WriteAllLines($batch, $commands, [Text.Encoding]::Default)
& $env:ComSpec /d /c $batch
if ($LASTEXITCODE) { throw 'Embedded resize test compilation failed.' }
if (-not $CompileOnly) {
    if ($Platform -eq 'ARM64' -and $env:PROCESSOR_ARCHITECTURE -ne 'ARM64') { throw 'Run ARM64 checks on an ARM64 machine or pass -CompileOnly.' }
    Push-Location $repository
    try {
        & (Join-Path $output 'resize.exe')
        if ($LASTEXITCODE) { throw 'Embedded resize checks failed.' }
    } finally { Pop-Location }
}
