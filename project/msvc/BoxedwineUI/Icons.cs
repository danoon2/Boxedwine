// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Boxedwine.Library;

namespace Boxedwine.UI;

internal static class Icons
{
    private static readonly Dictionary<string, ImageSource?> Cache = [];
    public static ImageSource? ForApp(LibraryRepository repository, LibraryApp app, string resources, IEnumerable<DemoRecipe> demos, string catalogDirectory)
    {
        if (app.CustomIconPNG is { } data)
        {
            try { return FromBytes(data); }
            catch (Exception error) when (error is NotSupportedException or IOException or ArgumentException) { /* Fall back to the automatic icon if image decoding fails. */ }
        }
        if (app.BuiltIn != null) return FromFile(Path.Combine(resources, "AppIcons", app.BuiltIn == "notepad" ? "notepad.png" : "winemine.png"));
        if (app.Demo != null && demos.FirstOrDefault(d => d.Origin.Id == app.Demo.Id) is { } demo && demo.Icon.Length > 0) return FromFile(Path.Combine(catalogDirectory, demo.Icon));
        if (app.Executable != null)
        {
            string path = SafeFiles.Beneath(repository.Root(app), app.Executable);
            if (Cache.TryGetValue(path, out var cached)) return cached;
            try
            {
                if (SHGetFileInfo(path, 0, out var info, (uint)Marshal.SizeOf<ShellFileInfo>(), 0x100) != IntPtr.Zero && info.Icon != IntPtr.Zero)
                {
                    try { var icon = Imaging.CreateBitmapSourceFromHIcon(info.Icon, Int32Rect.Empty, BitmapSizeOptions.FromWidthAndHeight(48, 48)); icon.Freeze(); return Cache[path] = icon; }
                    finally { DestroyIcon(info.Icon); }
                }
            }
            catch (Exception error) when (error is ArgumentException or IOException) { }
            Cache[path] = null;
        }
        return null;
    }
    public static BitmapImage? FromFile(string path)
    {
        if (!File.Exists(path)) return null;
        try { return FromBytes(File.ReadAllBytes(path)); } catch (Exception error) when (error is IOException or NotSupportedException or System.IO.FileFormatException) { return null; }
    }
    public static BitmapImage FromBytes(byte[] bytes)
    {
        using var stream = new MemoryStream(bytes); var image = new BitmapImage(); image.BeginInit(); image.CacheOption = BitmapCacheOption.OnLoad; image.DecodePixelWidth = 96; image.StreamSource = stream; image.EndInit(); image.Freeze(); return image;
    }
    public static byte[] CustomPng(string path)
    {
        if (new FileInfo(path).Length > 20 * 1024 * 1024) throw new IOException("Choose an image smaller than 20 MB.");
        var image = FromFile(path) ?? throw new IOException("The image could not be read.");
        var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(image)); using var stream = new MemoryStream(); encoder.Save(stream); return stream.ToArray();
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)] private struct ShellFileInfo { public IntPtr Icon; public int IconIndex; public uint Attributes; [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)] public string DisplayName; [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)] public string TypeName; }
    [DllImport("shell32.dll", CharSet = CharSet.Unicode)] private static extern IntPtr SHGetFileInfo(string path, uint attributes, out ShellFileInfo info, uint size, uint flags);
    [DllImport("user32.dll")] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool DestroyIcon(IntPtr icon);
}
