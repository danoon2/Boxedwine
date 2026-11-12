// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public static class SharedFiles
{
    public static string DirectoryPath(LibraryRepository repository, LibraryApp app)
    {
        string path = SafeFiles.Beneath(repository.Root(app), "home/username/Shared");
        Directory.CreateDirectory(path);
        return path;
    }

    public static async Task<IReadOnlyList<string>> Add(LibraryRepository repository, LibraryApp app,
        IEnumerable<string> sources, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        string directory = DirectoryPath(repository, app);
        string stage = SafeFiles.Beneath(repository.AppDirectory(app), "SharedTransfer-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(stage);
        try
        {
            var names = new HashSet<string>(Directory.EnumerateFileSystemEntries(directory).Select(Path.GetFileName)!, StringComparer.OrdinalIgnoreCase);
            var items = new List<(string Staged, string Target)>();
            foreach (string source in sources)
            {
                cancellation.ThrowIfCancellationRequested();
                string full = Path.TrimEndingDirectorySeparator(Path.GetFullPath(source));
                SafeFiles.NoLinks(full);
                if (full.Equals(directory, StringComparison.OrdinalIgnoreCase) || SafeFiles.IsWithin(directory, full))
                    throw new IOException("Choose files outside this app's Shared Files folder and its parent folders.");
                string original = Path.GetFileName(full), name = original;
                bool folder = Directory.Exists(full);
                for (int n = 2; !names.Add(name); n++)
                    name = folder ? $"{original} ({n})" : $"{Path.GetFileNameWithoutExtension(original)} ({n}){Path.GetExtension(original)}";
                string staged = SafeFiles.Beneath(stage, name), target = SafeFiles.Beneath(directory, name);
                if (folder) await SafeFiles.CopyTree(full, staged, cancellation, progress);
                else await SafeFiles.CopyFile(full, staged, cancellation, progress);
                items.Add((staged, target));
            }
            cancellation.ThrowIfCancellationRequested();
            // Publish complete top-level items only. The engine never sees a partial file.
            foreach (var item in items)
            {
                SafeFiles.NoLinks(item.Target);
                if (Directory.Exists(item.Staged)) Directory.Move(item.Staged, item.Target);
                else File.Move(item.Staged, item.Target);
            }
            return items.Select(item => item.Target).ToArray();
        }
        finally { SafeFiles.DeleteTree(stage, repository.AppDirectory(app)); }
    }
}
