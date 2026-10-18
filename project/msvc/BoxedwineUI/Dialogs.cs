// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
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
    private static void Choice(FormWindow form, string title, string description, Action selected)
    {
        var panel = new StackPanel(); panel.Children.Add(new TextBlock { Text = title, FontWeight = FontWeights.SemiBold });
        panel.Children.Add(new TextBlock { Text = description, TextWrapping = TextWrapping.Wrap, Margin = new Thickness(0, 5, 0, 0) });
        var button = new Button { Content = panel, HorizontalContentAlignment = HorizontalAlignment.Left, Padding = new Thickness(14), Margin = new Thickness(0, 0, 0, 10) };
        button.Click += (_, _) => { try { selected(); } catch (Exception error) { Error(form, error.Message); } }; form.Body.Children.Add(button);
    }
    public static string? AddPurpose(Window owner)
    {
        var form = new FormWindow(owner, "Add to Library", "What would you like to add?"); string? result = null;
        void Select(string purpose) { result = purpose; form.DialogResult = true; }
        Choice(form, "App or installer", "Install an app or add one that is ready to run.", () => Select("app"));
        Choice(form, "Collection", "Add a folder of games and apps to browse and install later.", () => Select("collection"));
        Choice(form, "Restore a backup", "Restore saved app files and settings.", () => Select("backup"));
        form.Cancel(); form.ShowDialog(); return result;
    }
    public sealed record SourceRequest(PreparedAppSource? Prepared, bool Remote, bool Back = false);
    public static SourceRequest? AddSource(Window owner, bool collection)
    {
        var form = new FormWindow(owner, collection ? "Add Collection" : "Add App or Installer", "Choose where your files are."); SourceRequest? result = null;
        CancellationTokenSource? cancellation = null;
        bool closed = false, closeAfterCancellation = false;
        var error = new TextBlock { TextWrapping = TextWrapping.Wrap, FontWeight = FontWeights.SemiBold, Visibility = Visibility.Collapsed };
        AutomationProperties.SetAutomationId(error, "add-source-error");
        AutomationProperties.SetLiveSetting(error, AutomationLiveSetting.Assertive);
        var status = FormWindow.Paragraph(""); status.Visibility = Visibility.Collapsed;
        var progress = new ProgressBar { Height = 5, Margin = new Thickness(0, 0, 0, 12), Visibility = Visibility.Collapsed };
        Control[] inputs = [];
        Button? download = null, back = null, cancel = null;
        void ShowError(string message)
        {
            error.Text = message.Length == 0 ? "" : "Could not use this source: " + message;
            error.Visibility = message.Length == 0 ? Visibility.Collapsed : Visibility.Visible;
            if (message.Length > 0)
            {
                error.BringIntoView();
                UIElementAutomationPeer.FromElement(error)?.RaiseAutomationEvent(AutomationEvents.LiveRegionChanged);
            }
        }
        void CancelDownload()
        {
            cancellation?.Cancel(); cancel!.IsEnabled = false; status.Text = "Cancelling and cleaning up…";
        }
        async Task Prepare(string source, bool remote)
        {
            if (cancellation != null || closed) return;
            ShowError("");
            using var attempt = new CancellationTokenSource(); cancellation = attempt;
            foreach (var input in inputs) input.IsEnabled = false;
            download!.IsEnabled = back!.IsEnabled = false; cancel!.Content = remote ? "Cancel Download" : "Cancel";
            status.Text = "Checking source…"; status.Visibility = progress.Visibility = Visibility.Visible; progress.IsIndeterminate = true;
            PreparedAppSource? prepared = null;
            try
            {
                var updates = new Progress<OperationProgress>(value =>
                {
                    if (closed || cancellation != attempt || attempt.IsCancellationRequested) return;
                    status.Text = value.Message; progress.IsIndeterminate = value.Total <= 0; progress.Value = value.Percent;
                });
                prepared = await Task.Run(() => PreparedAppSource.Prepare(source, remote, collection, attempt.Token, updates));
                attempt.Token.ThrowIfCancellationRequested();
            }
            catch (OperationCanceledException) when (attempt.IsCancellationRequested) { prepared?.Dispose(); prepared = null; }
            catch (Exception failure)
            {
                prepared?.Dispose(); prepared = null;
                if (!closed && !closeAfterCancellation) ShowError(failure is OperationCanceledException ? "The download timed out. Check the link and try again." : failure.Message);
            }
            finally
            {
                cancellation = null;
                if (!closed)
                {
                    foreach (var input in inputs) input.IsEnabled = true;
                    download.IsEnabled = back.IsEnabled = cancel.IsEnabled = true; cancel.Content = "Cancel";
                    status.Visibility = progress.Visibility = Visibility.Collapsed;
                }
            }
            if (closed || closeAfterCancellation) { prepared?.Dispose(); if (!closed) form.Close(); }
            else if (prepared != null) { result = new(prepared, remote); form.DialogResult = true; }
        }
        void Pick(bool folder)
        {
            string? source = folder ? Folder(form, "Choose Folder") : File(form, "Choose File", collection ? "Boxedwine collection|*.json" : "App files|*.zip;*.exe;*.msi;*.json");
            if (source != null) _ = Prepare(source, false);
        }
        Choice(form, "Choose file…", collection ? "Collection JSON" : "ZIP, EXE, or MSI", () => Pick(false));
        if (!collection) Choice(form, "Choose folder…", "An app or setup with supporting files.", () => Pick(true));
        var link = FormWindow.Text(""); form.Field("File download URL", link);
        inputs = form.Body.Children.OfType<Control>().ToArray();
        form.Body.Children.Add(error); form.Body.Children.Add(status); form.Body.Children.Add(progress);
        link.TextChanged += (_, _) => ShowError("");
        download = form.Button("Download & Continue", () =>
        {
            if (!Uri.TryCreate(link.Text.Trim(), UriKind.Absolute, out var uri) || !PreparedAppSource.ValidUrl(uri)) { ShowError("Enter a complete HTTP or HTTPS file link."); link.Focus(); return; }
            _ = Prepare(uri.AbsoluteUri, true);
        }, true);
        back = form.Button("Back", () => { result = new(null, false, true); form.DialogResult = true; });
        cancel = form.Button("Cancel", () => { if (cancellation != null) CancelDownload(); else form.DialogResult = false; });
        form.Closing += (_, e) =>
        {
            if (cancellation == null) return;
            e.Cancel = true; closeAfterCancellation = true; CancelDownload();
        };
        form.Closed += (_, _) => { closed = true; cancellation?.Cancel(); };
        form.ShowDialog(); return result;
    }
    public sealed record AddRequest(string Kind, string Program, string Name, string Windows, CatalogWine Wine);
    public static AddRequest? ReviewApp(Window owner, PreparedAppSource source, IReadOnlyList<CatalogWine> wines, WineReference? current)
    {
        var form = new FormWindow(owner, "Review App", source.Original, 620);
        var name = FormWindow.Text(source.Name); form.Field("App name", name);
        var action = FormWindow.Choices(new() { [""] = "Choose how to add these files", ["app"] = "Ready-to-run app", ["installer"] = "Run an installer" }, source.SuggestedAction); form.Field("Add as", action);
        var programs = new ComboBox { MinHeight = 34 }; form.Field("Program", programs);
        if (!source.IsDirectory) form.Body.Children.Add(FormWindow.Paragraph("For a ready-to-run app, choose its containing folder so supporting files are included."));
        else form.Body.Children.Add(FormWindow.Paragraph("Boxedwine copies these files into your library. Your original files stay in place."));
        var options = new StackPanel(); form.Body.Children.Add(new Expander { Header = "Compatibility options", Content = options });
        var wine = new ComboBox { ItemsSource = wines, SelectedItem = wines.FirstOrDefault(w => w.Sha256 == current?.Sha256) ?? wines.FirstOrDefault(), MinHeight = 34 };
        if (wines.Count > 1) { options.Children.Add(new Label { Content = "Wine version", Target = wine }); options.Children.Add(wine); }
        var windows = FormWindow.Choices(LaunchArguments.WindowsVersions, "wineDefault"); options.Children.Add(new Label { Content = "Windows version", Target = windows }); options.Children.Add(windows);
        AddRequest? result = null;
        form.Cancel(); var add = form.Button("Add App", () =>
        {
            if (action.SelectedValue is not string kind || kind.Length == 0 || programs.SelectedItem is not string program || name.Text.Trim().Length == 0) throw new InvalidOperationException("Choose an action and program, and enter an app name.");
            if (kind == "app" && !source.IsDirectory) throw new InvalidOperationException("Go back and choose the app’s containing folder.");
            if (wine.SelectedItem is not CatalogWine selected) throw new InvalidOperationException("The release Wine list is unavailable.");
            result = new(kind == "app" ? "folder" : source.IsDirectory ? "installerFolder" : "installer", program, name.Text.Trim(), (string)windows.SelectedValue, selected); form.DialogResult = true;
        }, true);
        form.Button("Back", () => { result = new("back", "", "", "", wines.FirstOrDefault()!); form.DialogResult = true; });
        void Changed()
        {
            string kind = (string)action.SelectedValue;
            var candidates = source.Programs.Where(p => kind != "app" || p.EndsWith(".exe", StringComparison.OrdinalIgnoreCase)).ToList(); programs.ItemsSource = candidates;
            var preferred = candidates.Where(p => kind == "app" ? !new ProgramCandidate(p).IsMaintenance : p.EndsWith(".msi", StringComparison.OrdinalIgnoreCase) || System.Text.RegularExpressions.Regex.IsMatch(Path.GetFileName(p), "^(setup|install)", System.Text.RegularExpressions.RegexOptions.IgnoreCase)).ToList();
            programs.SelectedItem = preferred.Count == 1 ? preferred[0] : candidates.Count == 1 ? candidates[0] : null;
            add.Content = kind == "installer" ? "Run Installer" : "Add App";
        }
        action.SelectionChanged += (_, _) => Changed(); Changed(); form.ShowDialog(); return result;
    }
    public static string? ReviewCollection(Window owner, CollectionSnapshot snapshot, CollectionStore store)
    {
        var form = new FormWindow(owner, "Review Collection", "Apps download when you choose to install them. Installed copies and saves are kept.", 620);
        var destination = new ComboBox { IsEditable = true, Text = snapshot.Folder, ItemsSource = new[] { "Demos" }.Concat(store.Load().Folders.Select(f => f.Name)).Distinct().ToList(), MinHeight = 34 };
        form.Field("Folder name", destination);
        var summary = FormWindow.Paragraph(""); form.Body.Children.Add(summary);
        void Changed() { try { summary.Text = store.Preview(snapshot, destination.Text).ToString() + ". Listings absent from this import are kept."; } catch (Exception error) { summary.Text = error.Message; } }
        destination.AddHandler(TextBox.TextChangedEvent, new TextChangedEventHandler((_, _) => Changed())); Changed();
        form.Body.Children.Add(FormWindow.Paragraph(string.Join(", ", snapshot.Apps.Take(8).Select(a => a["Name"]))));
        string? result = null; form.Cancel(); form.Button("Add Collection", () => { result = CollectionSnapshot.FolderName(destination.Text); form.DialogResult = true; }, true);
        form.Button("Back", () => { result = ""; form.DialogResult = true; }); form.ShowDialog(); return result;
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
    public static EditResult? Edit(Window owner, LibraryRepository repository, LibraryApp original, System.Runtime.InteropServices.Architecture architecture)
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
        int mousePercent = LaunchArguments.MouseSensitivity(app.BoxedwineArguments ?? []);
        var mouse = new Slider { Minimum = Math.Min(25, mousePercent), Maximum = Math.Max(400, mousePercent), Value = mousePercent, TickFrequency = 1, IsSnapToTickEnabled = true, SmallChange = 1, LargeChange = 10, VerticalAlignment = VerticalAlignment.Center };
        AutomationProperties.SetName(mouse, "Mouse sensitivity");
        var mouseValue = new TextBlock { Text = mousePercent + "%", MinWidth = 52, Margin = new Thickness(12, 0, 8, 0), VerticalAlignment = VerticalAlignment.Center };
        var mouseReset = new Button { Content = "Reset", Padding = new Thickness(10, 4, 10, 4) };
        AutomationProperties.SetName(mouseReset, "Reset mouse sensitivity");
        var mouseRow = new DockPanel(); DockPanel.SetDock(mouseReset, Dock.Right); mouseRow.Children.Add(mouseReset); DockPanel.SetDock(mouseValue, Dock.Right); mouseRow.Children.Add(mouseValue); mouseRow.Children.Add(mouse);
        form.Field("Mouse sensitivity", mouseRow, "100% is normal. Adjusts mouse movement while a game captures the cursor. Applies the next time you launch the app.");
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
        var implementations = new Dictionary<string, string> { ["default"] = "Use global setting (" + OpenGLDrivers.Label(OpenGLDrivers.GlobalDefault(repository.Preferences, architecture)) + ")" };
        foreach (var choice in OpenGLDrivers.Choices(architecture, app.PreferredWindowsOpenGL)) implementations.Add(choice.Key, choice.Value);
        var openGL = FormWindow.Choices(implementations, app.PreferredWindowsOpenGL); Field("OpenGL implementation", openGL);
        advanced.Children.Add(FormWindow.Paragraph(OpenGLDrivers.Help(architecture)));
        var backend = FormWindow.Choices(new() { ["wineDefault"] = "Use Wine's default", ["glx"] = "GLX", ["egl"] = "EGL" }, app.PreferredBackend); Field("Wine OpenGL interface", backend);
        var bw = FormWindow.Text(string.Join("\n", app.BoxedwineArguments ?? []), true); Field("Boxedwine arguments — one option or value per line", bw);
        bool updatingMouse = false;
        bw.TextChanged += (_, _) =>
        {
            if (updatingMouse) return;
            try
            {
                int percent = LaunchArguments.MouseSensitivity(LaunchArguments.Lines(bw.Text));
                updatingMouse = true;
                mouse.Minimum = Math.Min(25, percent); mouse.Maximum = Math.Max(400, percent); mouse.Value = percent;
                mouseValue.Text = percent + "%"; mouse.IsEnabled = mouseReset.IsEnabled = true;
            }
            catch (InvalidDataException) { mouse.IsEnabled = mouseReset.IsEnabled = false; }
            finally { updatingMouse = false; }
        };
        mouse.ValueChanged += (_, _) =>
        {
            if (updatingMouse) return;
            int percent = (int)Math.Round(mouse.Value);
            mouseValue.Text = percent + "%";
            updatingMouse = true;
            try { bw.Text = string.Join("\n", LaunchArguments.WithMouseSensitivity(LaunchArguments.Lines(bw.Text), percent)); }
            finally { updatingMouse = false; }
        };
        mouseReset.Click += (_, _) => mouse.Value = 100;
        var supported = new Button { Content = "Supported Options", HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(0, 6, 0, 6) }; supported.Click += (_, _) => Info(form, "Supported Boxedwine options", LaunchArguments.Help); advanced.Children.Add(supported);
        var args = FormWindow.Text(string.Join("\n", app.Arguments), true); Field("App arguments — one argument per line", args);
        advanced.Children.Add(FormWindow.Paragraph("Spaces inside each line are preserved. Do not add surrounding quotes."));
        EditResult? result = null;
        void Save(bool backup)
        {
            app.Name = name.Text.Trim(); app.Resolution = resolution.Text.Trim(); app.FullScreen = full.IsChecked == true;
            app.ChooseWindows((string)windows.SelectedValue); app.ChooseRenderer((string)renderer.SelectedValue); app.ChooseBackend((string)backend.SelectedValue);
            app.WindowsOpenGL = (string)openGL.SelectedValue;
            app.Arguments = LaunchArguments.Lines(args.Text);
            var overrides = LaunchArguments.Overrides(LaunchArguments.Lines(bw.Text));
            app.BoxedwineArguments = LaunchArguments.WithMouseSensitivity(overrides, LaunchArguments.MouseSensitivity(overrides));
            repository.ValidateApp(app); result = new(app, backup); form.DialogResult = true;
        }
        form.Button("Save and Back Up…", () => Save(true)); form.Cancel(); form.Button("Save", () => Save(false), true); form.ShowDialog(); return result;
    }
}
