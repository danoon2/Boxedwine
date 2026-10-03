# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
param(
    [Parameter(Mandatory = $true)][string]$Directory,
    [Parameter(Mandatory = $true)][ValidateSet('Win32', 'x64', 'ARM64')][string]$Platform
)
$ErrorActionPreference = 'Stop'
$expectedMachine = @{ Win32 = 0x014c; x64 = 0x8664; ARM64 = 0xaa64 }[$Platform]
function Require-File([string]$relative) {
    $path = Join-Path $Directory $relative
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Package is missing $relative" }
    return $path
}
function Check-Executable([string]$relative, [int]$subsystem = 0) {
    $reader = New-Object IO.BinaryReader([IO.File]::OpenRead((Require-File $relative)))
    try {
        if ($reader.ReadUInt16() -ne 0x5a4d) { throw "Invalid executable: $relative" }
        $reader.BaseStream.Position = 0x3c
        $offset = $reader.ReadInt32()
        $reader.BaseStream.Position = $offset
        if ($reader.ReadUInt32() -ne 0x4550 -or $reader.ReadUInt16() -ne $expectedMachine) { throw "Wrong executable architecture for ${Platform}: $relative" }
        $reader.BaseStream.Position = $offset + 24 + 68
        if ($subsystem -ne 0 -and $reader.ReadUInt16() -ne $subsystem) { throw "Wrong executable subsystem: $relative" }
    } finally { $reader.Dispose() }
}
Check-Executable 'Boxedwine.exe' 2
Check-Executable 'Boxedwine_console.exe' 3
foreach ($optional in @('libEGL.dll', 'libGLESv2.dll', 'Runtime\libEGL.dll', 'Runtime\libGLESv2.dll')) {
    if (Test-Path -LiteralPath (Join-Path $Directory $optional)) {
        throw "Windows packages must not include optional ANGLE DLLs: $optional"
    }
}
if ($Platform -ne 'Win32') {
    Check-Executable 'Runtime\BoxedwineEngine.exe' 2
    foreach ($file in @('Boxedwine.dll', 'Boxedwine.Library.dll', 'Boxedwine.deps.json', 'README.txt', 'Resources\WindowsSupport\packages.json', 'Resources\demo-catalog.lock.json', 'Resources\license.txt')) {
        $null = Require-File $file
    }
    $runtime = Get-Content -LiteralPath (Require-File 'Boxedwine.runtimeconfig.json') -Raw | ConvertFrom-Json
    $frameworks = @($runtime.runtimeOptions.frameworks) + @($runtime.runtimeOptions.framework) | Where-Object { $_ }
    $frameworkNames = @($frameworks | ForEach-Object { $_.name })
    if ($runtime.runtimeOptions.includedFrameworks -or
        $frameworkNames -notcontains 'Microsoft.NETCore.App' -or $frameworkNames -notcontains 'Microsoft.WindowsDesktop.App' -or
        @($frameworks | Where-Object { $_.version -notmatch '^10\.0\.\d+$' }).Count -ne 0) {
        throw 'The UI package must require the installed .NET 10 Desktop Runtime.'
    }
    foreach ($bundled in @('hostfxr.dll', 'hostpolicy.dll', 'coreclr.dll', 'PresentationNative_cor3.dll', 'wpfgfx_cor3.dll', 'System.Private.CoreLib.dll', 'PresentationFramework.dll')) {
        if (Test-Path -LiteralPath (Join-Path $Directory $bundled)) {
            throw "The UI package must not bundle .NET runtime files: $bundled"
        }
    }
    $catalogs = @(Get-ChildItem -LiteralPath (Join-Path $Directory 'Resources\Demos') -Filter '*.xml' -Recurse)
    if ($catalogs.Count -ne 1) { throw 'The package must contain exactly one demo catalog.' }
}
Write-Output "Verified $Platform package: $Directory"
