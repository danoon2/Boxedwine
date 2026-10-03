// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO.Compression;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace Boxedwine.Library;

public sealed record CatalogWine(string Name, string WineVersion, string Url, long Bytes, string Sha256, string FilesystemVersion)
{
    public string DisplayName => $"{Name} V{FilesystemVersion}";
    public override string ToString() => $"{Name} · {Bytes / 1048576.0:0} MB";
    public WineReference Reference => new(Sha256, Bytes, WineVersion, FilesystemVersion);
}
// One check per unchanged package in this launcher session, shared by all apps.
public sealed class WineLaunchChecks
{
    private readonly Dictionary<string, (long Size, DateTime Stamp, WineReference Reference)> verified = new(StringComparer.OrdinalIgnoreCase);
    public void Clear() => verified.Clear();
    public async Task<WineReference> Verify(string path, WineReference? expected, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        cancellation.ThrowIfCancellationRequested();
        path = Path.GetFullPath(path); SafeFiles.NoLinks(path);
        var info = new FileInfo(path);
        long size = info.Length; DateTime stamp = info.LastWriteTimeUtc;
        if (verified.TryGetValue(path, out var cached) && size == cached.Size && stamp == cached.Stamp &&
            (expected == null || expected == cached.Reference)) return cached.Reference;
        WineReference reference;
        if (expected != null)
        {
            await Packages.VerifyWineChecksum(path, expected, cancellation, progress);
            reference = expected;
        }
        else
        {
            // Older per-app Wine snapshots have no stored checksum to compare.
            reference = await Packages.ValidateWine(path, cancellation, progress);
        }
        verified[path] = (size, stamp, reference);
        return reference;
    }
}
public static class Packages
{
    private static readonly HttpClient Client = new(new HttpClientHandler { AllowAutoRedirect = false }) { Timeout = Timeout.InfiniteTimeSpan };
    public static List<CatalogWine> Catalog(string resources)
    {
        var pins = DataFormat.Read<Dictionary<string, JsonElement>>(Path.Combine(resources, "WindowsSupport/packages.json"));
        return pins.Select(pair => new CatalogWine("Wine " + Regex.Match(pair.Key, @"Wine([0-9.]+)\.zip$").Groups[1].Value,
            Regex.Match(pair.Key, @"Wine([0-9.]+)\.zip$").Groups[1].Value, pair.Key,
            pair.Value.GetProperty("bytes").GetInt64(), pair.Value.GetProperty("sha256").GetString()!, pair.Value.GetProperty("filesystemVersion").GetString()!))
            .OrderByDescending(w => Version.Parse(w.WineVersion)).ToList();
    }
    public static bool TrustedUrl(Uri url) => url.Scheme == "https" && url.Port == 443 && url.UserInfo.Length == 0 && url.Fragment.Length == 0 && (url.Host == "boxedwine.org" || url.Host == "www.boxedwine.org");
    public static async Task Download(string url, long bytes, string hash, string destination, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        Uri address = new(url);
        if (!TrustedUrl(address) || bytes <= 0 || bytes > 4L * 1024 * 1024 * 1024 || !Regex.IsMatch(hash, "^[a-f0-9]{64}$")) throw new InvalidDataException("The download is not from a supported, checksum-pinned source.");
        Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
        SafeFiles.NoLinks(destination);
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellation);
        timeout.CancelAfter(TimeSpan.FromMinutes(30));
        string partial = destination + "." + Guid.NewGuid().ToString("N") + ".partial";
        try
        {
            HttpResponseMessage response;
            int redirects = 0;
            while (true)
            {
                response = await Client.GetAsync(address, HttpCompletionOption.ResponseHeadersRead, timeout.Token);
                if ((int)response.StatusCode is >= 300 and < 400)
                {
                    var location = response.Headers.Location;
                    response.Dispose();
                    if (location == null || ++redirects > 5) throw new IOException("The download redirected too many times.");
                    address = location.IsAbsoluteUri ? location : new Uri(address, location);
                    if (!TrustedUrl(address)) throw new IOException("The download redirected outside boxedwine.org.");
                    continue;
                }
                break;
            }
            using (response)
            {
                response.EnsureSuccessStatusCode();
                if (response.Content.Headers.ContentLength is long length && length != bytes) throw new InvalidDataException("The download size differs from the release catalog.");
                await using var input = await response.Content.ReadAsStreamAsync(timeout.Token);
                await using var output = new FileStream(partial, FileMode.CreateNew, FileAccess.Write, FileShare.None, 128 * 1024, true);
                byte[] buffer = new byte[128 * 1024]; long copied = 0; int count;
                while ((count = await input.ReadAsync(buffer, timeout.Token)) != 0)
                {
                    copied += count;
                    if (copied > bytes) throw new InvalidDataException("The download exceeds its expected size.");
                    await output.WriteAsync(buffer.AsMemory(0, count), timeout.Token);
                    progress?.Report(new("Downloading " + Path.GetFileName(address.LocalPath), copied, bytes));
                }
                if (copied != bytes) throw new InvalidDataException("The download is incomplete.");
                await output.FlushAsync(timeout.Token); output.Flush(true);
            }
            progress?.Report(new("Verifying download…"));
            if (await SafeFiles.Hash(partial, timeout.Token) != hash) throw new InvalidDataException("The download failed its SHA-256 check. Nothing was installed.");
            File.Move(partial, destination, true);
        }
        finally { if (File.Exists(partial)) File.Delete(partial); }
    }
    public static async Task<WineReference> Ensure(LibraryRepository repository, CatalogWine wine, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        string path = repository.PackagePath(wine.Reference);
        if (File.Exists(path))
        {
            var reference = await ValidateWine(path, cancellation, progress);
            if (reference == wine.Reference) return reference;
            throw new InvalidDataException("The saved Wine package has changed. Import the matching package again in Settings.");
        }
        string staging = SafeFiles.Beneath(repository.DirectoryPath, "Downloads/" + Guid.NewGuid().ToString("N") + ".zip");
        try
        {
            await Download(wine.Url, wine.Bytes, wine.Sha256, staging, cancellation, progress);
            var reference = await ValidateWine(staging, cancellation, progress);
            if (reference != wine.Reference) throw new InvalidDataException("The Wine package metadata differs from its catalog.");
            Directory.CreateDirectory(Path.GetDirectoryName(path)!);
            File.Move(staging, path);
            return reference;
        }
        finally { if (File.Exists(staging)) File.Delete(staging); }
    }
    public static async Task<WineReference> Import(LibraryRepository repository, string source, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        string staging = SafeFiles.Beneath(repository.DirectoryPath, "Downloads/" + Guid.NewGuid().ToString("N") + ".zip");
        try
        {
            await SafeFiles.CopyFile(source, staging, cancellation, progress);
            var wine = await ValidateWine(staging, cancellation, progress);
            string destination = repository.PackagePath(wine);
            Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
            // Reuse matching storage even when a running app holds its read lease.
            if (File.Exists(destination) && new FileInfo(destination).Length == wine.Bytes && await SafeFiles.Hash(destination, cancellation) == wine.Sha256) return wine;
            File.Move(staging, destination, true);
            return wine;
        }
        finally { if (File.Exists(staging)) File.Delete(staging); }
    }
    public static async Task VerifyWineChecksum(string path, WineReference expected, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        LibraryRepository.ValidateReference(expected); SafeFiles.NoLinks(path);
        cancellation.ThrowIfCancellationRequested();
        progress?.Report(new("Checking Wine checksum…"));
        await using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, 128 * 1024, true);
        if (file.Length != expected.Bytes || Convert.ToHexStringLower(await System.Security.Cryptography.SHA256.HashDataAsync(file, cancellation)) != expected.Sha256)
            throw new InvalidDataException("This app's Wine package has changed. Its Windows files have been kept. Import the matching Wine package again in Settings.");
    }
    private sealed record Record(bool Directory, string? Link, byte[] Prefix, long Size);
    public static async Task<WineReference> ValidateWine(string path, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        SafeFiles.NoLinks(path);
        await using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, 128 * 1024, true);
        if (file.Length is <= 0 or > 4294967296) throw new InvalidDataException("Choose a complete Boxedwine Wine ZIP (at most 4 GB).");
        long packageBytes = file.Length;
        string hash = Convert.ToHexStringLower(await System.Security.Cryptography.SHA256.HashDataAsync(file, cancellation)); file.Position = 0;
        using var archive = new ZipArchive(file, ZipArchiveMode.Read, true);
        if (archive.Entries.Count is 0 or > 100000) throw new InvalidDataException("The Wine ZIP has too many files or is empty.");
        var records = new Dictionary<string, Record>(StringComparer.Ordinal);
        var metadata = new Dictionary<string, string>();
        long expanded = 0; int index = 0;
        foreach (var entry in archive.Entries)
        {
            cancellation.ThrowIfCancellationRequested();
            bool directory = entry.FullName.EndsWith('/'); string name = entry.FullName.TrimEnd('/');
            if (name.Length == 0 || name.Length > 1022 || name.Contains('\\') || name.Any(char.IsControl) || name.Split('/').Any(p => p is "" or "." or "..")) throw new InvalidDataException("The Wine ZIP contains an unsafe path.");
            int mode = (entry.ExternalAttributes >> 16) & 0xf000;
            if (mode is not (0 or 0x8000 or 0x4000) || mode == 0x4000 && !directory) throw new InvalidDataException("The Wine ZIP contains unsupported native links or devices.");
            if (entry.Length > 1024 * 1024 * 1024L || (expanded += entry.Length) > 8L * 1024 * 1024 * 1024) throw new InvalidDataException("The Wine ZIP expands beyond the supported limit.");
            bool link = !directory && name.EndsWith(".link"), meta = name is "wineVersion.txt" or "version.txt" or "name.txt" or "depends.txt";
            if ((link || meta) && entry.Length > 1022 || directory && entry.Length != 0) throw new InvalidDataException("Invalid Wine package metadata.");
            await using var input = entry.Open();
            byte[] buffer = new byte[64 * 1024]; using var capture = new MemoryStream(); long read = 0; uint crc = 0xffffffff; int count;
            while ((count = await input.ReadAsync(buffer, cancellation)) > 0)
            {
                read += count; if (read > entry.Length) throw new InvalidDataException("An archive file exceeds its declared size.");
                crc = Crc32.Update(crc, buffer.AsSpan(0, count));
                int keep = Math.Min(count, Math.Max(0, (link || meta ? 1022 : 64) - (int)capture.Length));
                capture.Write(buffer, 0, keep);
            }
            if (read != entry.Length || ~crc != entry.Crc32) throw new InvalidDataException("A Wine package file failed its CRC or size check.");
            byte[] prefix = capture.ToArray();
            string? target = null;
            if (link || meta)
            {
                string text = new UTF8Encoding(false, true).GetString(prefix);
                if (text.Contains('\0')) throw new InvalidDataException("Invalid Wine metadata text.");
                if (link) { if (text.Length == 0 || text.IndexOfAny(['\r', '\n', '\\']) >= 0) throw new InvalidDataException("Malformed guest link."); target = text; }
                else metadata[name] = text.Trim();
            }
            string key = link ? name[..^5] : name;
            if (records.TryGetValue(key, out var previous))
            {
                if (directory || previous.Directory || meta || entry.Length > 64 || previous.Size != entry.Length || !previous.Prefix.SequenceEqual(prefix)) throw new InvalidDataException("Conflicting Wine package entries.");
            }
            else records.Add(key, new(directory, target, prefix, entry.Length));
            progress?.Report(new("Checking Wine package", ++index, archive.Entries.Count));
        }
        if (!metadata.TryGetValue("wineVersion.txt", out string? version) || !Regex.IsMatch(version, @"^\d{1,3}\.\d{1,3}(?:[.\-][A-Za-z0-9]+)*$") ||
            !metadata.TryGetValue("version.txt", out string? filesystem) || !int.TryParse(filesystem, out int fs) || fs < 1 || metadata.GetValueOrDefault("depends.txt", "").Length > 0) throw new InvalidDataException("Choose a complete Wine filesystem with valid wineVersion.txt and version.txt metadata.");
        foreach (string key in records.Keys)
        {
            int slash = key.LastIndexOf('/');
            while (slash > 0)
            {
                string parent = key[..slash];
                if (records.TryGetValue(parent, out var entry) && !entry.Directory && entry.Link == null) throw new InvalidDataException("A Wine package file is also used as a directory.");
                slash = parent.LastIndexOf('/');
            }
        }
        Record Resolve(string input, HashSet<string> visited)
        {
            if (!visited.Add(input) || visited.Count > 40) throw new InvalidDataException("Wine package links form a loop.");
            var parts = input.Split('/');
            for (int i = 1; i <= parts.Length; i++)
            {
                string partial = string.Join('/', parts.Take(i));
                if (records.TryGetValue(partial, out var r) && r.Link is string target)
                {
                    var components = new List<string>();
                    string joined = (target.StartsWith('/') ? "" : string.Join('/', parts.Take(i - 1)) + "/") + target + "/" + string.Join('/', parts.Skip(i));
                    foreach (var segment in joined.Split('/'))
                    {
                        if (segment is "" or ".") continue;
                        if (segment == "..") { if (components.Count == 0) throw new InvalidDataException("Guest link escapes its root."); components.RemoveAt(components.Count - 1); }
                        else components.Add(segment);
                    }
                    return Resolve(string.Join('/', components), visited);
                }
            }
            return records.GetValueOrDefault(input) ?? throw new InvalidDataException("The package is missing /bin/wine or its link target.");
        }
        var launcher = Resolve("bin/wine", []);
        if (launcher.Prefix.Length < 52 || !launcher.Prefix.AsSpan(0, 7).SequenceEqual(new byte[] {127,69,76,70,1,1,1}) || launcher.Prefix[16] is not (2 or 3) || launcher.Prefix[18] != 3) throw new InvalidDataException("Wine must be a 32-bit x86 Linux executable for Boxedwine.");
        foreach (string dll in new[] { "ntdll", "kernel32" })
            if (!records.Any(p => !p.Value.Directory && p.Value.Link == null && (p.Key.EndsWith("/" + dll + ".dll") || p.Key.EndsWith("/" + dll + ".dll.so")))) throw new InvalidDataException("The package is missing Wine's " + dll + " library.");
        return new(hash, packageBytes, version, filesystem);
    }
    public static async Task Extract(string zip, string destination, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        using var archive = ZipFile.OpenRead(zip);
        if (archive.Entries.Count > 100000) throw new InvalidDataException("The archive contains too many files.");
        long expanded = 0; var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase); int index = 0;
        foreach (var entry in archive.Entries)
        {
            cancellation.ThrowIfCancellationRequested();
            string name = entry.FullName.Replace('\\', '/').TrimEnd('/');
            if (name.Length == 0) continue;
            string target = SafeFiles.Beneath(destination, name);
            int mode = (entry.ExternalAttributes >> 16) & 0xf000;
            if (mode is not (0 or 0x8000 or 0x4000) || !names.Add(name) || (expanded += entry.Length) > 8L * 1024 * 1024 * 1024 || entry.Length > 2L * 1024 * 1024 * 1024) throw new InvalidDataException("The archive has linked, duplicate or oversized files.");
            if (entry.FullName.EndsWith('/') || entry.FullName.EndsWith('\\')) { Directory.CreateDirectory(target); continue; }
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            await using var input = entry.Open();
            await using var output = new FileStream(target, FileMode.CreateNew, FileAccess.Write, FileShare.None, 128 * 1024, true);
            var buffer = new byte[128 * 1024]; int count; long copied = 0; uint crc = 0xffffffff;
            while ((count = await input.ReadAsync(buffer, cancellation)) != 0)
            {
                copied += count; if (copied > entry.Length) throw new InvalidDataException("An archive file exceeds its declared size.");
                crc = Crc32.Update(crc, buffer.AsSpan(0, count)); await output.WriteAsync(buffer.AsMemory(0, count), cancellation);
            }
            if (copied != entry.Length || ~crc != entry.Crc32) throw new InvalidDataException("An archive file failed its CRC or size check.");
            progress?.Report(new("Extracting " + name, ++index, archive.Entries.Count));
        }
    }
}
internal static class Crc32
{
    private static readonly uint[] Table = Enumerable.Range(0, 256).Select(n => { uint value = (uint)n; for (int bit = 0; bit < 8; bit++) value = (value & 1) != 0 ? 0xedb88320 ^ (value >> 1) : value >> 1; return value; }).ToArray();
    public static uint Update(uint crc, ReadOnlySpan<byte> data) { foreach (byte value in data) crc = Table[(crc ^ value) & 255] ^ (crc >> 8); return crc; }
}
