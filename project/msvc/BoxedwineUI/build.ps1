param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [switch]$SkipCatalog,
    [switch]$Test,
    [switch]$BuildRuntime,
    [switch]$Publish,
    [ValidateSet('x64','ARM64')][string]$Platform = 'x64',
    [string]$Emulator,
    [string]$OutputDirectory,
    [string]$MSBuildPath
)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot '..\..\..'))
$runtimeIdentifier = 'win-' + $Platform.ToLowerInvariant()
if ($Publish) {
    if (-not $BuildRuntime -and -not $Emulator) { throw 'Publishing requires -BuildRuntime or -Emulator so the download includes its native engine.' }
    if (-not $OutputDirectory) { $OutputDirectory = Join-Path $projectRoot "artifacts\publish\$runtimeIdentifier\$Configuration" }
} elseif (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $projectRoot "bin\$Configuration\net10.0-windows"
}
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if ($Publish) {
    # dotnet publish does not remove files left by an older self-contained release.
    # Jenkins supplies a clean destination; other callers must choose one too.
    foreach ($runtimeFile in @('hostfxr.dll', 'hostpolicy.dll', 'coreclr.dll', 'System.Private.CoreLib.dll', 'PresentationFramework.dll')) {
        if (Test-Path -LiteralPath (Join-Path $OutputDirectory $runtimeFile)) {
            throw "The publish folder contains a bundled .NET runtime ($runtimeFile). Remove the old publish output or choose a new -OutputDirectory."
        }
    }
}
if (-not $SkipCatalog) {
    $pin = Get-Content -LiteralPath (Join-Path $repositoryRoot 'resources\demo-catalog.lock.json') -Raw | ConvertFrom-Json
    $assets = Join-Path $projectRoot 'Assets'
    $archive = Join-Path $assets ($pin.version + '.zip')
    New-Item -ItemType Directory -Path $assets -Force | Out-Null
    if (-not (Test-Path -LiteralPath $archive) -or (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pin.sha256) {
        Invoke-WebRequest -Uri $pin.url -OutFile $archive -UseBasicParsing
    }
    if ((Get-Item -LiteralPath $archive).Length -ne $pin.bytes -or (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pin.sha256) { throw 'The demo catalog failed its pinned checksum or size check.' }
    $catalog = Join-Path $assets 'Catalog'
    New-Item -ItemType Directory -Path $catalog -Force | Out-Null
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead($archive)
    try {
        foreach ($entry in $zip.Entries) {
            if ($entry.FullName.Contains('/') -or $entry.FullName.Contains('\') -or $entry.FullName.Contains(':') -or $entry.FullName -eq '..') { throw 'The catalog must contain only flat files.' }
            $destination = [IO.Path]::GetFullPath((Join-Path $catalog $entry.FullName))
            if (-not $destination.StartsWith([IO.Path]::GetFullPath($catalog) + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Unsafe catalog path.' }
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $true)
        }
    } finally { $zip.Dispose() }
}
Push-Location $projectRoot
try {
    if ($BuildRuntime) {
        if (-not $MSBuildPath) {
            $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
            if ($command) { $MSBuildPath = $command.Source }
            else {
                $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
                $MSBuildPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
            }
        }
        if (-not $MSBuildPath) { throw 'Install Visual Studio with the C++ desktop workload to build the emulator.' }
        $solution = Join-Path $repositoryRoot 'project\msvc\BoxedWine\BoxedWine.sln'
        & $MSBuildPath $solution '/t:zlib;SDL2;SDL2main' "/p:Configuration=$Configuration" "/p:Platform=$Platform" /m /nologo /v:minimal
        if ($LASTEXITCODE -ne 0) { throw 'Emulator dependency build failed.' }
        $runtimeOutput = Join-Path $projectRoot "artifacts\runtime\$Platform\$Configuration\"
        $runtimeObjects = Join-Path $projectRoot "artifacts\runtime-obj\$Platform\$Configuration\"
        & $MSBuildPath (Join-Path $repositoryRoot 'project\msvc\BoxedWine\BoxedWine\BoxedWine.vcxproj') /t:Build "/p:Configuration=$Configuration" "/p:Platform=$Platform" /p:BuildProjectReferences=false /p:BoxedwineStandaloneRuntime=true /p:TargetName=BoxedwineEngine "/p:OutDir=$runtimeOutput" "/p:IntDir=$runtimeObjects" /m /nologo /v:minimal
        if ($LASTEXITCODE -ne 0) { throw 'Native emulator build failed.' }
        $Emulator = Join-Path $runtimeOutput 'BoxedwineEngine.exe'
    }
    if ($Publish) {
        # Publishing may need an architecture-specific apphost pack. Normal builds remain offline.
        & dotnet restore BoxedwineUI.csproj --configfile NuGet.Publish.Config -r $runtimeIdentifier -p:SelfContained=false --nologo
        if ($LASTEXITCODE -ne 0) { throw 'Publish restore failed.' }
        & dotnet publish BoxedwineUI.csproj --no-restore -c $Configuration -r $runtimeIdentifier --self-contained false -o $OutputDirectory -p:DebugType=None -p:DebugSymbols=false --nologo
        if ($LASTEXITCODE -ne 0) { throw 'Publish failed.' }
    } else {
        & dotnet restore BoxedwineUI.csproj --configfile NuGet.Config --nologo
        if ($LASTEXITCODE -ne 0) { throw 'Restore failed.' }
        & dotnet build BoxedwineUI.csproj --no-restore -c $Configuration -o $OutputDirectory --nologo
        if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    }
    if ($Test) {
        & dotnet restore Tests\Boxedwine.Tests.csproj --configfile NuGet.Config --nologo
        if ($LASTEXITCODE -ne 0) { throw 'Test restore failed.' }
        & dotnet run --project Tests\Boxedwine.Tests.csproj -c $Configuration --no-restore -- (Join-Path $repositoryRoot 'tmp\native-ui-tests')
        if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
    }
    if ($Emulator) {
        $emulatorFile = Get-Item -LiteralPath $Emulator
        $target = Join-Path $OutputDirectory 'Runtime'
        New-Item -ItemType Directory -Path $target -Force | Out-Null
        Copy-Item -LiteralPath $emulatorFile.FullName -Destination (Join-Path $target 'BoxedwineEngine.exe')
        # Do not carry an optional ANGLE backend over from an older engine build.
        foreach ($angleDll in @('libEGL.dll', 'libGLESv2.dll')) {
            $oldCopy = Join-Path $target $angleDll
            if (Test-Path -LiteralPath $oldCopy -PathType Leaf) { Remove-Item -LiteralPath $oldCopy -Force }
        }
        Get-ChildItem -LiteralPath $emulatorFile.DirectoryName -Filter '*.dll' |
            Where-Object { $_.Name -notin @('libEGL.dll', 'libGLESv2.dll') } | Copy-Item -Destination $target
    }
} finally { Pop-Location }
Write-Output "Built: $OutputDirectory\Boxedwine.exe"
