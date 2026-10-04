# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$sdkVersion = '10.0.302'
# The SDK runs on the worker's architecture, which may differ from the target.
$architecture = if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64' -or $env:PROCESSOR_ARCHITEW6432 -eq 'ARM64') { 'arm64' } else { 'x64' }
$installDirectory = Join-Path $repositoryRoot ".tools\dotnet\$sdkVersion\$architecture"
$dotnet = Join-Path $installDirectory 'dotnet.exe'

function Test-BoxedwineSdk {
    if (-not (Test-Path -LiteralPath $dotnet -PathType Leaf)) { return $false }
    $sdks = & $dotnet --list-sdks
    return $LASTEXITCODE -eq 0 -and @($sdks | Where-Object { $_ -match ('^' + [regex]::Escape($sdkVersion) + ' \[') }).Count -gt 0
}

if (-not (Test-BoxedwineSdk)) {
    New-Item -ItemType Directory -Path $installDirectory -Force | Out-Null
    $installer = Join-Path $installDirectory 'dotnet-install.ps1'
    # Microsoft's non-admin installer is intended for CI. Keep the SDK outside
    # the UI project and Deploy so it cannot enter compilation or the release ZIP.
    [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    Write-Host "Installing .NET SDK $sdkVersion ($architecture) in $installDirectory"
    Invoke-WebRequest -Uri 'https://dot.net/v1/dotnet-install.ps1' -OutFile $installer -UseBasicParsing
    & $installer -Version $sdkVersion -Architecture $architecture -InstallDir $installDirectory -NoPath
    if (-not (Test-BoxedwineSdk)) { throw "The .NET SDK $sdkVersion installation failed: $installDirectory" }
}

# These settings affect only this build process and its children. In particular,
# dotnet run's test apphost must find this runtime without registry installation.
$env:DOTNET_ROOT = $installDirectory
Set-Item -Path "Env:DOTNET_ROOT_$($architecture.ToUpperInvariant())" -Value $installDirectory
$env:DOTNET_MULTILEVEL_LOOKUP = '0'
$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
$env:DOTNET_NOLOGO = '1'
$env:PATH = $installDirectory + [IO.Path]::PathSeparator + $env:PATH
Write-Host "Using .NET SDK $sdkVersion ($architecture): $dotnet"
