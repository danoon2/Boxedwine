// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Diagnostics;
using System.Reflection.PortableExecutable;
using System.Runtime.InteropServices;

namespace Boxedwine.Library;

// These are host drivers, independent of Wine's GLX/EGL and Direct3D settings.
public sealed record OpenGLRuntime(string Implementation, string? LibraryPath = null)
{
    public void Configure(ProcessStartInfo start)
    {
        start.Environment.Remove("GALLIUM_DRIVER");
        if (LibraryPath == null) return;
        start.ArgumentList.Add("-opengl"); start.ArgumentList.Add(LibraryPath);
        start.Environment["GALLIUM_DRIVER"] = Implementation;
        // Mesa's OpenGL DLL loads companion DLLs from the same package.
        start.Environment.TryGetValue("PATH", out string? path);
        start.Environment["PATH"] = Path.GetDirectoryName(LibraryPath) + Path.PathSeparator + path;
    }
}

internal sealed record OpenGLPackage(Architecture Architecture, string Version, long Bytes, string Sha256)
{
    public string FileName => $"mesa_{Version}_Windows_{(Architecture == Architecture.Arm64 ? "Armv8" : "x64")}.zip";
    public string Folder => $"mesa-{Version}-{Architecture.ToString().ToLowerInvariant()}";
    public string Library(string implementation) => Architecture == Architecture.Arm64
        ? $"Armv8_v2/{(implementation == "d3d12" ? "d3d12" : "llvm")}/opengl32.dll" : "x64/opengl32.dll";
    public string[] RequiredLibraries => Architecture == Architecture.Arm64
        ? [Library("llvmpipe"), Library("d3d12")] : [Library("llvmpipe"), "x64/libgallium_wgl.dll", "x64/dxil.dll"];
}

public static class OpenGLDrivers
{
    public static string Help(Architecture architecture)
    {
        const string native = "Native uses your graphics card's OpenGL driver.";
        if (architecture is not (Architecture.X64 or Architecture.Arm64)) return native;
        string renderers = architecture == Architecture.X64
            ? "software rendering (LLVMpipe), Direct3D 12, or Vulkan (Zink)"
            : "software rendering (LLVMpipe) or Direct3D 12";
        return native + $" Mesa offers {renderers}. Mesa is downloaded once when first needed. Hardware acceleration requires a compatible graphics driver.";
    }
    public static bool IsKnown(string value) => value is "native" or "llvmpipe" or "d3d12" or "zink";
    public static string Label(string value) => value switch
    {
        "native" => "Native", "llvmpipe" => "Software — Mesa LLVMpipe", "d3d12" => "OpenGL on Direct3D 12",
        "zink" => "OpenGL on Vulkan — Zink", _ => throw new InvalidDataException("Unknown OpenGL implementation.")
    };
    public static Dictionary<string, string> Choices(Architecture architecture, string? selected = null)
    {
        var choices = new Dictionary<string, string> { ["native"] = Label("native") };
        if (architecture is Architecture.X64 or Architecture.Arm64)
        {
            choices.Add("llvmpipe", Label("llvmpipe")); choices.Add("d3d12", Label("d3d12"));
            if (architecture == Architecture.X64) choices.Add("zink", Label("zink"));
        }
        // Keep a setting from a backup or another engine visible so the user can change it.
        if (selected != null && IsKnown(selected) && !choices.ContainsKey(selected)) choices.Add(selected, Label(selected) + " (unavailable for this engine)");
        return choices;
    }
    // The legacy Arm default resolves to Armv8_v2/llvm/opengl32.dll.
    public static string GlobalDefault(LauncherPreferences preferences, Architecture architecture) =>
        preferences.OpenGLImplementation ?? (architecture == Architecture.Arm64 ? "llvmpipe" : "native");
    public static string Resolve(LibraryApp app, LauncherPreferences preferences, Architecture architecture)
    {
        string selected = app.PreferredWindowsOpenGL;
        if (selected == "default") selected = GlobalDefault(preferences, architecture);
        if (!IsKnown(selected)) throw new InvalidDataException("Choose a supported OpenGL implementation in Settings or App Settings → Advanced.");
        return selected;
    }
    public static Architecture EngineArchitecture(string path)
    {
        SafeFiles.NoLinks(path);
        using var file = File.OpenRead(path); using var pe = new PEReader(file);
        return pe.PEHeaders.CoffHeader.Machine switch
        {
            Machine.Amd64 => Architecture.X64, Machine.Arm64 => Architecture.Arm64, Machine.I386 => Architecture.X86,
            _ => throw new InvalidDataException("The selected emulator has an unsupported Windows architecture.")
        };
    }
    internal static OpenGLPackage Package(Architecture architecture) => architecture switch
    {
        // Same releases as the legacy Windows UI, pinned before loading native code.
        Architecture.X64 => new(architecture, "25.0.0", 42898640, "c047f102d724c871f7bf0720d8944d9b9e45f3c32f6db6645652af533993b48a"),
        Architecture.Arm64 => new(architecture, "26.0.3", 20945142, "a83978b94a2ed2a5ca6fff5d4e57d97c4d32c92dd22be2904478023d788557f0"),
        _ => throw new InvalidDataException("Mesa downloads require an x64 or Arm64 emulator. Choose Native in the OpenGL settings.")
    };
    public static async Task<OpenGLRuntime> Ensure(string library, string implementation, string emulator, CancellationToken cancellation, IProgress<OperationProgress>? progress = null)
    {
        cancellation.ThrowIfCancellationRequested();
        if (!IsKnown(implementation)) throw new InvalidDataException("Unknown OpenGL implementation.");
        if (implementation == "native") return new("native");
        var architecture = EngineArchitecture(emulator);
        if (!Choices(architecture).ContainsKey(implementation)) throw new InvalidDataException($"{Label(implementation)} is unavailable for the {architecture} emulator. Choose another OpenGL implementation in Settings or App Settings → Advanced.");
        return await EnsurePackage(library, implementation, Package(architecture), cancellation, progress,
            (package, zip, token, report) => Packages.Download("https://boxedwine.org/v2/OpenGL/" + package.FileName, package.Bytes, package.Sha256, zip, token, report));
    }
    internal sealed record InstalledFile(string Path, long Bytes);
    internal sealed record Installation(string Sha256, List<InstalledFile> Files);
    internal static async Task<OpenGLRuntime> EnsurePackage(string library, string implementation, OpenGLPackage package, CancellationToken cancellation,
        IProgress<OperationProgress>? progress, Func<OpenGLPackage, string, CancellationToken, IProgress<OperationProgress>?, Task> download)
    {
        cancellation.ThrowIfCancellationRequested();
        string cache = SafeFiles.Beneath(library, "OpenGL"), destination = SafeFiles.Beneath(cache, package.Folder);
        string dll = SafeFiles.Beneath(destination, package.Library(implementation));
        if (Directory.Exists(destination))
        {
            string marker = SafeFiles.Beneath(destination, "installed.json");
            var installed = File.Exists(marker) ? DataFormat.Read<Installation>(marker, 1024 * 1024) : null;
            if (installed?.Sha256 != package.Sha256 || installed.Files.Count == 0 ||
                package.RequiredLibraries.Any(p => !installed.Files.Any(f => f.Path == p)) ||
                installed.Files.Any(f => !File.Exists(SafeFiles.Beneath(destination, f.Path)) || new FileInfo(SafeFiles.Beneath(destination, f.Path)).Length != f.Bytes))
                throw new IOException("The downloaded OpenGL package is incomplete. Close running apps, remove this folder and launch again to download it:\n\n" + destination);
            return new(implementation, dll);
        }
        Directory.CreateDirectory(cache);
        string temporary = SafeFiles.Beneath(cache, ".install-" + Guid.NewGuid().ToString("N")), archive = temporary + ".zip";
        try
        {
            progress?.Report(new("Downloading " + Label(implementation) + "…"));
            await download(package, archive, cancellation, progress);
            if (new FileInfo(archive).Length != package.Bytes || await SafeFiles.Hash(archive, cancellation) != package.Sha256) throw new InvalidDataException("The OpenGL download failed its checksum or size check.");
            await Packages.Extract(archive, temporary, cancellation, progress);
            foreach (string required in package.RequiredLibraries)
                if (EngineArchitecture(SafeFiles.Beneath(temporary, required)) != package.Architecture) throw new InvalidDataException("The OpenGL download contains a library for the wrong architecture.");
            var files = SafeFiles.Tree(temporary).Where(File.Exists).Select(p => new InstalledFile(Path.GetRelativePath(temporary, p).Replace('\\', '/'), new FileInfo(p).Length)).ToList();
            SafeFiles.AtomicJson(SafeFiles.Beneath(temporary, "installed.json"), new Installation(package.Sha256, files));
            cancellation.ThrowIfCancellationRequested();
            Directory.Move(temporary, destination);
            return new(implementation, dll);
        }
        finally
        {
            if (File.Exists(archive)) File.Delete(archive);
            SafeFiles.DeleteTree(temporary, cache);
        }
    }
}
