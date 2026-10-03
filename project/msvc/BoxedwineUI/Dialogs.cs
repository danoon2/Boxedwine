// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Boxedwine.Library;
using Microsoft.Win32;

namespace Boxedwine.UI;

internal sealed class FormWindow : Window
{
    public StackPanel Body { get; } = new() { Margin = new Thickness(0, 0, 12, 0) };
    public StackPanel Buttons { get; } = new() { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right };
    public FormWindow(Window owner, string title, string? explanation = null, double width = 560, double height = 0)
    {
        ThemeMode = Application.Current.ThemeMode;
        SetResourceReference(StyleProperty, typeof(Window));
        SetResourceReference(BackgroundProperty, "PageBrush");
        SetResourceReference(ForegroundProperty, "TextBrush");
        Owner = owner; Title = title; Width = width; MinWidth = width; MaxHeight = Math.Max(480, SystemParameters.WorkArea.Height - 70);
        WindowStartupLocation = WindowStartupLocation.CenterOwner; ShowInTaskbar = false;
        if (height > 0) Height = Math.Min(height, MaxHeight); else SizeToContent = SizeToContent.Height;
        var dock = new DockPanel { Margin = new Thickness(24) };
        dock.SetResourceReference(Panel.BackgroundProperty, "PageBrush");
        var heading = new TextBlock { Text = title, FontSize = 25, FontWeight = FontWeights.SemiBold, Margin = new Thickness(0, 0, 0, 18), TextWrapping = TextWrapping.Wrap };
        DockPanel.SetDock(heading, Dock.Top); dock.Children.Add(heading);
        Buttons.Margin = new Thickness(0, 22, 0, 0); DockPanel.SetDock(Buttons, Dock.Bottom); dock.Children.Add(Buttons);
        var scroll = new ScrollViewer { Content = Body, VerticalScrollBarVisibility = ScrollBarVisibility.Auto, HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled };
        dock.Children.Add(scroll); Content = dock;
        if (explanation != null) Body.Children.Add(Paragraph(explanation));
        PreviewKeyDown += (_, e) => { if (e.Key == Key.Escape) { DialogResult = false; e.Handled = true; } };
    }
    public static TextBlock Paragraph(string text) => new() { Text = text, Style = (Style)Application.Current.FindResource("Body") };
    public static TextBlock Heading(string text) => new() { Text = text, FontSize = 17, FontWeight = FontWeights.SemiBold, Margin = new Thickness(0, 16, 0, 10), TextWrapping = TextWrapping.Wrap };
    public Button Button(string title, Action action, bool primary = false)
    {
        var button = new Button { Content = title, MinWidth = 85, Padding = new Thickness(14, 8, 14, 8), Margin = new Thickness(8, 0, 0, 0), IsDefault = primary };
        button.Click += (_, _) => { try { action(); } catch (Exception error) { Dialogs.Error(this, error.Message); } }; Buttons.Children.Add(button); return button;
    }
    public void Cancel() => Button("Cancel", () => DialogResult = false);
    public void Field(string label, FrameworkElement control, string? description = null)
    {
        Body.Children.Add(new Label { Content = label, Target = control, Padding = new Thickness(0, 0, 0, 6), FontWeight = FontWeights.SemiBold });
        control.Margin = new Thickness(0, 0, 0, 16); AutomationProperties.SetName(control, label); Body.Children.Add(control);
        if (description != null) Body.Children.Add(Paragraph(description));
    }
    public static TextBox Text(string value, bool multiline = false) => new() { Text = value, Padding = new Thickness(9, 7, 9, 7), AcceptsReturn = multiline, TextWrapping = multiline ? TextWrapping.Wrap : TextWrapping.NoWrap, Height = multiline ? 92 : double.NaN, VerticalScrollBarVisibility = multiline ? ScrollBarVisibility.Auto : ScrollBarVisibility.Hidden };
    public static ComboBox Choices(Dictionary<string, string> options, string selected)
    {
        var box = new ComboBox { ItemsSource = options, DisplayMemberPath = "Value", SelectedValuePath = "Key", SelectedValue = selected, HorizontalAlignment = HorizontalAlignment.Stretch, MinHeight = 34 };
        return box;
    }
}

internal static class Dialogs
{
    public static void Error(Window owner, string message) => Info(owner, "Something needs attention", message);
    public static void Info(Window owner, string title, string message)
    {
        var form = new FormWindow(owner, title, message); form.Button("Close", () => form.DialogResult = true, true); form.ShowDialog();
    }
    public static bool Confirm(Window owner, string title, string message, string action)
    {
        var form = new FormWindow(owner, title, message); form.Cancel(); form.Button(action, () => form.DialogResult = true, true); return form.ShowDialog() == true;
    }
    public static string? Folder(Window owner, string title, string? initial = null)
    {
        var picker = new OpenFolderDialog { Title = title, Multiselect = false };
        if (initial != null) picker.InitialDirectory = initial;
        return picker.ShowDialog(owner) == true ? picker.FolderName : null;
    }
    public static string? File(Window owner, string title, string filter, string? initial = null)
    {
        var picker = new OpenFileDialog { Title = title, Filter = filter, CheckFileExists = true };
        if (initial != null) picker.InitialDirectory = initial;
        return picker.ShowDialog(owner) == true ? picker.FileName : null;
    }
    public sealed record AddRequest(string Kind, string Source, string? Installer, string Windows, CatalogWine Wine);
    public static AddRequest? Add(Window owner, IReadOnlyList<CatalogWine> wines, WineReference? current, string? dropped = null)
    {
        var form = new FormWindow(owner, "Add a Windows app", "Choose the kind of files you have. Boxedwine copies them into an independent Windows environment.", 580);
        var wine = new ComboBox { ItemsSource = wines, SelectedItem = wines.FirstOrDefault(w => w.Sha256 == current?.Sha256) ?? wines.FirstOrDefault(), MinHeight = 34 };
        if (wines.Count > 1) form.Field("Wine version", wine, "The selected package is downloaded if needed and stays with this app.");
        else form.Body.Children.Add(FormWindow.Paragraph(wines.Count == 1 ? $"{wines[0].Name} is downloaded if needed and stays with this app." : "The release Wine list is unavailable."));
        var windows = FormWindow.Choices(LaunchArguments.WindowsVersions, "wineDefault"); form.Field("Windows version", windows);
        AddRequest? request = null;
        void Choose(string kind)
        {
            string? source = dropped;
            if (source != null && (kind == "installer") == Directory.Exists(source)) { Error(form, "Choose a file type matching the dropped item."); return; }
            source ??= kind == "installer" ? File(form, "Choose a Windows installer", "Windows installers|*.exe;*.msi") : Folder(form, kind == "folder" ? "Choose the app folder" : "Choose the complete installer folder");
            if (source == null) return;
            string? installer = kind == "installerFolder" ? File(form, "Choose the setup program inside this folder", "Windows installers|*.exe;*.msi", source) : null;
            if (kind == "installerFolder" && installer == null) return;
            if (wine.SelectedItem is not CatalogWine selected) throw new InvalidOperationException("The release Wine list is unavailable.");
            request = new(kind, source, installer, (string)windows.SelectedValue, selected); form.DialogResult = true;
        }
        foreach (var option in new[] { ("installer", "Windows installer", "A single .exe or .msi setup file."), ("installerFolder", "Installer folder", "A setup program with nearby data files."), ("folder", "App folder", "An app and its files, ready to run without setup.") })
        {
            var panel = new StackPanel(); panel.Children.Add(new TextBlock { Text = option.Item2, FontWeight = FontWeights.SemiBold }); panel.Children.Add(new TextBlock { Text = option.Item3, FontSize = 12, Margin = new Thickness(0, 5, 0, 0), TextWrapping = TextWrapping.Wrap });
            var button = new Button { Content = panel, HorizontalContentAlignment = HorizontalAlignment.Left, Padding = new Thickness(14), Margin = new Thickness(0, 0, 0, 10) };
            button.Click += (_, _) => { try { Choose(option.Item1); } catch (Exception e) { Error(form, e.Message); } }; form.Body.Children.Add(button);
        }
        if (dropped != null) form.Body.Children.Add(FormWindow.Paragraph("Selected: " + dropped));
        form.Cancel(); form.ShowDialog(); return request;
    }
    public static CatalogWine? Wine(Window owner, string title, IReadOnlyList<CatalogWine> wines, WineReference? current = null)
    {
        if (wines.Count == 1) return wines[0];
        if (wines.Count == 0) { Error(owner, "The release Wine list is unavailable."); return null; }
        var form = new FormWindow(owner, title, "Wine packages are checked against the release catalog. Identical packages share storage.");
        var choice = new ListBox { ItemsSource = wines, SelectedItem = wines.FirstOrDefault(w => w.Sha256 == current?.Sha256) ?? wines.FirstOrDefault(), MaxHeight = 340 };
        form.Field("Wine version", choice); form.Cancel(); form.Button("Use Wine", () => form.DialogResult = true, true);
        return form.ShowDialog() == true ? choice.SelectedItem as CatalogWine : null;
    }
    public sealed record ProgramChoice(string? Path, string? External);
    public static ProgramChoice? Program(Window owner, LibraryRepository repository, LibraryApp app, bool external = false)
    {
        var form = new FormWindow(owner, external ? "Run another program" : "Choose the program to open", external ? "Use this app's Windows files and settings. Your usual program stays selected." : "Select the app itself rather than its setup program or uninstaller.", 650, 650);
        var programs = repository.Programs(app); var search = FormWindow.Text(""); form.Field("Search programs", search);
        var list = new ListBox { ItemsSource = programs, Height = 290, SelectedItem = programs.FirstOrDefault(p => p.Path == app.Executable) ?? programs.FirstOrDefault() };
        form.Field("Programs", list); search.TextChanged += (_, _) => list.ItemsSource = programs.Where(p => p.WindowsPath.Contains(search.Text, StringComparison.CurrentCultureIgnoreCase)).ToList();
        ProgramChoice? result = null;
        if (external) form.Button("Choose File…", () =>
        {
            string? path = File(form, "Choose a program or installer", "Windows programs|*.exe;*.msi");
            if (path == null) return;
            if (!Confirm(form, "Run a file from this PC?", "The program can use and modify files in its containing folder:\n\n" + Path.GetDirectoryName(path), "Run")) return;
            result = new(null, path); form.DialogResult = true;
        });
        form.Cancel(); form.Button(external ? "Run" : "Use This Program", () => { if (list.SelectedItem is ProgramCandidate selected) { result = new(selected.Path, null); form.DialogResult = true; } }, true);
        list.MouseDoubleClick += (_, _) => { if (list.SelectedItem is ProgramCandidate selected) { result = new(selected.Path, null); form.DialogResult = true; } };
        form.ShowDialog(); return result;
    }
    public sealed record EditResult(LibraryApp App, bool Backup);
    public static EditResult? Edit(Window owner, LibraryRepository repository, LibraryApp original)
    {
        var app = DataFormat.Clone(original); var form = new FormWindow(owner, "App settings", null, 620, 740);
        var name = FormWindow.Text(app.Name); form.Field("Name", name);
        var program = new Button { Content = app.Executable == null ? "Choose Program…" : new ProgramCandidate(app.Executable).WindowsPath, HorizontalContentAlignment = HorizontalAlignment.Left, Padding = new Thickness(10), Margin = new Thickness(0, 0, 0, 16) };
        if (app.BuiltIn == null)
        {
            program.Click += (_, _) => { var choice = Program(form, repository, app); if (choice?.Path != null) { app.Executable = choice.Path; program.Content = new ProgramCandidate(choice.Path).WindowsPath; } };
            form.Field("Program", program);
        }
        var resolution = new ComboBox { IsEditable = true, ItemsSource = new[] { "640x480", "800x600", "1024x768", "1280x720", "1920x1080" }, Text = app.Resolution, MinHeight = 34 };
        form.Field("Window size", resolution);
        var full = new CheckBox { Content = "Open in full screen", IsChecked = app.FullScreen, Margin = new Thickness(0, 0, 0, 16) }; form.Body.Children.Add(full);
        form.Body.Children.Add(FormWindow.Paragraph("Wine " + (app.SavedWineVersion ?? repository.DefaultWine?.WineVersion ?? "not configured") + " · " + (app.SavedWineVersion != null ? "saved for this app" : "library default")));
        var windows = FormWindow.Choices(LaunchArguments.WindowsVersions, app.PreferredWindows); form.Field("Windows version", windows, "Changes apply and are checked before the next app or installer launch.");
        var advanced = new StackPanel(); var expander = new Expander { Header = "Advanced", Content = advanced, Margin = new Thickness(0, 0, 0, 8) }; form.Body.Children.Add(expander);
        void Field(string label, FrameworkElement control) { advanced.Children.Add(new Label { Content = label, Target = control, Padding = new Thickness(0, 12, 0, 6) }); advanced.Children.Add(control); AutomationProperties.SetName(control, label); }
        var iconButtons = new StackPanel { Orientation = Orientation.Horizontal };
        var image = new Button { Content = "Choose Image…", Padding = new Thickness(10, 6, 10, 6) }; var automatic = new Button { Content = "Use Automatic", Margin = new Thickness(8, 0, 0, 0), Padding = new Thickness(10, 6, 10, 6) };
        var iconStatus = FormWindow.Paragraph(app.CustomIconPNG == null ? "Using the program or demo icon." : "Using your custom image.");
        image.Click += (_, _) => { try { string? path = File(form, "Choose an app icon", "Images|*.png;*.jpg;*.jpeg;*.bmp;*.ico"); if (path != null) { app.CustomIconPNG = Icons.CustomPng(path); iconStatus.Text = "Custom image selected."; } } catch (Exception error) { Error(form, error.Message); } };
        automatic.Click += (_, _) => { app.CustomIconPNG = null; iconStatus.Text = "Using the program or demo icon."; };
        iconButtons.Children.Add(image); iconButtons.Children.Add(automatic); Field("App icon", iconButtons); advanced.Children.Add(iconStatus);
        var renderer = FormWindow.Choices(new() { ["wineDefault"] = "Use Wine's default", ["openGL"] = "OpenGL", ["gdi"] = "GDI (compatibility)" }, app.PreferredRenderer); Field("Wine renderer", renderer);
        advanced.Children.Add(FormWindow.Paragraph("GDI can help older 2D games and menus, but disables Direct3D acceleration."));
        var backend = FormWindow.Choices(new() { ["wineDefault"] = "Use Wine's default", ["glx"] = "GLX", ["egl"] = "EGL" }, app.PreferredBackend); Field("OpenGL backend", backend);
        var bw = FormWindow.Text(string.Join("\n", app.BoxedwineArguments ?? []), true); Field("Boxedwine arguments — one option or value per line", bw);
        var supported = new Button { Content = "Supported Options", HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(0, 6, 0, 6) }; supported.Click += (_, _) => Info(form, "Supported Boxedwine options", LaunchArguments.Help); advanced.Children.Add(supported);
        var args = FormWindow.Text(string.Join("\n", app.Arguments), true); Field("App arguments — one argument per line", args);
        advanced.Children.Add(FormWindow.Paragraph("Spaces inside each line are preserved. Do not add surrounding quotes."));
        EditResult? result = null;
        void Save(bool backup)
        {
            app.Name = name.Text.Trim(); app.Resolution = resolution.Text.Trim(); app.FullScreen = full.IsChecked == true;
            app.ChooseWindows((string)windows.SelectedValue); app.ChooseRenderer((string)renderer.SelectedValue); app.ChooseBackend((string)backend.SelectedValue);
            app.Arguments = LaunchArguments.Lines(args.Text); app.BoxedwineArguments = LaunchArguments.Overrides(LaunchArguments.Lines(bw.Text));
            repository.ValidateApp(app); result = new(app, backup); form.DialogResult = true;
        }
        form.Button("Save and Back Up…", () => Save(true)); form.Cancel(); form.Button("Save", () => Save(false), true); form.ShowDialog(); return result;
    }
}
