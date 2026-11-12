// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;
using System.Text.Json;

sealed partial class TestSuite
{
    private async Task WineToolTests(string[] args)
    {
        await Test("Wine tools use the app environment without changing its program or passing game arguments", async () =>
        {
            using var repository = Repository("wine-tools");
            var app = repository.AddBuiltIn("notepad", wine);
            app.IsNotepad = false; app.BuiltInProgram = null;
            app.Arguments = ["game-only argument"];
            app.BoxedwineArguments = ["-nosound"];
            app.DemoSettings = new() { BitsPerPixel = 16, CncDDraw = true };
            repository.Update(app);
            string before = JsonSerializer.Serialize(repository.Load(), DataFormat.Json);
            var commands = new Dictionary<WineTool, string[]>
            {
                [WineTool.Winecfg] = ["winecfg"], [WineTool.Regedit] = ["regedit"], [WineTool.Cmd] = ["wineconsole", "cmd"],
                [WineTool.Explorer] = ["explorer.exe", "C:\\"], [WineTool.Uninstaller] = ["uninstaller"],
                [WineTool.IExplore] = ["iexplore", "about:blank"], [WineTool.Notepad] = ["notepad"]
            };
            foreach (var (tool, command) in commands)
            {
                var launch = LaunchArguments.Build(repository, app, fixture, tool: tool);
                Check(launch.Take(4).SequenceEqual(["-root", repository.Root(app), "-zip", fixture]));
                Check(launch.Skip(launch.IndexOf("/bin/wine") + 1).SequenceEqual(command));
                Check(launch[launch.IndexOf("-w") + 1] == "/home/username");
                Check(launch[launch.IndexOf("-title") + 1] == WineTools.Get(tool).Name + " · " + app.Name);
                Check(launch.Contains("-nosound") && !launch.Contains("game-only argument") && !launch.Contains("-bpp") && !launch.Contains("-ddrawOverride"));
                Check(!launch.Contains("-mount") && !launch.Contains("/bin/sh"));
            }
            Check(JsonSerializer.Serialize(repository.Load(), DataFormat.Json) == before);
            Check(WineTools.All.Count == commands.Count && WineTools.All.Single(t => t.Matches("REGEDIT")).Tool == WineTool.Regedit);
            app.Executable = LibraryRepository.DriveC + "/missing-game.exe";
            Check(LaunchArguments.Build(repository, app, fixture, tool: WineTool.Regedit).Last() == "regedit");
            app.BuiltInProgram = "minesweeper";
            Check(LaunchArguments.Build(repository, app, fixture, tool: WineTool.Winecfg).Last() == "winecfg");
            foreach (var kind in new[] { "install", "alternate", "external", "pending", "invalid" })
            {
                app.WindowsVersionPending = kind == "pending" ? true : null;
                await Throws(() => Task.FromResult(LaunchArguments.Build(repository, app, fixture,
                    installing: kind == "install", alternate: kind == "alternate" ? app.Executable : null,
                    external: kind == "external" ? "setup.exe" : null, tool: kind == "invalid" ? (WineTool)999 : WineTool.Regedit)), kind);
            }
        });

        if (!args.Contains("--wine-tools-smoke")) return;
        await Test("Real Wine tools open and stop in an isolated app environment", async () =>
        {
            string package = args[Array.IndexOf(args, "--wine") + 1], engine = args[Array.IndexOf(args, "--emulator") + 1];
            using var repository = Repository("wine-tools-runtime");
            var reference = await Packages.Import(repository, package, default, null);
            var app = repository.AddBuiltIn("notepad", reference);
            foreach (var tool in WineTools.All)
            {
                var launch = LaunchArguments.Build(repository, app, repository.PackagePath(reference), tool: tool.Tool);
                using var session = new RuntimeSession(engine, launch, repository.PackagePath(reference), Path.Combine(repository.AppDirectory(app), "Logs", tool.Id + ".log"));
                try
                {
                    Check(await session.WindowShown.WaitAsync(TimeSpan.FromSeconds(60)), tool.Name + " did not show a window");
                    Check(!session.Completion.IsCompleted, tool.Name + " exited unexpectedly");
                    await session.Stop();
                    Check((await session.Completion).Stopped, tool.Name + " did not stop");
                    Console.WriteLine("Opened and stopped " + tool.Name);
                }
                finally { if (!session.Completion.IsCompleted) { session.ForceStop(); await session.Completion; } }
            }
        });
    }
}
