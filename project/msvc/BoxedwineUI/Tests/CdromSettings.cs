// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;

sealed partial class TestSuite
{
    private async Task CdromSettingsTests()
    {
        await Test("CD-ROM settings preserve order, paths and empty-drive launches", async () =>
        {
            using var repository = Repository("cdrom-settings");
            var app = repository.AddBuiltIn("notepad", wine);
            Check(!LaunchArguments.Build(repository, app, fixture).Contains("-cdrom"));
            string first = Path.Combine(work, "Disc 1 日本語.iso"), second = Path.Combine(work, "Disc 2.cue");
            File.WriteAllText(first, "fixture"); File.WriteAllText(second, "fixture");
            app.CdromImages = [second, first]; repository.Update(app);
            var saved = repository.Load().Apps.Single();
            Check(saved.CdromImages!.SequenceEqual([second, first]));
            var launch = LaunchArguments.Build(repository, saved, fixture);
            int offset = launch.IndexOf("-cdrom");
            Check(offset >= 0 && launch.Skip(offset).Take(6).SequenceEqual(["-cdrom", second, "e", "-cdrom", first, "e"]));
            Check(offset < launch.IndexOf("/bin/wine"), "CD options must precede the guest command");
            string devices = Path.Combine(repository.Root(saved), "home/username/.wine/dosdevices");
            Directory.CreateDirectory(devices);
            File.WriteAllText(Path.Combine(devices, "e(_colon_).link"), "/other");
            File.WriteAllText(Path.Combine(devices, "f(_colon_)(_colon_).link"), "/dev/other");
            launch = LaunchArguments.Build(repository, saved, fixture);
            Check(launch[launch.IndexOf("-cdrom") + 2] == "g", "Existing drive/device links must not be overwritten");
            File.Delete(first);
            Check(repository.Load().Apps.Single().CdromImages!.Count == 2, "A missing image must not prevent opening settings");
            await Throws(() => Task.FromResult(LaunchArguments.Build(repository, saved, fixture)), "missing disc must report a launch error");
            saved.CdromImages = []; repository.Update(saved);
            Check(!LaunchArguments.Build(repository, repository.Load().Apps.Single(), fixture).Contains("-cdrom"));
            Check(File.Exists(second), "Removing saved references must not remove disc files");
            foreach (var paths in new List<string>[] { ["relative.iso"], [first + "\0"], [null!], Enumerable.Repeat(second, 257).ToList() })
                await Throws(() => { CdromImages.Validate(paths); return Task.CompletedTask; }, "invalid CD-ROM metadata");
        });
        await Test("CD-ROM settings reach installed apps, installers, alternate programs and backups", async () =>
        {
            using var repository = Repository("cdrom-launch-modes");
            var package = await Packages.Import(repository, fixture, default, null);
            var app = repository.AddBuiltIn("notepad", package);
            string disc = Path.Combine(work, "Install disc.iso"); File.WriteAllText(disc, "fixture");
            app.CdromImages = [disc];
            app.IsNotepad = false; app.BuiltInProgram = null;
            app.Executable = AddProgram(repository, app, "Game/game.exe");
            string alternate = AddProgram(repository, app, "Game/config.exe");
            app.Installer = "Installer/setup.exe";
            string installer = Path.Combine(repository.AppDirectory(app), "Installer/setup.exe");
            Directory.CreateDirectory(Path.GetDirectoryName(installer)!); File.WriteAllText(installer, "MZfixture");
            repository.Update(app);
            foreach (var launch in new[] {
                LaunchArguments.Build(repository, app, fixture),
                LaunchArguments.Build(repository, app, fixture, installing: true),
                LaunchArguments.Build(repository, app, fixture, alternate: alternate),
                LaunchArguments.Build(repository, app, fixture, external: installer) })
            {
                Check(launch[launch.IndexOf("-cdrom") + 1] == disc);
                Check(launch.IndexOf("-cdrom") < launch.IndexOf("/bin/wine"));
            }
            string backup = Path.Combine(work, "cdrom.boxedwinebackup");
            await Backups.Export(repository, app, backup, default, null);
            var restored = await Backups.Restore(repository, backup, default, null);
            Check(restored.CdromImages!.SequenceEqual([disc]), "Backup must preserve the ordered image references");
        });
    }
}
