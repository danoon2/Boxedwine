using Boxedwine.Library;
using System.IO.Compression;
using System.Text;

sealed partial class TestSuite
{
    private async Task InstallerSourceTests()
    {
        await Test("Local EXE/MSI and nested installer folders mount source media without copying", async () =>
        {
            foreach (string extension in new[] { ".exe", ".MSI" })
            foreach (bool folder in new[] { false, true })
            {
                using var repository = Repository("mounted-" + extension + folder);
                var package = await Packages.Import(repository, fixture, default, null);
                string source = Path.Combine(work, "Source files " + extension + folder);
                string programDirectory = folder ? Path.Combine(source, "Setup Files") : source;
                Directory.CreateDirectory(programDirectory);
                string setup = Path.Combine(programDirectory, "TheMoviesSetup" + extension);
                File.WriteAllText(setup, "MZsetup"); File.WriteAllText(Path.Combine(source, "TheMoviesSetup.split.bin"), "payload");
                using var prepared = await PreparedAppSource.Prepare(folder ? source : setup, false, false, default, null);
                Check(!prepared.RequiresCopy);
                var app = await repository.Import(prepared.Source, folder ? "installerFolder" : "installer", "wineDefault", package, default,
                    installer: folder ? setup : null, copyInstaller: prepared.RequiresCopy);
                Check(app.InstallerSource == source && app.HasInstaller);
                Check(!Directory.Exists(Path.Combine(repository.AppDirectory(app), "Installer")), "Local installer media was copied");
                var saved = repository.Load().Apps.Single();
                var launch = LaunchArguments.Build(repository, saved, fixture, installing: true);
                Check(launch[launch.IndexOf("-mount") + 1] == source);
                Check(launch[launch.IndexOf("-w") + 1] == (folder ? "/mnt/installer/Setup Files" : "/mnt/installer"));
                Check(launch.Last() == "/mnt/installer/" + (folder ? "Setup Files/" : "") + Path.GetFileName(setup));
                Check(launch.Contains("/unix") == (extension == ".MSI"));
                saved.Executable = AddProgram(repository, saved, "Game/game.exe"); repository.Update(saved);
                Directory.Move(source, source + " moved");
                Check(!LaunchArguments.Build(repository, saved, fixture).Contains("-mount"));
                await Throws(() => Task.FromResult(LaunchArguments.Build(repository, saved, fixture, installing: true)), "missing installer source");
                string backup = Path.Combine(work, "mounted-" + extension + folder + ".boxedwinebackup");
                await Backups.Export(repository, saved, backup, default, null);
                var restored = await Backups.Restore(repository, backup, default, null);
                Check(restored.InstallerSource == source && !Directory.Exists(Path.Combine(repository.AppDirectory(restored), "Installer")));
                Directory.Move(source + " moved", source);
                Check(LaunchArguments.Build(repository, restored, fixture, installing: true).Contains(source));
                repository.Delete(restored.Id);
                Check(File.Exists(setup) && File.ReadAllText(Path.Combine(source, "TheMoviesSetup.split.bin")) == "payload");
            }
        });
        await Test("Extracted installers retain their payload after preparation is disposed", async () =>
        {
            string archive = Path.Combine(work, "setup-media.zip");
            using (var zip = ZipFile.Open(archive, ZipArchiveMode.Create))
            {
                Put(zip, "Setup/setup.exe", Encoding.UTF8.GetBytes("MZsetup"));
                Put(zip, "data.bin", Encoding.UTF8.GetBytes("payload"));
            }
            using var repository = Repository("retained-installer");
            LibraryApp app;
            using (var prepared = await PreparedAppSource.Prepare(archive, false, false, default, null))
            {
                Check(prepared.RequiresCopy);
                app = await repository.Import(prepared.Source, "installerFolder", "wineDefault", wine, default,
                    installer: Path.Combine(prepared.Source, "Setup/setup.exe"), copyInstaller: prepared.RequiresCopy);
            }
            Check(app.InstallerSource == null);
            Check(File.ReadAllText(Path.Combine(repository.AppDirectory(app), "Installer/data.bin")) == "payload");
            Check(LaunchArguments.Build(repository, app, fixture, installing: true).Contains(Path.Combine(repository.AppDirectory(app), "Installer")));
        });
    }
}
