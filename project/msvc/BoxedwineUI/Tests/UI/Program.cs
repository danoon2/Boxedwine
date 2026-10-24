using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Interop;
using System.Windows.Threading;
using System.Windows.Automation;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    [STAThread]
    public static int Main(string[] args)
    {
        try
        {
            if (args[0] == "--runtime-fixture") return RuntimeFixture(args[1], args[2]);
            string destination = Path.GetFullPath(args[0]); Directory.CreateDirectory(destination);
            var app = new Application { ShutdownMode = ShutdownMode.OnExplicitShutdown };
            SynchronizationContext.SetSynchronizationContext(new DispatcherSynchronizationContext(app.Dispatcher));
            app.ThemeMode = ThemeMode.System;
            app.Resources.MergedDictionaries.Add(new ResourceDictionary { Source = new Uri($"/{typeof(MainWindow).Assembly.GetName().Name};component/Styles.xaml", UriKind.Relative) });
            if (args.Contains("--disc-install")) { CheckDiscInstall(destination); return 0; }
            if (args.Contains("--cdrom-settings")) { CheckCdromSettings(destination); return 0; }
            if (args.Contains("--advanced-settings")) { CheckAdvancedSettings(destination); return 0; }
            if (args.Contains("--source-errors")) { CheckSourceImports(destination); return 0; }
            if (args.Contains("--storage")) { CheckStorage(destination); return 0; }
            CheckRuntimeIcons(destination, args);
            var catalog = Packages.Catalog(Path.Combine(AppContext.BaseDirectory, "Resources"));
            if (catalog.Count != 1 || catalog[0].WineVersion != "11.0" || catalog[0].FilesystemVersion != "20") throw new Exception("This release must offer only Wine 11 V20.");
            foreach (string theme in new[] { "Light", "Dark" })
            {
                string library = Path.Combine(destination, "library-" + theme);
                using (var repository = new LibraryRepository(library))
                {
                    if (repository.Load().Apps.Count == 0)
                    {
                        var fakeWine = new WineReference(new string('a', 64), 100, "11.0", "14");
                        repository.AddBuiltIn("notepad", fakeWine); repository.AddBuiltIn("minesweeper", fakeWine);
                    }
                }
                var window = new MainWindow(["--library", library, "--theme", theme]);
                // Render the application's own WPF visual tree without controlling the desktop.
                Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-library.png"));
                var navigation = (ListBox)window.FindName("Navigation"); navigation.SelectedIndex = 2;
                Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-demos.png"));
                navigation.SelectedIndex = 0;
                ((ListBox)window.FindName("Cards")).SelectedIndex = 0;
                Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-selection.png"));
                new WindowInteropHelper(window).EnsureHandle();
                CheckDialog(window, "ShowSettings", [], Path.Combine(destination, theme.ToLowerInvariant() + "-settings.png"), dialog =>
                {
                    if (Descendants<Button>(dialog).Any(b => b.Content is string label && label.Contains("BoxedwineEngine")) ||
                        Descendants<TextBlock>(dialog).Any(t => t.Text == "Emulator")) throw new Exception("Settings must not expose an engine picker.");
                    var choices = Descendants<ComboBox>(dialog).Single(c => AutomationProperties.GetName(c) == "App theme");
                    choices.SelectedValue = theme == "Dark" ? "Light" : "Dark";
                    if (dialog.ThemeMode != Application.Current.ThemeMode || window.ThemeMode != Application.Current.ThemeMode) throw new Exception("Theme changes did not reach every open window.");
                    choices.SelectedValue = theme;
                    dialog.UpdateLayout();
                    var openGL = Descendants<ComboBox>(dialog).Single(c => AutomationProperties.GetName(c) == "Default OpenGL implementation");
                    openGL.SelectedValue = "d3d12";
                    if (System.Text.Json.JsonDocument.Parse(File.ReadAllText(Path.Combine(library, "windows-settings.json"))).RootElement.GetProperty("openGLImplementation").GetString() != "d3d12") throw new Exception("Global OpenGL selection was not saved.");
                    dialog.UpdateLayout(); Descendants<ScrollViewer>(dialog).First().ScrollToVerticalOffset(210);
                });
                CheckDialog(window, "Add", [null!], Path.Combine(destination, theme.ToLowerInvariant() + "-add.png"));
                CheckDialog(window, "ShowHelp", [], Path.Combine(destination, theme.ToLowerInvariant() + "-help.png"), dialog =>
                {
                    var link = Descendants<TextBlock>(dialog).SelectMany(t => t.Inlines.OfType<Hyperlink>()).Single();
                    if (link.NavigateUri.LocalPath != library) throw new Exception("Help must link to the library actually in use.");
                    dialog.UpdateLayout(); Descendants<ScrollViewer>(dialog).First().ScrollToEnd();
                });
                var repositoryField = typeof(MainWindow).GetField("repository", BindingFlags.Instance | BindingFlags.NonPublic)!;
                var activeRepository = (LibraryRepository)repositoryField.GetValue(window)!;
                if (!args.Contains("--app-settings"))
                {
                    CheckWineChoices(window, activeRepository, destination, theme);
                    CheckLaunching(window, activeRepository.Load().Apps[0], destination, theme);
                    CheckInstallerCompletion(window, activeRepository, destination, theme);
                }
                CheckMouseSensitivity(window, activeRepository, destination, theme);
                CheckDialog(window, "Edit", [activeRepository.Load().Apps[0]], Path.Combine(destination, theme.ToLowerInvariant() + "-app-settings.png"));
                CheckDialog(window, "Edit", [activeRepository.Load().Apps[0]], Path.Combine(destination, theme.ToLowerInvariant() + "-advanced.png"), dialog =>
                {
                    SelectSettingsTab(dialog, "Advanced");
                    var openGL = Descendants<ComboBox>(dialog).Single(c => AutomationProperties.GetName(c) == "OpenGL implementation");
                    if ((string)openGL.SelectedValue != "default" || !openGL.Items.Cast<KeyValuePair<string, string>>().Single(c => c.Key == "default").Value.Contains("Direct3D 12")) throw new Exception("App OpenGL selection must show the inherited global setting.");
                    openGL.SelectedValue = "llvmpipe";
                    Render(dialog, Path.Combine(destination, theme.ToLowerInvariant() + "-advanced-top.png"), (int)dialog.ActualWidth - 16, (int)dialog.ActualHeight - 40);
                    dialog.Height = 520;
                    dialog.UpdateLayout();
                    var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                    viewport.ScrollToEnd(); dialog.UpdateLayout();
                    var arguments = Descendants<TextBox>(dialog).Single(t => AutomationProperties.GetName(t) == "App arguments — one argument per line");
                    var position = arguments.TransformToAncestor(viewport).Transform(new Point());
                    if (position.Y < 0 || position.Y + arguments.ActualHeight > viewport.ViewportHeight) throw new Exception("Advanced arguments must be reachable in a short settings window.");
                    var save = Descendants<Button>(dialog).Single(b => b.Content as string == "Save");
                    if (save.TransformToAncestor(dialog).Transform(new Point()).Y < viewport.TransformToAncestor(dialog).Transform(new Point(0, viewport.ActualHeight)).Y) throw new Exception("Save must remain below the scrolling page.");
                }, "Save");
                if (activeRepository.Load().Apps[0].WindowsOpenGL != "llvmpipe") throw new Exception("App OpenGL selection was not saved.");
                CheckDialog(window, "Edit", [activeRepository.Load().Apps[0]], Path.Combine(destination, theme.ToLowerInvariant() + "-advanced-cancel.png"), dialog =>
                {
                    SelectSettingsTab(dialog, "Advanced");
                    Descendants<ComboBox>(dialog).Single(c => AutomationProperties.GetName(c) == "OpenGL implementation").SelectedValue = "native";
                });
                if (activeRepository.Load().Apps[0].WindowsOpenGL != "llvmpipe") throw new Exception("Cancel changed the saved OpenGL selection.");
                CheckDialog(window, "Troubleshoot", [activeRepository.Load().Apps[0]], Path.Combine(destination, theme.ToLowerInvariant() + "-troubleshooting.png"));
                window.Close();
            }
            Console.WriteLine(args.Contains("--app-settings")
                ? "Rendered both themes and verified app settings tabs, scrolling, save/cancel and mouse sensitivity synchronization."
                : "Rendered both themes and verified launch readiness, early exit, stop during startup and installer program selection."); return 0;
        }
        catch (Exception error) { Console.Error.WriteLine(error); return 1; }
    }
    private static void CheckMouseSensitivity(MainWindow window, LibraryRepository repository, string destination, string theme)
    {
        Slider Slider(Window dialog)
        {
            SelectSettingsTab(dialog, "General");
            return Descendants<Slider>(dialog).Single(s => AutomationProperties.GetName(s) == "Mouse sensitivity");
        }
        TextBox Arguments(Window dialog)
        {
            SelectSettingsTab(dialog, "Advanced");
            return Descendants<TextBox>(dialog).Single(t => AutomationProperties.GetName(t) == "Boxedwine arguments — one option or value per line");
        }
        int Saved() => LaunchArguments.MouseSensitivity(repository.Load().Apps[0].BoxedwineArguments ?? []);
        void Check(string suffix, Action<Window> action, string? accept = null) => CheckDialog(window, "Edit", [repository.Load().Apps[0]], Path.Combine(destination, theme.ToLowerInvariant() + "-mouse-" + suffix + ".png"), action, accept);
        void Paste(TextBox text, string value)
        {
            // Exercise WPF's before-insertion paste event without touching the user's clipboard.
            var data = new DataObject(DataFormats.UnicodeText, value);
            text.RaiseEvent(new DataObjectPastingEventArgs(data, false, DataFormats.UnicodeText));
            text.SelectedText = value;
            text.Dispatcher.Invoke(() => { }, DispatcherPriority.Background);
        }
        Check("save", dialog =>
        {
            var slider = Slider(dialog);
            if (slider.Value != 100) throw new Exception("Mouse sensitivity must default to 100%.");
            slider.Value = 50;
            var text = Arguments(dialog);
            if (text.Text.Contains(LaunchArguments.MouseSensitivityOption)) throw new Exception("Slider leaked sensitivity into Advanced.");
            text.Text = "-env\nNAME=one two\n-nosound";
            if (Slider(dialog).Value != 50) throw new Exception("Editing unrelated options reset the slider.");
        }, "Save");
        if (Saved() != 50) throw new Exception("Mouse sensitivity was not saved.");
        Check("cancel", dialog =>
        {
            var slider = Slider(dialog);
            if (slider.Value != 50 || Arguments(dialog).Text.Contains(LaunchArguments.MouseSensitivityOption)) throw new Exception("Saved sensitivity was not moved into General.");
            slider.Value = 200;
        });
        if (Saved() != 50) throw new Exception("Cancel changed mouse sensitivity.");
        Check("paste", dialog =>
        {
            var slider = Slider(dialog); var text = Arguments(dialog);
            double height = dialog.ActualHeight;
            text.Text = "-env\r\nNAME=one two\r\n\r\n-nosound\r\n";
            string otherArguments = text.Text;
            text.CaretIndex = 0;
            Paste(text, "-rel_mouse_sensitivity\r\n750\r\n");
            if (slider.Value != 750 || slider.Maximum < 750 || text.Text != otherArguments || text.CaretIndex != 0)
                throw new Exception("Paste must move sensitivity to General and preserve other text and caret position.");
            if (dialog.ActualHeight != height) throw new Exception("Transferring sensitivity resized the window.");
            if (!Descendants<TextBlock>(dialog).Any(t => t.Text == "Mouse sensitivity set to 750% in General." && t.IsVisible)) throw new Exception("Paste transfer needs visible confirmation.");
        }, "Save");
        if (Saved() != 750) throw new Exception("Pasted mouse sensitivity was not saved.");
        var pasted = repository.Load().Apps[0];
        var launch = LaunchArguments.Build(repository, pasted, "fixture.zip");
        if (launch.Count(a => a == LaunchArguments.MouseSensitivityOption) != 1 || launch[launch.IndexOf(LaunchArguments.MouseSensitivityOption) + 1] != "750")
            throw new Exception("The launch must contain exactly one mouse sensitivity override.");
        Check("typing", dialog =>
        {
            var slider = Slider(dialog); var text = Arguments(dialog);
            text.Focus();
            text.Text = "-nosound\n-rel_mouse_sensitivity\n1";
            text.AppendText("2"); text.AppendText("3");
            if (slider.Value != 750 || !text.Text.EndsWith("123")) throw new Exception("Typing a sensitivity was consumed before the value was complete.");
            SelectSettingsTab(dialog, "General"); slider.Focus();
            if (slider.Value != 123) throw new Exception("Leaving Advanced did not transfer the typed sensitivity.");
            SelectSettingsTab(dialog, "Advanced");
            if (text.Text.Contains(LaunchArguments.MouseSensitivityOption)) throw new Exception("Typed sensitivity stayed in Advanced after transfer.");
        }, "Save");
        if (Saved() != 123) throw new Exception("Typed mouse sensitivity was not saved.");
        Check("invalid", dialog =>
        {
            var slider = Slider(dialog); var text = Arguments(dialog);
            foreach (string invalid in new[] { "-rel_mouse_sensitivity", "-rel_mouse_sensitivity\n1001", "-rel_mouse_sensitivity\noops", "-rel_mouse_sensitivity\n250\n-env" })
            {
                text.SelectAll(); Paste(text, invalid);
                if (text.Text != invalid || slider.Value != 123 || !slider.IsEnabled) throw new Exception("Invalid input must remain editable without altering sensitivity.");
            }
            slider.Value = 200;
            if (!text.Text.EndsWith("-env")) throw new Exception("Slider changes overwrote incomplete Advanced input.");
        });
        if (Saved() != 123) throw new Exception("Cancel changed the saved value after invalid input.");
        Check("duplicates", dialog =>
        {
            var slider = Slider(dialog); var text = Arguments(dialog);
            foreach (var (value, expected) in new[] { ("0", 100), ("1", 1), ("1000", 1000) })
            {
                text.SelectAll(); Paste(text, "-env\nNAME=value\n-rel_mouse_sensitivity\n50\n-rel_mouse_sensitivity\n" + value);
                if (slider.Value != expected || slider.Minimum > expected || slider.Maximum < expected || text.Text.Contains(LaunchArguments.MouseSensitivityOption))
                    throw new Exception("Duplicate, legacy or extended-range sensitivity did not transfer correctly.");
            }
        }, "Save");
        if (Saved() != 1000 || repository.Load().Apps[0].BoxedwineArguments!.Count(a => a == LaunchArguments.MouseSensitivityOption) != 1)
            throw new Exception("Duplicate mouse options were persisted.");
        Check("save-typed", dialog => Arguments(dialog).Text += "\n-rel_mouse_sensitivity\n250", "Save");
        if (Saved() != 250) throw new Exception("Save did not transfer a complete value still in Advanced.");
        Check("reset", dialog =>
        {
            Descendants<Button>(dialog).Single(b => AutomationProperties.GetName(b) == "Reset mouse sensitivity").RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            if (Slider(dialog).Value != 100 || Arguments(dialog).Text.Contains(LaunchArguments.MouseSensitivityOption)) throw new Exception("Reset did not restore default mouse sensitivity.");
            if (!Arguments(dialog).Text.Contains("NAME=value")) throw new Exception("Reset lost unrelated arguments.");
        }, "Save");
        if (Saved() != 100) throw new Exception("Mouse sensitivity reset was not saved.");
        Console.WriteLine($"PASS {theme} mouse sensitivity: hidden arguments, saved/pasted/typed transfer, invalid input, duplicates, launch, cancel and reset");
    }
    private static void SelectSettingsTab(Window dialog, string title)
    {
        var tabs = Descendants<TabControl>(dialog).Single();
        tabs.SelectedItem = tabs.Items.Cast<TabItem>().Single(t => t.Header as string == title);
        dialog.UpdateLayout();
    }
    private static void CheckWineChoices(MainWindow window, LibraryRepository repository, string destination, string theme)
    {
        var wines = (List<CatalogWine>)typeof(MainWindow).GetField("wines", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(window)!;
        // A previously imported, unsupported default must not become a selectable version.
        repository.SetDefaultWine(new WineReference(new string('c', 64), 100, "10.0", "5"));
        var available = (IReadOnlyList<CatalogWine>)typeof(MainWindow).GetMethod("AvailableWines", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, [])!;
        if (available.Count != 1 || available[0].WineVersion != "11.0") throw new Exception("An old imported default leaked into the release list.");
        bool HasButton(Window dialog, string title) => Descendants<Button>(dialog).Any(b => b.Content as string == title);
        void Check(bool multiple)
        {
            string suffix = multiple ? "multiple-wines" : "single-wine";
            CheckDialog(window, "ShowSettings", [], Path.Combine(destination, $"{theme}-{suffix}-settings.png"), dialog =>
            {
                if (HasButton(dialog, "Choose Wine Version…") != multiple || HasButton(dialog, "Import Wine ZIP…") != multiple) throw new Exception("Wine selection settings do not match catalog availability.");
                if (!multiple && !HasButton(dialog, "Set Up Windows Support")) throw new Exception("The sole supported package must still be available for setup.");
                if (!multiple && !Descendants<TextBlock>(dialog).Any(t => t.Text.Contains("Wine 11.0 V20"))) throw new Exception("Settings should retain the filesystem revision.");
            });
            CheckDialog(window, "Add", [null!], Path.Combine(destination, $"{theme}-{suffix}-add.png"), dialog =>
            {
                var picker = Descendants<ComboBox>(dialog).SingleOrDefault(c => AutomationProperties.GetName(c) == "Wine version");
                if ((picker != null) != multiple) throw new Exception("The Wine picker must be hidden for a single release.");
                if (picker != null && (picker.Items.Count != 2 || ((CatalogWine)picker.SelectedItem).WineVersion != "11.0")) throw new Exception("Multiple versions or the supported default were lost.");
                if (!multiple && (!Descendants<TextBlock>(dialog).Any(t => t.Text.Contains("Wine 11.0")) || Descendants<TextBlock>(dialog).Any(t => t.Text.Contains("V20")))) throw new Exception("App screens should show the Wine version without the filesystem revision.");
            });
            CheckDialog(window, "Troubleshoot", [repository.Load().Apps[0]], Path.Combine(destination, $"{theme}-{suffix}-troubleshooting.png"), dialog =>
            {
                if (HasButton(dialog, "Try Another Wine Version…") != multiple) throw new Exception("Wine trials do not match catalog availability.");
            });
            CheckDialog(window, "ShowHelp", [], Path.Combine(destination, $"{theme}-{suffix}-help.png"), dialog =>
            {
                if (Descendants<TextBlock>(dialog).Any(t => t.Text.Contains("Try Another Wine Version")) != multiple) throw new Exception("Help describes a hidden Wine action.");
            });
        }
        Check(false);
        // Test-only future release: no downloads or real Wine 12 metadata.
        var future = new CatalogWine("Wine 12.0", "12.0", "https://boxedwine.org/tests/Wine12.0.zip", 100, new string('d', 64), "14");
        wines.Add(future);
        try { Check(true); } finally { wines.Remove(future); }
        repository.SetDefaultWine(wines[0].Reference);
        Console.WriteLine($"PASS {theme} Wine controls: single release, old default excluded, future multi-version catalog");
    }
    // A controllable child exercises the real redirected-output and process lifecycle.
    private static int RuntimeFixture(string trigger, string mode)
    {
        var command = Task.Run(Console.ReadLine);
        while (!File.Exists(trigger) && !command.IsCompleted) Thread.Sleep(20);
        if (mode == "installed") return 0;
        if (mode == "exit") return 7;
        Console.Error.Write("Show"); Console.Error.Flush(); Thread.Sleep(25);
        Console.Error.Write("ing Window\r\nShowing Window\n"); Console.Error.Flush();
        return command.GetAwaiter().GetResult() == "quit" ? 0 : 1;
    }
    private static void CheckLaunching(MainWindow window, LibraryApp app, string destination, string theme)
    {
        // A shown, offscreen host gives Fluent's indeterminate animation a live
        // presentation source without taking focus or covering the user's apps.
        window.WindowStartupLocation = WindowStartupLocation.Manual;
        window.ShowActivated = false; window.ShowInTaskbar = false;
        window.Left = -16000; window.Top = -16000; window.Show();
        var banners = (ItemsControl)window.FindName("LaunchingApps");
        var cards = (ListBox)window.FindName("Cards");
        string Status() => cards.Items.Cast<LibraryRow>().Single(r => r.App?.Id == app.Id).Description;
        string wine = Path.Combine(destination, "launch-test-wine.bin"); File.WriteAllText(wine, "fixture");
        foreach (string mode in new[] { "window", "exit", "stop" })
        {
            string trigger = Path.Combine(destination, Guid.NewGuid() + ".ready");
            using var session = new RuntimeSession(Environment.ProcessPath!, ["--runtime-fixture", trigger, mode], wine, trigger + ".log");
            typeof(MainWindow).GetMethod("TrackSession", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, [app, session]);
            if (banners.Items.Count != 1 || Status() != "Launching…") throw new Exception("Launching status was not shown immediately.");
            if (mode == "window")
            {
                Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-launching.png"));
                var progress = Descendants<ProgressBar>(banners).Single();
                if (!progress.IsIndeterminate || progress.ActualWidth < 100) throw new Exception("The launch progress bar is missing.");
                byte[] firstFrame = ProgressPixels(progress);
                var nextFrame = DateTime.UtcNow.AddMilliseconds(350); PumpUntil(() => DateTime.UtcNow >= nextFrame);
                if (SystemParameters.ClientAreaAnimation && firstFrame.SequenceEqual(ProgressPixels(progress))) throw new Exception("The launch progress bar did not animate.");
                if (!Descendants<TextBlock>((DependencyObject)window.FindName("Details")).Any(t => t.Text == "Launching…")) throw new Exception("Details did not show launching status.");
                File.WriteAllText(trigger, "ready");
                PumpUntil(() => banners.Items.Count == 0 && Status() == "Running");
                if (!session.WindowShown.Result) throw new Exception("The runtime window signal was missed.");
                var stop = session.Stop(); PumpUntil(() => stop.IsCompleted && Status() == "Stopped"); stop.GetAwaiter().GetResult();
            }
            else if (mode == "exit")
            {
                File.WriteAllText(trigger, "exit");
                PumpUntil(() => banners.Items.Count == 0 && Status() == "Exited with code 7");
                if (session.WindowShown.Result) throw new Exception("Exit without a window must not report readiness.");
            }
            else
            {
                window.Dispatcher.BeginInvoke(() => Application.Current.Windows.Cast<Window>().Last(w => w != window).DialogResult = true, DispatcherPriority.ApplicationIdle);
                typeof(MainWindow).GetMethod("Stop", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, [app]);
                if (banners.Items.Count != 0) throw new Exception("Stopping during startup did not clear the launch banner.");
                PumpUntil(() => Status() == "Stopped");
                if (session.WindowShown.Result) throw new Exception("A late window signal must not undo a stop request.");
            }
            Console.WriteLine($"PASS {theme} launch lifecycle: {mode}");
        }
        window.Hide();
    }
    private static void CheckInstallerCompletion(MainWindow window, LibraryRepository repository, string destination, string theme)
    {
        var app = new LibraryApp { Name = "Demo selection fixture", Installer = "Installer/setup.exe", Demo = new("selection-test", "test-1", new string('a', 64), "game.exe") };
        string path = LibraryRepository.DriveC + "/Game/GAME.EXE";
        string file = SafeFiles.Beneath(repository.Root(app), path);
        Directory.CreateDirectory(Path.GetDirectoryName(file)!); File.WriteAllText(file, "MZprogram");
        var document = repository.Load(); document.Apps.Add(app); repository.Save(document);
        var flags = BindingFlags.Instance | BindingFlags.NonPublic;
        var statuses = (Dictionary<Guid, string>)typeof(MainWindow).GetField("runStatus", flags)!.GetValue(window)!;
        var sessions = (Dictionary<Guid, RuntimeSession>)typeof(MainWindow).GetField("sessions", flags)!.GetValue(window)!;
        var dialogs = new List<string>();
        var watcher = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(20) };
        watcher.Tick += (_, _) =>
        {
            foreach (var dialog in Application.Current.Windows.Cast<Window>().Where(w => w != window && w.IsVisible).ToArray())
            { dialogs.Add(dialog.Title); dialog.Close(); }
        };
        watcher.Start();
        try
        {
            foreach (bool ambiguous in new[] { false, true })
            {
                if (ambiguous)
                {
                    app.Executable = null; repository.Update(app);
                    string duplicate = SafeFiles.Beneath(repository.Root(app), LibraryRepository.DriveC + "/Backup/game.exe");
                    Directory.CreateDirectory(Path.GetDirectoryName(duplicate)!); File.WriteAllText(duplicate, "MZprogram");
                }
                statuses.Remove(app.Id); dialogs.Clear();
                string trigger = Path.Combine(destination, Guid.NewGuid() + ".installed");
                using var session = new RuntimeSession(Environment.ProcessPath!, ["--runtime-fixture", trigger, "installed"],
                    Path.Combine(destination, "launch-test-wine.bin"), trigger + ".log", installing: true);
                // Completion must survive a temporarily disabled owner (e.g. another dialog).
                window.IsEnabled = false;
                typeof(MainWindow).GetMethod("TrackSession", flags)!.Invoke(window, [app, session]);
                File.WriteAllText(trigger, "exit");
                PumpUntil(() => !sessions.ContainsKey(app.Id));
                if (repository.Load().Apps.Single(a => a.Id == app.Id).Executable != null || dialogs.Count != 0)
                    throw new Exception("Installer selection did not wait for the owner to become available.");
                window.IsEnabled = true;
                typeof(MainWindow).GetMethod("DrainProgramChoices", flags)!.Invoke(window, null);
                PumpUntil(() => !(bool)typeof(MainWindow).GetField("busy", flags)!.GetValue(window)! &&
                    (ambiguous ? dialogs.Count > 0 : statuses.GetValueOrDefault(app.Id) == "Ready to open" || dialogs.Count > 0));
                string? selected = repository.Load().Apps.Single(a => a.Id == app.Id).Executable;
                if (ambiguous ? selected != null || dialogs.Count != 1 || dialogs[0] != "Choose the program to open" : selected != path || dialogs.Count != 0)
                    throw new Exception("Installer completion selected the wrong program or showed an unnecessary dialog.");
                Console.WriteLine($"PASS {theme} installer completion: {(ambiguous ? "ambiguous program prompts" : "catalog program selected without prompting")}");
            }
        }
        finally
        {
            watcher.Stop(); window.IsEnabled = true;
            document = repository.Load(); document.Apps.RemoveAll(a => a.Id == app.Id); repository.Save(document);
        }
    }
    private static byte[] ProgressPixels(ProgressBar progress)
    {
        int width = (int)Math.Ceiling(progress.ActualWidth), height = (int)Math.Ceiling(progress.ActualHeight);
        var visual = new DrawingVisual();
        using (var drawing = visual.RenderOpen()) drawing.DrawRectangle(new VisualBrush(progress), null, new Rect(0, 0, width, height));
        var bitmap = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Pbgra32); bitmap.Render(visual);
        var pixels = new byte[width * height * 4]; bitmap.CopyPixels(pixels, width * 4, 0); return pixels;
    }
    private static void PumpUntil(Func<bool> complete)
    {
        var deadline = DateTime.UtcNow.AddSeconds(15);
        var frame = new DispatcherFrame(); var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(20) };
        timer.Tick += (_, _) => { if (complete() || DateTime.UtcNow >= deadline) { timer.Stop(); frame.Continue = false; } };
        timer.Start(); Dispatcher.PushFrame(frame);
        if (!complete()) throw new TimeoutException("The launch lifecycle did not reach the expected state.");
    }
    private static IEnumerable<T> Descendants<T>(DependencyObject parent) where T : DependencyObject
    {
        for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++)
        {
            var child = VisualTreeHelper.GetChild(parent, i);
            if (child is T match) yield return match;
            foreach (var descendant in Descendants<T>(child)) yield return descendant;
        }
    }
    private static void CheckDialog(Window owner, string method, object[] args, string path, Action<Window>? inspect = null, string? accept = null)
    {
        Exception? failure = null;
        owner.Dispatcher.BeginInvoke(() =>
        {
            var dialog = Application.Current.Windows.Cast<Window>().Last(w => w != owner);
            Console.WriteLine($"Dialog {dialog.Title}: theme={dialog.ThemeMode}, app={Application.Current.ThemeMode}, highContrast={SystemParameters.HighContrast}");
            try
            {
                inspect?.Invoke(dialog); Render(dialog, path, (int)dialog.ActualWidth - 16, (int)dialog.ActualHeight - 40);
                if (accept != null) Descendants<Button>(dialog).Single(b => b.Content as string == accept).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            }
            catch (Exception error) { failure = error; }
            finally { dialog.Close(); }
        }, DispatcherPriority.ApplicationIdle);
        typeof(MainWindow).GetMethod(method, BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(owner, args);
        if (failure != null) throw failure;
    }
    private static void Render(Window window, string path, int width = 1220, int height = 760)
    {
        var content = (FrameworkElement)window.Content;
        if (!window.IsVisible) { content.Measure(new Size(width, height)); content.Arrange(new Rect(0, 0, width, height)); }
        content.UpdateLayout();
        // Allow Fluent expander/theme transitions to finish before inspecting pixels.
        var frame = new DispatcherFrame(); var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(400) };
        timer.Tick += (_, _) => { timer.Stop(); frame.Continue = false; }; timer.Start(); Dispatcher.PushFrame(frame);
        if (window.IsVisible) { width = (int)Math.Ceiling(content.ActualWidth + content.Margin.Left + content.Margin.Right); height = (int)Math.Ceiling(content.ActualHeight + content.Margin.Top + content.Margin.Bottom); }
        var visual = new DrawingVisual();
        using (var drawing = visual.RenderOpen())
        {
            drawing.DrawRectangle((Brush)Application.Current.FindResource("PageBrush"), null, new Rect(0, 0, width, height));
        }
        var bitmap = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Pbgra32); bitmap.Render(visual); bitmap.Render(content);
        var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap)); using var output = File.Create(path); encoder.Save(output);
        Console.WriteLine(path);
    }
}
