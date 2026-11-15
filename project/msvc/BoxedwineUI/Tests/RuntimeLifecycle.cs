// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;
using System.Diagnostics;

sealed partial class TestSuite
{
    private static readonly TimeSpan StopTestDeadline = TimeSpan.FromSeconds(40);

    private async Task RuntimeShutdownTests()
    {
        await Test("Stop allows cleanup lasting longer than five seconds", async () =>
        {
            string executable = Environment.ProcessPath!;
            List<string> arguments = ["--shutdown-fixture"];
            if (Path.GetFileNameWithoutExtension(executable).Equals("dotnet", StringComparison.OrdinalIgnoreCase))
                arguments.Insert(0, typeof(TestSuite).Assembly.Location);
            string log = Path.Combine(work, "delayed-shutdown.log");
            using var session = new RuntimeSession(executable, arguments, fixture, log);
            try
            {
                Check(await session.WindowShown.WaitAsync(TimeSpan.FromSeconds(15)), "The test child did not become ready");
                await session.Stop().WaitAsync(StopTestDeadline);
                var exit = await session.Completion;
                Check(exit.Stopped && exit.Code == 0, $"Cleanup was interrupted: {exit}");
                Check(File.ReadAllLines(log).Contains("Shutdown cleanup complete"), "The child must finish cleanup before exiting");
            }
            finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
        });
    }

    private async Task RuntimeLifecycleTests(string[] args)
    {
        int engineIndex = Array.IndexOf(args, "--emulator"), wineIndex = Array.IndexOf(args, "--wine");
        if (engineIndex < 0 || wineIndex < 0) return;
        string engine = args[engineIndex + 1], package = args[wineIndex + 1];

        await Test("Real emulator cancels startup before its first window", async () =>
        {
            using var repository = Repository("startup-cancellation");
            var reference = await Packages.Import(repository, package, default, null);
            var app = repository.AddBuiltIn("notepad", reference);
            var launch = LaunchArguments.Build(repository, app, repository.PackagePath(reference));
            using var session = new RuntimeSession(engine, launch, repository.PackagePath(reference), Path.Combine(repository.AppDirectory(app), "Logs", "latest.log"));
            try
            {
                // Cancel a fresh prefix immediately, without guessing how long
                // Wine startup takes or hiding the readiness signal.
                Check(!session.WindowShown.IsCompleted && !session.Completion.IsCompleted, "Expected a launch still in progress");
                var elapsed = Stopwatch.StartNew();
                await session.Stop().WaitAsync(StopTestDeadline);
                var exit = await session.Completion;
                Check(!await session.WindowShown, "A cancelled startup must not report a ready window");
                Check(exit.Stopped && exit.Code == 0, $"Startup cancellation did not exit cleanly: {exit}");
                Console.WriteLine($"Startup cancelled cleanly in {elapsed.Elapsed.TotalSeconds:F2}s.");
            }
            finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
        });

        if (args.Contains("--window-check"))
        {
            await Test("Real emulator reports its first desktop frame", async () =>
            {
                using var repository = Repository("window-readiness");
                var reference = await Packages.Import(repository, package, default, null);
                var app = repository.AddBuiltIn("notepad", reference);
                var launch = LaunchArguments.Build(repository, app, repository.PackagePath(reference));
                using var session = new RuntimeSession(engine, launch, repository.PackagePath(reference), Path.Combine(repository.AppDirectory(app), "Logs", "latest.log"));
                try
                {
                    Check(await session.WindowShown.WaitAsync(TimeSpan.FromSeconds(90)), "The engine must report its visible desktop");
                    Check(!session.Completion.IsCompleted, "The launch exited before cancellation");
                    await session.Stop().WaitAsync(StopTestDeadline);
                    var exit = await session.Completion;
                    // The Wine desktop-close timer fix must let startup
                    // cancellation finish without the launcher's force-stop.
                    Check(exit.Stopped && exit.Code == 0, $"First-frame cancellation did not exit cleanly: {exit}");
                    Console.WriteLine("First-frame cancellation exited normally.");
                }
                finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
            });
        }

        await Test("Real emulator shuts down cleanly after Wine startup completes", async () =>
        {
            using var repository = Repository("clean-shutdown");
            var reference = await Packages.Import(repository, package, default, null);
            var app = repository.AddBuiltIn("notepad", reference);
            // The first SDL frame can precede app startup. Use a guest-written
            // marker instead: cscript has to finish starting before it can write
            // this file and wait. It requires no extra compiled test executable.
            string drive = SafeFiles.Beneath(repository.Root(app), LibraryRepository.DriveC);
            Directory.CreateDirectory(drive);
            string ready = Path.Combine(drive, "shutdown-ready.txt");
            File.WriteAllText(Path.Combine(drive, "shutdown.js"), """
                var marker = new ActiveXObject("Scripting.FileSystemObject").CreateTextFile("C:\\shutdown-ready.txt", true);
                marker.WriteLine("ready");
                marker.Close();
                WScript.Sleep(600000);
                """);
            List<string> launch = ["-root", repository.Root(app), "-zip", repository.PackagePath(reference), "-hideWindow", "/bin/wine", "cscript", "//nologo", @"C:\shutdown.js"];
            using var session = new RuntimeSession(engine, launch, repository.PackagePath(reference), Path.Combine(repository.AppDirectory(app), "Logs", "latest.log"));
            try
            {
                var startup = Stopwatch.StartNew();
                while (!File.Exists(ready) && !session.Completion.IsCompleted && startup.Elapsed < TimeSpan.FromMinutes(2))
                    await Task.Delay(100);
                Check(File.Exists(ready), "Wine must execute the script and write its ready marker before shutdown is tested");
                Check(!session.Completion.IsCompleted, "The ready Wine script exited before the stop request");
                var elapsed = Stopwatch.StartNew();
                await session.Stop().WaitAsync(StopTestDeadline);
                var exit = await session.Completion;
                Check(exit.Stopped && exit.Code == 0, $"Ready Wine app did not shut down cleanly: {exit}");
                Console.WriteLine($"Ready Wine app stopped cleanly in {elapsed.Elapsed.TotalSeconds:F2}s.");
            }
            finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
        });
    }
}
