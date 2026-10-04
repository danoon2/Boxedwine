// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Boxedwine.Library;

public static class DataFormat
{
    // Swift's default Codable Date is seconds from 2001, not a Unix timestamp.
    public static readonly DateTimeOffset Epoch = new(2001, 1, 1, 0, 0, 0, TimeSpan.Zero);
    public static double Now => (DateTimeOffset.UtcNow - Epoch).TotalSeconds;
    public static readonly JsonSerializerOptions Json = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        MaxDepth = 64
    };
    public static T Clone<T>(T value) => JsonSerializer.Deserialize<T>(JsonSerializer.Serialize(value, Json), Json)!;
    public static T Read<T>(string path, long limit = 32 * 1024 * 1024)
    {
        SafeFiles.NoLinks(path);
        if (new FileInfo(path).Length > limit) throw new InvalidDataException("The metadata file is too large.");
        return JsonSerializer.Deserialize<T>(File.ReadAllBytes(path), Json) ?? throw new InvalidDataException("The metadata file is empty.");
    }
}

public sealed class LibraryApp
{
    [JsonRequired] public Guid Id { get; set; } = Guid.NewGuid();
    [JsonRequired] public string Name { get; set; } = "New app";
    public string? Executable { get; set; }
    public string? Installer { get; set; }
    [JsonRequired] public double CreatedAt { get; set; } = DataFormat.Now;
    public double? LastOpened { get; set; }
    public string Resolution { get; set; } = "1024x768";
    public bool FullScreen { get; set; }
    public List<string> Arguments { get; set; } = [];
    public List<string>? BoxedwineArguments { get; set; }
    public bool IsNotepad { get; set; }
    public string? BuiltInProgram { get; set; }
    public string? SavedWineVersion { get; set; }
    public WineReference? WinePackage { get; set; }
    public DemoOrigin? Demo { get; set; }
    public DemoSettings? DemoSettings { get; set; }
    public string? WindowsVersion { get; set; }
    public bool? WindowsVersionPending { get; set; }
    public string? OpenGLBackend { get; set; }
    public bool? OpenGLBackendPending { get; set; }
    public string? WineRenderer { get; set; }
    public bool? WineRendererPending { get; set; }
    public string? WindowsOpenGL { get; set; }
    public byte[]? CustomIconPNG { get; set; }
    [JsonExtensionData] public Dictionary<string, JsonElement>? Extra { get; set; }
    [JsonIgnore] public string? BuiltIn => BuiltInProgram ?? (IsNotepad ? "notepad" : null);
    [JsonIgnore] public string PreferredWindows => WindowsVersion ?? DemoSettings?.WindowsVersion ?? "wineDefault";
    [JsonIgnore] public string PreferredBackend => OpenGLBackend ?? (DemoSettings?.UseEGL is bool egl ? egl ? "egl" : "glx" : "wineDefault");
    [JsonIgnore] public string PreferredRenderer => WineRenderer ?? (DemoSettings?.Gdi is bool gdi ? gdi ? "gdi" : "openGL" : "wineDefault");
    [JsonIgnore] public string PreferredWindowsOpenGL => WindowsOpenGL ?? (DemoSettings?.NativeOpenGL == true ? "native" : "default");
    [JsonIgnore] public bool HasPendingSettings => WindowsVersionPending == true || OpenGLBackendPending == true || WineRendererPending == true;
    public void ChooseWindows(string value) { if (value != PreferredWindows) { WindowsVersion = value; WindowsVersionPending = true; } }
    public void ChooseBackend(string value) { if (value != PreferredBackend) { OpenGLBackend = value; OpenGLBackendPending = true; } }
    public void ChooseRenderer(string value) { if (value != PreferredRenderer) { WineRenderer = value; WineRendererPending = true; } }
}

public sealed class RemovedApp
{
    [JsonRequired] public LibraryApp App { get; set; } = new();
    public double RemovedAt { get; set; } = DataFormat.Now;
    public double? DeletionStartedAt { get; set; }
}
public sealed class LibraryDocument
{
    [JsonRequired] public int Version { get; set; } = 13;
    [JsonRequired] public List<LibraryApp> Apps { get; set; } = [];
    public List<RemovedApp> RemovedApps { get; set; } = [];
}
public sealed record WineReference(string Sha256, long Bytes, string WineVersion, string FilesystemVersion);
public sealed record DemoOrigin(string Id, string CatalogRelease, string PackageSHA256, string ShortcutExe);
public sealed class DemoSettings
{
    public string? WindowsVersion { get; set; }
    public bool? Gdi { get; set; }
    public string? Resolution { get; set; }
    public string? InstallResolution { get; set; }
    public int? BitsPerPixel { get; set; }
    public int? CpuCount { get; set; }
    public bool? NativeOpenGL { get; set; }
    public bool? UseEGL { get; set; }
    public bool? CncDDraw { get; set; }
    public bool? DisableHideCursor { get; set; }
    public bool? ForceRelativeMouse { get; set; }
    public List<string> LaunchArguments(string cwd)
    {
        List<string> result = [];
        if (BitsPerPixel is int bpp) result.AddRange(["-bpp", bpp.ToString()]);
        if (CpuCount is int count) result.AddRange(["-cpuAffinity", count.ToString()]);
        if (CncDDraw == true) result.AddRange(["-ddrawOverride", cwd]);
        if (DisableHideCursor == true) result.Add("-disableHideCursor");
        if (ForceRelativeMouse == true) result.Add("-forceRelativeMouse");
        return result;
    }
}
public sealed class LauncherPreferences
{
    public string Theme { get; set; } = "System";
    public string? EmulatorPath { get; set; }
    public bool DeleteImmediately { get; set; }
    public string? OpenGLImplementation { get; set; }
}
public sealed record OperationProgress(string Message, long Completed = 0, long Total = 0)
{
    public double Percent => Total > 0 ? Math.Clamp(100.0 * Completed / Total, 0, 100) : 0;
}
public sealed class PendingOperation
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "File operation";
    public string Kind { get; set; } = "import";
    public LibraryApp? App { get; set; }
    public bool Ready { get; set; }
    public List<BackupEntry>? Entries { get; set; }
    public string? ExportPath { get; set; }
    [JsonIgnore] public string? Problem { get; set; }
    public double CreatedAt { get; set; } = DataFormat.Now;
}
public sealed record ProgramCandidate(string Path)
{
    public static string? CatalogPath(IEnumerable<ProgramCandidate> candidates, string expected)
    {
        var matches = candidates.Where(p => System.IO.Path.GetFileName(p.Path).Equals(expected, StringComparison.OrdinalIgnoreCase)).Take(2).ToList();
        return matches.Count == 1 ? matches[0].Path : null;
    }
    public string Name => System.IO.Path.GetFileNameWithoutExtension(Path);
    public string WindowsPath => "C:\\" + Path[(LibraryRepository.DriveC.Length + 1)..].Replace('/', '\\');
    public bool IsMaintenance => System.Text.RegularExpressions.Regex.IsMatch(Name, "^(unins|unwise|uninstall|setup|install|update|crash)", System.Text.RegularExpressions.RegexOptions.IgnoreCase);
    public override string ToString() => $"{Name}{(IsMaintenance ? " (setup / maintenance)" : "")}\n{WindowsPath}";
}
