using System.IO;
using System.Reflection;
using System.Windows.Controls;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckLaunchCards(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "launch-cards-" + theme + "-" + Guid.NewGuid());
            using (var repository = new LibraryRepository(library))
            {
                var wine = new WineReference(new string('a', 64), 100, "11.0", "39");
                for (int i = 0; i < 20; i++)
                {
                    var item = repository.AddBuiltIn("notepad", wine);
                    item.Name = $"App {i + 1:00} with a longer title"; repository.Update(item);
                }
            }
            var window = new MainWindow(["--library", library, "--theme", theme]);
            var active = (LibraryRepository)typeof(MainWindow).GetField("repository", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(window)!;
            var apps = active.Load().Apps;
            Render(window, Path.Combine(destination, theme + "-idle.png"));
            CheckLaunching(window, apps[0], destination, theme + "-top");
            var cards = (ListBox)window.FindName("Cards");
            var scroll = Descendants<ScrollViewer>(cards).First();
            scroll.ScrollToEnd(); window.UpdateLayout();
            if (scroll.VerticalOffset <= 0) throw new Exception("Expected a scrolled library.");
            CheckLaunching(window, apps[^1], destination, theme + "-scrolled");
            window.Close();
        }
    }
}
