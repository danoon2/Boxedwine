// Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;

sealed partial class TestSuite
{
    private async Task DiscInstallTests()
    {
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
