# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
param(
    [Parameter(Mandatory = $true)][ValidateSet('Win32', 'x64', 'ARM64')][string]$Platform,
    [string]$MSBuildPath,
    [string]$EditbinPath
)
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ($Platform -ne 'Win32') {
    # Resolve build prerequisites before cleaning packages or compiling C++.
    & (Join-Path $PSScriptRoot 'ensure-dotnet.ps1')
}
$folder = @{ Win32 = 'Win32'; x64 = 'Win64'; ARM64 = 'WinARM64' }[$Platform]
$deployRoot = Join-Path $repositoryRoot 'project\msvc\Deploy'
$destination = [IO.Path]::GetFullPath((Join-Path $deployRoot $folder))

# A reused Jenkins workspace must never contribute old files to a release.
# Check the absolute boundary and reject junctions before recursive removal.
if (-not $destination.StartsWith($deployRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid deployment path.' }
for ($ancestor = $destination; $ancestor; $ancestor = [IO.Path]::GetDirectoryName($ancestor)) {
    if ((Test-Path -LiteralPath $ancestor) -and ((Get-Item -LiteralPath $ancestor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "Deployment path contains a link or junction: $ancestor"
    }
}
if (Test-Path -LiteralPath $destination) { Remove-Item -LiteralPath $destination -Recurse -Force }
New-Item -ItemType Directory -Path $destination -Force | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not $MSBuildPath) {
    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($command) { $MSBuildPath = $command.Source }
    else { $MSBuildPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1 }
}
if (-not $EditbinPath) {
    $command = Get-Command editbin.exe -ErrorAction SilentlyContinue
    if ($command) { $EditbinPath = $command.Source }
    else {
        $hostArchitecture = if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64' -or $env:PROCESSOR_ARCHITEW6432 -eq 'ARM64') { 'arm64' } else { 'x64' }
        $targetArchitecture = if ($Platform -eq 'Win32') { 'x86' } else { $Platform.ToLowerInvariant() }
        $EditbinPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find "VC\Tools\MSVC\*\bin\Host$hostArchitecture\$targetArchitecture\editbin.exe" | Sort-Object -Descending | Select-Object -First 1
    }
}
if (-not $MSBuildPath -or -not $EditbinPath) { throw 'Install Visual Studio C++ tools for this target, or provide -MSBuildPath and -EditbinPath.' }

Push-Location $repositoryRoot
try {
    # Preserve the recorder/CLI build: the UI-owned engine has different stdin and exit semantics.
    & $MSBuildPath project/msvc/BoxedWine/BoxedWine.sln /t:Build /p:Configuration=Release "/p:Platform=$Platform" /p:BoxedwineStandaloneRuntime=false /m /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) { throw "$Platform emulator build failed." }
    $nativeOutput = if ($Platform -eq 'Win32') { 'project\msvc\BoxedWine\Release' } else { "project\msvc\BoxedWine\$Platform\Release" }
    $legacy = Join-Path $repositoryRoot "$nativeOutput\Boxedwine.exe"
    if ($Platform -eq 'Win32') {
        Copy-Item -LiteralPath $legacy -Destination (Join-Path $destination 'Boxedwine.exe')
    } else {
        & ./project/msvc/BoxedwineUI/build.ps1 -Configuration Release -Platform $Platform -BuildRuntime -Publish -Test -OutputDirectory $destination -MSBuildPath $MSBuildPath
    }
    $console = Join-Path $destination 'Boxedwine_console.exe'
    Copy-Item -LiteralPath $legacy -Destination $console
    # ANGLE is an optional GLES backend, not part of the standard Windows package.
    Get-ChildItem -LiteralPath (Split-Path $legacy) -Filter '*.dll' |
        Where-Object { $_.Name -notin @('libEGL.dll', 'libGLESv2.dll') } | Copy-Item -Destination $destination
    & $EditbinPath /nologo /subsystem:console $console
    if ($LASTEXITCODE -ne 0) { throw "$Platform console executable preparation failed." }

    & ./tools/jenkins/check-windows-package.ps1 -Directory $destination -Platform $Platform
    Write-Output "Packaged $Platform in $destination"
} finally { Pop-Location }
