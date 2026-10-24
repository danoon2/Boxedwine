// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public static class CdromImages
{
    public static void Validate(IReadOnlyList<string> paths)
    {
        if (paths.Count > 256 || paths.Any(path => string.IsNullOrWhiteSpace(path) || path.Length > 8192 ||
                path.IndexOfAny(['\0', '\r', '\n']) >= 0 || !Path.IsPathFullyQualified(path)))
            throw new InvalidDataException("Choose up to 256 CD-ROM images using full file paths.");
        // Files may be on a disconnected drive. Keep the library editable so
        // users can remove or replace unavailable images before launching.
    }

    public static List<string> LaunchOptions(IReadOnlyList<string> paths, string root)
    {
        Validate(paths);
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
