// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;

sealed partial class TestSuite
{
    private async Task SharedFileTests()
    {
        await Test("Shared files preserve folders, avoid overwrites, isolate apps and launch on Y:", async () =>
        {
            using var repository = Repository("shared-files");
            var app = repository.AddBuiltIn("notepad", wine);
            app.Arguments = ["game-only"]; repository.Update(app);
            string source = Path.Combine(work, "Patch Media");
            Directory.CreateDirectory(Path.Combine(source, "Payload"));
            File.WriteAllText(Path.Combine(source, "patch.exe"), "patch");
            File.WriteAllText(Path.Combine(source, "Payload", "data.bin"), "payload");
            var added = await SharedFiles.Add(repository, app, [source], default);
            Check(File.ReadAllText(Path.Combine(added.Single(), "Payload", "data.bin")) == "payload");
            var duplicate = await SharedFiles.Add(repository, app, [source], default);
            Check(Path.GetFileName(duplicate.Single()) == "Patch Media (2)");
            var second = repository.AddBuiltIn("minesweeper", wine);
            Check(!Directory.EnumerateFileSystemEntries(SharedFiles.DirectoryPath(repository, second)).Any());
            foreach (var launch in new[] {
                LaunchArguments.Build(repository, app, fixture),
                LaunchArguments.Build(repository, app, fixture, tool: WineTool.Explorer),
                LaunchArguments.Build(repository, app, fixture, external: Path.Combine(added[0], "patch.exe"))
            })
            {
                Check(launch[launch.IndexOf("-shared_folder") + 1] == SharedFiles.DirectoryPath(repository, app));
            }
            Check(!LaunchArguments.Build(repository, app, fixture, external: Path.Combine(added[0], "patch.exe")).Contains("game-only"));
            Check(repository.Load().Apps.First(a => a.Id == app.Id).Arguments.SequenceEqual(["game-only"]));
            using var cancelled = new CancellationTokenSource();
            cancelled.Cancel();
            await Throws(() => SharedFiles.Add(repository, app, [source], cancelled.Token), "cancelled copy");
            await Throws(() => SharedFiles.Add(repository, app, [repository.AppDirectory(app)], default), "recursive copy");
            Check(Directory.EnumerateFileSystemEntries(SharedFiles.DirectoryPath(repository, app)).Count() == 2);
            Check(!Directory.EnumerateDirectories(repository.AppDirectory(app), "SharedTransfer-*").Any());
        });
    }
}
