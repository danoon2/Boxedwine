# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
# Run with Windows PowerShell 5.1. Uses tiny local host fixtures; never downloads .NET.
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$sdkPinPath = Join-Path $repositoryRoot 'project\msvc\BoxedwineUI\global.json'
$sdkPin = Get-Content -LiteralPath $sdkPinPath -Raw | ConvertFrom-Json
$sdkVersion = $sdkPin.sdk.version
if ($sdkPin.sdk.rollForward -ne 'disable') { throw 'The UI must select the exact required SDK.' }
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('boxedwine-sdk-tests-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
$environmentNames = @('PATH', 'DOTNET_ROOT', 'DOTNET_ROOT_X64', 'DOTNET_ROOT_ARM64',
    'DOTNET_MULTILEVEL_LOOKUP', 'DOTNET_CLI_TELEMETRY_OPTOUT', 'DOTNET_NOLOGO',
    'DOTNET_CLI_UI_LANGUAGE', 'ProgramFiles', 'ProgramW6432', 'USERPROFILE',
    'PROCESSOR_ARCHITECTURE', 'PROCESSOR_ARCHITEW6432')
$savedEnvironment = @{}
foreach ($name in $environmentNames) { $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name) }
$fixtureHost = Join-Path $testRoot 'host.exe'

function New-SdkFixture([string]$Directory, [string]$Versions, [string]$Architecture, [string]$ExitCode = '0') {
    New-Item -ItemType Directory -Force -Path $Directory | Out-Null
    Copy-Item -LiteralPath $fixtureHost -Destination (Join-Path $Directory 'dotnet.exe')
    [IO.File]::WriteAllLines((Join-Path $Directory 'fixture.txt'), @($Versions, $Architecture, $ExitCode))
}

# Replace only the download boundary. The helper runs unchanged and launches
# real fixture processes, including its post-install version/architecture checks.
function Invoke-WebRequest {
    param($Uri, $OutFile, [switch]$UseBasicParsing)
    if ($Uri -ne 'https://dot.net/v1/dotnet-install.ps1') { throw "Unexpected download: $Uri" }
    $testState.downloads++
    @'
param($Version, $Architecture, $InstallDir, [switch]$NoPath)
$testState.installerArguments = @($Version, $Architecture, $InstallDir, $NoPath.IsPresent)
if ($testState.failInstall) { return }
New-SdkFixture $InstallDir $Version $Architecture
'@ | Set-Content -LiteralPath $OutFile
}

try {
    Add-Type -OutputAssembly $fixtureHost -OutputType ConsoleApplication -TypeDefinition @'
using System;
using System.IO;
public class SdkFixture
{
    public static int Main(string[] args)
    {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string[] config = File.ReadAllLines(Path.Combine(root, "fixture.txt"));
        if (args.Length == 1 && args[0] == "--list-sdks")
        {
            foreach (string version in config[0].Split(';'))
                if (version != "-") Console.WriteLine(version + " [" + Path.Combine(root, "sdk") + "]");
        }
        else if (args.Length == 1 && args[0] == "--info")
            Console.WriteLine("Host:\n  Architecture: " + config[1]);
        else return 91;
        return int.Parse(config[2]);
    }
}
'@
    $cases = @('installed', 'side-by-side', 'standard-location', 'explicit-root',
        'cached', 'missing', 'runtime-only', 'wrong-version', 'wrong-architecture',
        'broken-host', 'failed-install')
    foreach ($case in $cases) {
        $caseRoot = Join-Path $testRoot $case
        $helperDirectory = Join-Path $caseRoot 'tools\jenkins'
        $uiDirectory = Join-Path $caseRoot 'project\msvc\BoxedwineUI'
        New-Item -ItemType Directory -Force -Path $helperDirectory, $uiDirectory | Out-Null
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'ensure-dotnet.ps1') -Destination $helperDirectory
        Copy-Item -LiteralPath $sdkPinPath -Destination $uiDirectory
        $workerSdk = Join-Path $caseRoot 'Worker SDK'
        $standardRoot = Join-Path $caseRoot 'Program Files'
        $standardSdk = Join-Path $standardRoot 'dotnet'
        $architecture = if ($case -eq 'wrong-architecture') { 'arm64' } else { 'x64' }
        $cachedSdk = Join-Path $caseRoot ".tools\dotnet\$sdkVersion\$architecture"
        $expected = $cachedSdk
        $expectedDownloads = 0
        $testState = @{ downloads = 0; installerArguments = $null; failInstall = $case -eq 'failed-install' }
        $env:PATH = $workerSdk
        $env:ProgramFiles = $standardRoot
        $env:ProgramW6432 = $standardRoot
        $env:USERPROFILE = Join-Path $caseRoot 'User'
        $env:PROCESSOR_ARCHITECTURE = $architecture.ToUpperInvariant()
        $env:PROCESSOR_ARCHITEW6432 = $null
        $env:DOTNET_ROOT = $null
        $env:DOTNET_ROOT_X64 = $null
        $env:DOTNET_ROOT_ARM64 = $null
        $env:DOTNET_CLI_UI_LANGUAGE = 'fr'
        switch ($case) {
            'installed' { New-SdkFixture $workerSdk $sdkVersion 'x64'; $expected = $workerSdk }
            'side-by-side' { New-SdkFixture $workerSdk "10.0.404;$sdkVersion" 'x64'; $expected = $workerSdk }
            'standard-location' { New-SdkFixture $standardSdk $sdkVersion 'x64'; $expected = $standardSdk }
            'explicit-root' {
                New-SdkFixture $standardSdk $sdkVersion 'x64'
                $env:DOTNET_ROOT_X64 = $standardSdk
                $env:ProgramFiles = $null; $env:ProgramW6432 = $null
                $expected = $standardSdk
            }
            'cached' { New-SdkFixture $cachedSdk $sdkVersion 'x64' }
            'missing' { $expectedDownloads = 1 }
            'runtime-only' { New-SdkFixture $workerSdk '-' 'x64'; $expectedDownloads = 1 }
            'wrong-version' {
                New-SdkFixture $workerSdk '10.0.404' 'x64'
                New-SdkFixture $cachedSdk '10.0.302-preview.1' 'x64'
                $expectedDownloads = 1
            }
            'wrong-architecture' {
                New-SdkFixture $workerSdk $sdkVersion 'x64'
                New-SdkFixture $standardSdk $sdkVersion 'arm64'
                $expected = $standardSdk
            }
            'broken-host' { New-SdkFixture $workerSdk $sdkVersion 'x64' '7'; $expectedDownloads = 1 }
            'failed-install' { $expectedDownloads = 1 }
        }
        $caught = $null
        try { & (Join-Path $helperDirectory 'ensure-dotnet.ps1') } catch { $caught = $_ }
        if ($testState.failInstall) {
            if (-not $caught -or $caught.Exception.Message -notlike '*installation failed*') {
                throw "${case}: expected install validation failure, got $caught"
            }
        } else {
            if ($caught) { throw $caught }
            if ($env:DOTNET_ROOT -ne $expected -or
                [Environment]::GetEnvironmentVariable("DOTNET_ROOT_$($architecture.ToUpperInvariant())") -ne $expected -or
                $env:PATH.Split(';')[0] -ne $expected) { throw "${case}: selected incorrect SDK root" }
        }
        if ($testState.downloads -ne $expectedDownloads) { throw "${case}: downloaded $($testState.downloads) times, wanted $expectedDownloads" }
        $installerArguments = $testState.installerArguments
        if ($expectedDownloads -and ($installerArguments[0] -ne $sdkVersion -or $installerArguments[1] -ne $architecture -or
            $installerArguments[2] -ne $cachedSdk -or -not $installerArguments[3])) { throw "${case}: incorrect installer arguments" }
        if ($env:DOTNET_CLI_UI_LANGUAGE -ne 'fr') { throw "${case}: changed caller's CLI language" }
        Write-Host "PASS $case"
    }
    Write-Host "$($cases.Count) SDK discovery tests passed."
} finally {
    foreach ($name in $environmentNames) { [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name]) }
    $resolvedRoot = [IO.Path]::GetFullPath($testRoot)
    $tempDirectory = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    if (-not $resolvedRoot.StartsWith($tempDirectory, [StringComparison]::OrdinalIgnoreCase) -or
        [IO.Path]::GetFileName($resolvedRoot) -notlike 'boxedwine-sdk-tests-*') { throw 'Unexpected test cleanup path.' }
    Remove-Item -LiteralPath $resolvedRoot -Recurse -Force
}
