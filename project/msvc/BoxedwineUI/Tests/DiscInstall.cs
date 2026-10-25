// Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;
using System.IO.Compression;
using System.Text;

sealed partial class TestSuite
{
    private async Task DiscInstallTests()
    {
        await Test("Disc collections validate ordered ISO paths and keep invalid updates out", async () =>
        {
            var value = CollectionSnapshot.Load(Path.Combine(AppContext.BaseDirectory, "collection-example.json"));
            var entry = value.Apps[0];
            entry["InstallType"] = "Disc"; entry.Remove("InstallExe");
            entry["CDROMImages"] = "Disc 1.iso\r\nMedia/Disc 2.ISO";
            Check(value.Recipes()[0].DiscImages.SequenceEqual(["Disc 1.iso", "Media/Disc 2.ISO"]));
            using var repository = Repository("disc-collection-validation");
            var store = new CollectionStore(repository.DirectoryPath); store.Merge(value, value.Folder);
            string file = Path.Combine(repository.DirectoryPath, "collections.json"), saved = File.ReadAllText(file);
            foreach (string bad in new[] { "", "../Disc.iso", "/Disc.iso", "C:/Disc.iso", "a\\Disc.iso", "Disc.cue", "Disc.iso\n", "Disc.iso\ndisc.ISO", "a/../Disc.iso" })
            {
                entry["CDROMImages"] = bad;
                await Throws(() => Task.Run(() => store.Merge(value, value.Folder)), "Invalid archive image path");
                Check(File.ReadAllText(file) == saved);
            }
            entry["CDROMImages"] = "Disc.iso";
            entry["InstallType"] = "Zip";
            await Throws(() => Task.Run(() => value.Recipes()), "Images require a Disc recipe");
            entry["InstallType"] = "Disc"; entry["InstallExe"] = "setup.exe";
            await Throws(() => Task.Run(() => value.Recipes()), "Disc recipes use autorun");
            entry.Remove("InstallExe"); entry["FileURL"] = "https://example.com/disc.iso";
            await Throws(() => Task.Run(() => value.Recipes()), "Disc recipes require a ZIP");
        });
        await Test("Disc collection install and backup resolve owned media against the current app", async () =>
        {
            using var repository = Repository("disc-collection-install");
            var package = await Packages.Import(repository, fixture, default, null);
            using var memory = new MemoryStream();
            using (var zip = new ZipArchive(memory, ZipArchiveMode.Create, true))
            {
                Put(zip, "Disc 1.iso", Encoding.UTF8.GetBytes("first disc"));
                Put(zip, "Media/Disc 2.iso", Encoding.UTF8.GetBytes("second disc"));
            }
            byte[] payload = memory.ToArray();
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(20));
            var listener = new System.Net.Sockets.TcpListener(System.Net.IPAddress.Loopback, 0); listener.Start();
            var server = Task.Run(async () =>
            {
                for (int i = 0; i < 2; i++)
                {
                    using var client = await listener.AcceptTcpClientAsync(timeout.Token);
                    await using var stream = client.GetStream();
                    using var reader = new StreamReader(stream, leaveOpen: true);
                    while (!string.IsNullOrEmpty(await reader.ReadLineAsync(timeout.Token))) { }
                    await stream.WriteAsync(Encoding.ASCII.GetBytes($"HTTP/1.1 200 OK\r\nContent-Length: {payload.Length}\r\nConnection: close\r\n\r\n"), timeout.Token);
                    await stream.WriteAsync(payload, timeout.Token);
                }
            });
            try
            {
                var value = CollectionSnapshot.Load(Path.Combine(AppContext.BaseDirectory, "collection-example.json"));
                var entry = value.Apps[0];
                entry["InstallType"] = "Disc"; entry.Remove("InstallExe");
                entry["CDROMImages"] = "Disc 1.iso\nMedia/Disc 2.iso";
                entry["FileURL"] = $"http://127.0.0.1:{((System.Net.IPEndPoint)listener.LocalEndpoint).Port}/discs.zip";
                entry["FileSizeBytes"] = payload.Length.ToString();
                entry["FileSHA256"] = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(payload)).ToLowerInvariant();
                var app = await Demos.Install(repository, value.Recipes()[0], package, timeout.Token, null);
                app = repository.Load().Apps.Single();
                Check(app.HasInstaller && app.InstallFromDisc == true && app.Installer == null);
                Check(app.CdromImages!.SequenceEqual(["Discs/Disc 1.iso", "Discs/Media/Disc 2.iso"]));
                var command = LaunchArguments.Build(repository, app, fixture, installing: true);
                string first = Path.Combine(repository.AppDirectory(app), "Discs/Disc 1.iso");
                string second = Path.Combine(repository.AppDirectory(app), "Discs/Media/Disc 2.iso");
                Check(command.Last() == "-cdromInstall");
                Check(command.Skip(command.IndexOf("-cdrom")).Take(6).SequenceEqual(["-cdrom", Path.GetFullPath(first), "e", "-cdrom", Path.GetFullPath(second), "e"]));
                string executable = AddProgram(repository, app, "Game/game.exe");
                app = repository.SelectingInstalledDemoProgram(app, new(0, false, true)) ?? throw new Exception("Installed disc game was not selected");
                Check(app.Executable == executable); repository.Update(app);
                Check(!LaunchArguments.Build(repository, app, fixture).Contains("-cdromInstall"));
                string backup = Path.Combine(work, "owned-discs.boxedwinebackup");
                await Backups.Export(repository, app, backup, default, null);
                using var other = Repository("disc-collection-restored");
                var restored = await Backups.Restore(other, backup, default, null);
                Check(restored.InstallFromDisc == true && restored.CdromImages!.SequenceEqual(app.CdromImages));
                string restoredFirst = Path.GetFullPath(Path.Combine(other.AppDirectory(restored), "Discs/Disc 1.iso"));
                Check(File.ReadAllText(restoredFirst) == "first disc");
                var restoredCommand = LaunchArguments.Build(other, restored, other.WinePath(restored), installing: true);
                Check(restoredCommand.Contains(restoredFirst) && !restoredCommand.Contains(Path.GetFullPath(first)));
                entry["ID"] = "missing-disc"; entry["CDROMImages"] += "\nMissing.iso";
                await Throws(() => Demos.Install(repository, value.Recipes()[0], package, timeout.Token, null), "Missing media must discard the import");
                Check(repository.Load().Apps.Count == 1 && Directory.GetDirectories(Path.Combine(repository.DirectoryPath, "Applications")).Length == 1);
                await server;
            }
            finally { listener.Stop(); }
        });
        await Test("Disc drops retain images, order and CUE dependencies", async () =>
        {
            string folder = Path.Combine(work, "Disc images 日本語"); Directory.CreateDirectory(folder);
            string first = Path.Combine(folder, "Disc 1.iso"), cue = Path.Combine(folder, "Disc 2.CUE"), bin = Path.Combine(folder, "Track 1.bin");
            File.WriteAllText(first, "image fixture"); File.WriteAllText(bin, "track fixture");
            File.WriteAllText(cue, "FILE \"Track 1.bin\" BINARY\r\n  TRACK 01 MODE1/2352\r\n  INDEX 01 00:00:00\r\n");
            using var source = await PreparedAppSource.Prepare(first, false, false, default, null);
            Check(source.DiscImages!.SequenceEqual([first]) && source.Name == "Disc 1");
            using var multiple = PreparedAppSource.PrepareDiscs([cue, first, cue]);
            Check(multiple.DiscImages!.SequenceEqual([cue, first]));
            await Throws(() => PreparedAppSource.Prepare(cue, false, true, default, null), "Disc is not a collection");
            File.Delete(bin);
            await Throws(() => PreparedAppSource.Prepare(cue, false, false, default, null), "Missing CUE dependency");
            File.WriteAllText(cue, "FILE \"Track 1.bin\" MP3\n");
            await Throws(() => PreparedAppSource.Prepare(cue, false, false, default, null), "Unsupported CUE dependency");
            Check(File.Exists(first) && File.Exists(cue), "Validation never deletes source media");
        });
        await Test("Disc installs autorun once, browse explicitly, and retain reinstall metadata", async () =>
        {
            using var repository = Repository("disc-installs");
            var package = await Packages.Import(repository, fixture, default, null);
            string first = Path.Combine(work, "Install disc.iso"), second = Path.Combine(work, "Play disc.iso");
            File.WriteAllText(first, "disc"); File.WriteAllText(second, "disc");
            var app = await repository.Import(first, "disc", "wineDefault", package, default, name: "Disc game", discImages: [first, second]);
            Check(app.HasInstaller && app.Installer == null && app.InstallFromDisc == true);
            app = repository.Load().Apps.Single();
            Check(app.CdromImages!.SequenceEqual([first, second]));
            var command = LaunchArguments.Build(repository, app, fixture, installing: true);
            Check(command.Last() == "-cdromInstall" && !command.Contains("/bin/wine"));
            Check(command.Skip(command.IndexOf("-cdrom")).Take(6).SequenceEqual(["-cdrom", first, "e", "-cdrom", second, "e"]));
            string devices = Path.Combine(repository.Root(app), "home/username/.wine/dosdevices"); Directory.CreateDirectory(devices);
            File.WriteAllText(Path.Combine(devices, "e(_colon_).link"), "/other");
            var browse = LaunchArguments.Build(repository, app, fixture, installing: true, browseDisc: true);
            Check(browse.TakeLast(3).SequenceEqual(["/bin/wine", "explorer", "f:\\"]) && !browse.Contains("-cdromInstall"));
            app.Executable = AddProgram(repository, app, "Game/game.exe"); repository.Update(app);
            Check(!LaunchArguments.Build(repository, app, fixture).Contains("-cdromInstall"), "Normal launches must not run the installer");
            string backup = Path.Combine(work, "disc-install.boxedwinebackup");
            await Backups.Export(repository, app, backup, default, null);
            var restored = await Backups.Restore(repository, backup, default, null);
            Check(restored.HasInstaller && restored.InstallFromDisc == true && restored.CdromImages!.SequenceEqual([first, second]));
            restored.CdromImages = [];
            await Throws(() => Task.FromResult(LaunchArguments.Build(repository, restored, fixture, installing: true)), "Empty install disc list");
        });
    }
}
