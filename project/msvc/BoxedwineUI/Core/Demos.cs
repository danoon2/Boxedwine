// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml;
using System.Xml.Linq;

namespace Boxedwine.Library;

public sealed record DemoRecipe(DemoOrigin Origin, string Name, string Summary, string Help, string Icon, string Url, long Bytes, string InstallType,
    string? InstallExe, string WineVersion, DemoSettings Settings, string? Glide, string? CncRenderer, bool CncUncapped, string? CncMode);
public static class Demos
{
    public static List<DemoRecipe> Load(string path)
    {
        SafeFiles.NoLinks(path);
        using var reader = XmlReader.Create(path, new XmlReaderSettings { DtdProcessing = DtdProcessing.Prohibit, XmlResolver = null, MaxCharactersInDocument = 1024 * 1024 });
        var xml = XDocument.Load(reader); var root = xml.Root ?? throw new InvalidDataException("The demo catalog is empty.");
        if (root.Name != "XML" || !int.TryParse((string?)root.Attribute("schemaVersion"), out int schema) || schema is < 1 or > 7 || string.IsNullOrEmpty((string?)root.Attribute("release"))) throw new InvalidDataException("Unsupported demo catalog version.");
        string release = (string)root.Attribute("release")!;
        var recipes = new List<DemoRecipe>(); var ids = new HashSet<string>();
        foreach (var item in root.Elements())
        {
            if (item.Name != "Demo" || recipes.Count >= 256 || item.Elements().Select(e => e.Name).Distinct().Count() != item.Elements().Count()) throw new InvalidDataException("Invalid or duplicate demo fields.");
            string Field(string key) => ((string?)item.Element(key) ?? "").Trim();
            string? Optional(string key) => item.Element(key) == null ? null : Field(key);
            bool? Boolean(string key) => Optional(key) is string value ? value switch { "true" => true, "false" => false, _ => throw new InvalidDataException("Invalid demo boolean: " + key) } : null;
            int? Number(string key) => Optional(key) is string value ? int.Parse(value) : null;
            string id = Field("ID"), name = Field("Name"), hash = Field("FileSHA256"), shortcut = Field("ShortcutExe"), url = Field("FileURL");
            if (!Regex.IsMatch(id, "^[a-z0-9][a-z0-9-]{0,63}$") || !ids.Add(id) || name.Length is 0 or > 256 || !Regex.IsMatch(hash, "^[a-f0-9]{64}$") ||
                shortcut.Contains('/') || !shortcut.EndsWith(".exe", StringComparison.OrdinalIgnoreCase) || !Uri.TryCreate(url, UriKind.Absolute, out var address) || !Packages.TrustedUrl(address) ||
                !long.TryParse(Field("FileSizeBytes"), out long bytes) || bytes is <= 0 or > 1073741824 || Field("InstallType") is not ("Zip" or "Installer")) throw new InvalidDataException("Invalid demo identity, package, or install recipe.");
            _ = SafeFiles.Beneath(Path.GetDirectoryName(path)!, shortcut);
            if (Field("Icon").Length > 0) _ = SafeFiles.Beneath(Path.GetDirectoryName(path)!, Field("Icon"));
            foreach (var unsupported in new[] { "Options", "InstallOptions", "Options_Mac", "InstallOptions_Mac" }) if (Field(unsupported).Length > 0) throw new InvalidDataException("Unsupported legacy options in demo " + name);
            var settings = new DemoSettings { WindowsVersion = Optional("WindowsVersion"), Gdi = Boolean("GDIRenderer"), Resolution = Optional("Resolution"), InstallResolution = Optional("InstallResolution"), BitsPerPixel = Number("BitsPerPixel"), CpuCount = Number("CPUCount"), NativeOpenGL = Boolean("NativeOpenGL"), UseEGL = Boolean("UseEGL"), CncDDraw = Boolean("CNCDDraw"), DisableHideCursor = Boolean("DisableHideCursor"), ForceRelativeMouse = Boolean("ForceRelativeMouse") };
            if (settings.Resolution != null) LaunchArguments.ValidateResolution(settings.Resolution);
            if (settings.InstallResolution != null) LaunchArguments.ValidateResolution(settings.InstallResolution);
            if (settings.WindowsVersion is not (null or "win98" or "winxp") || settings.NativeOpenGL == false || settings.BitsPerPixel is int b && b is not (8 or 16 or 32) || settings.CpuCount is int c && c is < 1 or > 64) throw new InvalidDataException("Unsupported demo display, Windows, or CPU setting.");
            if (Field("InstallType") == "Zip" && !address.LocalPath.EndsWith(".zip", StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("A portable demo requires a ZIP package.");
            if (Field("InstallType") == "Installer" && address.LocalPath.EndsWith(".zip", StringComparison.OrdinalIgnoreCase)) _ = SafeFiles.Beneath(Path.GetDirectoryName(path)!, Field("InstallExe"));
            if (Optional("Glide") is string glide && glide != "psVoodoo") throw new InvalidDataException("Unsupported Glide provider.");
            if (Optional("CNCDDrawRenderer") is string renderer && renderer is not ("gdi" or "opengl" or "direct3d9")) throw new InvalidDataException("Unknown CNC DDraw renderer.");
            if (Optional("CNCDDrawFakeMode") is string mode && !Regex.IsMatch(mode, "^[1-9][0-9]{0,3}x[1-9][0-9]{0,3}x(8|16|32)$")) throw new InvalidDataException("Invalid CNC DDraw display mode.");
            recipes.Add(new(new(id, release, hash, shortcut), name, Field("Summary"), Field("Help").Replace("\\n", "\n").Replace("\\t", "    "), Field("Icon"), url, bytes,
                Field("InstallType"), Optional("InstallExe"), Field("WineVersion"), settings, Optional("Glide"), Optional("CNCDDrawRenderer"), Boolean("CNCDDrawUncapped") == true, Optional("CNCDDrawFakeMode")));
        }
        return recipes;
    }
    public static async Task<LibraryApp> Install(LibraryRepository repository, DemoRecipe demo, WineReference wine, CancellationToken cancellation, IProgress<OperationProgress>? progress)
    {
        var document = repository.Load();
        if (document.Apps.Concat(document.RemovedApps.Select(a => a.App)).Any(a => a.Demo?.Id == demo.Origin.Id)) throw new InvalidOperationException("This demo is already in the library or Removed Apps. Open or restore that copy.");
        if (wine.WineVersion != demo.WineVersion) throw new InvalidOperationException("This demo requires Wine " + demo.WineVersion);
        var app = new LibraryApp { Name = demo.Name, Demo = demo.Origin, DemoSettings = demo.Settings, WinePackage = wine, SavedWineVersion = wine.WineVersion, Resolution = demo.Settings.Resolution ?? "1024x768" };
        if (demo.Settings.WindowsVersion is string version) app.ChooseWindows(version);
        // Explicit pending flags are required: the preferred values already inherit the recipe.
        if (demo.Settings.Gdi != null) { app.WineRenderer = app.PreferredRenderer; app.WineRendererPending = true; }
        if (demo.Settings.UseEGL != null) { app.OpenGLBackend = app.PreferredBackend; app.OpenGLBackendPending = true; }
        if (demo.Settings.WindowsVersion != null) { app.WindowsVersion = app.PreferredWindows; app.WindowsVersionPending = true; }
        var operation = repository.Begin(app, "demo");
        try
        {
            var uri = new Uri(demo.Url);
            string download = SafeFiles.Beneath(repository.AppDirectory(app), "Download/" + Path.GetFileName(uri.LocalPath));
            await Packages.Download(demo.Url, demo.Bytes, demo.Origin.PackageSHA256, download, cancellation, progress);
            string payload = demo.InstallType == "Installer" ? SafeFiles.Beneath(repository.AppDirectory(app), "Installer") : SafeFiles.Beneath(repository.Root(app), LibraryRepository.DriveC + "/App");
            if (uri.LocalPath.EndsWith(".zip", StringComparison.OrdinalIgnoreCase))
            {
                await Packages.Extract(download, payload, cancellation, progress);
                if (demo.InstallType == "Installer") { LibraryRepository.CheckInstaller(SafeFiles.Beneath(payload, demo.InstallExe ?? "")); app.Installer = "Installer/" + demo.InstallExe; }
            }
            else
            {
                Directory.CreateDirectory(payload);
                app.Installer = "Installer/" + Path.GetFileName(uri.LocalPath);
                File.Move(download, SafeFiles.Beneath(repository.AppDirectory(app), app.Installer));
            }
            if (demo.InstallType == "Zip")
            {
                app.Executable = ProgramCandidate.CatalogPath(repository.Programs(app), demo.Origin.ShortcutExe)
                    ?? throw new InvalidDataException("The demo's program could not be identified uniquely.");
            }
            using (var zip = ZipFile.OpenRead(repository.PackagePath(wine)))
            {
                if (demo.Glide != null && (demo.Glide != "psVoodoo" || zip.GetEntry(LibraryRepository.DriveC + "/windows/system32/glide2x.dll") == null)) throw new InvalidDataException("This demo needs a Wine package with built-in Glide support.");
                if (demo.Settings.CncDDraw == true)
                {
                    string relative = LibraryRepository.DriveC + "/ddraw/ddraw.ini";
                    var entry = zip.GetEntry(relative) ?? throw new InvalidDataException("This Wine package is missing CNC DDraw settings.");
                    if (entry.Length > 65536) throw new InvalidDataException("Invalid CNC DDraw settings.");
                    using var input = new StreamReader(entry.Open()); string ini = await input.ReadToEndAsync(cancellation);
                    ini = ConfigureCnc(ini, demo);
                    string target = SafeFiles.Beneath(repository.Root(app), relative); Directory.CreateDirectory(Path.GetDirectoryName(target)!); await File.WriteAllTextAsync(target, ini, new UTF8Encoding(false), cancellation);
                }
            }
            SafeFiles.DeleteTree(Path.Combine(repository.AppDirectory(app), "Download"), repository.AppDirectory(app));
            cancellation.ThrowIfCancellationRequested(); repository.Commit(operation); return app;
        }
        catch { if (!operation.Ready) repository.Discard(operation); throw; }
    }
    public static string ConfigureCnc(string ini, DemoRecipe demo)
    {
        var lines = ini.Replace("\r\n", "\n").Split('\n').ToList();
        void Set(string section, string key, string value)
        {
            var sections = lines.Select((line, i) => (line, i)).Where(x => x.line.Trim().Equals("[" + section + "]", StringComparison.OrdinalIgnoreCase)).Select(x => x.i).ToList();
            if (sections.Count > 1) throw new InvalidDataException("Duplicate CNC DDraw sections.");
            if (sections.Count == 0) { lines.AddRange(["", "[" + section + "]", key + "=" + value]); return; }
            int start = sections[0], end = lines.FindIndex(start + 1, s => s.Trim().StartsWith('[')); if (end < 0) end = lines.Count;
            for (int i = end - 1; i > start; i--) if (lines[i].Split('=')[0].Trim().Equals(key, StringComparison.OrdinalIgnoreCase)) lines.RemoveAt(i);
            lines.Insert(start + 1, key + "=" + value);
        }
        if (demo.CncRenderer != null) Set("ddraw", "renderer", demo.CncRenderer);
        if (demo.CncUncapped) { Set("ddraw", "maxfps", "0"); Set("ddraw", "vsync", "false"); Set("ddraw", "maxgameticks", "-1"); }
        if (demo.CncMode != null) Set(Path.GetFileNameWithoutExtension(demo.Origin.ShortcutExe), "fake_mode", demo.CncMode);
        return string.Join(ini.Contains("\r\n") ? "\r\n" : "\n", lines);
    }
}
