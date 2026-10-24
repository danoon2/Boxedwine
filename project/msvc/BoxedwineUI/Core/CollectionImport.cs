// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace Boxedwine.Library;

/// <summary>Optional PNG artwork, cached by its complete URL for offline use.</summary>
public sealed class CollectionIcons(string directory)
{
    public static bool IsRemote(string value) => !value.Any(c => char.IsWhiteSpace(c) || char.IsControl(c)) && Uri.TryCreate(value, UriKind.Absolute, out var url) && PreparedAppSource.ValidUrl(url);
    public static bool ValidPng(byte[] data) => data.Length is >= 24 and <= 1048576 &&
        data.AsSpan(0, 16).SequenceEqual(new byte[] { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82 }) &&
        System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(16, 4)) is > 0 and <= 1024 &&
        System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(20, 4)) is > 0 and <= 1024;
    private string Pathname(string value) => SafeFiles.Beneath(directory, "CollectionIcons/" + Convert.ToHexStringLower(SHA256.HashData(Encoding.UTF8.GetBytes(value))) + ".png");
    public string? CachedPath(string value)
    {
        if (!IsRemote(value)) return null;
        try
        {
            string path = Pathname(value);
            return File.Exists(path) && new FileInfo(path).Length <= 1048576 && ValidPng(File.ReadAllBytes(path)) ? path : null;
        }
        catch (Exception error) when (error is IOException or UnauthorizedAccessException) { return null; }
    }
    public async Task Fetch(string value)
    {
        if (!IsRemote(value) || CachedPath(value) != null) return;
        string? temporary = null;
        try
        {
            string target = Pathname(value);
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            temporary = target + "." + Guid.NewGuid().ToString("N") + ".tmp";
            await PreparedAppSource.Download(value, temporary, 1048576, default, null, TimeSpan.FromSeconds(20));
            if (ValidPng(File.ReadAllBytes(temporary))) File.Move(temporary, target, true);
        }
        catch (Exception error) when (error is IOException or UnauthorizedAccessException or HttpRequestException or OperationCanceledException) { /* Use the normal fallback. */ }
        finally
        {
            try { if (temporary != null && File.Exists(temporary)) File.Delete(temporary); }
            catch (Exception error) when (error is IOException or UnauthorizedAccessException) { }
        }
    }
}

public sealed class CollectionSnapshot
{
    public int SchemaVersion { get; set; }
    public string Folder { get; set; } = "";
    public List<Dictionary<string, string>> Apps { get; set; } = [];
    // One literal argument per line, matching App Settings; no shell parsing.
    public static List<string> ParseAppArguments(string? value)
    {
        if (string.IsNullOrEmpty(value)) return [];
        var arguments = value.Replace("\r\n", "\n").Split('\n').ToList();
        if (arguments.Count > 256 || arguments.Sum(a => Encoding.UTF8.GetByteCount(a)) > 65536 ||
            arguments.Any(a => Encoding.UTF8.GetByteCount(a) > 8192 || a.IndexOfAny(['\0', '\r']) >= 0))
            throw new InvalidDataException("AppArguments needs at most 256 arguments, one per line, with at most 8192 bytes each and 65536 bytes total, without NUL or bare carriage returns.");
        return arguments;
    }
    public static string FolderName(string value)
    {
        string name = value.Trim();
        if (name.Length is 0 or > 80 || name.Any(char.IsControl) || name.Contains('/') || name.Contains('\\') ||
            new[] { "all apps", "recently opened", "removed apps", "recovery", "unfinished work" }.Contains(name.ToLowerInvariant()))
            throw new InvalidDataException("Use a collection folder name of 1–80 characters. Library view names and slashes are reserved.");
        return name.Equals("Demos", StringComparison.OrdinalIgnoreCase) ? "Demos" : name;
    }
    public static CollectionSnapshot Load(string path)
    {
        var value = DataFormat.Read<JsonElement>(path, 1024 * 1024);
        if (value.ValueKind != JsonValueKind.Object || !value.EnumerateObject().Select(p => p.Name).Order().SequenceEqual(new[] { "apps", "folder", "schemaVersion" }))
            throw new InvalidDataException("Use a collection JSON file with schemaVersion, folder, and apps.");
        var snapshot = value.Deserialize<CollectionSnapshot>(DataFormat.Json) ?? throw new InvalidDataException("Empty collection.");
        snapshot.Folder = FolderName(snapshot.Folder);
        _ = snapshot.Recipes();
        return snapshot;
    }
    public List<DemoRecipe> Recipes()
    {
        if (SchemaVersion != 1 || Apps.Count is < 1 or > 256) throw new InvalidDataException("A version 1 collection needs 1–256 apps.");
        string[] allowed = ("ID Name Summary Help Icon FileURL FileSizeBytes FileSHA256 InstallType InstallExe ShortcutExe WineVersion WindowsVersion GDIRenderer Resolution InstallResolution BitsPerPixel CPUCount NativeOpenGL UseEGL CNCDDraw DisableHideCursor ForceRelativeMouse Glide CNCDDrawRenderer CNCDDrawUncapped CNCDDrawFakeMode WineDllOverrides AppArguments").Split(' ');
        if (Apps.Any(a => a == null || !a.TryGetValue("ID", out string? id) || !Regex.IsMatch(id, "^[a-z0-9][a-z0-9-]{0,63}$") || a.Any(p => !allowed.Contains(p.Key) || p.Value == null))) throw new InvalidDataException("Unsupported collection app fields.");
        foreach (var app in Apps)
        {
            string Field(string key) => app.GetValueOrDefault(key, "").Trim();
            string icon = Field("Icon"), installer = Field("InstallExe");
            if (!Uri.TryCreate(Field("FileURL"), UriKind.Absolute, out var address)) throw new InvalidDataException("Invalid collection download URL.");
            string extension = Path.GetExtension(address.LocalPath).ToLowerInvariant();
            bool needsSetup = Field("InstallType") == "Installer" && extension == ".zip";
            if (app.ContainsKey("WineVersion") && (Field("WineVersion").Length == 0 || Encoding.UTF8.GetByteCount(Field("WineVersion")) > 128) || !new[] { ".zip", ".exe", ".msi" }.Contains(extension) ||
                icon.Length > 0 && !CollectionIcons.IsRemote(icon) && (icon.Contains('/') || icon.Contains('\\') || !icon.EndsWith(".png", StringComparison.Ordinal)) ||
                needsSetup && !new[] { ".exe", ".msi" }.Contains(Path.GetExtension(installer).ToLowerInvariant()) || !needsSetup && installer.Length > 0)
                throw new InvalidDataException("Invalid collection Wine version, icon, or installation recipe.");
        }
        var root = new XElement("XML", new XAttribute("schemaVersion", "8"), new XAttribute("release", "custom-collection"),
            Apps.Select(app => new XElement("Demo", app.Where(p => p.Key != "AppArguments").Select(p => new XElement(p.Key, p.Value)))));
        return Demos.Parse(new XDocument(root), Path.GetTempPath(), true)
            .Select((recipe, index) => recipe with { AppArguments = ParseAppArguments(Apps[index].GetValueOrDefault("AppArguments")) }).ToList();
    }
}
public sealed class CollectionFolder
{
    public string Name { get; set; } = "";
    public List<Dictionary<string, string>> Apps { get; set; } = [];
    public List<string> Sources { get; set; } = [];
}
public sealed class CollectionDocument
{
    public int Version { get; set; } = 1;
    public List<CollectionFolder> Folders { get; set; } = [];
}
public sealed record CollectionChanges(int Added, int Updated, int Unchanged)
{
    public override string ToString() => $"{Added} new · {Updated} updated · {Unchanged} unchanged";
    public string RefreshDescription => $"{Added} new {(Added == 1 ? "app" : "apps")} · {Updated} updated {(Updated == 1 ? "listing" : "listings")} · {Unchanged} unchanged";
}
public sealed record CollectionRefresh(CollectionFolder Before, CollectionSnapshot Snapshot, CollectionChanges Changes);
public sealed class CollectionStore(string directory, IEnumerable<string>? defaultIDs = null)
{
    private string Pathname => Path.Combine(directory, "collections.json");
    public CollectionDocument Load()
    {
        var document = File.Exists(Pathname) ? DataFormat.Read<CollectionDocument>(Pathname) : new();
        if (document.Version != 1 || document.Folders.Count > 64) throw new InvalidDataException("Unsupported saved collections. The file was kept.");
        var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var folder in document.Folders)
        {
            if (CollectionSnapshot.FolderName(folder.Name) != folder.Name || !names.Add(folder.Name)) throw new InvalidDataException("Duplicate or invalid collection folder.");
            _ = new CollectionSnapshot { SchemaVersion = 1, Folder = folder.Name, Apps = folder.Apps }.Recipes();
            ValidateSources(folder.Sources);
        }
        return document;
    }
    public static void ValidateSources(List<string> sources)
    {
        if (sources == null || sources.Count > 32 || sources.Distinct(StringComparer.Ordinal).Count() != sources.Count ||
            sources.Any(url => url == null || url.Length > 8192 || !CollectionIcons.IsRemote(url)))
            throw new InvalidDataException("A collection can remember up to 32 unique HTTP or HTTPS source URLs.");
    }
    public CollectionChanges Preview(CollectionSnapshot snapshot, string destination)
    {
        string name = CollectionSnapshot.FolderName(destination);
        var previous = Load().Folders.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase))?.Apps ?? [];
        return Changes(snapshot, name, previous);
    }
    private CollectionChanges Changes(CollectionSnapshot snapshot, string name, List<Dictionary<string, string>> previousApps)
    {
        var old = previousApps.ToDictionary(a => a["ID"]);
        int added = 0, updated = 0, unchanged = 0;
        foreach (var app in snapshot.Apps)
        {
            if (!old.TryGetValue(app["ID"], out var previous)) { if (name == "Demos" && (defaultIDs?.Contains(app["ID"]) ?? false)) updated++; else added++; }
            else if (previous.Count == app.Count && previous.All(p => app.TryGetValue(p.Key, out string? v) && v == p.Value)) unchanged++;
            else updated++;
        }
        return new(added, updated, unchanged);
    }
    public string Merge(CollectionSnapshot snapshot, string destination, string? source = null, CollectionFolder? expected = null)
    {
        _ = snapshot.Recipes(); string name = CollectionSnapshot.FolderName(destination);
        var document = Load();
        var folder = document.Folders.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        if (expected != null && JsonSerializer.Serialize(folder, DataFormat.Json) != JsonSerializer.Serialize(expected, DataFormat.Json))
            throw new InvalidDataException("This collection changed while refreshing. Refresh again to review the latest changes.");
        if (folder == null)
        {
            if (document.Folders.Count >= 64) throw new InvalidDataException("A library can have up to 64 collection folders.");
            folder = new() { Name = name }; document.Folders.Add(folder);
        }
        if (source != null && !folder.Sources.Contains(source)) folder.Sources.Add(source);
        ValidateSources(folder.Sources);
        foreach (var app in snapshot.Apps)
        {
            int index = folder.Apps.FindIndex(a => a["ID"] == app["ID"]);
            if (index >= 0) folder.Apps[index] = app; else folder.Apps.Add(app);
        }
        if (folder.Apps.Count > 256) throw new InvalidDataException("A folder can contain up to 256 custom listings.");
        SafeFiles.AtomicJson(Pathname, document);
        return folder.Name;
    }
    public CollectionRefresh ReviewRefresh(CollectionFolder before, List<CollectionSnapshot> snapshots)
    {
        if (before.Sources.Count == 0 || snapshots.Count != before.Sources.Count) throw new InvalidDataException("Every saved collection source must be downloaded before applying changes.");
        var apps = new Dictionary<string, Dictionary<string, string>>();
        foreach (var snapshot in snapshots)
        {
            _ = snapshot.Recipes();
            foreach (var app in snapshot.Apps) apps[app["ID"]] = app;
        }
        var incoming = new CollectionSnapshot { SchemaVersion = 1, Folder = before.Name, Apps = apps.Values.ToList() };
        _ = incoming.Recipes();
        if (before.Apps.Select(a => a["ID"]).Union(apps.Keys).Count() > 256) throw new InvalidDataException("A folder can contain up to 256 custom listings.");
        return new(before, incoming, Changes(incoming, before.Name, before.Apps));
    }
    public string ApplyRefresh(CollectionRefresh review) => Merge(review.Snapshot, review.Before.Name, expected: review.Before);
    public async Task<CollectionRefresh> Refresh(string name, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        var before = Load().Folders.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        if (before == null || before.Sources.Count == 0) throw new InvalidDataException("This collection has no saved source URLs. Import it from a URL first.");
        var snapshots = new List<CollectionSnapshot>();
        foreach (string source in before.Sources)
        {
            cancellation.ThrowIfCancellationRequested();
            using var prepared = await PreparedAppSource.Prepare(source, true, true, cancellation, progress);
            snapshots.Add(prepared.Collection ?? throw new InvalidDataException("The source did not contain a collection."));
        }
        cancellation.ThrowIfCancellationRequested();
        return ReviewRefresh(before, snapshots);
    }
    public void Remove(string name)
    {
        var document = Load(); document.Folders.RemoveAll(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        SafeFiles.AtomicJson(Pathname, document);
    }
    public List<DemoRecipe> Recipes(string name, List<DemoRecipe> defaults, CollectionDocument? document = null)
    {
        var result = name == "Demos" ? defaults.ToList() : [];
        var folder = (document ?? Load()).Folders.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        if (folder != null)
        {
            foreach (var entry in new CollectionSnapshot { SchemaVersion = 1, Folder = folder.Name, Apps = folder.Apps }.Recipes())
            {
                string id = name == "Demos" ? entry.Origin.Id : "collection-" + Convert.ToHexStringLower(SHA256.HashData(Encoding.UTF8.GetBytes(folder.Name.ToLowerInvariant() + "\0" + entry.Origin.Id)))[..48];
                var demo = entry with { Origin = entry.Origin with { Id = id } };
                int index = result.FindIndex(d => d.Origin.Id == demo.Origin.Id);
                if (index >= 0) result[index] = demo; else result.Add(demo);
            }
        }
        return result;
    }
}
public sealed class PreparedAppSource : IDisposable
{
    public string Original { get; private set; } = "";
    public string Source { get; private set; } = "";
    public string Name { get; private set; } = "";
    public bool IsDirectory => Directory.Exists(Source);
    public List<string>? DiscImages { get; private set; }
    public List<string> Programs { get; } = [];
    public CollectionSnapshot? Collection { get; private set; }
    private readonly string temporary = Path.Combine(Path.GetTempPath(), "BoxedwineImport-" + Guid.NewGuid().ToString("N"));
    public static PreparedAppSource PrepareDiscs(IEnumerable<string> paths)
    {
        var images = CdromImages.PrepareImport(paths);
        return new PreparedAppSource { Original = images[0], Source = images[0], Name = Path.GetFileNameWithoutExtension(images[0]), DiscImages = images };
    }
    public static bool ValidUrl(Uri url) => url.IsAbsoluteUri && url.Scheme is "http" or "https" && url.Host.Length > 0 && url.UserInfo.Length == 0 && url.Fragment.Length == 0 && url.Port != 0;
    public static async Task Download(string url, string destination, long limit, CancellationToken cancellation, IProgress<OperationProgress>? progress, TimeSpan? timeLimit = null)
    {
        Uri address = new(url); string scheme = address.Scheme;
        if (!ValidUrl(address)) throw new InvalidDataException("Use a complete HTTP or HTTPS file link without a login or fragment.");
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellation); timeout.CancelAfter(timeLimit ?? TimeSpan.FromMinutes(30));
        using var handler = new HttpClientHandler { AllowAutoRedirect = false, UseCookies = false, UseDefaultCredentials = false };
        using var client = new HttpClient(handler) { Timeout = TimeSpan.FromMinutes(30) };
        try
        {
            for (int redirects = 0; ; redirects++)
            {
                using var response = await client.GetAsync(address, HttpCompletionOption.ResponseHeadersRead, timeout.Token);
                if ((int)response.StatusCode is >= 300 and < 400)
                {
                    var location = response.Headers.Location;
                    if (location == null || redirects >= 5) throw new IOException("The download redirected too many times.");
                    address = location.IsAbsoluteUri ? location : new Uri(address, location);
                    if (!ValidUrl(address) || scheme == "https" && address.Scheme != "https") throw new IOException("The download redirected to an unsupported location.");
                    continue;
                }
                if (response.StatusCode != System.Net.HttpStatusCode.OK) throw new IOException("The server did not return a file.");
                long total = response.Content.Headers.ContentLength ?? 0;
                if (total < 0 || total > limit) throw new IOException("The download is too large.");
                await using var input = await response.Content.ReadAsStreamAsync(timeout.Token);
                await using var output = new FileStream(destination, FileMode.CreateNew, FileAccess.Write, FileShare.None, 128 * 1024, true);
                byte[] buffer = new byte[128 * 1024]; long copied = 0; int count;
                while ((count = await input.ReadAsync(buffer, timeout.Token)) != 0)
                {
                    copied += count;
                    if (copied > limit) throw new IOException("The download is too large.");
                    await output.WriteAsync(buffer.AsMemory(0, count), timeout.Token);
                    progress?.Report(new("Downloading " + address.Host, copied, total));
                }
                if (copied == 0 || total != 0 && copied != total) throw new IOException("The file is empty or incomplete.");
                await output.FlushAsync(timeout.Token); output.Flush(true); break;
            }
        }
        catch { if (File.Exists(destination)) File.Delete(destination); throw; }
    }
    public static async Task<PreparedAppSource> Prepare(string source, bool remote, bool collectionOnly, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        cancellation.ThrowIfCancellationRequested();
        var result = new PreparedAppSource { Original = source };
        Directory.CreateDirectory(result.temporary);
        try
        {
            if (remote)
            {
                var uri = new Uri(source);
                string extension = Path.GetExtension(uri.LocalPath).ToLowerInvariant();
                if (!new[] { ".zip", ".exe", ".msi", ".json" }.Contains(extension)) throw new InvalidDataException("Use a direct link to a ZIP, EXE, MSI, or collection JSON file.");
                result.Source = Path.Combine(result.temporary, "Download" + extension);
                result.Name = Path.GetFileNameWithoutExtension(uri.LocalPath);
                await Download(source, result.Source, collectionOnly ? 1024 * 1024 : 1024L * 1024 * 1024, cancellation, progress);
            }
            else { SafeFiles.NoLinks(source); result.Source = source; result.Name = Path.GetFileNameWithoutExtension(source); }
            if (!result.IsDirectory && Path.GetExtension(result.Source).Equals(".json", StringComparison.OrdinalIgnoreCase)) result.Collection = CollectionSnapshot.Load(result.Source);
            if (collectionOnly && result.Collection == null) throw new InvalidDataException("Choose a Boxedwine collection JSON file.");
            if (!result.IsDirectory && Path.GetExtension(result.Source).Equals(".zip", StringComparison.OrdinalIgnoreCase))
            {
                string contents = Path.Combine(result.temporary, "Contents");
                await Packages.Extract(result.Source, contents, cancellation, progress); result.Source = contents;
            }
            if (!remote && !collectionOnly && !result.IsDirectory && CdromImages.IsImage(result.Source))
            {
                result.DiscImages = CdromImages.PrepareImport([result.Source]);
                return result;
            }
            if (result.Collection == null)
            {
                if (result.IsDirectory)
                {
                    int count = 0;
                    foreach (string file in SafeFiles.Tree(result.Source))
                    {
                        cancellation.ThrowIfCancellationRequested();
                        if (++count > 100000 || result.Programs.Count > 4096) throw new IOException("The folder contains too many entries or programs.");
                        if (File.Exists(file) && new[] { ".exe", ".msi" }.Contains(Path.GetExtension(file).ToLowerInvariant())) result.Programs.Add(Path.GetRelativePath(result.Source, file).Replace('\\', '/'));
                    }
                }
                else if (new[] { ".exe", ".msi" }.Contains(Path.GetExtension(result.Source).ToLowerInvariant())) result.Programs.Add(Path.GetFileName(result.Source));
                if (result.Programs.Count == 0) throw new IOException("No Windows program was found. Choose an app folder, ZIP, EXE, MSI, ISO, or CUE.");
                result.Programs.Sort(StringComparer.OrdinalIgnoreCase);
            }
            cancellation.ThrowIfCancellationRequested();
            return result;
        }
        catch { result.Dispose(); throw; }
    }
    public string SuggestedAction
    {
        get
        {
            var setup = Programs.Where(p => p.EndsWith(".msi", StringComparison.OrdinalIgnoreCase) || Regex.IsMatch(Path.GetFileName(p), "^(setup|install)", RegexOptions.IgnoreCase)).ToList();
            var apps = Programs.Where(p => p.EndsWith(".exe", StringComparison.OrdinalIgnoreCase) && !new ProgramCandidate(p).IsMaintenance).ToList();
            return setup.Count > 0 && apps.Count == 0 ? "installer" : apps.Count > 0 && setup.Count == 0 && IsDirectory ? "app" : "";
        }
    }
    public void Dispose() { SafeFiles.DeleteTree(temporary, Path.GetTempPath()); }
}
