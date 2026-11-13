# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$sdkPin = Get-Content -LiteralPath (Join-Path $repositoryRoot 'project\msvc\BoxedwineUI\global.json') -Raw | ConvertFrom-Json
$sdkVersion = $sdkPin.sdk.version
# The SDK runs on the worker's architecture, which may differ from the target.
$architecture = if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64' -or $env:PROCESSOR_ARCHITEW6432 -eq 'ARM64') { 'arm64' } else { 'x64' }
$installDirectory = Join-Path $repositoryRoot ".tools\dotnet\$sdkVersion\$architecture"
function Test-BoxedwineSdk {
    param([string]$Executable)
    if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) { return $false }
    $previousLanguage = $env:DOTNET_CLI_UI_LANGUAGE
    try {
        $env:DOTNET_CLI_UI_LANGUAGE = 'en'
        $sdks = & $Executable --list-sdks 2>$null
        if ($LASTEXITCODE -ne 0 -or -not @($sdks | Where-Object { $_ -match ('^' + [regex]::Escape($sdkVersion) + '\s+\[') }).Count) {
            return $false
        }
        # An x86/x64 host on PATH must not hide the native ARM64 SDK.
        $info = & $Executable --info 2>$null
        return $LASTEXITCODE -eq 0 -and @($info | Where-Object { $_ -match ('^\s*Architecture:\s*' + $architecture + '\s*$') }).Count -gt 0
    } catch {
        Write-Verbose "Unable to use .NET host ${Executable}: $_"
        return $false
    } finally {
        $env:DOTNET_CLI_UI_LANGUAGE = $previousLanguage
    }
}

# Probe all PATH hosts, explicit roots and standard install locations. Jenkins
# can retain an old PATH after an administrator installs the SDK on the worker.
$candidates = @(Get-Command dotnet.exe -CommandType Application -All -ErrorAction SilentlyContinue | ForEach-Object { $_.Source })
$roots = @([Environment]::GetEnvironmentVariable("DOTNET_ROOT_$($architecture.ToUpperInvariant())"), $env:DOTNET_ROOT)
foreach ($programFiles in @($env:ProgramW6432, $env:ProgramFiles)) {
    if ($programFiles) { $roots += Join-Path $programFiles 'dotnet' }
}
if ($env:USERPROFILE) { $roots += Join-Path $env:USERPROFILE '.dotnet' }
foreach ($root in $roots) {
    if ($root) { $candidates += Join-Path $root 'dotnet.exe' }
}
# Retain the workspace cache as the final local candidate before downloading.
$candidates += Join-Path $installDirectory 'dotnet.exe'
$dotnet = $null
foreach ($candidate in ($candidates | Select-Object -Unique)) {
    if (Test-BoxedwineSdk $candidate) {
        $dotnet = $candidate
        break
    }
}

if (-not $dotnet) {
    $dotnet = Join-Path $installDirectory 'dotnet.exe'
    New-Item -ItemType Directory -Path $installDirectory -Force | Out-Null
    $installer = Join-Path $installDirectory 'dotnet-install.ps1'
    # Microsoft's non-admin installer is intended for CI. Keep the SDK outside
    # the UI project and Deploy so it cannot enter compilation or the release ZIP.
    [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    Write-Host "Installing .NET SDK $sdkVersion ($architecture) in $installDirectory"
    Invoke-WebRequest -Uri 'https://dot.net/v1/dotnet-install.ps1' -OutFile $installer -UseBasicParsing
    & $installer -Version $sdkVersion -Architecture $architecture -InstallDir $installDirectory -NoPath
    if (-not (Test-BoxedwineSdk $dotnet)) { throw "The .NET SDK $sdkVersion installation failed: $installDirectory" }
}

$installDirectory = Split-Path -Parent $dotnet
# These settings affect only this build process and its children. In particular,
# dotnet run's test apphost must find this runtime without registry installation.
$env:DOTNET_ROOT = $installDirectory
Set-Item -Path "Env:DOTNET_ROOT_$($architecture.ToUpperInvariant())" -Value $installDirectory
$env:DOTNET_MULTILEVEL_LOOKUP = '0'
$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
$env:DOTNET_NOLOGO = '1'
$env:PATH = $installDirectory + [IO.Path]::PathSeparator + $env:PATH
Write-Host "Using .NET SDK $sdkVersion ($architecture): $dotnet"
