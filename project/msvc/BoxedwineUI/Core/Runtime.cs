// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;

namespace Boxedwine.Library;

public sealed record RuntimeExit(int Code, bool Stopped, bool Installing);
public sealed class RuntimeSession : IDisposable
{
    public static string PlainText(string text) => Regex.Replace(text, @"\x1B\[[0-?]*[ -/]*[@-~]", "");
    private readonly Process process;
    private readonly FileStream wineLease;
    private readonly BoundedLog log;
    private readonly ProcessJob job;
    private volatile bool stopped;
    private readonly TaskCompletionSource<bool> windowShown = new(TaskCreationOptions.RunContinuationsAsynchronously);
    // False means the session ended or was stopped before displaying a window.
    public Task<bool> WindowShown => windowShown.Task;
    public Task<RuntimeExit> Completion { get; }
    public RuntimeSession(string executable, IReadOnlyList<string> arguments, string wine, string logPath, bool installing = false, bool rotate = true, OpenGLRuntime? openGL = null)
    {
        if (!File.Exists(executable)) throw new FileNotFoundException("Choose BoxedwineEngine.exe in Settings, or place it next to Boxedwine.exe.", executable);
        wineLease = new FileStream(wine, FileMode.Open, FileAccess.Read, FileShare.Read);
        try { log = new BoundedLog(logPath, rotate); } catch { wineLease.Dispose(); throw; }
        process = new Process { StartInfo = new ProcessStartInfo(executable) { UseShellExecute = false, CreateNoWindow = true, RedirectStandardInput = true, RedirectStandardOutput = true, RedirectStandardError = true, WorkingDirectory = Path.GetDirectoryName(executable)! } };
        openGL?.Configure(process.StartInfo);
        foreach (var argument in arguments) process.StartInfo.ArgumentList.Add(argument);
        try { job = new ProcessJob(); } catch { log.Dispose(); wineLease.Dispose(); process.Dispose(); throw; }
        try
        {
            log.Write($"Boxedwine launch · {DateTimeOffset.Now:O}\n" + (openGL == null ? "" : "OpenGL: " + OpenGLDrivers.Label(openGL.Implementation) + "\n") + string.Join(" ", process.StartInfo.ArgumentList.Select(a => a.Contains(' ') ? '"' + a + '"' : a)) + "\n");
            process.Start(); job.Assign(process);
            Completion = Observe(installing);
        }
        catch { process.Dispose(); wineLease.Dispose(); log.Dispose(); job.Dispose(); throw; }
    }
    private async Task Drain(StreamReader reader)
    {
        var marker = new RuntimeWindowMarker();
        char[] buffer = new char[4096]; int count;
        while ((count = await reader.ReadAsync(buffer)) != 0)
        {
            if (marker.Consume(buffer.AsSpan(0, count)) && !stopped) windowShown.TrySetResult(true);
            log.Write(new string(buffer, 0, count));
        }
    }
    private async Task<RuntimeExit> Observe(bool installing)
    {
        try
        {
            var output = Drain(process.StandardOutput); var error = Drain(process.StandardError);
            var streams = Task.WhenAll(output, error);
            // A log write failure must not leave the child blocked on a full stdout pipe.
            _ = output.ContinueWith(_ => ForceStop(), CancellationToken.None, TaskContinuationOptions.OnlyOnFaulted, TaskScheduler.Default);
            _ = error.ContinueWith(_ => ForceStop(), CancellationToken.None, TaskContinuationOptions.OnlyOnFaulted, TaskScheduler.Default);
            await process.WaitForExitAsync(); await streams;
            int code = process.ExitCode; log.Write($"\nProcess exited with code {code}{(stopped ? " (stopped)" : "")}.\n");
            log.Finish(code != 0 && !stopped);
            return new(code, stopped, installing);
        }
        finally { windowShown.TrySetResult(false); wineLease.Dispose(); log.Dispose(); job.Dispose(); }
    }
    public async Task Stop()
    {
        if (Completion.IsCompleted) return;
        stopped = true;
        windowShown.TrySetResult(false);
        try { await process.StandardInput.WriteAsync("quit\n"); await process.StandardInput.FlushAsync(); }
        catch (Exception error) when (error is IOException or InvalidOperationException)
        {
            // Do not send WM_CLOSE as well as quit: the SDL window may already
            // be tearing down. Use a window-close request only if stdin failed.
            try { process.CloseMainWindow(); } catch (InvalidOperationException) { }
        }
        if (await Task.WhenAny(Completion, Task.Delay(5000)) != Completion) ForceStop();
        await Completion;
    }
    public void ForceStop()
    {
        stopped = true;
        windowShown.TrySetResult(false);
        try { if (!process.HasExited) process.Kill(true); } catch (InvalidOperationException) { }
    }
    public void Dispose() { if (!Completion.IsCompleted) ForceStop(); process.Dispose(); }
}
// Match the same complete line as the Mac launcher, across arbitrary pipe reads.
// Keep bounded state even when an app writes an extremely long line. Each output
// stream has its own matcher so unrelated stdout/stderr fragments cannot combine.
internal sealed class RuntimeWindowMarker
{
    private const string Marker = "Showing Window";
    private int matched;
    private bool ignoringLine, reported;
    public bool Consume(ReadOnlySpan<char> text)
    {
        if (reported) return false;
        foreach (char value in text)
        {
            if (value == '\n')
            {
                if (!ignoringLine && matched >= Marker.Length) { reported = true; return true; }
                matched = 0; ignoringLine = false;
            }
            else if (!ignoringLine)
            {
                if (matched < Marker.Length && value == Marker[matched] || matched == Marker.Length && value == '\r') matched++;
                else ignoringLine = true;
            }
        }
        return false;
    }
}
internal sealed class BoundedLog : IDisposable
{
    private readonly object gate = new();
    private readonly string path;
    private readonly FileStream stream;
    private long count;
    private bool truncated;
    public BoundedLog(string path, bool rotate)
    {
        this.path = path; SafeFiles.NoLinks(path);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        string previous = Path.Combine(Path.GetDirectoryName(path)!, "previous.log"); SafeFiles.NoLinks(previous);
        if (rotate && File.Exists(path)) File.Move(path, previous, true);
        stream = new FileStream(path, rotate ? FileMode.Create : FileMode.Append, FileAccess.Write, FileShare.Read);
        count = stream.Length;
    }
    public void Write(string text)
    {
        lock (gate)
        {
            if (count > 4 * 1024 * 1024)
            {
                if (!truncated) { stream.Write(Encoding.UTF8.GetBytes("\n[Log truncated at 4 MB; process output continues to be drained.]\n")); stream.Flush(); truncated = true; }
                return;
            }
            byte[] bytes = Encoding.UTF8.GetBytes(text); stream.Write(bytes); stream.Flush(); count += bytes.Length;
        }
    }
    public void Finish(bool failed)
    {
        lock (gate) { stream.Flush(); if (failed) { string failure = Path.Combine(Path.GetDirectoryName(path)!, "last-failed.log"); SafeFiles.NoLinks(failure); File.Copy(path, failure, true); } }
    }
    public void Dispose() { lock (gate) stream.Dispose(); }
}

public sealed class WineConfiguration(string emulator, OpenGLRuntime? openGL = null)
{
    private static readonly HashSet<string> Versions = [.. LaunchArguments.WindowsVersions.Keys.Where(k => k != "wineDefault"), "win2008r2", "win2008", "win2003", "winxp64", "nt351", "win30", "win20"];
    public static string ReadVersion(string output)
    {
        var versions = output.Split('\n').Select(s => s.Trim()).Where(Versions.Contains).ToList();
        return versions.Count == 1 ? versions[0] : throw new InvalidDataException("Wine did not confirm a Windows version. See the launch log.");
    }
    public static string ReadRegistry(string output, string kind)
    {
        string key = kind == "backend" ? @"HKEY_CURRENT_USER\Software\Wine\X11 Driver" : @"HKEY_CURRENT_USER\Software\Wine\Direct3D";
        var lines = output.Split('\n').Select(s => s.Trim()).ToArray();
        if (lines.Count(s => s.Equals(key, StringComparison.OrdinalIgnoreCase)) != 1) throw new InvalidDataException("Wine did not confirm its graphics settings. See the launch log.");
        string? Value(string name)
        {
            var values = lines.Select(s => Regex.Split(s, @"\s+")).Where(s => s.Length > 0 && s[0].Equals(name, StringComparison.OrdinalIgnoreCase)).ToArray();
            if (values.Length > 1 || values.Length == 1 && (values[0].Length != 3 || values[0][1] != "REG_SZ")) throw new InvalidDataException("Unexpected Wine registry output.");
            return values.Length == 0 ? null : values[0][2];
        }
        if (kind == "backend") return Value("UseEGL") switch { null => "wineDefault", "Y" => "egl", "N" => "glx", _ => throw new InvalidDataException("Unexpected OpenGL backend.") };
        return (Value("DirectDrawRenderer"), Value("renderer")) switch { (null, null) => "wineDefault", ("gdi", "gdi") => "gdi", ("opengl", "gl") => "openGL", _ => throw new InvalidDataException("Unexpected Wine renderer.") };
    }
    public static string Script(string kind, string? value, string token)
    {
        if (!Guid.TryParse(token, out _)) throw new InvalidDataException("Invalid configuration job.");
        string body;
        if (kind == "version")
        {
            if (value != null && !Versions.Contains(value)) throw new InvalidDataException("Unknown Windows version.");
            body = (value != null ? "/bin/wine winecfg /v " + value + " &&\n" : "") + "/bin/wine winecfg /v > /tmp/boxedwine-configuration/result.txt 2>&1";
        }
        else
        {
            string key, setter;
            if (kind == "backend")
            {
                if (value is not ("wineDefault" or "egl" or "glx")) throw new InvalidDataException("Unknown OpenGL backend.");
                key = @"'HKCU\Software\Wine\X11 Driver'";
                setter = value == "wineDefault" ? $"/bin/wine reg add {key} /f && {{ /bin/wine reg delete {key} /v UseEGL /f; true; }}" : $"/bin/wine reg add {key} /v UseEGL /t REG_SZ /d {(value == "egl" ? "Y" : "N")} /f";
            }
            else
            {
                if (value is not ("wineDefault" or "gdi" or "openGL")) throw new InvalidDataException("Unknown Wine renderer.");
                key = @"'HKCU\Software\Wine\Direct3D'";
                setter = value == "wineDefault" ? $"/bin/wine reg add {key} /f && {{ /bin/wine reg delete {key} /v DirectDrawRenderer /f; /bin/wine reg delete {key} /v renderer /f; true; }}" :
                    $"/bin/wine reg add {key} /v DirectDrawRenderer /t REG_SZ /d {(value == "gdi" ? "gdi" : "opengl")} /f && /bin/wine reg add {key} /v renderer /t REG_SZ /d {(value == "gdi" ? "gdi" : "gl")} /f";
            }
            body = setter + " && /bin/wine reg query " + key + " > /tmp/boxedwine-configuration/result.txt 2>&1";
        }
        return "if " + body + "\nthen\n/bin/cat /tmp/boxedwine-configuration/result.txt\nprintf '%s' '" + token + "' > /tmp/boxedwine-configuration/completed\nfi\n/opt/wine/bin/wineserver -k\n";
    }
    public async Task<string> Run(string root, string wine, string log, string kind, string? value, string jobs, CancellationToken cancellation)
    {
        string token = Guid.NewGuid().ToString(), directory = SafeFiles.Beneath(jobs, token);
        Directory.CreateDirectory(directory);
        try
        {
            // Configuration repeatedly forks short-lived Wine tools. Use the conservative
            // memory path for this background work; app launches keep their normal settings.
            List<string> arguments = ["-root", root, "-zip", wine, "-hideWindow", "-disableLinearMemory", "-title", "Preparing Windows", "-mount", directory, "/tmp/boxedwine-configuration", "-w", "/home/username", "/bin/sh", "-c", Script(kind, value, token)];
            using var session = new RuntimeSession(emulator, arguments, wine, log, rotate: false, openGL: openGL);
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellation); timeout.CancelAfter(TimeSpan.FromMinutes(2));
            RuntimeExit exit;
            try { exit = await session.Completion.WaitAsync(timeout.Token); }
            catch (OperationCanceledException) { session.ForceStop(); await session.Completion; if (!cancellation.IsCancellationRequested) throw new TimeoutException("Preparing Windows took more than two minutes. Your settings are kept for another attempt."); throw; }
            if (exit.Code != 0 || exit.Stopped) throw new InvalidDataException("The emulator stopped while preparing Wine settings. Your requested settings remain pending. See the launch log.");
            string completion = SafeFiles.Beneath(directory, "completed"), output = SafeFiles.Beneath(directory, "result.txt");
            if (!File.Exists(completion) || new FileInfo(completion).Length > 128 || File.ReadAllText(completion) != token || !File.Exists(output) || new FileInfo(output).Length > 65536) throw new InvalidDataException("Wine did not finish preparing its settings. Check the launch log and try again.");
            string text = File.ReadAllText(output);
            return kind == "version" ? ReadVersion(text) : ReadRegistry(text, kind);
        }
        finally { SafeFiles.DeleteTree(directory, jobs); }
    }
    public async Task<LibraryApp> Apply(LibraryRepository repository, LibraryApp app, string wine, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        if (!app.HasPendingSettings) return app;
        string jobs = SafeFiles.Beneath(repository.DirectoryPath, "ConfigurationJobs"), root = repository.Root(app), log = SafeFiles.Beneath(repository.AppDirectory(app), "Logs/latest.log");
        if (app.WindowsVersionPending == true)
        {
            string expected = app.PreferredWindows;
            if (expected == "wineDefault")
            {
                string scratch = SafeFiles.Beneath(jobs, "default-" + Guid.NewGuid().ToString("N")); Directory.CreateDirectory(scratch);
                try { expected = await Run(scratch, wine, log, "version", null, jobs, cancellation); }
                finally { SafeFiles.DeleteTree(scratch, jobs); }
            }
            progress?.Report(new("Applying and checking Windows version…"));
            if (await Run(root, wine, log, "version", expected, jobs, cancellation) != expected) throw new InvalidDataException("Wine reported a different Windows version. Your app has not been started.");
        }
        foreach (var request in new[] { (Pending: app.OpenGLBackendPending == true, Kind: "backend", Value: app.PreferredBackend), (Pending: app.WineRendererPending == true, Kind: "renderer", Value: app.PreferredRenderer) }.Where(r => r.Pending))
        {
            progress?.Report(new("Applying and checking Wine graphics settings…"));
            if (await Run(root, wine, log, request.Kind, request.Value, jobs, cancellation) != request.Value) throw new InvalidDataException("Wine reported different graphics settings. Your app has not been started.");
        }
        cancellation.ThrowIfCancellationRequested();
        var ready = DataFormat.Clone(app); ready.WindowsVersionPending = null; ready.OpenGLBackendPending = null; ready.WineRendererPending = null;
        repository.Update(ready); return ready;
    }
}
