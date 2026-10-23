// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.ComponentModel;
using System.Runtime.InteropServices;
using Boxedwine.Library;
using Microsoft.Win32.SafeHandles;

namespace Boxedwine.UI;

public static class AppStorage
{
    private enum FileInfoClass { Standard = 1, Compression = 8, AttributeTag = 9, Identity = 18 }
    public static long? Measure(string root, CancellationToken cancellation = default)
    {
        cancellation.ThrowIfCancellationRequested();
        SafeFiles.NoLinks(root);
        try { _ = File.GetAttributes(root); }
        catch (FileNotFoundException) { return null; }
        catch (DirectoryNotFoundException) { return null; }
        long total = 0;
        var seen = new HashSet<(ulong Volume, ulong Low, ulong High)>();
        foreach (string path in SafeFiles.Tree(root).Prepend(root))
        {
            cancellation.ThrowIfCancellationRequested();
            // Query metadata only, without following a host link or locking app files.
            using var handle = WindowsFiles.OpenMetadata(path);
            if (!GetAttributes(handle, FileInfoClass.AttributeTag, out var attributes, (uint)Marshal.SizeOf<AttributeInfo>())) throw new Win32Exception(Marshal.GetLastWin32Error());
            if ((attributes.Attributes & FileAttributes.ReparsePoint) != 0) throw new IOException("Linked files and junctions are not supported here: " + path);
            if (!GetIdentity(handle, FileInfoClass.Identity, out var identity, (uint)Marshal.SizeOf<IdentityInfo>())) throw new Win32Exception(Marshal.GetLastWin32Error());
            if (!seen.Add((identity.Volume, identity.Low, identity.High))) continue;
            if (!GetStandard(handle, FileInfoClass.Standard, out var standard, (uint)Marshal.SizeOf<StandardInfo>())) throw new Win32Exception(Marshal.GetLastWin32Error());
            long allocated = standard.AllocationSize;
            if (standard.Directory == 0 && (attributes.Attributes & (FileAttributes.Compressed | FileAttributes.SparseFile)) != 0)
            {
                if (!GetCompression(handle, FileInfoClass.Compression, out var compression, (uint)Marshal.SizeOf<CompressionInfo>())) throw new Win32Exception(Marshal.GetLastWin32Error());
                allocated = compression.CompressedSize;
            }
            total = checked(total + allocated);
        }
        return total;
    }

    public static string Format(long bytes)
    {
        string[] units = ["bytes", "KB", "MB", "GB", "TB", "PB", "EB"];
        double value = bytes; int unit = 0;
        while (value >= 1000 && unit < units.Length - 1) { value /= 1000; unit++; }
        return unit == 0 ? $"{bytes:N0} bytes" : $"{value:0.0} {units[unit]}";
    }

    [StructLayout(LayoutKind.Sequential)] private struct AttributeInfo { public FileAttributes Attributes; public uint ReparseTag; }
    [StructLayout(LayoutKind.Sequential)] private struct IdentityInfo { public ulong Volume, Low, High; }
    [StructLayout(LayoutKind.Sequential)] private struct StandardInfo { public long AllocationSize, EndOfFile; public uint Links; public byte DeletePending, Directory; }
    [StructLayout(LayoutKind.Sequential)] private struct CompressionInfo { public long CompressedSize; public ushort Format; public byte UnitShift, ChunkShift, ClusterShift, Reserved1, Reserved2, Reserved3; }
    [DllImport("kernel32.dll", EntryPoint = "GetFileInformationByHandleEx", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetAttributes(SafeFileHandle handle, FileInfoClass kind, out AttributeInfo info, uint size);
    [DllImport("kernel32.dll", EntryPoint = "GetFileInformationByHandleEx", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetIdentity(SafeFileHandle handle, FileInfoClass kind, out IdentityInfo info, uint size);
    [DllImport("kernel32.dll", EntryPoint = "GetFileInformationByHandleEx", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetStandard(SafeFileHandle handle, FileInfoClass kind, out StandardInfo info, uint size);
    [DllImport("kernel32.dll", EntryPoint = "GetFileInformationByHandleEx", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetCompression(SafeFileHandle handle, FileInfoClass kind, out CompressionInfo info, uint size);
}
