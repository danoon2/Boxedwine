// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Text.RegularExpressions;

namespace Boxedwine.Library;

public static class LaunchArguments
{
    public const string MouseSensitivityOption = "-rel_mouse_sensitivity";
    public static readonly Dictionary<string, string> WindowsVersions = new()
    {
        ["wineDefault"] = "Use Wine's default", ["win11"] = "Windows 11", ["win10"] = "Windows 10", ["win81"] = "Windows 8.1",
        ["win8"] = "Windows 8", ["win7"] = "Windows 7", ["vista"] = "Windows Vista", ["winxp"] = "Windows XP",
        ["win2k"] = "Windows 2000", ["winme"] = "Windows ME", ["win98"] = "Windows 98", ["win95"] = "Windows 95",
        ["nt40"] = "Windows NT 4.0", ["win31"] = "Windows 3.1"
    };
    private static readonly HashSet<string> Flags = ["-nosound", "-p2", "-p3", "-dpiAware", "-disableHideCursor", "-forceRelativeMouse", "-cacheReads", "-disableLinearMemory"];
    private static readonly Dictionary<string, (int Min, int Max)> Ranges = new()
    {
        ["-vsync"] = (0, 2), ["-scale"] = (1, 1000), ["-cpuAffinity"] = (1, 64), ["-pollRate"] = (0, 10000), ["-skipFrameFPS"] = (0, 1000), ["-rel_mouse_sensitivity"] = (0, 1000)
    };
    private static readonly Dictionary<string, string[]> Choices = new()
    {
        ["-bpp"] = ["8", "16", "32"], ["-scale_quality"] = ["0", "1", "2", "nearest", "linear", "best"],
        ["-dxvk"] = ["0", "1", "false", "true", "no", "yes"], ["-opengl"] = ["osmesa"]
    };
    public static string Help => "Put each option and each value on a separate line. Spaces inside a value are preserved; do not add shell quotes.\n\n" +
        string.Join(", ", Flags) + "\n\n" + string.Join("\n", Ranges.Select(p => $"{p.Key}: {p.Value.Min}–{p.Value.Max}")) +
        "\n\n" + string.Join("\n", Choices.Select(p => p.Key + ": " + string.Join(", ", p.Value))) +
        "\n-env: NAME=value\n-glext: allowed OpenGL extensions\n\n-rel_mouse_sensitivity is a percentage for captured, hidden mouse input that the game recenters; 100 is normal, and legacy 0 also means normal. It is synchronized with the Mouse sensitivity slider.\n\nPaths, Wine packages, window size and mounts are managed by Boxedwine. The obsolete -opengl osmesa option is accepted when importing Mac metadata and omitted at launch.";
    public static int MouseSensitivity(IReadOnlyList<string> args)
    {
        ValidateOverrides(args);
        int percent = 100;
        for (int i = 0; i < args.Count; i++)
        {
            string option = args[i];
            if (Flags.Contains(option)) continue;
            string value = args[++i];
            if (option == MouseSensitivityOption) percent = int.Parse(value);
        }
        return percent == 0 ? 100 : percent;
    }
    public static List<string> WithMouseSensitivity(IReadOnlyList<string> args, int percent)
    {
        if (percent is < 1 or > 1000) throw new InvalidDataException("Mouse sensitivity must be from 1% to 1000%.");
        ValidateOverrides(args);
        List<string> result = [];
        for (int i = 0; i < args.Count; i++)
        {
            string option = args[i];
            if (option == MouseSensitivityOption) { i++; continue; }
            result.Add(option);
            if (!Flags.Contains(option)) result.Add(args[++i]);
        }
        if (percent != 100) result.AddRange([MouseSensitivityOption, percent.ToString()]);
        return result;
    }
    public static List<string> Lines(string text) => text.Replace("\r\n", "\n").Split('\n').Where(s => !string.IsNullOrWhiteSpace(s)).ToList();
    public static void ValidateResolution(string resolution)
    {
        var match = Regex.Match(resolution, @"^([1-9]\d{2,3})x([1-9]\d{2,3})$");
        if (!match.Success || !int.TryParse(match.Groups[1].Value, out int x) || !int.TryParse(match.Groups[2].Value, out int y) || x is < 320 or > 8192 || y is < 320 or > 8192) throw new InvalidDataException("Use a resolution from 320x320 to 8192x8192.");
    }
    public static void ValidateOverrides(IReadOnlyList<string> arguments)
    {
        if (arguments.Count > 256 || arguments.Sum(s => s?.Length ?? 0) > 65536 || arguments.Any(s => string.IsNullOrEmpty(s) || s.Length > 8192 || s.IndexOfAny(['\0', '\r', '\n']) >= 0)) throw new InvalidDataException("Use at most 256 Boxedwine arguments with no empty values or control characters.");
        for (int i = 0; i < arguments.Count; i++)
        {
            string option = arguments[i];
            if (Flags.Contains(option)) continue;
            if (!Ranges.ContainsKey(option) && !Choices.ContainsKey(option) && option is not ("-env" or "-glext")) throw new InvalidDataException(option + " is unsupported or managed by the launcher. See Supported Options.");
            if (++i >= arguments.Count || arguments[i].StartsWith('-')) throw new InvalidDataException(option + " needs a value on the next line.");
            string value = arguments[i];
            if (Ranges.TryGetValue(option, out var range) && (!int.TryParse(value, out int number) || number.ToString() != value || number < range.Min || number > range.Max)) throw new InvalidDataException($"{option} needs a whole number from {range.Min} to {range.Max}.");
            if (Choices.TryGetValue(option, out var choices) && !choices.Contains(value)) throw new InvalidDataException(option + " accepts " + string.Join(", ", choices));
            if (option == "-env" && !Regex.IsMatch(value, @"^[A-Za-z_][A-Za-z0-9_]*=")) throw new InvalidDataException("Use NAME=value after -env.");
        }
    }
    public static List<string> Overrides(IReadOnlyList<string> args)
    {
        ValidateOverrides(args);
        List<string> result = [];
        for (int i = 0; i < args.Count; i++)
        {
            string option = args[i];
            if (option == "-opengl") { i++; continue; }
            result.Add(option);
            if (!Flags.Contains(option)) result.Add(args[++i]);
        }
        return result;
    }
    public static List<string> Build(LibraryRepository repository, LibraryApp app, string wine, bool installing = false, string? alternate = null, string? external = null)
    {
        repository.ValidateApp(app);
        if (app.HasPendingSettings) throw new InvalidOperationException("Wine settings must be prepared before launching this app.");
        if (new[] { installing, alternate != null, external != null }.Count(x => x) > 1) throw new InvalidOperationException("Choose one launch mode.");
        List<string> result = ["-root", repository.Root(app), "-zip", wine, "-title", app.Name, "-resolution", installing && app.DemoSettings != null ? app.DemoSettings.InstallResolution ?? "1024x768" : app.Resolution];
        if (app.FullScreen) result.Add("-fullscreenAspect");
        var overrides = Overrides(app.BoxedwineArguments ?? []);
        if (app.BuiltIn != null && !installing && alternate == null && external == null)
        {
            result.AddRange(overrides); result.AddRange(["/bin/wine", app.BuiltIn == "notepad" ? "notepad" : "winemine"]); result.AddRange(app.Arguments); return result;
        }
        string guest, cwd;
        if (external != null)
        {
            LibraryRepository.CheckInstaller(external);
            guest = "/home/username/boxedwine-program/" + Path.GetFileName(external);
            cwd = "/home/username/boxedwine-program";
            result.AddRange(app.DemoSettings?.LaunchArguments(cwd) ?? []);
            result.AddRange(overrides);
            result.AddRange(["-mount", Path.GetDirectoryName(Path.GetFullPath(external))!, cwd]);
        }
        else if (installing)
        {
            if (app.Installer == null) throw new InvalidOperationException("This app has no saved installer.");
            string source = SafeFiles.Beneath(repository.AppDirectory(app), app.Installer);
            LibraryRepository.CheckInstaller(source);
            bool staged = app.Installer.StartsWith("Installer/");
            guest = "/mnt/installer/" + (staged ? app.Installer[10..] : Path.GetFileName(source));
            cwd = guest[..guest.LastIndexOf('/')];
            result.AddRange(overrides);
            result.AddRange(["-mount", staged ? SafeFiles.Beneath(repository.AppDirectory(app), "Installer") : Path.GetDirectoryName(source)!, "/mnt/installer"]);
        }
        else
        {
            string executable = alternate ?? app.Executable ?? throw new InvalidOperationException("Choose a program in App Settings first.");
            if (!executable.StartsWith(LibraryRepository.DriveC + "/") || !executable.EndsWith(".exe", StringComparison.OrdinalIgnoreCase) || !File.Exists(SafeFiles.Beneath(repository.Root(app), executable))) throw new IOException("The selected Windows program is missing. Choose a program again.");
            guest = "/" + executable; cwd = guest[..guest.LastIndexOf('/')];
            result.AddRange(app.DemoSettings?.LaunchArguments(cwd) ?? []);
            result.AddRange(overrides);
        }
        result.AddRange(["-w", cwd, "/bin/wine"]);
        if (guest.EndsWith(".msi", StringComparison.OrdinalIgnoreCase)) result.AddRange(["start", "/wait", "/unix"]);
        result.Add(guest);
        if (!installing && external == null && alternate == null) result.AddRange(app.Arguments);
        return result;
    }
}
