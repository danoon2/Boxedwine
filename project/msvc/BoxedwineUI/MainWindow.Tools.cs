// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Threading;
using Boxedwine.Library;
using Microsoft.Win32;

namespace Boxedwine.UI;

public partial class MainWindow
{
    private void SettingsClick(object sender, RoutedEventArgs e) { if (!busy) Safe(ShowSettings); }
    private void ShowSettings()
    {
        var form = new FormWindow(this, "Settings", null, 640, 740);
        form.Body.Children.Add(FormWindow.Heading("Appearance"));
        var theme = FormWindow.Choices(new() { ["System"] = "Use Windows setting", ["Light"] = "Light", ["Dark"] = "Dark" }, preferences.Theme);
        form.Field("App theme", theme); theme.SelectionChanged += (_, _) => { if (theme.SelectedValue is string value) { preferences.Theme = value; App.ApplyTheme(value); repository.SavePreferences(preferences); } };
        form.Body.Children.Add(FormWindow.Heading("Library"));
        var deletion = new CheckBox { Content = "Delete apps immediately", IsChecked = preferences.DeleteImmediately, Margin = new Thickness(0, 0, 0, 10) };
        deletion.Click += (_, _) => { preferences.DeleteImmediately = deletion.IsChecked == true; repository.SavePreferences(preferences); };
        form.Body.Children.Add(deletion); form.Body.Children.Add(FormWindow.Paragraph("Skip Removed Apps and ask to permanently delete an app's files and saves. Existing Removed Apps are kept."));
        form.Body.Children.Add(FormWindow.Paragraph(repository.DirectoryPath));
        var reveal = new Button { Content = "Open Library Folder", HorizontalAlignment = HorizontalAlignment.Left, Padding = new Thickness(12, 7, 12, 7) }; reveal.Click += (_, _) => Safe(() => Reveal(repository.DirectoryPath)); form.Body.Children.Add(reveal);
        form.Body.Children.Add(FormWindow.Heading("Graphics"));
        var architecture = OpenGLArchitecture();
        string selectedOpenGL = OpenGLDrivers.GlobalDefault(preferences, architecture);
        var openGL = FormWindow.Choices(OpenGLDrivers.Choices(architecture, selectedOpenGL), selectedOpenGL);
        form.Field("Default OpenGL implementation", openGL, "Used by apps set to use the global setting. Changes take effect on the next launch. Override this in App Settings → Advanced.");
        form.Body.Children.Add(FormWindow.Paragraph(OpenGLDrivers.Help(architecture)));
        openGL.SelectionChanged += (_, _) => Safe(() => { if (openGL.SelectedValue is string value) { preferences.OpenGLImplementation = value; repository.SavePreferences(preferences); } });
        form.Body.Children.Add(FormWindow.Heading("Windows support"));
        var current = repository.DefaultWine;
        form.Body.Children.Add(FormWindow.Paragraph(!HasWineChoices && wines.Count == 1
            ? $"{wines[0].DisplayName}\nDownloaded and verified when needed. Each app keeps its own Windows files and saves."
            : current == null ? "Windows support is downloaded when you add an app." : $"Default Wine version: {current.WineVersion}\nFilesystem version: {current.FilesystemVersion}\nSource: Saved on this PC"));
        bool canChange = sessions.Count == 0;
        void ActionButton(string title, Action action)
        {
            var button = new Button { Content = title, HorizontalAlignment = HorizontalAlignment.Left, Padding = new Thickness(12, 7, 12, 7), Margin = new Thickness(0, 0, 0, 8), IsEnabled = canChange };
            button.Click += (_, _) => { form.Close(); Safe(action); }; form.Body.Children.Add(button);
        }
        if (HasWineChoices) ActionButton("Choose Wine Version…", () =>
        {
            var wine = Dialogs.Wine(this, "Choose default Wine version", wines, current); if (wine == null) return;
            RunOperation("Preparing Windows support…", async (token, progress) => { var reference = await EnsureWine(wine, token, progress); repository.SetDefaultWine(reference); return reference; }, _ => Refresh());
        });
        if (!HasWineChoices && wines.Count == 1 && current != wines[0].Reference) ActionButton("Set Up Windows Support", () =>
            RunOperation("Preparing Windows support…", async (token, progress) => { var reference = await EnsureWine(wines[0], token, progress); repository.SetDefaultWine(reference); return reference; }, _ => Refresh()));
        if (HasWineChoices) ActionButton("Import Wine ZIP…", () =>
        {
            string? zip = Dialogs.File(this, "Choose a complete Boxedwine Wine package", "Wine filesystem ZIP|*.zip"); if (zip == null) return;
            RunOperation("Checking Windows support…", async (token, progress) => { var wine = await Packages.Import(repository, zip, token, progress); repository.SetDefaultWine(wine); return wine; }, _ => Refresh());
        });
        if (current != null) ActionButton("Check Package Again", () => RunOperation("Checking Windows support…", async (token, progress) =>
        {
            var wine = await Packages.ValidateWine(repository.PackagePath(current), token, progress);
            if (wine != current) throw new InvalidDataException("The saved Wine package no longer matches its identity."); return wine;
        }, _ => Dialogs.Info(this, "Windows support checked", "The Wine package passed its integrity and structure checks. Compatibility still depends on the Windows app.")));
        ActionButton("Clean Unused Wine Packages", () => RunOperation("Cleaning unused Wine packages…", (_, _) => { repository.PruneWine(); return Task.FromResult(true); }, _ => Dialogs.Info(this, "Wine storage organized", "Packages used by apps, Removed Apps, unfinished copies, and the library default were kept.")));
        if (!canChange) form.Body.Children.Add(FormWindow.Paragraph("Close running apps before changing Windows support or the emulator."));
        else if (HasWineChoices) form.Body.Children.Add(FormWindow.Paragraph("Changing the default affects apps that use the library default. Apps pinned to a package keep that package."));
        form.Body.Children.Add(FormWindow.Heading("Emulator"));
        string emulatorPath; try { emulatorPath = Emulator(); } catch { emulatorPath = "BoxedwineEngine.exe was not found."; }
        form.Body.Children.Add(FormWindow.Paragraph(emulatorPath));
        ActionButton("Choose BoxedwineEngine.exe…", () =>
        {
            string? path = Dialogs.File(this, "Choose the Boxedwine emulator", "Boxedwine executable|*.exe"); if (path == null) return;
            preferences.EmulatorPath = path; repository.SavePreferences(preferences); checkedWine.Clear();
        });
        form.Button("Done", () => form.DialogResult = true, true); form.ShowDialog(); Refresh();
    }
    private void Troubleshoot(LibraryApp app)
    {
        var form = new FormWindow(this, "Troubleshooting", "Help for " + app.Name, 640, 690);
        form.Body.Children.Add(FormWindow.Heading("Check the selected program"));
        form.Body.Children.Add(FormWindow.Paragraph("Choose the app itself rather than its setup program or uninstaller. Run Another Program opens a configuration tool without changing your usual program."));
        string? action = null;
        void Button(string title, string key, bool enabled = true)
        {
            var button = new Button { Content = title, HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(0, 0, 0, 10), Padding = new Thickness(12, 7, 12, 7), IsEnabled = enabled };
            button.Click += (_, _) => { action = key; form.DialogResult = true; }; form.Body.Children.Add(button);
        }
        Button("Choose Program…", "choose", CanChange(app) && app.BuiltIn == null);
        Button("Run Another Program…", "another", CanChange(app));
        form.Body.Children.Add(FormWindow.Heading("Display and compatibility"));
        form.Body.Children.Add(FormWindow.Paragraph("Try another window size or Windows version in App Settings. GDI rendering can help older 2D apps. In Advanced, try another OpenGL implementation or Wine's GLX/EGL interface if graphics fail to start or draw correctly."));
        Button("App Settings…", "edit", CanChange(app));
        if (HasWineChoices)
        {
            form.Body.Children.Add(FormWindow.Heading("Try another Wine version"));
            form.Body.Children.Add(FormWindow.Paragraph("Create an independent test copy using another Wine release. The original app and saves stay intact."));
            Button("Try Another Wine Version…", "trial", CanChange(app));
        }
        form.Body.Children.Add(FormWindow.Heading("Installation and launch output"));
        if (app.Installer != null) Button("Run Installer Again", "installer", CanChange(app));
        Button("View Launch Log…", "log");
        form.Body.Children.Add(FormWindow.Paragraph("Warnings and a successful exit do not guarantee every app feature works. Logs can include paths and personal app output; review them before sharing."));
        form.Button("Close", () => form.DialogResult = false, true); form.ShowDialog();
        Safe(() => { switch (action) { case "choose": ChooseMain(app); break; case "another": Another(app); break; case "edit": Edit(app); break; case "trial": Trial(app); break; case "installer": Open(app, installing: true); break; case "log": ShowLog(app); break; } });
    }
    private void Storage(LibraryApp app)
    {
        RunOperation("Measuring app storage…", (token, progress) =>
        {
            long total = 0; int files = 0;
            foreach (var path in SafeFiles.Tree(repository.AppDirectory(app))) { token.ThrowIfCancellationRequested(); if (File.Exists(path)) { total += new FileInfo(path).Length; files++; } }
            return Task.FromResult((Total: total, Files: files));
        }, size =>
        {
            var form = new FormWindow(this, "App storage", $"{app.Name}\n\n{size.Total / 1048576.0:0.0} MB in {size.Files:N0} files\nShared Wine package: {(app.WinePackage?.Bytes ?? 0) / 1048576.0:0.0} MB\n\nWindows files, saves, installers and logs stay with this app.");
            form.Button("Show Files", () => Reveal(repository.AppDirectory(app))); form.Button("Close", () => form.DialogResult = true, true); form.ShowDialog();
        });
    }
    private void ShowLog(LibraryApp app)
    {
        var form = new FormWindow(this, "Launch log — " + app.Name, "Review output before sharing. It may contain file paths or personal app output.", 820, 730);
        var selection = FormWindow.Choices(new() { ["latest.log"] = "Latest attempt", ["previous.log"] = "Previous attempt", ["last-failed.log"] = "Last failed attempt" }, "latest.log"); form.Field("Attempt", selection);
        var text = new TextBox { IsReadOnly = true, AcceptsReturn = true, TextWrapping = TextWrapping.NoWrap, FontFamily = new System.Windows.Media.FontFamily("Cascadia Mono, Consolas"), FontSize = 12, Height = 430, VerticalScrollBarVisibility = ScrollBarVisibility.Auto, HorizontalScrollBarVisibility = ScrollBarVisibility.Auto, Padding = new Thickness(10) };
        form.Body.Children.Add(text);
        DateTime stamp = DateTime.MinValue;
        void RefreshLog()
        {
            string path = SafeFiles.Beneath(repository.AppDirectory(app), "Logs/" + (string)selection.SelectedValue);
            if (!File.Exists(path)) { text.Text = "No log is available for this attempt."; return; }
            var changed = File.GetLastWriteTimeUtc(path); if (changed == stamp) return; stamp = changed;
            using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
            if (stream.Length > 4 * 1024 * 1024 + 16384) { text.Text = "The log exceeds the supported display size. Open its folder to inspect it."; return; }
            using var reader = new StreamReader(stream); text.Text = RuntimeSession.PlainText(reader.ReadToEnd());
        }
        selection.SelectionChanged += (_, _) => { stamp = DateTime.MinValue; Safe(RefreshLog); };
        var timer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) }; timer.Tick += (_, _) => { try { RefreshLog(); } catch (IOException) { } }; timer.Start(); form.Closed += (_, _) => timer.Stop();
        form.Button("Save Log…", () => { var picker = new SaveFileDialog { Filter = "Text log|*.log", FileName = app.Name + ".log" }; if (picker.ShowDialog(form) == true) File.WriteAllText(picker.FileName, text.Text, new UTF8Encoding(false)); });
        form.Button("Refresh", () => { stamp = DateTime.MinValue; RefreshLog(); }); form.Button("Close", () => form.DialogResult = true, true); RefreshLog(); form.ShowDialog();
    }
    private void HelpClick(object sender, RoutedEventArgs e) => ShowHelp();
    private void ShowHelp()
    {
        var form = new FormWindow(this, "Boxedwine Help", "Bring familiar Windows apps to your PC. Compatibility varies, so try the features you care about before relying on an app.", 700, 760);
        foreach (var topic in new[]
        {
            ("Start with a demo", "Choose Demos, then Download and Install. Portable apps are ready to open; installers run in a separate emulator window. Each demo stays pinned to its Wine package."),
            ("Add an app you already have", "Choose File → Add App (Ctrl+N). Windows Installer accepts one self-contained .exe or .msi. Installer Folder copies setup and its nearby data files together. App Folder copies a portable app and its supporting files. You can also drop a folder or installer onto this window."),
            ("Choose the program after setup", "After the installer closes, select the program you want to open. Setup and maintenance tools are marked. If installation did not finish, use Troubleshooting → Run Installer Again."),
            ("Keep your files and saves", "Back Up App saves the app's Windows files, settings and Wine package in a .boxedwinebackup folder. Restore App Backup creates a separate library entry. Keep backups outside the library. Removed Apps retains files until restored or permanently deleted."),
            ("Run another program", "Use an installed utility without changing the usual program. Choose File can run an .exe or .msi from your PC; that containing folder is used directly and may be modified by the program."),
            ("Windows and Wine versions", "The Windows version changes what the app sees. Wine is the compatibility package running inside Boxedwine. App Settings changes the Windows version." + (HasWineChoices ? " Troubleshooting → Try Another Wine Version creates a separate test copy using another Wine package." : wines.Count == 1 ? $" New apps use {wines[0].Name}, downloaded automatically when needed." : "")),
            ("Keyboard shortcuts", "Ctrl+N: Add App\nCtrl+F: Find\nCtrl+O / Enter: Open selected app\nCtrl+.: Stop selected app\nCtrl+I: App Settings\nCtrl+Shift+S: Back Up App\nCtrl+Shift+L: Launch Log\nCtrl+1…5: Library sections\nCtrl+,: Settings\nDelete: Remove selected app\nF1: Help")
        }) { form.Body.Children.Add(FormWindow.Heading(topic.Item1)); form.Body.Children.Add(FormWindow.Paragraph(topic.Item2)); }
        form.Body.Children.Add(FormWindow.Heading("Where your files live"));
        string defaultDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Boxedwine");
        string label = repository.DirectoryPath.Equals(defaultDirectory, StringComparison.OrdinalIgnoreCase) ? @"%LOCALAPPDATA%\Boxedwine" : repository.DirectoryPath;
        var link = new Hyperlink(new Run(label)) { NavigateUri = new Uri(repository.DirectoryPath), ToolTip = "Open library folder in File Explorer" };
        System.Windows.Automation.AutomationProperties.SetName(link, "Open Boxedwine library folder in File Explorer");
        link.RequestNavigate += (_, e) => { e.Handled = true; Safe(() => Reveal(repository.DirectoryPath)); };
        var location = FormWindow.Paragraph("Boxedwine stores your apps in ");
        location.Inlines.Add(link);
        location.Inlines.Add(new Run(". Each app has its own Windows files and saves; identical Wine ZIPs share storage. Unfinished Work lists interrupted copies that need review."));
        form.Body.Children.Add(location);
        form.Button("Close", () => form.DialogResult = true, true); form.ShowDialog();
    }
    private void AboutClick(object sender, RoutedEventArgs e)
    {
        var form = new FormWindow(this, "About Boxedwine", "Native Windows UI\n\nBoxedwine runs 16-bit and 32-bit Windows apps by emulating a Linux environment and running Wine.\n\nCopyright © 2026 The Boxedwine Team\nGNU General Public License version 2 or later.");
        form.Button("License", () => Dialogs.Info(form, "GNU General Public License", File.ReadAllText(Path.Combine(resources, "license.txt"))));
        form.Button("Wine icon licenses", () => Dialogs.Info(form, "Wine icons", File.ReadAllText(Path.Combine(resources, "AppIcons/Wine-LICENSE.txt"))));
        form.Button("Close", () => form.DialogResult = true, true); form.ShowDialog();
    }
}
