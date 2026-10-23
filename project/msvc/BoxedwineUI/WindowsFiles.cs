// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32.SafeHandles;

namespace Boxedwine.UI;

public static class WindowsFiles
{
    internal static SafeFileHandle OpenMetadata(string path)
    {
        const uint openExisting = 3, backupSemantics = 0x02000000, openReparsePoint = 0x00200000;
        var handle = CreateFileW(ExtendedPath(path), 0, FileShare.ReadWrite | FileShare.Delete, IntPtr.Zero, openExisting, backupSemantics | openReparsePoint, IntPtr.Zero);
        if (handle.IsInvalid) { int error = Marshal.GetLastWin32Error(); handle.Dispose(); throw new Win32Exception(error); }
        return handle;
    }

    public static string PathForShell(string path)
    {
        // A packaged parent can redirect AppData for this process. Explorer needs
        // the physical location, not the path visible inside that package context.
        using var handle = OpenMetadata(path);
        var buffer = new StringBuilder(32768);
        uint length = GetFinalPathNameByHandleW(handle, buffer, (uint)buffer.Capacity, 0);
        if (length == 0) throw new Win32Exception(Marshal.GetLastWin32Error());
        if (length >= buffer.Capacity) throw new PathTooLongException();
        string full = buffer.ToString();
        if (full.StartsWith(@"\\?\UNC\", StringComparison.OrdinalIgnoreCase)) return @"\\" + full[8..];
        return full.StartsWith(@"\\?\", StringComparison.Ordinal) && full.Length > 6 && full[5] == ':' ? full[4..] : full;
    }

    private static string ExtendedPath(string path)
    {
        // Native calls retain MAX_PATH when the system-wide long-path setting is
        // off, even though .NET can enumerate the same installed files.
        string full = Path.GetFullPath(path);
        if (full.StartsWith(@"\\?\", StringComparison.Ordinal)) return full;
        return full.StartsWith(@"\\", StringComparison.Ordinal) ? @"\\?\UNC\" + full[2..] : @"\\?\" + full;
    }
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] private static extern SafeFileHandle CreateFileW(string path, uint access, FileShare sharing, IntPtr security, uint disposition, uint flags, IntPtr template);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] private static extern uint GetFinalPathNameByHandleW(SafeFileHandle file, StringBuilder path, uint length, uint flags);
}
