// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public sealed record BackupEntry(string Path, string Kind, long Size = 0, string? Digest = null, string? Target = null);
public sealed class BackupManifest
{
    public int Format { get; set; } = 8;
    public double CreatedAt { get; set; } = DataFormat.Now;
    public LibraryApp App { get; set; } = new();
    public string WineVersion { get; set; } = "";
    public List<BackupEntry> Entries { get; set; } = [];
}
public static class Backups
{
    public static async Task<List<BackupEntry>> Inventory(string root, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        List<BackupEntry> entries = [];
        foreach (var path in SafeFiles.Tree(root))
        {
            cancellation.ThrowIfCancellationRequested();
            string relative = Path.GetRelativePath(root, path).Replace('\\', '/');
            progress?.Report(new("Checking " + relative));
            entries.Add(Directory.Exists(path) ? new(relative, "directory") : new(relative, "file", new FileInfo(path).Length, await SafeFiles.Hash(path, cancellation).ConfigureAwait(false)));
            if (entries.Count > 100000) throw new InvalidDataException("This app has too many files for a backup.");
        }
        return entries.OrderBy(e => e.Path, StringComparer.Ordinal).ToList();
    }
    public static async Task Export(LibraryRepository repository, LibraryApp app, string destination, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        destination = Path.GetFullPath(destination); SafeFiles.NoLinks(destination);
        if (Directory.Exists(destination) || File.Exists(destination)) throw new IOException("Choose a new backup name. An existing backup is never overwritten.");
        if (SafeFiles.IsWithin(destination, repository.DirectoryPath)) throw new IOException("Save backups outside the Boxedwine library.");
        repository.ValidateApp(app);
        var operation = repository.BeginExport(app.Name, destination);
        string staging = repository.ExportStaging(operation);
        try
        {
            var original = await Inventory(repository.AppDirectory(app), cancellation, progress);
            string application = Path.Combine(staging, "Application");
            await SafeFiles.CopyTree(repository.AppDirectory(app), application, cancellation, progress);
            if (!(await Inventory(application, cancellation, progress)).SequenceEqual(original)) throw new IOException("The app changed while its backup was copied.");
            string wine = repository.WinePath(app), snapshot = SafeFiles.Beneath(application, "WindowsSupport/wine.zip");
            if (!File.Exists(snapshot)) await SafeFiles.CopyFile(wine, snapshot, cancellation, progress);
            else if (app.WinePackage != null || app.SavedWineVersion == null) { File.Delete(snapshot); await SafeFiles.CopyFile(wine, snapshot, cancellation, progress); }
            var package = await Packages.ValidateWine(snapshot, cancellation, progress);
            if (app.WinePackage != null && package != app.WinePackage) throw new IOException("The Wine package changed while backing up.");
            var saved = DataFormat.Clone(app); saved.WinePackage = null; saved.SavedWineVersion = package.WineVersion;
            var manifest = new BackupManifest { App = saved, WineVersion = package.WineVersion, Entries = await Inventory(application, cancellation, progress) };
            cancellation.ThrowIfCancellationRequested(); SafeFiles.AtomicJson(Path.Combine(staging, "Manifest.json"), manifest);
            repository.MarkExportReady(operation);
            await FinishExport(repository, operation, cancellation, progress);
        }
        catch { if (!operation.Ready) repository.Discard(operation); throw; }
    }
    public static async Task FinishExport(LibraryRepository repository, PendingOperation operation, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        if (!operation.Ready) throw new InvalidDataException("This backup did not finish copying.");
        string staging = repository.ExportStaging(operation);
        if (!Directory.Exists(staging) && Directory.Exists(operation.ExportPath)) { repository.ForgetExport(operation); return; }
        repository.CheckExportMarker(operation);
        var manifest = DataFormat.Read<BackupManifest>(Path.Combine(staging, "Manifest.json"));
        if (!(await Inventory(Path.Combine(staging, "Application"), cancellation, progress)).SequenceEqual(manifest.Entries.OrderBy(e => e.Path, StringComparer.Ordinal))) throw new InvalidDataException("The backup changed before it could be completed. Its files have been kept.");
        cancellation.ThrowIfCancellationRequested();
        Directory.Move(staging, operation.ExportPath!);
        File.Delete(Path.Combine(operation.ExportPath!, ".BoxedwineExport"));
        repository.ForgetExport(operation);
    }
    public static async Task<LibraryApp> Restore(LibraryRepository repository, string source, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        var manifest = DataFormat.Read<BackupManifest>(Path.Combine(source, "Manifest.json"));
        if (manifest.Format is < 1 or > 8 || manifest.Entries.Count > 100000 || manifest.Entries.Any(e => e.Kind is not ("file" or "directory"))) throw new InvalidDataException("Unsupported backup format or native filesystem links. Boxedwine guest .link files are supported.");
        string application = Path.Combine(source, "Application");
        var expected = manifest.Entries.OrderBy(e => e.Path, StringComparer.Ordinal).ToList();
        if (!(await Inventory(application, cancellation, progress)).SequenceEqual(expected)) throw new InvalidDataException("The backup files do not match their contents list. Nothing has been restored.");
        var app = DataFormat.Clone(manifest.App); app.Id = Guid.NewGuid(); app.Name += " (restored)"; app.CreatedAt = DataFormat.Now; app.LastOpened = null; app.WinePackage = null; app.SavedWineVersion = manifest.WineVersion;
        repository.ValidateApp(app);
        var operation = repository.Begin(app, "restore");
        try
        {
            await SafeFiles.CopyTree(application, repository.AppDirectory(app), cancellation, progress);
            if (!(await Inventory(repository.AppDirectory(app), cancellation, progress)).SequenceEqual(expected)) throw new InvalidDataException("The restored files failed verification.");
            var wine = await Packages.Import(repository, SafeFiles.Beneath(repository.AppDirectory(app), "WindowsSupport/wine.zip"), cancellation, progress);
            if (wine.WineVersion != manifest.WineVersion) throw new InvalidDataException("The backup contains a different Wine version.");
            app.WinePackage = wine;
            File.Delete(SafeFiles.Beneath(repository.AppDirectory(app), "WindowsSupport/wine.zip"));
            cancellation.ThrowIfCancellationRequested(); repository.Commit(operation); return app;
        }
        catch { if (!operation.Ready) repository.Discard(operation); throw; }
    }
}
