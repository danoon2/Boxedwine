// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Threading;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckWineTools(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            App.ApplyTheme(theme);
            using var repository = new LibraryRepository(Path.Combine(destination, "wine-tools-" + theme + "-" + Guid.NewGuid().ToString("N")));
            var app = repository.AddBuiltIn("notepad", new WineReference(new string('a', 64), 100, "11.0", "30"));
            app.Arguments = ["keep this argument"]; repository.Update(app);
            var owner = new Window { ShowActivated = false, ShowInTaskbar = false, Left = -16000, Top = -16000 }; owner.Show();
            try
            {
                var picker = typeof(MainWindow).Assembly.GetType("Boxedwine.UI.Dialogs")!.GetMethod("Program", BindingFlags.Static | BindingFlags.Public)!;
                foreach (bool runOnce in new[] { true, false })
                {
                    Exception? failure = null;
                    owner.Dispatcher.BeginInvoke(() =>
                    {
                        var form = Application.Current.Windows.Cast<Window>().Single(w => w != owner);
                        try
                        {
                            var tools = Descendants<ListBox>(form).SingleOrDefault(l => AutomationProperties.GetName(l) == "Wine Tools");
                            if (!runOnce)
                            {
                                if (tools != null) throw new Exception("Choosing the main program must not include Wine tools.");
                                Descendants<Button>(form).Single(b => b.Content as string == "Cancel").RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                                return;
                            }
                            if (tools == null || tools.Items.Count != 7) throw new Exception("All seven tools must be offered even with no installed programs.");
                            var run = Descendants<Button>(form).Single(b => b.Content as string == "Run");
                            if (run.IsEnabled) throw new Exception("Run must wait for a selection.");
                            Render(form, Path.Combine(destination, theme + "-wine-tools.png"));
                            var search = Descendants<TextBox>(form).Single(t => AutomationProperties.GetName(t) == "Search programs");
                            search.Text = "REGEDIT";
                            var visible = tools.Items.Cast<ListBoxItem>().Where(i => i.Visibility == Visibility.Visible).ToList();
                            if (visible.Count != 1 || ((WineToolInfo)visible[0].Tag).Tool != WineTool.Regedit) throw new Exception("Search must include the Wine command name.");
                            tools.SelectedItem = visible[0];
                            if (!run.IsEnabled) throw new Exception("Selecting a Wine tool must enable Run.");
                            search.Text = "notepad";
                            if (run.IsEnabled || tools.SelectedItem != null) throw new Exception("A filtered-out tool must not remain runnable.");
                            search.Text = "regedit"; tools.SelectedItem = visible[0];
                            Render(form, Path.Combine(destination, theme + "-wine-tools-search.png"));
                            run.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                        }
                        catch (Exception error) { failure = error; form.Close(); }
                    }, DispatcherPriority.ApplicationIdle);
                    var choice = picker.Invoke(null, [owner, repository, app, runOnce]);
                    if (failure != null) throw failure;
                    if (runOnce && (choice == null || !Equals(choice.GetType().GetProperty("Tool")!.GetValue(choice), WineTool.Regedit) || choice.GetType().GetProperty("Path")!.GetValue(choice) != null)) throw new Exception("The picker did not return the selected Wine tool.");
                    var saved = repository.Load().Apps.Single();
                    if (saved.Executable != app.Executable || !saved.Arguments.SequenceEqual(app.Arguments)) throw new Exception("The picker changed the main program.");
                }
            }
            finally { owner.Close(); }
            Console.WriteLine("PASS " + theme + " Wine tool discovery, search, selection, cancellation and main-program isolation");
        }
    }
}
