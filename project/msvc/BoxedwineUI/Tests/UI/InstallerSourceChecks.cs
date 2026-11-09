using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckInstallerSources(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            App.ApplyTheme(theme);
            string media = Path.Combine(destination, "Setup media " + theme); Directory.CreateDirectory(media);
            string setup = Path.Combine(media, "TheMoviesSetup.exe"); File.WriteAllText(setup, "MZsetup");
            File.WriteAllText(Path.Combine(media, "TheMoviesSetup.split.bin"), "payload");
            using var prepared = PreparedAppSource.Prepare(setup, false, false, default, null).GetAwaiter().GetResult();
            var owner = new Window { Title = "Installer source review", Width = 640, Height = 480, ShowActivated = false, ShowInTaskbar = false, Left = -16000, Top = -16000 }; owner.Show();
            Exception? failure = null;
            owner.Dispatcher.BeginInvoke(() =>
            {
                var form = Application.Current.Windows.Cast<Window>().Single(w => w != owner);
                try
                {
                    var action = Descendants<ComboBox>(form).Single(c => c.ItemsSource is Dictionary<string, string> values && values.ContainsKey("installer"));
                    action.SelectedValue = "installer";
                    if (!Descendants<TextBlock>(form).Any(t => t.Text.Contains(media) && t.Text.Contains("without copying"))) throw new Exception("Installer review did not disclose the mounted source directory.");
                    Render(form, Path.Combine(destination, theme + "-installer-source.png"));
                    Descendants<Button>(form).Single(b => b.Content as string == "Run Installer").RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                }
                catch (Exception error) { failure = error; form.Close(); }
            }, DispatcherPriority.ApplicationIdle);
            var method = typeof(MainWindow).Assembly.GetType("Boxedwine.UI.Dialogs")!.GetMethod("ReviewApp", BindingFlags.Static | BindingFlags.Public)!;
            var wine = new CatalogWine("Wine 11", "11.0", "https://example.com/wine.zip", 100, new string('a', 64), "14");
            var review = method.Invoke(null, [owner, prepared, new[] { wine }, null]);
            owner.Close();
            if (failure != null) throw failure;
            if (review?.GetType().GetProperty("Kind")!.GetValue(review) as string != "installer" || prepared.RequiresCopy) throw new Exception("Local installer review must select the source-mount import path.");
            Console.WriteLine("PASS " + theme + " local installer review names the mounted folder and retains EXE selection");
        }
    }
}
