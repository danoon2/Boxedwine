// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public static class EngineLocation
{
    public static string? CommandLineOverride(string[] arguments)
    {
        int index = Array.IndexOf(arguments, "--emulator");
        if (index < 0) return null;
        if (index + 1 == arguments.Length || string.IsNullOrWhiteSpace(arguments[index + 1]) || arguments[index + 1].StartsWith("--"))
            throw new ArgumentException("--emulator requires a path to the Boxedwine engine.");
        return Path.GetFullPath(arguments[index + 1]);
    }

    public static string Resolve(string launcherDirectory, string? commandLineOverride = null)
    {
        string path = Path.GetFullPath(commandLineOverride ?? Path.Combine(launcherDirectory, "Runtime", "BoxedwineEngine.exe"));
        if (path.Equals(Path.GetFullPath(Path.Combine(launcherDirectory, "Boxedwine.exe")), StringComparison.OrdinalIgnoreCase))
            throw new ArgumentException("--emulator must name the engine executable, not the Boxedwine UI.");
        if (!File.Exists(path))
            throw new FileNotFoundException(commandLineOverride == null
                ? $"The bundled Boxedwine engine is missing: {path}\nRebuild or reinstall Boxedwine. For debugging, use --emulator <path>."
                : $"The engine specified by --emulator was not found: {path}", path);
        return path;
    }
}
