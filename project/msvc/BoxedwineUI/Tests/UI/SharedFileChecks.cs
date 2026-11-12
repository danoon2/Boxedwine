using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckSharedFiles(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "shared-" + theme + "-" + Guid.NewGuid().ToString("N"));
            using (var repository = new LibraryRepository(library))
                repository.AddBuiltIn("notepad", new WineReference(new string('a', 64), 100, "11.0", "32"));
            var window = new MainWindow(["--library", library, "--theme", theme]);
            var cards = (ListBox)window.FindName("Cards");
            cards.SelectedIndex = 0;
            Render(window, Path.Combine(destination, theme + "-shared-library.png"));
            new System.Windows.Interop.WindowInteropHelper(window).EnsureHandle();
            var app = ((LibraryRow)cards.SelectedItem).App!;
            var target = cards.ItemContainerGenerator.ContainerFromItem(cards.SelectedItem) as ListBoxItem;
            var route = typeof(MainWindow).GetMethod("SharedDropApp", BindingFlags.Instance | BindingFlags.NonPublic)!;
            if (target == null || route.Invoke(window, [target]) is not LibraryApp routed || routed.Id != app.Id)
                throw new Exception("App-card drops must select the card's own app.");
            if (route.Invoke(window, [cards]) != null) throw new Exception("Empty library space must keep the add-app flow.");
            var sessions = (Dictionary<Guid, RuntimeSession>)typeof(MainWindow).GetField("sessions", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(window)!;
            foreach (bool running in new[] { false, true })
            {
                if (running) sessions.Add(app.Id, null!);
                try
                {
                    CheckDialog(window, "ShowSharedFiles", [app, null!], Path.Combine(destination, theme + "-shared-" + running + ".png"), dialog =>
                    {
                        var run = Descendants<Button>(dialog).Single(b => b.Content as string == "Run…");
                        if (run.IsEnabled == running) throw new Exception("Run must be offered only when the app is stopped.");
                        if (!Descendants<Button>(dialog).Single(b => b.Content as string == "Show Folder").IsEnabled)
                            throw new Exception("Shared Files must remain accessible while running.");
                    });
                }
                finally { sessions.Remove(app.Id); }
            }
            window.Close();
            Console.WriteLine("PASS " + theme + " shared-files target routing and stopped/running controls");
        }
    }
}
