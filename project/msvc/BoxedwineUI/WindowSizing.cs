// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;

namespace Boxedwine.UI;

internal static class WindowSizing
{
    public static void FitHeightOnOpen(Window window, double minimum = 640)
    {
        // Keep fractional control sizes from creating overflow after the native window rounds its height.
        window.UseLayoutRounding = true;
        window.SizeToContent = SizeToContent.Height;
        window.SourceInitialized += (_, _) =>
        {
            var owner = window.Owner ?? window;
            var info = new MonitorInfo { Size = Marshal.SizeOf<MonitorInfo>() };
            double available = SystemParameters.WorkArea.Height;
            if (GetMonitorInfo(MonitorFromWindow(new WindowInteropHelper(owner).Handle, 2), ref info))
                available = (info.Work.Bottom - info.Work.Top) / VisualTreeHelper.GetDpi(owner).DpiScaleY;
            window.MaxHeight = available * .9;
            window.MinHeight = Math.Min(minimum, window.MaxHeight);
        };
        window.Loaded += FreezeHeight;
        void FreezeHeight(object sender, RoutedEventArgs e)
        {
            window.Loaded -= FreezeHeight;
            double height = window.ActualHeight;
            window.SizeToContent = SizeToContent.Manual;
            window.Height = Math.Clamp(height, window.MinHeight, window.MaxHeight);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct NativeRect { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)]
    private struct MonitorInfo { public int Size; public NativeRect Monitor, Work; public uint Flags; }
    [DllImport("user32.dll")]
    private static extern IntPtr MonitorFromWindow(IntPtr window, uint flags);
    [DllImport("user32.dll", CharSet = CharSet.Auto)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool GetMonitorInfo(IntPtr monitor, ref MonitorInfo info);
}
