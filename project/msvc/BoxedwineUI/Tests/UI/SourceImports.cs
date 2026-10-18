using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Threading;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckSourceImports(string destination)
    {
        const string json = """
            {"schemaVersion":1,"folder":"Retry Test","apps":[{"ID":"test","Name":"Test","FileURL":"https://example.com/game.zip","FileSizeBytes":"1","FileSHA256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","InstallType":"Zip","ShortcutExe":"game.exe"}]}
            """;
        using var stop = new CancellationTokenSource();
        using var slowRequested = new ManualResetEventSlim();
        using var listener = new TcpListener(IPAddress.Loopback, 0); listener.Start();
        string address = "http://127.0.0.1:" + ((IPEndPoint)listener.LocalEndpoint).Port;
        var server = Task.Run(async () =>
        {
            try
            {
                while (!stop.IsCancellationRequested)
                {
                    using var client = await listener.AcceptTcpClientAsync(stop.Token);
                    using var stream = client.GetStream(); using var reader = new StreamReader(stream, leaveOpen: true);
                    try
                    {
                        string request = await reader.ReadLineAsync(stop.Token) ?? "";
                        while (!string.IsNullOrEmpty(await reader.ReadLineAsync(stop.Token))) { }
                        if (request.Contains("/slow.json")) { slowRequested.Set(); await Task.Delay(700, stop.Token); }
                        byte[] body = Encoding.UTF8.GetBytes(request.Contains("/valid.json") ? json : "not a collection or ZIP");
                        string status = request.Contains("/missing.") ? "404 Not Found" : "200 OK";
                        byte[] header = Encoding.ASCII.GetBytes($"HTTP/1.1 {status}\r\nContent-Length: {body.Length}\r\nConnection: close\r\n\r\n");
                        await stream.WriteAsync(header, stop.Token); await stream.WriteAsync(body, stop.Token);
                    }
                    catch (IOException) { /* A cancelled request closes its connection. */ }
                }
            }
            catch (OperationCanceledException) when (stop.IsCancellationRequested) { }
        });
        try
        {
            foreach (string theme in new[] { "Light", "Dark" })
            {
                App.ApplyTheme(theme);
                var owner = new Window { Title = "Import regression", Width = 640, Height = 480 }; owner.Show();
                foreach (bool collection in new[] { true, false })
                {
                    Exception? failure = null;
                    owner.Dispatcher.BeginInvoke(() =>
                    {
                        var form = Application.Current.Windows.Cast<Window>().Single(w => w != owner);
                        try
                        {
                            var link = Descendants<TextBox>(form).Single();
                            var error = Descendants<TextBlock>(form).Single(w => AutomationProperties.GetAutomationId(w) == "add-source-error");
                            var download = Descendants<Button>(form).Single(w => w.Content as string == "Download & Continue");
                            void Click(Button b) => b.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                            void NoModal()
                            {
                                if (!form.IsVisible || Application.Current.Windows.Count != 2) throw new Exception("Import failure must stay inline in the source dialog.");
                            }
                            link.Text = "typo"; Click(download); NoModal();
                            if (!error.IsVisible || link.Text != "typo") throw new Exception("Invalid URL lost its inline error or entered text.");
                            foreach (string path in new[] { "/missing.json", collection ? "/invalid.json" : "/invalid.zip" })
                            {
                                link.Text = address + path;
                                if (error.Visibility != Visibility.Collapsed) throw new Exception("Editing must clear the old error.");
                                Click(download); PumpUntil(() => download.IsEnabled); NoModal();
                                if (!error.IsVisible || link.Text != address + path) throw new Exception("Failed download lost its inline error or URL.");
                            }
                            Render(form, Path.Combine(destination, $"{theme}-{collection}-import-error.png"));
                            slowRequested.Reset(); link.Text = address + "/slow.json"; Click(download);
                            PumpUntil(() => slowRequested.IsSet);
                            Click(Descendants<Button>(form).Single(w => w.Content as string == "Cancel Download"));
                            PumpUntil(() => download.IsEnabled); NoModal();
                            if (error.IsVisible || link.Text != address + "/slow.json") throw new Exception("Cancel must preserve the source without an error.");
                            link.Text = address + "/valid.json"; Click(download);
                            PumpUntil(() => !form.IsVisible || download.IsEnabled);
                            if (form.IsVisible) throw new Exception("Corrected URL did not continue: " + error.Text);
                        }
                        catch (Exception error) { failure = error; form.Close(); }
                    }, DispatcherPriority.ApplicationIdle);
                    var method = typeof(MainWindow).Assembly.GetType("Boxedwine.UI.Dialogs")!.GetMethod("AddSource", BindingFlags.Public | BindingFlags.Static)!;
                    object? selection = method.Invoke(null, [owner, collection]);
                    using var prepared = (PreparedAppSource?)selection?.GetType().GetProperty("Prepared")!.GetValue(selection);
                    if (failure != null) throw failure;
                    if (prepared?.Collection?.Folder != "Retry Test" || prepared.Original != address + "/valid.json") throw new Exception("Successful retry did not return the corrected collection.");
                    if (Application.Current.Windows.Count != 1) throw new Exception("A stale error dialog remained after successful retry.");
                }
                owner.Close();
            }
        }
        finally { stop.Cancel(); server.GetAwaiter().GetResult(); }
        Console.WriteLine("PASS: inline URL validation, HTTP/content failures, cancellation, corrected URL retry, and no delayed alerts in both themes.");
    }
}
