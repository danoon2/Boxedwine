// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Text.RegularExpressions;
using System.Runtime.CompilerServices;

namespace Boxedwine.Library;

public sealed class LibraryRepository : IDisposable
{
    public const string DriveC = "home/username/.wine/drive_c";
    public string DirectoryPath { get; }
    private readonly FileStream libraryLock;
    public LibraryRepository(string directory)
    {
        DirectoryPath = Path.GetFullPath(directory);
        SafeFiles.NoLinks(DirectoryPath);
        Directory.CreateDirectory(DirectoryPath);
        try { libraryLock = new FileStream(Path.Combine(DirectoryPath, ".windows-library.lock"), FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None); }
        catch (IOException error) { throw new IOException("This library is already open in another copy of Boxedwine. Close that copy first.", error); }
        try { Load(); } catch { libraryLock.Dispose(); throw; } // Never silently replace an unreadable library.
    }
    public void Dispose() => libraryLock.Dispose();
    public string AppDirectory(LibraryApp app) => SafeFiles.Beneath(DirectoryPath, "Applications/" + app.Id.ToString().ToUpperInvariant());
    public string Root(LibraryApp app) => SafeFiles.Beneath(AppDirectory(app), "root");
    public string PackagePath(WineReference wine)
    {
        ValidateReference(wine);
        return SafeFiles.Beneath(DirectoryPath, "WinePackages/" + wine.Sha256 + ".zip");
    }
    public WineReference? DefaultWine => File.Exists(Path.Combine(DirectoryPath, "WindowsSupport/imported-wine.json")) ? DataFormat.Read<WineReference>(Path.Combine(DirectoryPath, "WindowsSupport/imported-wine.json")) : null;
    public void SetDefaultWine(WineReference wine) { ValidateReference(wine); SafeFiles.AtomicJson(Path.Combine(DirectoryPath, "WindowsSupport/imported-wine.json"), wine); }
    public string WinePath(LibraryApp app) => app.WinePackage is { } wine ? PackagePath(wine) : app.SavedWineVersion != null ? SafeFiles.Beneath(AppDirectory(app), "WindowsSupport/wine.zip") : DefaultWine is { } fallback ? PackagePath(fallback) : throw new InvalidOperationException("Windows support is missing. Set up Windows support in Settings.");
    public LauncherPreferences Preferences => File.Exists(Path.Combine(DirectoryPath, "windows-settings.json")) ? DataFormat.Read<LauncherPreferences>(Path.Combine(DirectoryPath, "windows-settings.json")) : new();
    public void SavePreferences(LauncherPreferences settings) => SafeFiles.AtomicJson(Path.Combine(DirectoryPath, "windows-settings.json"), settings);
    [MethodImpl(MethodImplOptions.Synchronized)]
    public LibraryDocument Load()
    {
        string path = Path.Combine(DirectoryPath, "library.json");
        var document = File.Exists(path) ? DataFormat.Read<LibraryDocument>(path) : new();
        Validate(document);
        return document;
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Save(LibraryDocument document)
    {
        Validate(document);
        document.Version = 13;
        string path = Path.Combine(DirectoryPath, "library.json");
        if (File.Exists(path))
        {
            Load();
            SafeFiles.NoLinks(Path.Combine(DirectoryPath, "library-previous.json"));
            File.Copy(path, Path.Combine(DirectoryPath, "library-previous.json"), true);
        }
        SafeFiles.AtomicJson(path, document);
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Update(LibraryApp app)
    {
        var document = Load();
        int index = document.Apps.FindIndex(a => a.Id == app.Id);
        if (index < 0) throw new InvalidOperationException("This app is no longer in the library.");
        document.Apps[index] = app;
        Save(document);
    }
    public void Validate(LibraryDocument document)
    {
        if (document.Version is < 1 or > 13) throw new InvalidDataException($"Unsupported library format {document.Version}. Your library has not been changed.");
        if (document.Apps == null || document.RemovedApps == null) throw new InvalidDataException("The library is incomplete.");
        var all = document.Apps.Concat(document.RemovedApps.Select(x => x.App)).ToList();
        if (all.Select(a => a.Id).Distinct().Count() != all.Count) throw new InvalidDataException("Duplicate app identities in the library.");
        foreach (var app in all) ValidateApp(app);
    }
    public void ValidateApp(LibraryApp app)
    {
        if (app.Id == Guid.Empty || string.IsNullOrWhiteSpace(app.Name) || app.Name.Length > 1024) throw new InvalidDataException("Invalid app name or identity.");
        LaunchArguments.ValidateResolution(app.Resolution);
        if (app.PreferredWindowsOpenGL != "default" && !OpenGLDrivers.IsKnown(app.PreferredWindowsOpenGL)) throw new InvalidDataException("Unknown Windows OpenGL implementation.");
        LaunchArguments.ValidateOverrides(app.BoxedwineArguments ?? []);
        if (app.Arguments == null || app.Arguments.Count > 256 || app.Arguments.Any(a => a == null || a.Length > 8192 || a.Contains('\0'))) throw new InvalidDataException("Invalid app arguments.");
        if (app.Executable != null)
        {
            if (!app.Executable.StartsWith(DriveC + "/") || !app.Executable.EndsWith(".exe", StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("The selected program must be an .exe inside drive C.");
            _ = SafeFiles.Beneath(Root(app), app.Executable);
        }
        if (app.Installer != null) _ = SafeFiles.Beneath(AppDirectory(app), app.Installer);
        if (app.WinePackage is { } wine) { ValidateReference(wine); if (wine.WineVersion != app.SavedWineVersion) throw new InvalidDataException("The saved Wine version does not match its package."); }
        if (!LaunchArguments.WindowsVersions.ContainsKey(app.PreferredWindows) || !new[] { "wineDefault", "glx", "egl" }.Contains(app.PreferredBackend) || !new[] { "wineDefault", "openGL", "gdi" }.Contains(app.PreferredRenderer)) throw new InvalidDataException("Unknown Wine configuration setting.");
        if (app.WindowsVersionPending == true && app.WindowsVersion == null || app.OpenGLBackendPending == true && app.OpenGLBackend == null || app.WineRendererPending == true && app.WineRenderer == null) throw new InvalidDataException("A pending Wine setting has no requested value.");
        if (app.BuiltIn is not (null or "notepad" or "minesweeper")) throw new InvalidDataException("Unknown built-in program.");
        if (app.CustomIconPNG is { } png && (png.Length > 4 * 1024 * 1024 || png.Length < 8 || !png.AsSpan(0, 8).SequenceEqual(new byte[] {137,80,78,71,13,10,26,10}))) throw new InvalidDataException("The app icon must be a PNG of at most 4 MB.");
        if (app.DemoSettings is { } demo)
        {
            if (demo.Resolution != null) LaunchArguments.ValidateResolution(demo.Resolution);
            if (demo.InstallResolution != null) LaunchArguments.ValidateResolution(demo.InstallResolution);
            if (demo.BitsPerPixel is int b && b is not (8 or 16 or 32) || demo.CpuCount is int c && c is < 1 or > 64) throw new InvalidDataException("Invalid demo display or CPU setting.");
        }
    }
    public static void ValidateReference(WineReference wine)
    {
        if (!Regex.IsMatch(wine.Sha256, "^[a-f0-9]{64}$") || wine.Bytes is <= 0 or > 4294967296 || !Regex.IsMatch(wine.WineVersion, @"^\d{1,3}\.\d{1,3}(?:[.\-][A-Za-z0-9]+)*$") || !int.TryParse(wine.FilesystemVersion, out int version) || version < 1) throw new InvalidDataException("Invalid Wine package identity.");
    }
    public List<ProgramCandidate> Programs(LibraryApp app)
    {
        string drive = SafeFiles.Beneath(Root(app), DriveC);
        if (!Directory.Exists(drive)) return [];
        return SafeFiles.Tree(drive).Where(p => File.Exists(p) && p.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
            .Select(p => new ProgramCandidate(Path.GetRelativePath(Root(app), p).Replace('\\', '/')))
            .OrderBy(p => p.IsMaintenance).ThenBy(p => p.Name, StringComparer.CurrentCultureIgnoreCase).ToList();
    }
    public LibraryApp? SelectingInstalledDemoProgram(LibraryApp app, RuntimeExit exit)
    {
        if (app.Demo == null || app.Installer == null || app.BuiltIn != null || !exit.Installing || exit.Code != 0 || exit.Stopped) return null;
        // Match the Mac launcher: system programs are not installed-demo candidates.
        var candidates = Programs(app).Where(p => !p.Path.StartsWith(DriveC + "/windows/", StringComparison.OrdinalIgnoreCase)).ToList();
        if (app.Executable != null && candidates.Any(p => p.Path.Equals(app.Executable, StringComparison.OrdinalIgnoreCase))) return DataFormat.Clone(app);
        string? path = ProgramCandidate.CatalogPath(candidates, app.Demo.ShortcutExe);
        if (path == null) return null;
        // The caller commits after scanning; keep its original metadata unchanged.
        var selected = DataFormat.Clone(app); selected.Executable = path; return selected;
    }
    public PendingOperation Begin(LibraryApp app, string kind)
    {
        if (Directory.Exists(AppDirectory(app))) throw new IOException("The app directory already exists.");
        var operation = new PendingOperation { Id = app.Id, Name = app.Name, Kind = kind, App = app };
        SaveOperation(operation);
        Directory.CreateDirectory(Root(app));
        return operation;
    }
    private string OperationPath(Guid id) => SafeFiles.Beneath(DirectoryPath, "WindowsOperations/" + id.ToString("D") + ".json");
    private void SaveOperation(PendingOperation operation) => SafeFiles.AtomicJson(OperationPath(operation.Id), operation);
    public PendingOperation BeginExport(string name, string destination)
    {
        var operation = new PendingOperation { Name = name, Kind = "backup", ExportPath = Path.GetFullPath(destination) };
        string staging = ExportStaging(operation);
        if (Directory.Exists(staging) || File.Exists(staging)) throw new IOException("The temporary backup folder already exists.");
        SaveOperation(operation); Directory.CreateDirectory(staging);
        File.WriteAllText(Path.Combine(staging, ".BoxedwineExport"), operation.Id.ToString("D"));
        return operation;
    }
    public string ExportStaging(PendingOperation operation)
    {
        if (operation.Kind != "backup" || operation.Id == Guid.Empty || operation.ExportPath == null || !Path.IsPathFullyQualified(operation.ExportPath) || operation.App != null) throw new InvalidDataException("Invalid backup recovery record.");
        string path = Path.GetFullPath(operation.ExportPath);
        if (SafeFiles.IsWithin(path, DirectoryPath) || path.Equals(DirectoryPath, StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("Backup recovery cannot point into the library.");
        string staging = path + ".partial-" + operation.Id.ToString("N"); SafeFiles.NoLinks(staging); return staging;
    }
    public void CheckExportMarker(PendingOperation operation)
    {
        string marker = Path.Combine(ExportStaging(operation), ".BoxedwineExport"); SafeFiles.NoLinks(marker);
        if (!File.Exists(marker) || new FileInfo(marker).Length > 128 || File.ReadAllText(marker) != operation.Id.ToString("D")) throw new InvalidDataException("This folder cannot be matched to the interrupted backup. Its files have been kept.");
    }
    public void MarkExportReady(PendingOperation operation) { CheckExportMarker(operation); operation.Ready = true; SaveOperation(operation); }
    public void ForgetExport(PendingOperation operation) { _ = ExportStaging(operation); File.Delete(OperationPath(operation.Id)); }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Commit(PendingOperation operation)
    {
        if (operation.App == null || operation.App.Id != operation.Id) throw new InvalidDataException("Invalid operation identity.");
        ValidateApp(operation.App);
        operation.Entries = Backups.Inventory(AppDirectory(operation.App), default, null).GetAwaiter().GetResult();
        operation.Ready = true;
        SaveOperation(operation);
        Finish(operation);
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Finish(PendingOperation operation)
    {
        if (!operation.Ready || operation.App == null || operation.Id != operation.App.Id) throw new InvalidDataException("This partial copy cannot be added. Inspect its files or discard it.");
        ValidateApp(operation.App);
        var document = Load();
        if (!document.Apps.Concat(document.RemovedApps.Select(a => a.App)).Any(a => a.Id == operation.Id))
        {
            if (!Directory.Exists(Root(operation.App))) throw new InvalidDataException("The copied Windows files are missing.");
            if (operation.Entries == null || !Backups.Inventory(AppDirectory(operation.App), default, null).GetAwaiter().GetResult().SequenceEqual(operation.Entries)) throw new InvalidDataException("The completed copy has changed. Its files have been kept for review.");
            document.Apps.Add(operation.App); Save(document);
        }
        File.Delete(OperationPath(operation.Id));
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Discard(PendingOperation operation)
    {
        if (operation.Problem != null) throw new InvalidDataException(operation.Problem);
        if (operation.Kind == "backup")
        {
            string staging = ExportStaging(operation);
            if (Directory.Exists(staging))
            {
                CheckExportMarker(operation);
                if (File.Exists(Path.Combine(staging, "Manifest.json"))) throw new InvalidDataException("This backup may be complete. Its files are kept; finish the backup or inspect its folder.");
                SafeFiles.DeleteTree(staging, Path.GetDirectoryName(staging)!);
            }
            ForgetExport(operation); return;
        }
        if (Load().Apps.Concat(Load().RemovedApps.Select(x => x.App)).Any(a => a.Id == operation.Id)) { File.Delete(OperationPath(operation.Id)); return; }
        if (operation.App != null && operation.Id == operation.App.Id) SafeFiles.DeleteTree(AppDirectory(operation.App), Path.Combine(DirectoryPath, "Applications"));
        File.Delete(OperationPath(operation.Id));
    }
    public List<PendingOperation> Pending()
    {
        string directory = Path.Combine(DirectoryPath, "WindowsOperations");
        if (!Directory.Exists(directory)) return [];
        SafeFiles.NoLinks(directory);
        List<PendingOperation> result = [];
        foreach (var path in Directory.EnumerateFiles(directory, "*.json"))
        {
            Guid.TryParse(Path.GetFileNameWithoutExtension(path), out var id);
            try
            {
                var operation = DataFormat.Read<PendingOperation>(path);
                if (id == Guid.Empty || operation.Id != id) throw new InvalidDataException("Invalid recovery identity.");
                if (operation.Kind == "backup") _ = ExportStaging(operation);
                else if (operation.App == null || operation.App.Id != id) throw new InvalidDataException("Invalid app recovery record.");
                result.Add(operation);
            }
            catch (Exception error) when (error is IOException or System.Text.Json.JsonException or ArgumentException)
            { result.Add(new() { Id = id, Name = "Unreadable recovery record", Problem = error.Message }); }
        }
        return result.OrderBy(p => p.CreatedAt).ToList();
    }
    public async Task<LibraryApp> Import(string source, string kind, string windows, WineReference wine, CancellationToken cancellation, IProgress<OperationProgress>? progress = null, string? installer = null)
    {
        source = Path.GetFullPath(source);
        if (SafeFiles.IsWithin(DirectoryPath, source) || SafeFiles.IsWithin(source, DirectoryPath) || source.Equals(DirectoryPath, StringComparison.OrdinalIgnoreCase)) throw new IOException("Choose a folder outside the Boxedwine library.");
        var app = new LibraryApp { Name = kind == "installer" ? Path.GetFileNameWithoutExtension(source) : Path.GetFileName(Path.TrimEndingDirectorySeparator(source)), WinePackage = wine, SavedWineVersion = wine.WineVersion };
        app.ChooseWindows(windows);
        var operation = Begin(app, kind);
        try
        {
            if (kind == "folder")
            {
                await SafeFiles.CopyTree(source, SafeFiles.Beneath(Root(app), DriveC + "/App"), cancellation, progress);
                var programs = Programs(app);
                if (programs.Count == 0) throw new IOException("This folder contains no Windows programs (.exe).");
                var primary = programs.Where(p => !p.IsMaintenance).ToList();
                if (primary.Count == 1) app.Executable = primary[0].Path;
            }
            else if (kind == "installerFolder")
            {
                if (installer == null || !SafeFiles.IsWithin(installer, source)) throw new IOException("Choose an installer inside the selected folder.");
                CheckInstaller(installer);
                app.Installer = "Installer/" + Path.GetRelativePath(source, installer).Replace('\\', '/');
                await SafeFiles.CopyTree(source, SafeFiles.Beneath(AppDirectory(app), "Installer"), cancellation, progress);
            }
            else
            {
                CheckInstaller(source);
                app.Installer = "Installer/" + Path.GetFileName(source);
                await SafeFiles.CopyFile(source, SafeFiles.Beneath(AppDirectory(app), app.Installer), cancellation, progress);
            }
            cancellation.ThrowIfCancellationRequested();
            Commit(operation);
            return app;
        }
        catch { if (!operation.Ready) Discard(operation); throw; }
    }
    public LibraryApp AddBuiltIn(string program, WineReference wine)
    {
        var app = new LibraryApp { Name = program == "notepad" ? "Notepad" : "Minesweeper", BuiltInProgram = program, WinePackage = wine, SavedWineVersion = wine.WineVersion };
        var operation = Begin(app, "builtin");
        Commit(operation);
        return app;
    }
    public static void CheckInstaller(string path)
    {
        SafeFiles.NoLinks(path);
        if (!File.Exists(path) || !new[] { ".exe", ".msi" }.Contains(Path.GetExtension(path).ToLowerInvariant())) throw new IOException("Choose a Windows .exe or .msi installer.");
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Remove(Guid id)
    {
        var document = Load();
        var app = document.Apps.Single(a => a.Id == id);
        document.Apps.Remove(app); document.RemovedApps.Insert(0, new() { App = app }); Save(document);
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Restore(Guid id)
    {
        var document = Load();
        var removed = document.RemovedApps.Single(a => a.App.Id == id);
        if (removed.DeletionStartedAt != null) throw new IOException("Deletion has already started. Finish deleting this app.");
        document.RemovedApps.Remove(removed); document.Apps.Add(removed.App); Save(document);
    }
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Delete(Guid id)
    {
        var document = Load();
        if (document.Apps.Any(a => a.Id == id)) { Remove(id); document = Load(); }
        var removed = document.RemovedApps.Single(a => a.App.Id == id);
        removed.DeletionStartedAt = DataFormat.Now; Save(document);
        SafeFiles.DeleteTree(AppDirectory(removed.App), Path.Combine(DirectoryPath, "Applications"));
        document.RemovedApps.Remove(removed); Save(document);
    }
    public async Task<LibraryApp> Trial(LibraryApp original, string name, WineReference wine, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        var app = DataFormat.Clone(original); app.Id = Guid.NewGuid(); app.Name = name; app.CreatedAt = DataFormat.Now; app.LastOpened = null;
        app.WinePackage = wine; app.SavedWineVersion = wine.WineVersion;
        app.WindowsVersion = original.PreferredWindows; app.WindowsVersionPending = true;
        app.OpenGLBackend = original.PreferredBackend; app.OpenGLBackendPending = true;
        app.WineRenderer = original.PreferredRenderer; app.WineRendererPending = true;
        var operation = Begin(app, "trial");
        try
        {
            var before = await Backups.Inventory(AppDirectory(original), cancellation, progress);
            await SafeFiles.CopyTree(AppDirectory(original), AppDirectory(app), cancellation, progress);
            if (!(await Backups.Inventory(AppDirectory(app), cancellation, progress)).SequenceEqual(before)) throw new IOException("The original app changed during the copy.");
            // A trial uses its new shared package and has not launched yet.
            string oldWine = SafeFiles.Beneath(AppDirectory(app), "WindowsSupport/wine.zip"); if (File.Exists(oldWine)) File.Delete(oldWine);
            string logs = SafeFiles.Beneath(AppDirectory(app), "Logs"); if (Directory.Exists(logs)) SafeFiles.DeleteTree(logs, AppDirectory(app));
            cancellation.ThrowIfCancellationRequested(); Commit(operation); return app;
        }
        catch { if (!operation.Ready) Discard(operation); throw; }
    }
    public void PruneWine()
    {
        var document = Load();
        var pending = Pending();
        if (pending.Any(p => p.Problem != null)) throw new InvalidDataException("Inspect unreadable records in Unfinished Work before cleaning Wine packages.");
        var referenced = document.Apps.Concat(document.RemovedApps.Select(a => a.App)).Select(a => a.WinePackage?.Sha256)
            .Concat(pending.Select(p => p.App?.WinePackage?.Sha256)).Append(DefaultWine?.Sha256).Where(s => s != null).ToHashSet();
        string directory = Path.Combine(DirectoryPath, "WinePackages");
        if (!Directory.Exists(directory)) return;
        SafeFiles.NoLinks(directory);
        foreach (var path in Directory.EnumerateFiles(directory, "*.zip"))
            if (Regex.IsMatch(Path.GetFileNameWithoutExtension(path), "^[a-f0-9]{64}$") && !referenced.Contains(Path.GetFileNameWithoutExtension(path))) { SafeFiles.NoLinks(path); File.Delete(path); }
    }
}
