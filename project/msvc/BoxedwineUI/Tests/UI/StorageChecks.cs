using System.ComponentModel;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckStorage(string destination)
    {
        string files = Path.Combine(destination, "measurement"); Directory.CreateDirectory(files);
        if (AppStorage.Measure(Path.Combine(files, "missing")) != null) throw new Exception("Missing app files must not be reported as zero bytes.");
        string payload = Path.Combine(files, "payload.bin");
        File.WriteAllBytes(payload, new byte[2_000_001]);
        long measured = AppStorage.Measure(files)!.Value;
        if (measured < new FileInfo(payload).Length) throw new Exception("Storage must include the allocated space of an ordinary file.");
        if (!CreateHardLinkW(Path.Combine(files, "another-name.bin"), payload, IntPtr.Zero)) throw new Win32Exception(Marshal.GetLastWin32Error());
        if (AppStorage.Measure(files) != measured) throw new Exception("Hard links must not count the same storage twice.");
        string sparse = Path.Combine(files, "sparse.bin"); File.WriteAllBytes(sparse, []);
        var sparseCommand = new System.Diagnostics.ProcessStartInfo("fsutil.exe") { UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true, RedirectStandardError = true };
        foreach (string argument in new[] { "sparse", "setflag", sparse }) sparseCommand.ArgumentList.Add(argument);
        using (var process = System.Diagnostics.Process.Start(sparseCommand)!)
        {
            process.WaitForExit();
            if (process.ExitCode != 0) throw new Exception("Could not create sparse storage fixture: " + process.StandardError.ReadToEnd());
        }
        using (var stream = File.OpenWrite(sparse)) stream.SetLength(100_000_000);
        if (AppStorage.Measure(files) >= measured + 100_000_000) throw new Exception("Sparse files must use their allocated size, not their logical length.");
        long beforeLongPath = AppStorage.Measure(files)!.Value;
        string deep = Path.Combine(files, new string('a', 100), new string('b', 100));
        Directory.CreateDirectory(deep);
        string longFile = Path.Combine(deep, new string('c', 80) + ".bin");
        File.WriteAllBytes(longFile, new byte[4096]);
        if (longFile.Length <= 260 || AppStorage.Measure(files) < beforeLongPath + 4096) throw new Exception("Storage must include installed files beyond MAX_PATH.");
        if (File.ReadAllBytes(WindowsFiles.PathForShell(longFile)).Length != 4096 || !Directory.Exists(WindowsFiles.PathForShell(files))) throw new Exception("Shell paths must resolve to the original files and folders.");
        using (var cancellation = new CancellationTokenSource())
        {
            cancellation.Cancel();
            try { AppStorage.Measure(files, cancellation.Token); throw new Exception("Storage measurement ignored cancellation."); }
            catch (OperationCanceledException) { }
        }
        Console.WriteLine("PASS storage measurement: allocated bytes, hard links, sparse files, long paths, missing files and cancellation");
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "library-" + theme);
            using (var repository = new LibraryRepository(library))
            {
                var wine = new WineReference(new string('a', 64), 100, "11.0", "20");
                repository.AddBuiltIn("notepad", wine); repository.AddBuiltIn("minesweeper", wine);
                var apps = repository.Load().Apps;
                File.WriteAllBytes(Path.Combine(repository.Root(apps[0]), "first.bin"), new byte[2_000_001]);
                File.WriteAllBytes(Path.Combine(repository.Root(apps[1]), "second.bin"), new byte[8_000_001]);
            }
            var window = new MainWindow(["--library", library, "--theme", theme]);
            var active = (LibraryRepository)typeof(MainWindow).GetField("repository", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(window)!;
            var cards = (ListBox)window.FindName("Cards");
            var details = (StackPanel)window.FindName("Details");
            TextBlock Value() => Descendants<TextBlock>(details).Single(t => AutomationProperties.GetAutomationId(t) == "app-storage-value");
            void Refresh() => typeof(MainWindow).GetMethod("RenderDetails", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, []);
            void WaitForValue()
            {
                PumpUntil(() => Value().Text != "Calculating…");
                var selected = ((LibraryRow)cards.SelectedItem).App!;
                if (Value().Text != AppStorage.Format(AppStorage.Measure(active.AppDirectory(selected))!.Value)) throw new Exception("Storage value belongs to the wrong app or could not be measured.");
            }
            Render(window, Path.Combine(destination, theme + "-before-selection.png"));
            // Change selection before the first worker can publish its result.
            cards.SelectedIndex = 0; cards.SelectedIndex = 1;
            WaitForValue();
            if (Descendants<Button>(details).Any(b => b.Content as string == "Storage and Files…")) throw new Exception("Storage must be displayed directly, not behind a button.");
            Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-storage.png"));
            var selectedApp = ((LibraryRow)cards.SelectedItem).App!;
            var sessions = (Dictionary<Guid, RuntimeSession>)typeof(MainWindow).GetField("sessions", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(window)!;
            sessions.Add(selectedApp.Id, null!);
            try
            {
                Refresh();
                if (Value().Text != "Available after app closes") throw new Exception("Storage must wait for a running app to close.");
            }
            finally { sessions.Remove(selectedApp.Id); }
            Refresh(); WaitForValue();
            new System.Windows.Interop.WindowInteropHelper(window).EnsureHandle();
            CheckDialog(window, "Troubleshoot", [selectedApp], Path.Combine(destination, theme.ToLowerInvariant() + "-files-action.png"), dialog =>
            {
                if (!Descendants<Button>(dialog).Any(b => b.Content as string == "Show App Files")) throw new Exception("Troubleshooting must retain access to app files.");
            });
            active.Remove(selectedApp.Id);
            // Refresh navigation visibility after changing the repository directly.
            typeof(MainWindow).GetMethod("Refresh", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, [null]);
            typeof(MainWindow).GetMethod("Navigate", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, ["Removed Apps", selectedApp.Id.ToString()]);
            WaitForValue();
            Render(window, Path.Combine(destination, theme.ToLowerInvariant() + "-removed-storage.png"));
            Refresh(); window.Close();
            Console.WriteLine($"PASS {theme} storage UI: selection changes, running app, removed app and file access");
        }
    }
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CreateHardLinkW(string path, string existing, IntPtr security);
}
