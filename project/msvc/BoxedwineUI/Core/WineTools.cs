// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
namespace Boxedwine.Library;

public enum WineTool { Winecfg, Regedit, Cmd, Winefile, Uninstaller, IExplore, Notepad }

public sealed record WineToolInfo(WineTool Tool, string Id, string Name, string Description, IReadOnlyList<string> Arguments)
{
    public bool Matches(string query) => (Id + " " + Name + " " + Description).Contains(query, StringComparison.CurrentCultureIgnoreCase);
}

public static class WineTools
{
    public static IReadOnlyList<WineToolInfo> All { get; } = Array.AsReadOnly(new WineToolInfo[]
    {
        new(WineTool.Winecfg, "winecfg", "Wine Configuration", "Windows version, DLL overrides, graphics, audio, and drives", Array.AsReadOnly(new[] { "winecfg" })),
        new(WineTool.Regedit, "regedit", "Registry Editor", "Edit this app’s Windows registry", Array.AsReadOnly(new[] { "regedit" })),
        new(WineTool.Cmd, "cmd", "Command Prompt", "Run Windows commands and batch files", Array.AsReadOnly(new[] { "wineconsole", "cmd" })),
        new(WineTool.Winefile, "winefile", "File Manager", "Browse this app’s Windows drives and files", Array.AsReadOnly(new[] { "winefile" })),
        new(WineTool.Uninstaller, "uninstaller", "Add/Remove Programs", "Manage installed programs and components", Array.AsReadOnly(new[] { "uninstaller" })),
        new(WineTool.IExplore, "iexplore", "Wine Internet Explorer", "Open Wine’s browser in this environment", Array.AsReadOnly(new[] { "iexplore", "about:blank" })),
        new(WineTool.Notepad, "notepad", "Notepad", "Edit text files in this app’s environment", Array.AsReadOnly(new[] { "notepad" }))
    });

    public static WineToolInfo Get(WineTool tool) => All.FirstOrDefault(item => item.Tool == tool) ?? throw new ArgumentOutOfRangeException(nameof(tool));
}
