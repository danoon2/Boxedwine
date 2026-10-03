// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Security.Cryptography;
using System.Text.Json;

namespace Boxedwine.Library;

public static class SafeFiles
{
    public static bool IsWithin(string path, string parent) => Path.GetFullPath(path).StartsWith(Path.TrimEndingDirectorySeparator(Path.GetFullPath(parent)) + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase);
    public static string Beneath(string directory, string relative)
    {
        if (string.IsNullOrEmpty(relative) || relative.Contains('\\') || relative.Contains(':') || relative.Any(char.IsControl) ||
            relative.Split('/').Any(p => p is "" or "." or ".." || p.EndsWith('.') || p.EndsWith(' ') || p.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0 ||
                System.Text.RegularExpressions.Regex.IsMatch(p, @"^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(\.|$)", System.Text.RegularExpressions.RegexOptions.IgnoreCase)))
            throw new InvalidDataException("A file path is outside the app's Windows environment or cannot be stored on Windows.");
        string path = Path.GetFullPath(Path.Combine(directory, relative.Replace('/', Path.DirectorySeparatorChar)));
        if (!IsWithin(path, directory)) throw new InvalidDataException("A file path escapes its folder.");
        NoLinks(path);
        return path;
    }
    // Never follow a junction or host symlink during copy, backup, or deletion.
    // Boxedwine guest symlinks are ordinary .link files and are preserved.
    public static void NoLinks(string path)
    {
        for (string? part = Path.GetFullPath(path); part != null; part = Path.GetDirectoryName(part))
        {
            try { if ((File.GetAttributes(part) & FileAttributes.ReparsePoint) != 0) throw new IOException("Linked files and junctions are not supported here: " + part); }
            catch (FileNotFoundException) { }
            catch (DirectoryNotFoundException) { }
        }
    }
    public static IEnumerable<string> Tree(string root)
    {
        NoLinks(root);
        foreach (string path in Directory.EnumerateFileSystemEntries(root))
        {
            NoLinks(path);
            yield return path;
            if (Directory.Exists(path)) foreach (var child in Tree(path)) yield return child;
        }
    }
    public static void AtomicJson<T>(string path, T value)
    {
        NoLinks(path);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            using (var stream = new FileStream(temporary, FileMode.CreateNew, FileAccess.Write, FileShare.None))
            {
                JsonSerializer.Serialize(stream, value, DataFormat.Json);
                if (stream.Length > 32 * 1024 * 1024) throw new InvalidDataException("The metadata exceeds the supported 32 MB limit. Existing data was kept.");
                stream.Flush(true);
            }
            File.Move(temporary, path, true);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
    }
    public static async Task<string> Hash(string path, CancellationToken cancellation = default)
    {
        NoLinks(path);
        await using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, 128 * 1024, true);
        return Convert.ToHexStringLower(await SHA256.HashDataAsync(file, cancellation).ConfigureAwait(false));
    }
    public static async Task CopyFile(string source, string destination, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        NoLinks(source); NoLinks(destination);
        Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
        await using var input = new FileStream(source, FileMode.Open, FileAccess.Read, FileShare.Read, 128 * 1024, true);
        await using var output = new FileStream(destination, FileMode.CreateNew, FileAccess.Write, FileShare.None, 128 * 1024, true);
        var buffer = new byte[128 * 1024];
        long total = input.Length, copied = 0;
        int count;
        while ((count = await input.ReadAsync(buffer, cancellation)) > 0)
        {
            await output.WriteAsync(buffer.AsMemory(0, count), cancellation);
            copied += count;
            progress?.Report(new("Copying " + Path.GetFileName(source), copied, total));
        }
        await output.FlushAsync(cancellation);
        output.Flush(true);
    }
    public static async Task CopyTree(string source, string destination, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        NoLinks(source); NoLinks(destination);
        if (IsWithin(destination, source) || Path.GetFullPath(source).Equals(Path.GetFullPath(destination), StringComparison.OrdinalIgnoreCase)) throw new IOException("Choose a source outside the destination folder.");
        Directory.CreateDirectory(destination);
        foreach (string path in Tree(source))
        {
            cancellation.ThrowIfCancellationRequested();
            var target = Beneath(destination, Path.GetRelativePath(source, path).Replace('\\', '/'));
            if (Directory.Exists(path)) Directory.CreateDirectory(target);
            else await CopyFile(path, target, cancellation, progress);
        }
    }
    public static void DeleteTree(string path, string ownedParent)
    {
        if (!IsWithin(path, ownedParent)) throw new IOException("Refusing to delete outside the owned folder.");
        NoLinks(path);
        if (!Directory.Exists(path)) return;
        // Check the complete tree before removing any files.
        var files = Tree(path).ToList();
        foreach (var file in files.Where(File.Exists)) { NoLinks(file); File.SetAttributes(file, FileAttributes.Normal); File.Delete(file); }
        foreach (var directory in files.Where(Directory.Exists).OrderByDescending(p => p.Length)) { NoLinks(directory); Directory.Delete(directory); }
        Directory.Delete(path);
    }
}
