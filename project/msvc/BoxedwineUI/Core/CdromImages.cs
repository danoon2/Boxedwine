// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public static class CdromImages
{
    public static bool IsImage(string path) => new[] { ".iso", ".cue" }.Contains(Path.GetExtension(path).ToLowerInvariant());

    // Collection payloads stay relative to the app so moves, backups and Wine
    // trial copies keep the images with the app. User-selected files stay absolute.
    public static bool IsOwned(string path) => path.StartsWith("Discs/", StringComparison.Ordinal);
    public static List<string> ArchivePaths(string? value)
    {
        var paths = (value ?? "").Replace("\r\n", "\n").Split('\n').ToList();
        if (paths.Count is < 1 or > 256 || paths.Distinct(StringComparer.OrdinalIgnoreCase).Count() != paths.Count)
            throw new InvalidDataException("CDROMImages needs 1–256 distinct ISO paths, one per line.");
        foreach (string path in paths)
        {
            if (path.Length > 4096 || !path.EndsWith(".iso", StringComparison.OrdinalIgnoreCase))
                throw new InvalidDataException("CDROMImages must name ISO files inside the ZIP.");
            _ = SafeFiles.Beneath(Path.GetTempPath(), path);
        }
        return paths;
    }
    public static List<string> Resolve(IReadOnlyList<string> paths, string appDirectory)
    {
        Validate(paths);
        return paths.Select(path => IsOwned(path) ? SafeFiles.Beneath(appDirectory, path) : path).ToList();
    }

    public static List<string> PrepareImport(IEnumerable<string> paths)
    {
        var images = paths.Select(Path.GetFullPath).Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        Validate(images);
        if (images.Count == 0) throw new InvalidDataException("Choose an ISO or CUE disc image.");
        foreach (string image in images)
        {
            SafeFiles.NoLinks(image);
            if (!IsImage(image) || !File.Exists(image)) throw new IOException("Choose an available ISO or CUE disc image: " + image);
            if (!image.EndsWith(".cue", StringComparison.OrdinalIgnoreCase)) continue;
            if (new FileInfo(image).Length > 1024 * 1024) throw new InvalidDataException("The CUE sheet is too large.");
            int files = 0;
            foreach (string line in File.ReadLines(image))
            {
                if (!System.Text.RegularExpressions.Regex.IsMatch(line, @"^\s*FILE(?:\s|$)", System.Text.RegularExpressions.RegexOptions.IgnoreCase)) continue;
                var match = System.Text.RegularExpressions.Regex.Match(line, "^\\s*FILE\\s+(?:\"([^\"]+)\"|(\\S+))\\s+(BINARY|MOTOROLA|WAVE)\\s*$", System.Text.RegularExpressions.RegexOptions.IgnoreCase);
                if (!match.Success) throw new InvalidDataException("Invalid or unsupported FILE entry in CUE sheet: " + image);
                string name = match.Groups[1].Success ? match.Groups[1].Value : match.Groups[2].Value;
                string track = Path.GetFullPath(name, Path.GetDirectoryName(image)!);
                SafeFiles.NoLinks(track);
                if (!File.Exists(track)) throw new FileNotFoundException("The CUE sheet needs this track file. Keep its track files beside the CUE sheet:\n" + track, track);
                files++;
            }
            if (files == 0) throw new InvalidDataException("The CUE sheet contains no track files: " + image);
        }
        return images;
    }

    public static void Validate(IReadOnlyList<string> paths)
    {
        if (paths.Count > 256 || paths.Any(path => string.IsNullOrWhiteSpace(path) || path.Length > 8192 ||
                path.IndexOfAny(['\0', '\r', '\n']) >= 0 || !(Path.IsPathFullyQualified(path) || IsOwned(path))))
            throw new InvalidDataException("Choose up to 256 CD-ROM images using full paths or app-owned Discs/ paths.");
        foreach (string path in paths.Where(IsOwned)) _ = ArchivePaths(path);
        // Files may be on a disconnected drive. Keep the library editable so
        // users can remove or replace unavailable images before launching.
    }

    public static List<string> LaunchOptions(IReadOnlyList<string> paths, string root)
    {
        paths = Resolve(paths, Directory.GetParent(root)!.FullName);
        if (paths.Count == 0) return [];
        foreach (string path in paths)
            if (!File.Exists(path)) throw new FileNotFoundException("CD-ROM image is unavailable: " + path +
                "\nChoose or remove it in App Settings → General.", path);
        // D: is normally the writable host mount. Match the engine's empty
        // drive allocation and avoid existing Wine drive/device links.
        string devices = Path.Combine(root, "home/username/.wine/dosdevices");
        for (char letter = 'e'; letter <= 'z'; letter++)
        {
            if (new[] { letter + "(_colon_)", letter + "(_colon_)(_colon_)",
                    letter + "(_colon_).link", letter + "(_colon_)(_colon_).link" }
                    .Any(name => Path.Exists(Path.Combine(devices, name)))) continue;
            return paths.SelectMany(path => new[] { "-cdrom", path, letter.ToString() }).ToList();
        }
        throw new InvalidOperationException("No free CD-ROM drive letter is available from E: to Z:.");
    }
}
