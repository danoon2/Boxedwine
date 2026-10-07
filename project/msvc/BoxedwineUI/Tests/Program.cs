// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;
using System.IO.Compression;
using System.Text;
using System.Text.Json;

// Deliberately dependency-free: tests work offline with the same SDK as the app.
var suite = new TestSuite();
await suite.Run(args);
return suite.Failures == 0 ? 0 : 1;

sealed partial class TestSuite
{
    public int Failures { get; private set; }
    private int passes;
    private string work = "";
    private string fixture = "";
    private WineReference wine = null!;
    private static void Check(bool condition, string message = "Assertion failed") { if (!condition) throw new Exception(message); }
    private static async Task Throws(Func<Task> action, string reason)
    {
        try { await action(); } catch { return; }
        throw new Exception("Expected rejection: " + reason);
    }
    private async Task Test(string name, Func<Task> body)
    {
        try { await body(); Console.WriteLine("PASS " + name); passes++; }
        catch (Exception error) { Console.WriteLine("FAIL " + name + "\n" + error); Failures++; }
    }
    private LibraryRepository Repository(string name) => new(Path.Combine(work, name));
    private sealed class ProgressCount : IProgress<OperationProgress>
    {
        public int Count { get; private set; }
        public void Report(OperationProgress value) => Count++;
    }
    private static string AddProgram(LibraryRepository repository, LibraryApp app, string relative)
    {
        string path = LibraryRepository.DriveC + "/" + relative;
        string file = SafeFiles.Beneath(repository.Root(app), path);
        Directory.CreateDirectory(Path.GetDirectoryName(file)!); File.WriteAllText(file, "MZprogram"); return path;
    }
    private static LibraryApp InstallerDemo() => new()
    {
        Name = "Installer selection fixture", Installer = "Installer/setup.exe",
        Demo = new("selection-test", "test-1", new string('a', 64), "empires.exe"),
        Arguments = ["an app argument"], BoxedwineArguments = ["-nosound"],
        DemoSettings = new() { WindowsVersion = "winxp", Gdi = true }
    };
    private static void Put(ZipArchive archive, string name, byte[] bytes) { var entry = archive.CreateEntry(name, CompressionLevel.NoCompression); using var output = entry.Open(); output.Write(bytes); }
    private string WineFixture(string name, Action<ZipArchive>? extra = null)
    {
        string path = Path.Combine(work, name);
        using var archive = ZipFile.Open(path, ZipArchiveMode.Create);
        Put(archive, "wineVersion.txt", Encoding.UTF8.GetBytes("11.0\n")); Put(archive, "version.txt", Encoding.UTF8.GetBytes("14\n"));
        Put(archive, "bin/wine.link", Encoding.UTF8.GetBytes("../opt/wine/bin/wine"));
        byte[] elf = new byte[64]; new byte[] {127,69,76,70,1,1,1}.CopyTo(elf, 0); elf[16] = 2; elf[18] = 3; Put(archive, "opt/wine/bin/wine", elf);
        Put(archive, "opt/wine/lib/wine/ntdll.dll", [1]); Put(archive, "opt/wine/lib/wine/kernel32.dll", [1]); extra?.Invoke(archive); return path;
    }
    public async Task Run(string[] args)
    {
        work = Path.Combine(Path.GetFullPath(args.FirstOrDefault(a => !a.StartsWith('-')) ?? "../../../../../../tmp/native-ui-tests"), Guid.NewGuid().ToString("N")); Directory.CreateDirectory(work);
        Console.WriteLine("Test workspace: " + work);
        fixture = WineFixture("wine.zip"); wine = await Packages.ValidateWine(fixture, default);
        await Test("Engine location stays with each UI; overrides never fall back or enter preferences", async () =>
        {
            string first = Path.Combine(work, "first portable UI"), second = Path.Combine(work, "second portable UI");
            foreach (string folder in new[] { first, second })
            {
                Directory.CreateDirectory(Path.Combine(folder, "Runtime"));
                File.WriteAllText(Path.Combine(folder, "Runtime", "BoxedwineEngine.exe"), "engine fixture");
                File.WriteAllText(Path.Combine(folder, "Boxedwine.exe"), "UI fixture");
                File.WriteAllText(Path.Combine(folder, "BoxedwineEngine.exe"), "old adjacent engine fixture");
            }
            string firstEngine = EngineLocation.Resolve(first), secondEngine = EngineLocation.Resolve(second);
            Check(firstEngine == Path.Combine(first, "Runtime", "BoxedwineEngine.exe") && firstEngine != secondEngine);
            string? alternate = EngineLocation.CommandLineOverride(["--emulator", secondEngine]);
            Check(EngineLocation.Resolve(first, alternate) == secondEngine && EngineLocation.Resolve(first) == firstEngine);
            await Throws(() => Task.FromResult(EngineLocation.CommandLineOverride(["--emulator"])), "missing override argument");
            await Throws(() => Task.FromResult(EngineLocation.CommandLineOverride(["--emulator", "--theme", "Dark"])), "option used as engine path");
            await Throws(() => Task.FromResult(EngineLocation.Resolve(first, Path.Combine(first, "missing.exe"))), "invalid override must not use bundled engine");
            await Throws(() => Task.FromResult(EngineLocation.Resolve(first, Path.Combine(first, "Boxedwine.exe"))), "UI cannot launch itself as engine");
            File.Delete(firstEngine);
            await Throws(() => Task.FromResult(EngineLocation.Resolve(first)), "missing bundled engine must not use adjacent legacy engine");
            var preferences = JsonSerializer.Deserialize<LauncherPreferences>("{\"theme\":\"Dark\",\"deleteImmediately\":true,\"emulatorPath\":\"old-engine.exe\"}", DataFormat.Json)!;
            Check(preferences.Theme == "Dark" && preferences.DeleteImmediately);
            Check(!JsonSerializer.Serialize(preferences, DataFormat.Json).Contains("emulatorPath"), "Retired engine preference must not be saved again");
        });
        await Test("Runtime app identity and icon metadata are isolated per launch", () =>
        {
            var first = new System.Diagnostics.ProcessStartInfo();
            var second = new System.Diagnostics.ProcessStartInfo();
            var app = new RuntimeIdentity(Guid.NewGuid(), new byte[64 * 64 * 4]);
            RuntimeIdentity.Configure(first, app);
            RuntimeIdentity.Configure(second, new(Guid.NewGuid()));
            Check(first.Environment[RuntimeIdentity.AppIdVariable] != second.Environment[RuntimeIdentity.AppIdVariable]);
            Check(Convert.FromBase64String(first.Environment[RuntimeIdentity.IconVariable]!).SequenceEqual(app.IconBgra!));
            Check(first.Environment[RuntimeIdentity.IconVariable]!.Length < 32767 && !second.Environment.ContainsKey(RuntimeIdentity.IconVariable));
            RuntimeIdentity.Configure(first, new(app.AppId, new byte[20000]));
            Check(!first.Environment.ContainsKey(RuntimeIdentity.IconVariable), "Reject oversized icon metadata");
            RuntimeIdentity.Configure(first, null);
            Check(!first.Environment.ContainsKey(RuntimeIdentity.IconVariable) && !first.Environment.ContainsKey(RuntimeIdentity.AppIdVariable), "Background Wine tasks must not inherit another app's identity");
            return Task.CompletedTask;
        });
        await OpenGLTests(args);
        await Test("Swift Codable metadata round trip and epoch", () =>
        {
            var app = new LibraryApp { Name = "A \"Unicode\" app 日本語", CreatedAt = 100, BuiltInProgram = "minesweeper", CustomIconPNG = [137,80,78,71,13,10,26,10] };
            string json = JsonSerializer.Serialize(app, DataFormat.Json);
            Check(json.Contains("\"customIconPNG\"")); Check(json.Contains("\"createdAt\": 100"));
            Check(DataFormat.Clone(app).Name == app.Name); Check(DataFormat.Epoch.AddSeconds(100).Year == 2001); return Task.CompletedTask;
        });
        await Test("Wine structure, content hash, CRC and guest links", async () =>
        {
            Check(wine.WineVersion == "11.0" && wine.FilesystemVersion == "14"); Check(wine.Sha256 == await SafeFiles.Hash(fixture));
            string broken = WineFixture("broken.zip", a => Put(a, "depends.txt", Encoding.UTF8.GetBytes("other.zip")));
            await Throws(() => Packages.ValidateWine(broken, default), "dependency package");
            string loop = Path.Combine(work, "loop.zip"); System.IO.File.Copy(fixture, loop);
            using (var archive = ZipFile.Open(loop, ZipArchiveMode.Update)) { archive.GetEntry("bin/wine.link")!.Delete(); Put(archive, "bin/wine.link", Encoding.UTF8.GetBytes("wine")); }
            await Throws(() => Packages.ValidateWine(loop, default), "link cycle");
            string corrupt = Path.Combine(work, "corrupt.zip"); byte[] bytes = File.ReadAllBytes(fixture);
            // ZIP headers contain binary timestamps: a UTF-8 character offset
            // can point into the header instead of the payload we must corrupt.
            int position = bytes.AsSpan().IndexOf("11.0\n"u8);
            Check(position >= 0, "The stored Wine version payload must be present.");
            bytes[position] = (byte)'9'; File.WriteAllBytes(corrupt, bytes);
            await Throws(() => Packages.ValidateWine(corrupt, default), "CRC mismatch");
        });
        await Test("Launch verification checks SHA-256 without inspecting archive contents", async () =>
        {
            string path = Path.Combine(work, "checksum-only.zip");
            await File.WriteAllTextAsync(path, "Previously verified package bytes; deliberately not a ZIP for this check.");
            var expected = new WineReference(await SafeFiles.Hash(path), new FileInfo(path).Length, "11.0", "14");
            await Packages.VerifyWineChecksum(path, expected, default);
            await Throws(() => Packages.VerifyWineChecksum(path, expected with { Bytes = expected.Bytes + 1 }, default), "incorrect package size");
            byte[] changed = await File.ReadAllBytesAsync(path); changed[0] ^= 1; await File.WriteAllBytesAsync(path, changed);
            await Throws(() => Packages.VerifyWineChecksum(path, expected, default), "same-size checksum mismatch");
            using var cancelled = new CancellationTokenSource(); cancelled.Cancel();
            await Throws(() => Packages.VerifyWineChecksum(path, expected, cancelled.Token), "cancelled verification");
        });
        await Test("Launch checks reuse a package and invalidate changed files or expectations", async () =>
        {
            string path = Path.Combine(work, "cached-wine.zip"); File.Copy(fixture, path);
            var checks = new WineLaunchChecks(); var progress = new ProgressCount();
            Check(await checks.Verify(path, wine, default, progress) == wine && progress.Count == 1);
            Check(await checks.Verify(path, wine, default, progress) == wine && progress.Count == 1, "Apps sharing a package must reuse verification");
            using var cancelled = new CancellationTokenSource(); cancelled.Cancel();
            await Throws(() => checks.Verify(path, wine, cancelled.Token), "cancellation on cached verification");
            await Throws(() => checks.Verify(path, wine with { Sha256 = new string('b', 64) }, default), "different expected checksum");
            DateTime stamp = File.GetLastWriteTimeUtc(path);
            byte[] bytes = await File.ReadAllBytesAsync(path); bytes[0] ^= 1; await File.WriteAllBytesAsync(path, bytes);
            File.SetLastWriteTimeUtc(path, stamp.AddSeconds(2));
            await Throws(() => checks.Verify(path, wine, default), "changed package cannot use cached result");
            File.Copy(fixture, path, true); File.SetLastWriteTimeUtc(path, stamp.AddSeconds(4));
            Check(await checks.Verify(path, wine, default, progress) == wine && progress.Count == 2);
            checks.Clear(); Check(await checks.Verify(path, wine, default, progress) == wine && progress.Count == 3);
            string legacy = Path.Combine(work, "legacy-wine.zip"); File.Copy(fixture, legacy);
            Check(await checks.Verify(legacy, null, default) == wine, "Older Wine snapshots remain supported");
        });
        await Test("Owned path containment and Windows special names", async () =>
        {
            foreach (var path in new[] { "../escape", "/absolute", "C:/evil", "a/../b", "a\\b", "a:stream", "NUL.txt", "a. ", "a/CON/b" }) await Throws(() => Task.FromResult(SafeFiles.Beneath(work, path)), path);
            Check(SafeFiles.Beneath(work, "a b/日本語.exe").StartsWith(work));
        });
        await Test("ZIP traversal, duplicate case and native links rejected", async () =>
        {
            foreach (var kind in new[] { "traversal", "collision", "link" })
            {
                string zip = Path.Combine(work, kind + ".zip");
                using (var archive = ZipFile.Open(zip, ZipArchiveMode.Create))
                {
                    if (kind == "link") { var link = archive.CreateEntry("Game.exe"); link.ExternalAttributes = unchecked((int)0xa0000000); using var stream = link.Open(); stream.WriteByte(1); }
                    else Put(archive, kind == "traversal" ? "../escape.exe" : "Game.exe", [1]);
                    if (kind == "collision") Put(archive, "game.exe", [2]);
                }
                await Throws(() => Packages.Extract(zip, Path.Combine(work, "extract-" + kind), default, null), kind);
            }
        });
        await Test("Library lock, version guard, remove/restore and deletion", async () =>
        {
            using var repository = Repository("library");
            await Throws(() => { using var second = new LibraryRepository(repository.DirectoryPath); return Task.CompletedTask; }, "concurrent writer");
            var app = repository.AddBuiltIn("notepad", wine); repository.Remove(app.Id); Check(repository.Load().Apps.Count == 0); Check(Directory.Exists(repository.Root(app)));
            repository.Restore(app.Id); Check(repository.Load().Apps.Single().Id == app.Id); repository.Delete(app.Id); Check(!Directory.Exists(repository.AppDirectory(app))); Check(repository.Load().RemovedApps.Count == 0);
            await Throws(() => { repository.Save(new LibraryDocument { Version = 14 }); return Task.CompletedTask; }, "newer format");
        });
        await Test("Portable import, companion files and cancellation rollback", async () =>
        {
            using var repository = Repository("imports"); var source = Path.Combine(work, "portable"); Directory.CreateDirectory(source); File.WriteAllText(Path.Combine(source, "My Game.exe"), "EXE"); File.WriteAllText(Path.Combine(source, "data.bin"), "DATA");
            var app = await repository.Import(source, "folder", "win98", wine, default);
            Check(app.Executable!.EndsWith("My Game.exe")); Check(app.WindowsVersionPending == true); Check(File.ReadAllText(SafeFiles.Beneath(repository.Root(app), LibraryRepository.DriveC + "/App/data.bin")) == "DATA");
            using var cancellation = new CancellationTokenSource(); cancellation.Cancel();
            await Throws(() => repository.Import(source, "folder", "wineDefault", wine, cancellation.Token), "cancelled import");
            Check(repository.Pending().Count == 0); Check(repository.Load().Apps.Count == 1);
            Check(Directory.EnumerateDirectories(Path.Combine(repository.DirectoryPath, "Applications")).Count() == 1);
        });
        await Test("Nested installer media and MSI launch boundaries", async () =>
        {
            using var repository = Repository("installer"); string source = Path.Combine(work, "media"); Directory.CreateDirectory(Path.Combine(source, "Disk1")); string installer = Path.Combine(source, "Disk1", "My Setup.msi"); File.WriteAllText(installer, "MSI"); File.WriteAllText(Path.Combine(source, "payload.cab"), "CAB");
            var app = await repository.Import(source, "installerFolder", "wineDefault", wine, default, installer: installer);
            var args = LaunchArguments.Build(repository, app, fixture, installing: true);
            Check(args.Contains("/mnt/installer/Disk1/My Setup.msi")); Check(args.Contains("start") && args.Contains("/wait")); Check(args[args.IndexOf("-mount") + 1] == Path.Combine(repository.AppDirectory(app), "Installer"));
            Check(args[args.IndexOf("-w") + 1] == "/mnt/installer/Disk1");
        });
        await Test("Argument validation, managed options and alternate programs", async () =>
        {
            foreach (var args in new[] { new[] { "-root", "x" }, new[] { "-cpuAffinity", "65" }, new[] { "-nosound", "/bin/sh" }, new[] { "-env", "bad name=x" }, new[] { "-scale" }, new[] { "-scale", "1", "-w", "/" } }) await Throws(() => { LaunchArguments.ValidateOverrides(args); return Task.CompletedTask; }, string.Join(' ', args));
            Check(LaunchArguments.Overrides(["-opengl", "osmesa", "-nosound"]).SequenceEqual(["-nosound"]));
            using var repository = Repository("arguments"); var app = repository.AddBuiltIn("notepad", wine); app.Arguments = ["one argument with spaces", "$(not-a-shell)"]; var launch = LaunchArguments.Build(repository, app, fixture); Check(launch.TakeLast(2).SequenceEqual(app.Arguments));
            app.WindowsVersionPending = true; await Throws(() => Task.FromResult(LaunchArguments.Build(repository, app, fixture)), "pending configuration");
        });
        await Test("Mouse sensitivity defaults, overrides, persistence and launch", async () =>
        {
            const string option = LaunchArguments.MouseSensitivityOption;
            Check(LaunchArguments.MouseSensitivity([]) == 100);
            Check(LaunchArguments.MouseSensitivity([option, "0"]) == 100);
            Check(LaunchArguments.MouseSensitivity([option, "50", option, "200"]) == 200);
            var updated = LaunchArguments.WithMouseSensitivity(["-env", "NAME=one two", option, "50", "-nosound", option, "200"], 75);
            Check(updated.SequenceEqual(["-env", "NAME=one two", "-nosound", option, "75"]));
            Check(LaunchArguments.WithMouseSensitivity(updated, 100).SequenceEqual(["-env", "NAME=one two", "-nosound"]));
            foreach (int value in new[] {1, 25, 50, 100, 200, 400, 750, 1000})
                Check(LaunchArguments.MouseSensitivity(LaunchArguments.WithMouseSensitivity([], value)) == value);
            foreach (string value in new[] {"-1", "1001", "50.5", "oops"})
                await Throws(() => { LaunchArguments.MouseSensitivity([option, value]); return Task.CompletedTask; }, "invalid sensitivity");
            await Throws(() => { LaunchArguments.MouseSensitivity([option]); return Task.CompletedTask; }, "missing sensitivity");
            using var repository = Repository("mouse-sensitivity");
            var app = repository.AddBuiltIn("notepad", wine);
            app.BoxedwineArguments = updated; repository.Update(app);
            var saved = repository.Load().Apps.Single();
            Check(LaunchArguments.MouseSensitivity(saved.BoxedwineArguments!) == 75);
            var launch = LaunchArguments.Build(repository, saved, fixture);
            Check(launch[launch.IndexOf(option) + 1] == "75" && !launch.Contains("-forceRelativeMouse"));
        });
        await Test("Backup verifies contents and restore gets a new identity", async () =>
        {
            using var repository = Repository("backups"); var saved = await Packages.Import(repository, fixture, default, null); var app = repository.AddBuiltIn("minesweeper", saved);
            app.BoxedwineArguments = LaunchArguments.WithMouseSensitivity([], 75); repository.Update(app);
            File.WriteAllText(Path.Combine(repository.Root(app), "save.dat"), "Important save"); string backup = Path.Combine(work, "test.boxedwinebackup");
            await Backups.Export(repository, app, backup, default, null); var restored = await Backups.Restore(repository, backup, default, null);
            Check(app.Id != restored.Id && restored.WinePackage == app.WinePackage); Check(File.ReadAllText(Path.Combine(repository.Root(restored), "save.dat")) == "Important save");
            Check(LaunchArguments.MouseSensitivity(restored.BoxedwineArguments!) == 75, "Backup lost mouse sensitivity");
            File.WriteAllText(Path.Combine(backup, "Application/root/save.dat"), "Changed"); await Throws(() => Backups.Restore(repository, backup, default, null), "changed backup"); Check(repository.Load().Apps.Count == 2);
        });
        await Test("Wine trial preserves source and shared package collection", async () =>
        {
            using var repository = Repository("trial"); var saved = await Packages.Import(repository, fixture, default, null); var original = repository.AddBuiltIn("notepad", saved); File.WriteAllText(Path.Combine(repository.Root(original), "save.txt"), "original");
            var trial = await repository.Trial(original, "Wine trial", saved, default, null); File.WriteAllText(Path.Combine(repository.Root(trial), "save.txt"), "test"); Check(File.ReadAllText(Path.Combine(repository.Root(original), "save.txt")) == "original");
            repository.Remove(original.Id); repository.Delete(trial.Id); repository.PruneWine(); Check(File.Exists(repository.PackagePath(saved)), "removed apps must retain Wine"); repository.Delete(original.Id); repository.PruneWine(); Check(!File.Exists(repository.PackagePath(saved)));
        });
        await Test("Shared Wine import reuses a package held by a running app", async () =>
        {
            using var repository = Repository("wine-lease"); var reference = await Packages.Import(repository, fixture, default, null);
            using var lease = new FileStream(repository.PackagePath(reference), FileMode.Open, FileAccess.Read, FileShare.Read);
            Check(await Packages.Import(repository, fixture, default, null) == reference);
            Check(Directory.GetFiles(Path.Combine(repository.DirectoryPath, "Downloads")).Length == 0);
        });
        await Test("Interrupted operation recovery verifies files and is idempotent", async () =>
        {
            using var repository = Repository("recovery"); var app = new LibraryApp { Name = "Recover me", WinePackage = wine, SavedWineVersion = wine.WineVersion }; var operation = repository.Begin(app, "import");
            Check(repository.Pending().Count == 1); operation.Entries = await Backups.Inventory(repository.AppDirectory(app), default, null); operation.Ready = true; repository.Finish(operation); Check(repository.Load().Apps.Count == 1); repository.Finish(operation); Check(repository.Load().Apps.Count == 1); repository.Discard(operation); Check(Directory.Exists(repository.Root(app)));
            var changed = new LibraryApp { Name = "Changed copy" }; var partial = repository.Begin(changed, "import"); partial.Entries = await Backups.Inventory(repository.AppDirectory(changed), default, null); partial.Ready = true;
            File.WriteAllText(Path.Combine(repository.Root(changed), "changed.txt"), "changed"); await Throws(() => { repository.Finish(partial); return Task.CompletedTask; }, "changed completed copy"); repository.Discard(partial);
            string invalid = Path.Combine(repository.DirectoryPath, "WindowsOperations", Guid.NewGuid() + ".json"); File.WriteAllText(invalid, "{invalid");
            Check(repository.Pending().Single().Problem != null); await Throws(() => { repository.PruneWine(); return Task.CompletedTask; }, "unreadable journal must preserve packages");
        });
        await Test("Interrupted backup cleanup verifies its ownership marker", async () =>
        {
            using var repository = Repository("backup-recovery"); var operation = repository.BeginExport("Partial backup", Path.Combine(work, "unfinished.boxedwinebackup"));
            string staging = repository.ExportStaging(operation); File.WriteAllText(Path.Combine(staging, "partial-file"), "copied"); repository.Discard(operation);
            Check(!Directory.Exists(staging) && repository.Pending().Count == 0);
            var changed = repository.BeginExport("Changed backup", Path.Combine(work, "changed.boxedwinebackup")); string other = repository.ExportStaging(changed);
            File.WriteAllText(Path.Combine(other, ".BoxedwineExport"), Guid.NewGuid().ToString());
            await Throws(() => { repository.Discard(changed); return Task.CompletedTask; }, "changed ownership marker"); Check(Directory.Exists(other));
        });
        await Test("Wine configuration verifies independent query output", async () =>
        {
            Check(WineConfiguration.ReadVersion("noise\r\nwin98\r\n") == "win98");
            await Throws(() => Task.FromResult(WineConfiguration.ReadVersion("win98\nwinxp")), "ambiguous output");
            Check(WineConfiguration.ReadRegistry("HKEY_CURRENT_USER\\Software\\Wine\\X11 Driver\n    UseEGL    REG_SZ    Y\n", "backend") == "egl");
            Check(WineConfiguration.ReadRegistry("HKEY_CURRENT_USER\\Software\\Wine\\Direct3D\n", "renderer") == "wineDefault");
            await Throws(() => Task.FromResult(WineConfiguration.ReadRegistry("nothing", "renderer")), "query failure");
            await Throws(() => Task.FromResult(WineConfiguration.Script("version", "win98;bad", Guid.NewGuid().ToString())), "script injection");
        });
        await Test("Window readiness requires a complete marker across pipe reads", () =>
        {
            foreach (string line in new[] { "Showing Window\n", "Showing Window\r\n" })
            {
                for (int split = 0; split <= line.Length; split++)
                {
                    var marker = new RuntimeWindowMarker();
                    bool first = marker.Consume(line.AsSpan(0, split));
                    bool second = marker.Consume(line.AsSpan(split));
                    Check(first != second, "Each split must report readiness exactly once");
                    Check(!marker.Consume(line), "Repeated windows must not signal readiness again");
                }
            }
            var strict = new RuntimeWindowMarker();
            Check(!strict.Consume("argument: Showing Window\nShowing Window later\nShowing Window\r\r\n"));
            Check(!strict.Consume(new string('x', 5 * 1024 * 1024)));
            Check(!strict.Consume("Showing Window\nShowing Window"), "An incomplete or embedded marker is not readiness");
            Check(strict.Consume("\n"));
            return Task.CompletedTask;
        });
        await Test("Installer demos select the catalog program and preserve metadata", () =>
        {
            using var repository = Repository("demo-selection"); var app = InstallerDemo();
            repository.Save(new() { Apps = [app] });
            string game = AddProgram(repository, app, "Program Files/Age of Empires/EMPIRES.EXE");
            AddProgram(repository, app, "Program Files/Age of Empires/uninstall.exe");
            AddProgram(repository, app, "WINDOWS/system32/empires.exe");
            string media = SafeFiles.Beneath(repository.AppDirectory(app), "Installer/empires.exe");
            Directory.CreateDirectory(Path.GetDirectoryName(media)!); File.WriteAllText(media, "MZsetup");
            var selected = repository.SelectingInstalledDemoProgram(app, new(0, false, true));
            Check(selected?.Executable == game && app.Executable == null && repository.Load().Apps.Single().Executable == null);
            var expected = DataFormat.Clone(app); expected.Executable = game;
            Check(JsonSerializer.Serialize(selected, DataFormat.Json) == JsonSerializer.Serialize(expected, DataFormat.Json), "Selection must preserve all other settings");
            repository.Update(selected!); Check(repository.Load().Apps.Single().Executable == game);
            app.Executable = AddProgram(repository, app, "Game/alternate.exe"); app.Name = "My custom name";
            Check(repository.SelectingInstalledDemoProgram(app, new(0, false, true))?.Executable == app.Executable, "Reinstall must preserve a valid user choice");
            app.Executable = LibraryRepository.DriveC + "/missing/empires.exe";
            var repaired = repository.SelectingInstalledDemoProgram(app, new(0, false, true));
            Check(repaired?.Executable == game && repaired.Name == app.Name);
            return Task.CompletedTask;
        });
        await Test("Missing or ambiguous demo programs still require a choice", () =>
        {
            using var repository = Repository("demo-selection-fallback"); var app = InstallerDemo();
            AddProgram(repository, app, "Game/not-empires.exe"); AddProgram(repository, app, "windows/empires.exe");
            Check(repository.SelectingInstalledDemoProgram(app, new(0, false, true)) == null);
            AddProgram(repository, app, "Game/empires.exe"); AddProgram(repository, app, "Backup/EMPIRES.EXE");
            Check(repository.SelectingInstalledDemoProgram(app, new(0, false, true)) == null);
            var candidates = new[] { new ProgramCandidate(LibraryRepository.DriveC + "/Game/Descent 3 Demo 2.exe") };
            Check(ProgramCandidate.CatalogPath(candidates, "descent 3 demo 2.EXE") == candidates[0].Path);
            Check(ProgramCandidate.CatalogPath(candidates, "Demo 2.exe") == null);
            return Task.CompletedTask;
        });
        await Test("Stopped or failed installers and manual imports do not auto-select", () =>
        {
            using var repository = Repository("demo-selection-exit"); var app = InstallerDemo();
            AddProgram(repository, app, "Game/empires.exe");
            foreach (var exit in new[] { new RuntimeExit(0, true, true), new RuntimeExit(1, false, true), new RuntimeExit(0, false, false) })
                Check(repository.SelectingInstalledDemoProgram(app, exit) == null);
            var manual = DataFormat.Clone(app); manual.Demo = null;
            Check(repository.SelectingInstalledDemoProgram(manual, new(0, false, true)) == null);
            var portable = DataFormat.Clone(app); portable.Installer = null;
            Check(repository.SelectingInstalledDemoProgram(portable, new(0, false, true)) == null);
            Check(app.Executable == null); return Task.CompletedTask;
        });
        int realIndex = Array.IndexOf(args, "--wine");
        if (realIndex >= 0)
        {
            await Test("Real Wine filesystem validation and launch checksum timing", async () =>
            {
                var watch = System.Diagnostics.Stopwatch.StartNew();
                var reference = await Packages.ValidateWine(args[realIndex + 1], default);
                double fullMilliseconds = watch.Elapsed.TotalMilliseconds;
                var checks = new WineLaunchChecks(); watch.Restart();
                Check(await checks.Verify(args[realIndex + 1], reference, default) == reference);
                double hashMilliseconds = watch.Elapsed.TotalMilliseconds; watch.Restart();
                Check(await checks.Verify(args[realIndex + 1], reference, default) == reference);
                Console.WriteLine($"Wine {reference.WineVersion}: full validation {fullMilliseconds:0} ms; SHA-256 launch check {hashMilliseconds:0} ms; shared cache {watch.Elapsed.TotalMilliseconds:0.0} ms.");
            });
        }
        int catalogIndex = Array.IndexOf(args, "--catalog");
        if (catalogIndex >= 0) await Test("Pinned demo catalog parses", () => { var demos = Demos.Load(args[catalogIndex + 1]); Check(demos.Count > 0); Console.WriteLine($"{demos.Count} demo recipes"); return Task.CompletedTask; });
        int emulatorIndex = Array.IndexOf(args, "--emulator");
        if (args.Contains("--network") && catalogIndex >= 0 && realIndex >= 0)
        {
            await Test("Pinned portable and installer demo downloads", async () =>
            {
                using var repository = Repository("network-demos");
                var reference = await Packages.Import(repository, args[realIndex + 1], default, null);
                var recipes = Demos.Load(args[catalogIndex + 1]);
                using var timeout = new CancellationTokenSource(TimeSpan.FromMinutes(3));
                foreach (var id in new[] { "bang-bang", "netsurf-3-10" })
                {
                    var recipe = recipes.Single(d => d.Origin.Id == id);
                    var app = await Demos.Install(repository, recipe, reference, timeout.Token, null);
                    Check(app.WinePackage == reference && app.Demo == recipe.Origin);
                    Check(recipe.InstallType == "Zip" ? app.Executable != null && File.Exists(SafeFiles.Beneath(repository.Root(app), app.Executable)) : app.Installer != null && File.Exists(SafeFiles.Beneath(repository.AppDirectory(app), app.Installer)));
                    Check(!Directory.Exists(Path.Combine(repository.AppDirectory(app), "Download")));
                    await Throws(() => Demos.Install(repository, recipe, reference, timeout.Token, null), "duplicate demo");
                    Console.WriteLine("Downloaded, verified and staged " + recipe.Name);
                }
                Check(repository.Pending().Count == 0 && repository.Load().Apps.Count == 2);
                var cnc = recipes.First(d => d.CncUncapped);
                string configured = Demos.ConfigureCnc("[ddraw]\r\nmaxfps=60\r\nvsync=true\r\nmaxgameticks=60\r\n", cnc);
                Check(configured.Contains("maxfps=0") && configured.Contains("vsync=false") && configured.Contains("maxgameticks=-1"));
            });
        }
        if (emulatorIndex >= 0 && realIndex >= 0 && args.Contains("--window-check"))
        {
            await Test("Real emulator reports its first visible window", async () =>
            {
                using var repository = Repository("window-readiness");
                var reference = await Packages.Import(repository, args[realIndex + 1], default, null);
                var app = repository.AddBuiltIn("notepad", reference);
                var launch = LaunchArguments.Build(repository, app, repository.PackagePath(reference));
                using var session = new RuntimeSession(args[emulatorIndex + 1], launch, repository.PackagePath(reference), Path.Combine(repository.AppDirectory(app), "Logs", "latest.log"));
                try
                {
                    Check(await session.WindowShown.WaitAsync(TimeSpan.FromSeconds(45)), "Notepad must report a visible window");
                    Check(!session.Completion.IsCompleted, "Notepad should still be running after showing its window");
                    await session.Stop();
                    var exit = await session.Completion;
                    Check(exit.Stopped && (exit.Code == 0 || exit.Code == -1), $"Visible Notepad did not stop cleanly or through the timeout fallback: {exit}");
                    Console.WriteLine(exit.Code == 0 ? "Visible Notepad stopped gracefully." : "Visible Notepad needed the five-second forced-stop fallback.");
                }
                finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
            });
        }
        else if (emulatorIndex >= 0 && realIndex >= 0)
        {
            await Test("Real emulator configures and verifies Wine settings", async () =>
            {
                using var repository = Repository("real-emulator");
                var reference = await Packages.Import(repository, args[realIndex + 1], default, null);
                var app = repository.AddBuiltIn("notepad", reference); app.ChooseWindows("win98"); app.ChooseRenderer("gdi"); app.ChooseBackend("glx"); repository.Update(app);
                var configuration = new WineConfiguration(args[emulatorIndex + 1]);
                using var timeout = new CancellationTokenSource(TimeSpan.FromMinutes(3));
                app = await configuration.Apply(repository, app, repository.PackagePath(reference), timeout.Token, null);
                Check(!app.HasPendingSettings); Check(!repository.Load().Apps.Single().HasPendingSettings);
                app.ChooseWindows("wineDefault"); app.ChooseBackend("wineDefault"); app.ChooseRenderer("wineDefault"); repository.Update(app);
                app = await configuration.Apply(repository, app, repository.PackagePath(reference), timeout.Token, null); Check(!app.HasPendingSettings);
                Console.WriteLine("Wine settings applied, independently queried, reset to defaults and persisted.");
                string log = Path.Combine(repository.AppDirectory(app), "Logs", "latest.log");
                List<string> launch = ["-root", repository.Root(app), "-zip", repository.PackagePath(reference), "-hideWindow", "/bin/wine", "cmd", "/c", "echo BOXEDWINE_NATIVE_UI_OK"];
                using var session = new RuntimeSession(args[emulatorIndex + 1], launch, repository.PackagePath(reference), log);
                RuntimeExit exit;
                try { exit = await session.Completion.WaitAsync(TimeSpan.FromSeconds(45)); }
                catch { session.ForceStop(); await session.Completion; throw; }
                Check(exit.Code == 0, "Wine cmd returned a failure"); Check(RuntimeSession.PlainText(File.ReadAllText(log)).Split('\n').Any(line => line.Trim() == "BOXEDWINE_NATIVE_UI_OK"), "The marker must come from Wine output, not the logged command line");
                var notepadArguments = LaunchArguments.Build(repository, app, repository.PackagePath(reference)); notepadArguments.Insert(0, "-hideWindow");
                using var notepad = new RuntimeSession(args[emulatorIndex + 1], notepadArguments, repository.PackagePath(reference), log);
                await Task.Delay(4000); Check(!notepad.Completion.IsCompleted, "Notepad should stay open until stopped");
                await notepad.Stop(); Check((await notepad.Completion).Stopped, "Stop must report an intentional shutdown");
                Check((await notepad.Completion).Code == 0, "The native engine should accept a graceful stop over stdin");
                Console.WriteLine("Wine cmd and hidden Notepad launch/stop succeeded.");
            });
        }
        Console.WriteLine($"\n{passes} passed, {Failures} failed.");
    }
}
