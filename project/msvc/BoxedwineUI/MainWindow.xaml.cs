// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Boxedwine.Library;

namespace Boxedwine.UI;

public sealed class LibraryRow
{
    public LibraryApp? App { get; init; }
    public RemovedApp? Removed { get; init; }
    public DemoRecipe? Demo { get; init; }
    public PendingOperation? Operation { get; init; }
    public string Name { get; init; } = "";
    public string Description { get; init; } = "";
    public string Glyph { get; init; } = "\uE8A5";
    public ImageSource? Icon { get; init; }
    public string Key => App?.Id.ToString() ?? Demo?.Origin.Id ?? Operation?.Id.ToString() ?? Name;
}

public partial class MainWindow : Window
{
    private readonly LibraryRepository repository;
    private readonly string resources = Path.Combine(AppContext.BaseDirectory, "Resources");
    private readonly List<CatalogWine> wines;
    private List<DemoRecipe> demos = [];
    private string? catalogError;
    private string catalogDirectory = "";
    private readonly Dictionary<Guid, RuntimeSession> sessions = [];
    private readonly Dictionary<Guid, LibraryApp> launching = [];
    private readonly HashSet<Guid> stopping = [];
    private readonly Dictionary<Guid, string> runStatus = [];
    private readonly Queue<(Guid AppId, RuntimeExit Exit)> pendingProgramChoices = new();
    private readonly WineLaunchChecks checkedWine = new();
    private LauncherPreferences preferences;
    private CancellationTokenSource? operationCancellation;
    private Task? currentOperation;
    private bool busy, refreshing, shuttingDown, shutdownComplete, ready;
    private LibraryDocument displayedLibrary = new();
    private List<PendingOperation> displayedOperations = [];
    private string section = "All Apps";
    private LibraryRow? Selected => Cards.SelectedItem as LibraryRow;
    public MainWindow(string[] args)
    {
        string? Argument(string name) { int i = Array.IndexOf(args, name); return i >= 0 && i + 1 < args.Length ? args[i + 1] : null; }
        string directory = Argument("--library") ?? Environment.GetEnvironmentVariable("BOXEDWINE_LIBRARY_DIRECTORY") ?? Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Boxedwine");
        repository = new(directory); preferences = repository.Preferences;
        if (Argument("--emulator") is string emulator) preferences.EmulatorPath = Path.GetFullPath(emulator);
        if (Argument("--theme") is string theme) preferences.Theme = theme;
        App.ApplyTheme(preferences.Theme);
        wines = Packages.Catalog(resources);
        LoadCatalog();
        InitializeComponent(); ready = true; Refresh();
        Activated += (_, _) => DrainProgramChoices();
        Cards.PreviewMouseRightButtonDown += (_, e) =>
        {
            if (e.OriginalSource is DependencyObject source && ItemsControl.ContainerFromElement(Cards, source) is ListBoxItem item) item.IsSelected = true;
        };
    }
    private void LoadCatalog()
    {
        try
        {
            string folder = Path.Combine(resources, "Demos");
            var pin = DataFormat.Read<System.Text.Json.JsonElement>(Path.Combine(resources, "demo-catalog.lock.json"));
            string saved = SafeFiles.Beneath(repository.DirectoryPath, "Catalog/" + pin.GetProperty("version").GetString());
            if (!Directory.Exists(folder) && Directory.Exists(saved)) folder = saved;
            catalogDirectory = folder;
            string? catalog = Directory.Exists(folder) ? Directory.EnumerateFiles(folder, "*.xml", SearchOption.AllDirectories).FirstOrDefault() : null;
            if (catalog == null) { catalogError = "The demo catalog is not installed. Use Download Catalog below, or run build.ps1 to include it."; return; }
            demos = Demos.Load(catalog); catalogError = null;
        }
        catch (Exception error) { catalogError = error.Message; }
    }
    private string Emulator()
    {
        string launcher = Path.Combine(AppContext.BaseDirectory, "Boxedwine.exe");
        bool IsEngine(string path) => File.Exists(path) && !Path.GetFullPath(path).Equals(launcher, StringComparison.OrdinalIgnoreCase);
        if (preferences.EmulatorPath is string saved && IsEngine(saved)) return Path.GetFullPath(saved);
        string architecture = System.Runtime.InteropServices.RuntimeInformation.OSArchitecture == System.Runtime.InteropServices.Architecture.Arm64 ? "ARM64" : "x64";
        var candidates = new List<string> { Path.Combine(AppContext.BaseDirectory, "Runtime", "BoxedwineEngine.exe"), Path.Combine(AppContext.BaseDirectory, "BoxedwineEngine.exe"), Path.Combine(AppContext.BaseDirectory, "Runtime", "BoxedWine.exe") };
        for (var parent = new DirectoryInfo(AppContext.BaseDirectory); parent != null; parent = parent.Parent)
        {
            candidates.Add(Path.Combine(parent.FullName, "project", "msvc", "BoxedWine", architecture, "Release", "BoxedWine.exe"));
            candidates.Add(Path.Combine(parent.FullName, "BoxedWine", architecture, "Release", "BoxedWine.exe"));
        }
        return candidates.FirstOrDefault(IsEngine) ?? throw new FileNotFoundException("BoxedwineEngine.exe was not found. Open Settings and choose the emulator executable, or place it next to Boxedwine.exe.");
    }
    private bool Running(LibraryApp app) => sessions.ContainsKey(app.Id);
    private string AppStatus(LibraryApp app) => stopping.Contains(app.Id) ? "Stopping…" : launching.ContainsKey(app.Id) ? "Launching…" : Running(app) ? "Running" : runStatus.GetValueOrDefault(app.Id, "App closed");
    private bool HasWineChoices => wines.Count > 1;
    // Existing apps retain their pinned packages, but new selections use the release catalog.
    private IReadOnlyList<CatalogWine> AvailableWines() => wines;
    private bool CanChange(LibraryApp? app) => app != null && !busy && !Running(app) && !shuttingDown;
    private void Refresh(string? select = null)
    {
        if (!ready) return;
        string? previous = select ?? Selected?.Key;
        // Keep navigation responsive while a worker holds the repository's write lock.
        if (!busy) { displayedLibrary = repository.Load(); displayedOperations = repository.Pending(); }
        var document = displayedLibrary; var pending = displayedOperations;
        refreshing = true;
        try
        {
            RemovedAppsNavigation.Visibility = document.RemovedApps.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
            UnfinishedWorkNavigation.Visibility = pending.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
            if (section == "Removed Apps" && document.RemovedApps.Count == 0 || section == "Unfinished Work" && pending.Count == 0)
            {
                section = "All Apps"; Search.Clear(); Navigation.SelectedIndex = 0;
            }
        }
        finally { refreshing = false; }
        var rows = new List<LibraryRow>();
        LibraryRow Row(LibraryApp app, RemovedApp? removed = null) => new() { App = app, Removed = removed, Name = app.Name, Description = removed != null ? removed.DeletionStartedAt != null ? "Deletion needs finishing" : "Files kept · ready to restore" : AppStatus(app), Icon = Icons.ForApp(repository, app, resources, demos, catalogDirectory), Glyph = app.BuiltIn == "minesweeper" ? "\uE7FC" : "\uE8A5" };
        switch (section)
        {
            case "Demos":
                rows.AddRange(demos.Select(d => new LibraryRow { Demo = d, Name = d.Name, Description = document.Apps.Any(a => a.Demo?.Id == d.Origin.Id) ? "In your library" : document.RemovedApps.Any(a => a.App.Demo?.Id == d.Origin.Id) ? "In Removed Apps" : $"{Math.Max(1, d.Bytes / 1048576.0):0.#} MB · Wine {d.WineVersion}", Icon = Icons.FromFile(Path.Combine(catalogDirectory, d.Icon)), Glyph = "\uE7FC" })); break;
            case "Removed Apps": rows.AddRange(document.RemovedApps.OrderByDescending(a => a.RemovedAt).Select(a => Row(a.App, a))); break;
            case "Unfinished Work": rows.AddRange(pending.Select(o => new LibraryRow { Operation = o, Name = o.Name, Description = o.Ready ? "Ready to finish" : "Interrupted copy", Glyph = "\uE823" })); break;
            case "Recently Opened": rows.AddRange(document.Apps.Where(a => a.LastOpened != null).OrderByDescending(a => a.LastOpened).Select(a => Row(a))); break;
            default: rows.AddRange(document.Apps.OrderBy(a => a.Name, StringComparer.CurrentCultureIgnoreCase).Select(a => Row(a))); break;
        }
        rows = rows.Where(r => r.Name.Contains(Search.Text, StringComparison.CurrentCultureIgnoreCase)).ToList();
        refreshing = true; Cards.ItemsSource = rows; Cards.SelectedItem = rows.FirstOrDefault(r => r.Key == previous); refreshing = false;
        PageTitle.Text = section == "All Apps" ? "Your Windows apps" : section;
        PageSubtitle.Text = section switch { "Demos" => "Discover a classic. Every demo gets its own Windows environment.", "Removed Apps" => "Your files and saves stay here until you restore or delete them.", "Recently Opened" => "Pick up where you left off.", "Unfinished Work" => "Review interrupted file operations.", _ => "A home for old favorites." };
        CountLabel.Text = $"{rows.Count} {(section == "Unfinished Work" ? "operations" : "apps")}" + (section == "All Apps" ? "   ·   Drop an app folder or installer here" : "");
        EmptyState.Visibility = rows.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        EmptyTitle.Text = Search.Text.Length > 0 ? "No matches" : section switch { "Demos" => "Explore Windows classics", "Recently Opened" => "Your recent apps appear here", "Removed Apps" => "No removed apps", "Unfinished Work" => "Everything is up to date", _ => "Make room for your favorites" };
        EmptyDescription.Text = Search.Text.Length > 0 ? "Try another name or clear the search." : section switch { "Demos" => catalogError ?? "The catalog is empty.", "Recently Opened" => "Open an app from All Apps to get started.", "Removed Apps" => "Removing an app from the library keeps its files here.", "Unfinished Work" => "There are no interrupted copies to review.", _ => "Add an app folder or installer, browse a demo, or try Wine's Minesweeper to get started." };
        EmptyButton.Content = section == "Demos" ? "Download Catalog" : Search.Text.Length > 0 ? "Clear search" : section == "All Apps" ? "Add your first app" : "Go to All Apps";
        AddButton.IsEnabled = !busy; EmptyButton.IsEnabled = !busy;
        LaunchingApps.ItemsSource = launching.Values.ToArray();
        RenderDetails();
    }
    private Button DetailAction(string text, Action action, bool enabled = true)
    {
        var button = new Button { Content = text, Style = (Style)FindResource("DetailButton"), IsEnabled = enabled };
        button.Click += (_, _) => Safe(action); Details.Children.Add(button); return button;
    }
    private void RenderDetails()
    {
        Details.Children.Clear();
        var row = Selected;
        if (row == null)
        {
            Details.Children.Add(FormWindow.Heading("Welcome to Boxedwine"));
            Details.Children.Add(FormWindow.Paragraph("Run classic Windows apps in their own environments. Select an app to see its settings and files."));
            Details.Children.Add(FormWindow.Heading("Windows support"));
            var wine = wines.FirstOrDefault(w => w.Reference == repository.DefaultWine) ?? wines.FirstOrDefault();
            Details.Children.Add(FormWindow.Paragraph(wine != null ? $"{wine.Name}\nDownloaded and verified when needed for new apps." : "The release Wine list is unavailable."));
            DetailAction("Open Settings", ShowSettings, !busy);
            if (section == "Removed Apps" && displayedLibrary.RemovedApps.Count > 0) DetailAction("Delete All Removed Apps…", DeleteAll, !busy);
            return;
        }
        if (row.Icon != null) Details.Children.Add(new Image { Source = row.Icon, Width = 64, Height = 64, HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(0, 0, 0, 18) });
        Details.Children.Add(new TextBlock { Text = row.Name, FontSize = 20, FontWeight = FontWeights.SemiBold, TextWrapping = TextWrapping.Wrap, Margin = new Thickness(0, 0, 0, 16) });
        if (row.Operation is { } operation)
        {
            if (operation.Problem != null)
            {
                Details.Children.Add(FormWindow.Paragraph("The recovery record could not be read. Its files have been kept.\n\n" + operation.Problem));
                DetailAction("Show Recovery Folder", () => Reveal(Path.Combine(repository.DirectoryPath, "WindowsOperations"))); return;
            }
            if (operation.Kind == "backup")
            {
                Details.Children.Add(FormWindow.Paragraph(operation.Ready ? "The backup was copied. Verify and finish saving it." : "This backup did not finish. Inspect its partial files before discarding it."));
                DetailAction("Finish Backup", () => RunOperation("Verifying backup…", async (token, progress) => { await Backups.FinishExport(repository, operation, token, progress); return true; }), operation.Ready && !busy);
                DetailAction("Show Backup Files", () => { var staging = repository.ExportStaging(operation); Reveal(Directory.Exists(staging) ? staging : operation.ExportPath!); });
                DetailAction("Discard Partial Backup…", () => { if (Dialogs.Confirm(this, "Discard this partial backup?", "Only this incomplete backup will be deleted. The original app is kept.", "Discard Copy")) RunOperation("Discarding partial backup…", (_, _) => { repository.Discard(operation); return Task.FromResult(true); }, cancellable: false); }, !busy && !operation.Ready); return;
            }
            Details.Children.Add(FormWindow.Paragraph(operation.Ready ? "The files were copied. Finish adding this app to the library." : "This copy did not finish. Inspect its files before discarding it."));
            DetailAction("Finish Adding App", () => RunOperation("Checking copied files…", (_, _) => { repository.Finish(operation); return Task.FromResult(true); }, cancellable: false), operation.Ready && !busy);
            DetailAction("Show Copied Files", () => { if (operation.App != null) Reveal(repository.AppDirectory(operation.App)); });
            DetailAction("Discard Partial Copy…", () => { if (Dialogs.Confirm(this, "Discard this copy?", "Only this interrupted copy will be deleted. The original source files are kept.", "Discard Copy")) RunOperation("Discarding partial copy…", (_, _) => { repository.Discard(operation); return Task.FromResult(true); }, cancellable: false); }, !busy); return;
        }
        if (row.Demo is { } demo)
        {
            Details.Children.Add(FormWindow.Paragraph(demo.Summary));
            var document = displayedLibrary; var installed = document.Apps.FirstOrDefault(a => a.Demo?.Id == demo.Origin.Id); var removed = document.RemovedApps.FirstOrDefault(a => a.App.Demo?.Id == demo.Origin.Id);
            DetailAction(installed != null ? "Show in Library" : removed != null ? "Show in Removed Apps" : "Download and Install", () => ActivateDemo(demo), !busy);
            Details.Children.Add(FormWindow.Paragraph($"{Math.Max(1, demo.Bytes / 1048576.0):0.#} MB download\nRequires Wine {demo.WineVersion}"));
            if (!string.IsNullOrWhiteSpace(demo.Help)) { Details.Children.Add(FormWindow.Heading("About this demo")); Details.Children.Add(FormWindow.Paragraph(demo.Help)); }
            return;
        }
        var app = row.App!;
        if (row.Removed != null)
        {
            Details.Children.Add(FormWindow.Paragraph(row.Removed.DeletionStartedAt != null ? "Deletion was interrupted. Finish deleting this app to reclaim its remaining files." : "This app's Windows files, settings and saves are still on this PC."));
            DetailAction("Restore to Library", () => { repository.Restore(app.Id); Navigate("All Apps", app.Id.ToString()); }, row.Removed.DeletionStartedAt == null && !busy);
            DetailAction("Back Up App…", () => Backup(app), row.Removed.DeletionStartedAt == null && !busy);
            DetailAction("Delete Permanently…", () => Delete(app), !busy); DetailAction("Show Files", () => Reveal(repository.AppDirectory(app))); return;
        }
        Details.Children.Add(FormWindow.Paragraph(AppStatus(app)));
        DetailAction(Running(app) ? "■   Stop App" : "▶   Open App", () => { if (Running(app)) Stop(app); else Open(app); }, (!busy || Running(app)) && !stopping.Contains(app.Id));
        Details.Children.Add(new Separator { Margin = new Thickness(0, 12, 0, 18) });
        Details.Children.Add(FormWindow.Paragraph($"{(app.FullScreen ? "Full screen" : "Windowed")} · {app.Resolution}\nWine {app.SavedWineVersion ?? repository.DefaultWine?.WineVersion ?? "not configured"}\n{LaunchArguments.WindowsVersions[app.PreferredWindows]}"));
        DetailAction("Run Another Program…", () => Another(app), CanChange(app));
        DetailAction("App Settings…", () => Edit(app), CanChange(app));
        DetailAction("Troubleshooting…", () => Troubleshoot(app), !busy);
        DetailAction("Back Up App…", () => Backup(app), CanChange(app));
        Details.Children.Add(new Separator { Margin = new Thickness(0, 12, 0, 8) });
        DetailAction("Storage and Files…", () => Storage(app), !busy);
        DetailAction("Remove from Library…", () => Remove(app), CanChange(app));
        Details.Children.Add(FormWindow.Paragraph("Compatibility varies between Windows apps. Opening an app does not mean all of its features will work."));
    }
    private void Safe(Action action) { try { action(); } catch (Exception error) { Dialogs.Error(this, error.Message); } }
    private void Navigate(string destination, string? selection = null)
    {
        var item = Navigation.Items.Cast<ListBoxItem>().First(i => (string)i.Tag == destination);
        if (item.Visibility != Visibility.Visible) return;
        section = destination; Search.Text = "";
        Navigation.SelectedItem = item; Refresh(selection);
    }
    private void NavigationChanged(object sender, SelectionChangedEventArgs e) { if (ready && !refreshing && Navigation.SelectedItem is ListBoxItem item) { section = (string)item.Tag; Search.Text = ""; Safe(() => Refresh()); } }
    private void NavigateClick(object sender, RoutedEventArgs e) => Navigate((string)((MenuItem)sender).Tag);
    private void SearchChanged(object sender, TextChangedEventArgs e) { if (ready && !refreshing) Safe(() => Refresh()); }
    private void SelectionChanged(object sender, SelectionChangedEventArgs e) { if (ready && !refreshing) Safe(RenderDetails); }
    private void FindClick(object sender, RoutedEventArgs e) { Search.Focus(); Search.SelectAll(); }
    private void ExitClick(object sender, RoutedEventArgs e) => Close();
    private void EmptyClick(object sender, RoutedEventArgs e) { if (Search.Text.Length > 0) Search.Clear(); else if (section == "Demos") DownloadCatalog(); else if (section == "All Apps") Add(); else Navigate("All Apps"); }
    private void CardDoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (e.ChangedButton != MouseButton.Left || e.OriginalSource is not DependencyObject source ||
            ItemsControl.ContainerFromElement(Cards, source) is not ListBoxItem { Content: LibraryRow row }) return;
        if (row.Demo is { } demo) { e.Handled = true; Safe(() => ActivateDemo(demo)); }
        else if (row.App is { } app && row.Removed == null && CanChange(app)) { e.Handled = true; Safe(() => Open(app)); }
    }
    private void FilesDragOver(object sender, DragEventArgs e) { e.Effects = !busy && e.Data.GetDataPresent(DataFormats.FileDrop) ? DragDropEffects.Copy : DragDropEffects.None; e.Handled = true; }
    private void FilesDropped(object sender, DragEventArgs e)
    {
        if (busy || e.Data.GetData(DataFormats.FileDrop) is not string[] paths) return;
        if (paths.Length != 1) { Dialogs.Info(this, "Add one app at a time", "Drop a complete app folder, an installer folder, or a single installer file."); return; }
        Safe(() => Add(paths[0]));
    }
    private void KeyboardShortcut(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.F1) { ShowHelp(); e.Handled = true; return; }
        bool control = Keyboard.Modifiers.HasFlag(ModifierKeys.Control), shift = Keyboard.Modifiers.HasFlag(ModifierKeys.Shift);
        if (control)
        {
            if (e.Key == Key.F) { Search.Focus(); Search.SelectAll(); }
            else if (e.Key == Key.N && !busy) Add();
            else if (e.Key == Key.OemComma && !busy) ShowSettings();
            else if (e.Key >= Key.D1 && e.Key <= Key.D5) Navigate(new[] { "All Apps", "Recently Opened", "Demos", "Removed Apps", "Unfinished Work" }[(int)e.Key - (int)Key.D1]);
            else if (Selected?.App is { } app && Selected.Removed == null)
            {
                if (e.Key == Key.O && CanChange(app)) Safe(() => Open(app));
                else if (e.Key == Key.I && CanChange(app)) Edit(app);
                else if (e.Key == Key.OemPeriod && Running(app)) Stop(app);
                else if (e.Key == Key.S && shift && CanChange(app)) Backup(app);
                else if (e.Key == Key.L && shift) ShowLog(app);
                else return;
            }
            else return;
            e.Handled = true;
        }
        else if (e.Key == Key.Delete && Cards.IsKeyboardFocusWithin && Selected?.App is { } selected && CanChange(selected)) { if (Selected.Removed != null) Delete(selected); else Remove(selected); e.Handled = true; }
        else if (e.Key == Key.Enter && Cards.IsKeyboardFocusWithin && Selected?.App is { } open && Selected.Removed == null && CanChange(open)) { Safe(() => Open(open)); e.Handled = true; }
    }
    private void AppMenuOpened(object sender, RoutedEventArgs e) => EnableMenu(AppMenu.Items);
    private void ContextOpened(object sender, RoutedEventArgs e) => EnableMenu(((ContextMenu)sender).Items);
    private void EnableMenu(ItemCollection items)
    {
        foreach (var item in items.OfType<MenuItem>()) item.IsEnabled = Selected?.App is { } app && Selected.Removed == null && ((string)item.Tag == "log" || ((string)item.Tag == "stop" ? Running(app) : CanChange(app)));
    }
    private void AppActionClick(object sender, RoutedEventArgs e)
    {
        if (Selected?.App is not { } app || Selected.Removed != null) return;
        Safe(() => { switch ((string)((MenuItem)sender).Tag) { case "open": if (CanChange(app)) Open(app); break; case "stop": Stop(app); break; case "another": if (CanChange(app)) Another(app); break; case "edit": if (CanChange(app)) Edit(app); break; case "backup": if (CanChange(app)) Backup(app); break; case "trouble": Troubleshoot(app); break; case "log": ShowLog(app); break; case "remove": if (CanChange(app)) Remove(app); break; } });
    }
    private static void Reveal(string path)
    {
        if (!Directory.Exists(path) && !File.Exists(path)) throw new IOException("These files are no longer present.");
        Process.Start(new ProcessStartInfo("explorer.exe") { UseShellExecute = true, Arguments = '"' + Path.GetFullPath(path) + '"' });
    }
    private async void WindowClosing(object? sender, CancelEventArgs e)
    {
        if (shutdownComplete) return;
        if (shuttingDown) { e.Cancel = true; return; }
        if (!busy && sessions.Count == 0) { repository.Dispose(); return; }
        e.Cancel = true;
        if (!Dialogs.Confirm(this, "Quit Boxedwine?", "Save your work in running Windows apps first. Boxedwine will stop them and finish or cancel the current file operation before quitting.", "Stop Apps and Quit")) return;
        shuttingDown = true; IsEnabled = false; operationCancellation?.Cancel();
        try
        {
            try { await Task.WhenAll(sessions.Values.ToArray().Select(s => s.Stop())); }
            finally { if (currentOperation != null) await currentOperation; }
        }
        catch (Exception) { /* The operation owner reports failures; shutdown still waits for cleanup. */ }
        finally { repository.Dispose(); shutdownComplete = true; Close(); }
    }
}
