// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Media;
using Microsoft.Win32;

namespace Boxedwine.UI;

public partial class App : Application
{
    public static string Appearance { get; private set; } = "System";
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);
        DispatcherUnhandledException += (_, args) => { MessageBox.Show(args.Exception.Message, "Boxedwine", MessageBoxButton.OK, MessageBoxImage.Error); args.Handled = true; };
        SystemEvents.UserPreferenceChanged += PreferenceChanged;
        SystemParameters.StaticPropertyChanged += (_, change) => { if (change.PropertyName == nameof(SystemParameters.HighContrast)) Dispatcher.BeginInvoke(() => ApplyTheme(Appearance)); };
        try { MainWindow = new MainWindow(e.Args); MainWindow.Show(); }
        catch (Exception error) { MessageBox.Show(error.Message, "Boxedwine could not open the library", MessageBoxButton.OK, MessageBoxImage.Error); Shutdown(1); }
    }
    private void PreferenceChanged(object sender, UserPreferenceChangedEventArgs e) => Dispatcher.BeginInvoke(() => ApplyTheme(Appearance));
    protected override void OnExit(ExitEventArgs e) { SystemEvents.UserPreferenceChanged -= PreferenceChanged; base.OnExit(e); }
    public static void ApplyTheme(string mode)
    {
        Appearance = mode is "Dark" or "Light" ? mode : "System";
        Current.ThemeMode = Appearance switch { "Dark" => ThemeMode.Dark, "Light" => ThemeMode.Light, _ => ThemeMode.System };
        foreach (Window window in Current.Windows) window.ThemeMode = Current.ThemeMode;
        bool dark = Appearance == "Dark" || Appearance == "System" && (int?)Registry.GetValue(@"HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize", "AppsUseLightTheme", 1) == 0;
        var colors = dark ? new[] { "#202020", "#292929", "#F3F3F3", "#BDBDBD", "#414141" } : new[] { "#F3F3F3", "#FFFFFF", "#1B1B1B", "#5C5C5C", "#DFDFDF" };
        string[] keys = ["PageBrush", "PanelBrush", "TextBrush", "MutedBrush", "LineBrush"];
        for (int i = 0; i < keys.Length; i++) Current.Resources[keys[i]] = new SolidColorBrush((Color)ColorConverter.ConvertFromString(colors[i]));
        if (SystemParameters.HighContrast)
        {
            Current.Resources["PageBrush"] = SystemColors.WindowBrush; Current.Resources["PanelBrush"] = SystemColors.WindowBrush;
            Current.Resources["TextBrush"] = SystemColors.WindowTextBrush; Current.Resources["MutedBrush"] = SystemColors.WindowTextBrush; Current.Resources["LineBrush"] = SystemColors.WindowTextBrush;
        }
    }
}
