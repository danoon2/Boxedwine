// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Media;
using Boxedwine.Library;

namespace Boxedwine.UI;

internal sealed class AppIconEditor : DockPanel
{
    private readonly LibraryApp app;
    private readonly Func<LibraryApp, ImageSource?> resolveIcon;
    private readonly Image preview = new() { Stretch = Stretch.Uniform };
    private readonly TextBlock fallback = new() { Text = "\uE8A5", FontFamily = new FontFamily("Segoe Fluent Icons"), FontSize = 24,
        HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center };
    private readonly TextBlock status = new() { FontSize = 12, Margin = new Thickness(12, 0, 0, 0), VerticalAlignment = VerticalAlignment.Center };
    private readonly Button automatic = new() { Content = "Use automatic", Padding = new Thickness(10, 6, 10, 6), Margin = new Thickness(8, 0, 0, 0) };

    public AppIconEditor(Window owner, LibraryApp app, Func<LibraryApp, ImageSource?> resolveIcon)
    {
        this.app = app; this.resolveIcon = resolveIcon;
        var icon = new Grid { Width = 40, Height = 40, Margin = new Thickness(0, 0, 12, 0), VerticalAlignment = VerticalAlignment.Center };
        var background = new Border { CornerRadius = new CornerRadius(4), Child = fallback };
        background.SetResourceReference(Border.BackgroundProperty, "PanelBrush");
        icon.Children.Add(background); icon.Children.Add(preview); Children.Add(icon);
        AutomationProperties.SetName(preview, "App icon preview");
        var controls = new WrapPanel { VerticalAlignment = VerticalAlignment.Center };
        var choose = new Button { Content = "Choose image…", Padding = new Thickness(10, 6, 10, 6) };
        choose.Click += (_, _) =>
        {
            try
            {
                string? path = Dialogs.File(owner, "Choose an app icon", "Images|*.png;*.jpg;*.jpeg;*.bmp;*.ico");
                if (path != null) { app.CustomIconPNG = Icons.CustomPng(path); Refresh(); }
            }
            catch (Exception error) { Dialogs.Error(owner, error.Message); }
        };
        automatic.Click += (_, _) => { app.CustomIconPNG = null; Refresh(); };
        status.SetResourceReference(TextBlock.ForegroundProperty, "MutedBrush");
        AutomationProperties.SetLiveSetting(status, AutomationLiveSetting.Polite);
        controls.Children.Add(choose); controls.Children.Add(automatic); controls.Children.Add(status); Children.Add(controls);
        Refresh();
    }

    public void Refresh()
    {
        try { preview.Source = resolveIcon(app); }
        catch (Exception error) when (error is IOException or ArgumentException or NotSupportedException or UnauthorizedAccessException)
        {
            preview.Source = null;
        }
        fallback.Visibility = preview.Source == null ? Visibility.Visible : Visibility.Collapsed;
        status.Text = app.CustomIconPNG == null ? "Automatic icon" : "Custom icon";
        automatic.IsEnabled = app.CustomIconPNG != null;
    }
}
