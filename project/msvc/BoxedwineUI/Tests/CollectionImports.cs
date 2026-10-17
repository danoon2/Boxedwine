using Boxedwine.Library;
using System.IO.Compression;
using System.Text;
using System.Text.Json;

sealed partial class TestSuite
{
    private async Task CollectionImports()
    {
        var original = CollectionSnapshot.Load(Path.Combine(AppContext.BaseDirectory, "collection-example.json"));
        await Test("Repeated collections merge stable IDs, preserve missing entries, and survive reopening", () =>
        {
            using var repository = Repository("collections-repeat");
            var store = new CollectionStore(repository.DirectoryPath);
            store.Merge(original, original.Folder);
            string id = store.Recipes(original.Folder, [])[0].Origin.Id;
            Check(store.Preview(original, "classic games") == new CollectionChanges(0, 0, 1));
            store.Merge(original, "classic games");
            var changed = DataFormat.Clone(original);
            changed.Apps[0]["Name"] = "Renamed game";
            changed.Apps[0]["FileURL"] = "https://other.example/new.zip";
            changed.Apps[0]["FileSHA256"] = new string('b', 64);
            var added = new Dictionary<string, string>(changed.Apps[0]) { ["ID"] = "new-game" };
            changed.Apps.Add(added);
            Check(store.Preview(changed, original.Folder) == new CollectionChanges(1, 1, 0));
            store.Merge(changed, original.Folder); store.Merge(changed, original.Folder);
            var reopened = new CollectionStore(repository.DirectoryPath);
            Check(reopened.Load().Folders.Count == 1);
            var recipes = reopened.Recipes(original.Folder, []);
            Check(recipes.Count == 2 && recipes[0].Name == "Renamed game" && recipes[0].Origin.Id == id);
            reopened.Merge(original, original.Folder);
            Check(reopened.Recipes(original.Folder, []).Count == 2);
            return Task.CompletedTask;
        });
        await Test("Deleting collections and resetting Demos preserve installed files", () =>
        {
            using var repository = Repository("collections-delete");
            string save = Path.Combine(repository.DirectoryPath, "save.dat"); File.WriteAllText(save, "progress");
            var store = new CollectionStore(repository.DirectoryPath);
            store.Merge(original, "Demos"); store.Merge(original, original.Folder);
            Check(store.Recipes("Demos", [])[0].Origin.Id != store.Recipes(original.Folder, [])[0].Origin.Id);
            store.Remove("Demos"); store.Remove(original.Folder);
            Check(store.Load().Folders.Count == 0 && File.ReadAllText(save) == "progress");
            return Task.CompletedTask;
        });
        await Test("Longest collection IDs keep a portable saved library identity", () =>
        {
            using var repository = Repository("collections-long-id");
            var store = new CollectionStore(repository.DirectoryPath);
            var value = DataFormat.Clone(original); value.Apps[0]["ID"] = new string('a', 64);
            store.Merge(value, value.Folder);
            string id = store.Recipes(value.Folder, [])[0].Origin.Id;
            Check(System.Text.RegularExpressions.Regex.IsMatch(id, "^[a-z0-9][a-z0-9-]{0,63}$"));
            Check(store.Recipes("classic games", [])[0].Origin.Id == id);
            return Task.CompletedTask;
        });
        await Test("Duplicate IDs and invalid sources cannot replace saved collections", async () =>
        {
            using var repository = Repository("collections-invalid"); var store = new CollectionStore(repository.DirectoryPath);
            store.Merge(original, original.Folder);
            string path = Path.Combine(repository.DirectoryPath, "collections.json"); string saved = File.ReadAllText(path);
            var invalid = DataFormat.Clone(original); invalid.Apps.Add(invalid.Apps[0]);
            await Throws(() => Task.Run(() => store.Merge(invalid, original.Folder)), "duplicate ID");
            invalid = DataFormat.Clone(original); invalid.Apps[0]["FileURL"] = "file:///etc/passwd";
            await Throws(() => Task.Run(() => store.Merge(invalid, original.Folder)), "local URL");
            await Throws(() => Task.Run(() => store.Merge(original, "All Apps")), "reserved folder");
            Check(File.ReadAllText(path) == saved);
        });
        await Test("ZIP inspection preserves data and ambiguous choices; selected program imports", async () =>
        {
            string zipPath = Path.Combine(work, "source-game.zip");
            using (var zip = ZipFile.Open(zipPath, ZipArchiveMode.Create))
            {
                Put(zip, "game/Game.exe", Encoding.UTF8.GetBytes("MZgame")); Put(zip, "game/data.bin", Encoding.UTF8.GetBytes("support")); Put(zip, "setup.exe", Encoding.UTF8.GetBytes("MZsetup"));
            }
            string staged;
            using (var prepared = await PreparedAppSource.Prepare(zipPath, false, false, default, null))
            {
                staged = prepared.Source;
                Check(prepared.SuggestedAction == "" && prepared.Programs.Contains("game/Game.exe") && prepared.Programs.Contains("setup.exe"));
                Check(File.ReadAllText(Path.Combine(prepared.Source, "game/data.bin")) == "support");
                using var repository = Repository("zip-import");
                var app = await repository.Import(prepared.Source, "folder", "wineDefault", wine, default, program: "game/Game.exe", name: "Game");
                Check(app.Executable == LibraryRepository.DriveC + "/App/game/Game.exe");
            }
            Check(!Directory.Exists(staged) && File.Exists(zipPath));
        });
        await Test("Unsafe ZIPs and cancelled inspection do not escape staging", async () =>
        {
            string path = Path.Combine(work, "unsafe.zip");
            using (var zip = ZipFile.Open(path, ZipArchiveMode.Create)) Put(zip, "../escape.exe", [1]);
            await Throws(() => PreparedAppSource.Prepare(path, false, false, default, null), "traversal");
            Check(!File.Exists(Path.Combine(work, "escape.exe")));
            using var cancel = new CancellationTokenSource(); cancel.Cancel();
            await Throws(() => PreparedAppSource.Prepare(path, false, false, cancel.Token, null), "cancelled");
        });
        await Test("User URL ZIP download enters the same review and removes temporary files", async () =>
        {
            using var memory = new MemoryStream();
            using (var zip = new ZipArchive(memory, ZipArchiveMode.Create, true)) Put(zip, "Game.exe", Encoding.UTF8.GetBytes("MZgame"));
            byte[] payload = memory.ToArray();
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(10));
            var listener = new System.Net.Sockets.TcpListener(System.Net.IPAddress.Loopback, 0); listener.Start();
            try
            {
                int port = ((System.Net.IPEndPoint)listener.LocalEndpoint).Port;
                var server = Task.Run(async () =>
                {
                    using var client = await listener.AcceptTcpClientAsync(timeout.Token);
                    await using var stream = client.GetStream();
                    byte[] request = new byte[4096];
                    Check(await stream.ReadAsync(request, timeout.Token) > 0);
                    await stream.WriteAsync(Encoding.ASCII.GetBytes($"HTTP/1.1 200 OK\r\nContent-Length: {payload.Length}\r\nConnection: close\r\n\r\n"), timeout.Token);
                    await stream.WriteAsync(payload, timeout.Token);
                });
                string staging;
                using (var prepared = await PreparedAppSource.Prepare($"http://127.0.0.1:{port}/game.zip", true, false, timeout.Token, null))
                {
                    staging = prepared.Source;
                    Check(prepared.SuggestedAction == "app" && prepared.Programs.SequenceEqual(new[] { "Game.exe" }));
                }
                Check(!Directory.Exists(staging));
                await server;
            }
            finally { listener.Stop(); }
        });
    }
}
