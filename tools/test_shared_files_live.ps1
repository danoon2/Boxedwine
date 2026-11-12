param(
    [Parameter(Mandatory)][string]$Wine,
    [Parameter(Mandatory)][string]$Probe,
    [string]$Engine = "$PSScriptRoot/../project/msvc/BoxedwineUI/bin/Release/net10.0-windows/Runtime/BoxedwineEngine.exe",
    [string]$Output = "$PSScriptRoot/../tmp/shared-files-live"
)
$ErrorActionPreference = 'Stop'
$run = [IO.Path]::GetFullPath((Join-Path $Output ([Guid]::NewGuid().ToString('N'))))
$root = Join-Path $run 'root'
$shared = Join-Path $root 'home/username/Shared'
New-Item -ItemType Directory -Path (Join-Path $shared 'Nested') -Force | Out-Null
Copy-Item -LiteralPath $Probe -Destination (Join-Path $shared 'probe.exe')
[IO.File]::WriteAllText((Join-Path $shared 'Nested/changed.txt'), 'before')
[IO.File]::WriteAllText((Join-Path $shared 'old.txt'), 'rename')
[IO.File]::WriteAllText((Join-Path $shared 'removed.txt'), 'remove')
$start = [Diagnostics.ProcessStartInfo]::new([IO.Path]::GetFullPath($Engine))
$start.UseShellExecute = $false
$start.CreateNoWindow = $true
$start.RedirectStandardInput = $true
$start.RedirectStandardOutput = $true
$start.RedirectStandardError = $true
foreach ($arg in @('-root', $root, '-zip', [IO.Path]::GetFullPath($Wine), '-shared_folder', $shared,
    '-nosound', '-w', '/home/username/Shared', '/bin/wine', '/home/username/Shared/probe.exe')) {
    $start.ArgumentList.Add($arg)
}
$process = [Diagnostics.Process]::new()
$process.StartInfo = $start
$log = [Collections.Generic.List[string]]::new()
$ready = $false
$passed = $false
try {
    $null = $process.Start()
    $readers = @($process.StandardOutput, $process.StandardError)
    $reads = @($readers[0].ReadLineAsync(), $readers[1].ReadLineAsync())
    $timer = [Diagnostics.Stopwatch]::StartNew()
    while ($timer.Elapsed.TotalSeconds -lt 100 -and -not $passed) {
        for ($i = 0; $i -lt 2; $i++) {
            if ($null -eq $reads[$i] -or -not $reads[$i].IsCompleted) { continue }
            $line = $reads[$i].GetAwaiter().GetResult()
            if ($null -eq $line) { $reads[$i] = $null; continue }
            $log.Add($line)
            $plain = [regex]::Replace($line, '\x1B\[[0-?]*[ -/]*[@-~]', '')
            if ($plain.Contains('SHARED_READY') -and -not $ready) {
                $ready = $true
                # Allow the engine to establish its modification-time snapshot.
                Start-Sleep -Milliseconds 650
                [IO.File]::WriteAllText((Join-Path $shared 'patch.txt'), 'copied')
                [IO.File]::WriteAllText((Join-Path $shared 'Nested/changed.txt'), 'updated')
                Move-Item -LiteralPath (Join-Path $shared 'old.txt') -Destination (Join-Path $shared 'renamed.txt')
                Remove-Item -LiteralPath (Join-Path $shared 'removed.txt')
                Write-Output 'Guest ready; applied host additions, modification, rename and deletion.'
            }
            if ($plain.Contains('SHARED_PASS')) { $passed = $true }
            $reads[$i] = $readers[$i].ReadLineAsync()
        }
        if ($null -eq $reads[0] -and $null -eq $reads[1]) { break }
        Start-Sleep -Milliseconds 25
    }
    if (-not $passed) { throw 'Live Shared Files probe failed or timed out. See the run log.' }
    if ([IO.File]::ReadAllText((Join-Path $shared 'guest-output.txt')) -ne 'from Wine') { throw 'Guest writes did not reach the shared host folder.' }
    Write-Output 'PASS Y: live additions, nested modifications, rename, deletion, Wine change notifications and guest writes.'
}
finally {
    [IO.File]::WriteAllLines((Join-Path $run 'engine.log'), $log)
    if (-not $process.HasExited) {
        try { $process.StandardInput.WriteLine('quit'); $process.StandardInput.Flush() } catch {}
        if (-not $process.WaitForExit(5000)) { $process.Kill($true); $process.WaitForExit() }
    }
    $process.Dispose()
    Write-Output "Run log: $run/engine.log"
}
