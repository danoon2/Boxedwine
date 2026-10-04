// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;
using System.Diagnostics;
using System.IO.Compression;
using System.Runtime.InteropServices;
using System.Text.Json;

sealed partial class TestSuite
{
    private static byte[] PortableExecutable(Architecture architecture)
    {
        // Minimal PE/COFF header; architecture checks never execute this fixture.
        byte[] bytes = new byte[512]; bytes[0] = (byte)'M'; bytes[1] = (byte)'Z'; bytes[60] = 64;
        bytes[64] = (byte)'P'; bytes[65] = (byte)'E';
        BitConverter.GetBytes((ushort)(architecture == Architecture.Arm64 ? 0xaa64 : architecture == Architecture.X64 ? 0x8664 : 0x014c)).CopyTo(bytes, 68);
        BitConverter.GetBytes((ushort)(architecture == Architecture.X86 ? 224 : 240)).CopyTo(bytes, 84);
        BitConverter.GetBytes((ushort)(architecture == Architecture.X86 ? 0x10b : 0x20b)).CopyTo(bytes, 88);
        return bytes;
    }
    private async Task OpenGLTests(string[] args)
    {
        await Test("OpenGL defaults, demo preferences, overrides and persistence", async () =>
        {
            using var repository = Repository("opengl-settings");
            var app = repository.AddBuiltIn("notepad", wine);
            var preferences = JsonSerializer.Deserialize<LauncherPreferences>("{\"theme\":\"Dark\"}", DataFormat.Json)!;
            Check(OpenGLDrivers.Resolve(app, preferences, Architecture.X64) == "native");
            Check(OpenGLDrivers.Resolve(app, preferences, Architecture.Arm64) == "llvmpipe");
            preferences.OpenGLImplementation = "d3d12"; repository.SavePreferences(preferences);
            Check(OpenGLDrivers.Resolve(app, repository.Preferences, Architecture.Arm64) == "d3d12");
            app.DemoSettings = new() { NativeOpenGL = true };
            Check(OpenGLDrivers.Resolve(app, preferences, Architecture.Arm64) == "native", "Keep catalog native OpenGL recommendations");
            app.WindowsOpenGL = "default";
            Check(OpenGLDrivers.Resolve(app, preferences, Architecture.Arm64) == "d3d12", "Explicit inheritance overrides a demo recommendation");
            app.WindowsOpenGL = "llvmpipe"; repository.Update(app);
            Check(OpenGLDrivers.Resolve(repository.Load().Apps.Single(), preferences, Architecture.X64) == "llvmpipe");
            var savedWine = await Packages.Import(repository, fixture, default, null);
            app.WinePackage = savedWine; repository.Update(app);
            string backup = Path.Combine(work, "opengl.boxedwinebackup"); await Backups.Export(repository, app, backup, default, null);
            Check((await Backups.Restore(repository, backup, default, null)).WindowsOpenGL == "llvmpipe", "Backups preserve the per-app choice");
            app.WindowsOpenGL = "untrusted.dll"; await Throws(() => { repository.Update(app); return Task.CompletedTask; }, "arbitrary host library paths");
        });
        await Test("OpenGL engine architecture and supported choices", async () =>
        {
            foreach (var architecture in new[] { Architecture.X64, Architecture.Arm64, Architecture.X86 })
            {
                string engine = Path.Combine(work, architecture + ".exe"); File.WriteAllBytes(engine, PortableExecutable(architecture));
                Check(OpenGLDrivers.EngineArchitecture(engine) == architecture);
                Check(OpenGLDrivers.Choices(architecture).ContainsKey("zink") == (architecture == Architecture.X64));
                if (architecture != Architecture.X64)
                    await Throws(() => OpenGLDrivers.Ensure(work, "zink", engine, default), "unsupported Mesa backend before downloading");
            }
            Check(OpenGLDrivers.Choices(Architecture.Arm64, "zink")["zink"].Contains("unavailable"));
            Check((await OpenGLDrivers.Ensure(work, "native", "unused", default)).LibraryPath == null, "Native never downloads");
        });
        await Test("OpenGL flags and driver environment are isolated per launch", () =>
        {
            string? parent = Environment.GetEnvironmentVariable("GALLIUM_DRIVER");
            string library = Path.Combine(work, "Mesa with spaces", "opengl32.dll");
            var mesa = new ProcessStartInfo(); mesa.Environment["PATH"] = "original path";
            new OpenGLRuntime("d3d12", library).Configure(mesa);
            Check(mesa.ArgumentList.SequenceEqual(new[] { "-opengl", library }));
            Check(mesa.Environment["GALLIUM_DRIVER"] == "d3d12");
            Check(mesa.Environment["PATH"] == Path.GetDirectoryName(library) + Path.PathSeparator + "original path");
            var native = new ProcessStartInfo(); native.Environment["GALLIUM_DRIVER"] = "zink";
            new OpenGLRuntime("native").Configure(native);
            Check(!native.Environment.ContainsKey("GALLIUM_DRIVER") && native.ArgumentList.Count == 0);
            Check(mesa.Environment["GALLIUM_DRIVER"] == "d3d12" && parent == Environment.GetEnvironmentVariable("GALLIUM_DRIVER"));
            return Task.CompletedTask;
        });
        await Test("Mesa verified installation, shared cache, cancellation and rollback", async () =>
        {
            foreach (var architecture in new[] { Architecture.X64, Architecture.Arm64 })
            {
                using var repository = Repository("mesa-" + architecture);
                var package = OpenGLDrivers.Package(architecture) with { Version = "test" };
                string zip = Path.Combine(work, architecture + "-mesa.zip");
                using (var archive = ZipFile.Open(zip, ZipArchiveMode.Create))
                    foreach (string name in package.RequiredLibraries) Put(archive, name, PortableExecutable(architecture));
                package = package with { Bytes = new FileInfo(zip).Length, Sha256 = await SafeFiles.Hash(zip) };
                int downloads = 0;
                Task Download(OpenGLPackage _, string target, CancellationToken token, IProgress<OperationProgress>? progress) { downloads++; return SafeFiles.CopyFile(zip, target, token); }
                using var cancelled = new CancellationTokenSource(); cancelled.Cancel();
                await Throws(() => OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "llvmpipe", package, cancelled.Token, null, Download), "cancel before download");
                Check(downloads == 0);
                await Throws(() => OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "llvmpipe", package with { Sha256 = new string('a', 64) }, default, null, Download), "checksum mismatch");
                Check(!Directory.EnumerateFileSystemEntries(Path.Combine(repository.DirectoryPath, "OpenGL")).Any(), "Failed downloads must leave no cache or temporary files");
                using var during = new CancellationTokenSource();
                await Throws(() => OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "llvmpipe", package, during.Token, null, async (_, target, token, _) => { await SafeFiles.CopyFile(zip, target, token); during.Cancel(); }), "cancel after download");
                Check(!Directory.EnumerateFileSystemEntries(Path.Combine(repository.DirectoryPath, "OpenGL")).Any());
                var first = await OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "llvmpipe", package, default, null, Download);
                var second = await OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "d3d12", package, default, null, Download);
                Check(downloads == 2 && File.Exists(first.LibraryPath) && File.Exists(second.LibraryPath), "Both backends reuse one download");
                Check((first.LibraryPath == second.LibraryPath) == (architecture == Architecture.X64), "Arm uses separate LLVMpipe/D3D12 libraries");
                File.Delete(first.LibraryPath!);
                await Throws(() => OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "llvmpipe", package, default, null, Download), "incomplete cache");
            }
        });
        await Test("Mesa rejects unsafe archives and libraries for another architecture", async () =>
        {
            foreach (string problem in new[] { "path", "architecture" })
            {
                using var repository = Repository("mesa-invalid-" + problem);
                var package = OpenGLDrivers.Package(Architecture.Arm64) with { Version = "test" };
                string zip = Path.Combine(work, "mesa-invalid-" + problem + ".zip");
                using (var archive = ZipFile.Open(zip, ZipArchiveMode.Create))
                {
                    foreach (string name in package.RequiredLibraries) Put(archive, name, PortableExecutable(Architecture.X64));
                    if (problem == "path") Put(archive, "../outside.dll", [1]);
                }
                package = package with { Bytes = new FileInfo(zip).Length, Sha256 = await SafeFiles.Hash(zip) };
                await Throws(() => OpenGLDrivers.EnsurePackage(repository.DirectoryPath, "d3d12", package, default, null, (_, target, token, _) => SafeFiles.CopyFile(zip, target, token)), problem);
                Check(!Directory.EnumerateFileSystemEntries(Path.Combine(repository.DirectoryPath, "OpenGL")).Any());
            }
        });
        int packages = Array.IndexOf(args, "--opengl-packages");
        if (packages >= 0) await Test("Release Mesa archives match pins, layout and PE architecture", async () =>
        {
            foreach (var architecture in new[] { Architecture.X64, Architecture.Arm64 })
            {
                using var repository = Repository("mesa-release-" + architecture);
                var package = OpenGLDrivers.Package(architecture);
                string zip = Path.Combine(args[packages + 1], package.FileName);
                Task Download(OpenGLPackage _, string target, CancellationToken token, IProgress<OperationProgress>? report) => SafeFiles.CopyFile(zip, target, token);
                foreach (string implementation in OpenGLDrivers.Choices(architecture).Keys.Where(k => k != "native"))
                {
                    var ready = await OpenGLDrivers.EnsurePackage(repository.DirectoryPath, implementation, package, default, null, Download);
                    Check(OpenGLDrivers.EngineArchitecture(ready.LibraryPath!) == architecture);
                }
            }
        });
    }
}
