using Boxedwine.Library;
using System.IO.Compression;
using System.Text;
using System.Text.Json;

sealed partial class TestSuite
{
    private async Task CollectionImports()
    {
        var original = CollectionSnapshot.Load(Path.Combine(AppContext.BaseDirectory, "collection-example.json"));
        await Test("Missing Wine version selects highest current catalog version without persisting a pin", async () =>
        {
            using var repository = Repository("collection-automatic-wine"); var store = new CollectionStore(repository.DirectoryPath);
            var value = DataFormat.Clone(original); value.Apps[0].Remove("WineVersion");
            store.Merge(value, value.Folder); var recipe = store.Recipes(value.Folder, [])[0];
            CatalogWine Wine(string version) => new("Wine " + version, version, $"https://boxedwine.org/Wine{version}.zip", 1, new string('a', 64), "20");
            var wines = new[] { "9.0", "11.9", "11.10", "10.0" }.Select(Wine).ToList();
            Check(recipe.WineVersion == null && recipe.SelectWine(wines)?.WineVersion == "11.10");
            Check(recipe.SelectWine(wines.Append(Wine("12.0")))?.WineVersion == "12.0");
            Check(recipe.SelectWine([]) == null && !store.Load().Folders[0].Apps[0].ContainsKey("WineVersion"));
            Check(store.Preview(value, value.Folder).Unchanged == 1);
            value.Apps[0]["WineVersion"] = "9.0"; var pinned = value.Recipes()[0];
            Check(pinned.SelectWine(wines)?.WineVersion == "9.0" && pinned.SelectWine([Wine("12.0")]) == null);
            foreach (string bad in new[] { "", " \n ", new string('a', 129) })
            {
                value.Apps[0]["WineVersion"] = bad;
                await Throws(() => Task.Run(() => value.Recipes()), "invalid explicit Wine version");
            }
        });
        await Test("Collection app arguments round trip literally and invalid updates preserve saved data", async () =>
        {
            using var repository = Repository("collection-app-arguments"); var store = new CollectionStore(repository.DirectoryPath);
            var value = DataFormat.Clone(original);
            Check(value.Recipes()[0].AppArguments.Count == 0);
            string[] arguments = ["+timedemo", "1", "+map", "demo1.dm2", " two words ", "", "$(literal);&<>"];
            value.Apps[0]["AppArguments"] = string.Join("\r\n", arguments);
            store.Merge(value, value.Folder);
            Check(store.Recipes(value.Folder, [])[0].AppArguments.SequenceEqual(arguments));
            Check(store.Preview(value, value.Folder).Unchanged == 1);
            string path = Path.Combine(repository.DirectoryPath, "collections.json"), saved = File.ReadAllText(path);
            foreach (string bad in new[] { "a\0b", "a\rb", string.Concat(Enumerable.Repeat("a\n", 256)), new string('é', 4097), string.Join('\n', Enumerable.Repeat(new string('a', 8192), 9)) })
            {
                value.Apps[0]["AppArguments"] = bad;
                await Throws(() => Task.Run(() => store.Merge(value, value.Folder)), "invalid app arguments");
                Check(File.ReadAllText(path) == saved);
            }
            value.Apps[0]["AppArguments"] = "";
            Check(value.Recipes()[0].AppArguments.Count == 0);
        });
        await Test("Saved collection sources migrate, persist, and deduplicate", async () =>
        {
            using var repository = Repository("collection-refresh-sources"); var store = new CollectionStore(repository.DirectoryPath);
            string path = Path.Combine(repository.DirectoryPath, "collections.json");
            File.WriteAllText(path, JsonSerializer.Serialize(new { version = 1, folders = new[] { new { name = original.Folder, apps = original.Apps } } }));
            Check(store.Load().Folders[0].Sources.Count == 0);
            string source = "https://example.com/collection.json";
            store.Merge(original, original.Folder, source); store.Merge(original, "classic games", source); store.Merge(original, original.Folder);
            Check(store.Load().Folders[0].Sources.SequenceEqual(new[] { source }));
            string saved = File.ReadAllText(path);
            foreach (string bad in new[] { "file:///tmp/catalog.json", "https://user:pass@example/catalog.json", "https://example/catalog.json#fragment" })
            {
                await Throws(() => Task.Run(() => store.Merge(original, original.Folder, bad)), "invalid source URL");
                Check(File.ReadAllText(path) == saved);
            }
        });
        await Test("Refresh reviews all sources atomically and rejects stale reviews", async () =>
        {
            using var repository = Repository("collection-refresh-review"); var store = new CollectionStore(repository.DirectoryPath);
            foreach (string source in new[] { "https://example.com/a.json", "https://example.com/b.json" }) store.Merge(original, original.Folder, source);
            var retained = DataFormat.Clone(original); retained.Apps[0]["ID"] = "retained"; store.Merge(retained, original.Folder);
            var before = store.Load().Folders[0];
            var first = DataFormat.Clone(original); first.Folder = "Server renamed folder"; first.Apps[0]["Name"] = "First source";
            var last = DataFormat.Clone(first); last.Apps[0]["Name"] = "Last source wins"; last.Apps.Add(new(last.Apps[0]) { ["ID"] = "new-app" });
            string path = Path.Combine(repository.DirectoryPath, "collections.json"), saved = File.ReadAllText(path);
            var review = store.ReviewRefresh(before, [first, last]);
            Check(review.Changes == new CollectionChanges(1, 1, 0) && File.ReadAllText(path) == saved);
            await Throws(() => Task.Run(() => store.ReviewRefresh(before, [first])), "missing source");
            var invalid = DataFormat.Clone(last); invalid.Apps.Add(invalid.Apps[0]);
            await Throws(() => Task.Run(() => store.ReviewRefresh(before, [first, invalid])), "invalid source");
            Check(File.ReadAllText(path) == saved);
            store.ApplyRefresh(review); var folder = store.Load().Folders[0];
            Check(folder.Name == original.Folder && folder.Apps.Count == 3 && folder.Apps[0]["Name"] == "Last source wins" && folder.Sources.SequenceEqual(before.Sources));
            var again = store.ReviewRefresh(folder, [first, last]); Check(again.Changes == new CollectionChanges(0, 0, 2));
            await Throws(() => Task.Run(() => store.ApplyRefresh(review)), "stale review");
            store.Remove(original.Folder);
            await Throws(() => Task.Run(() => store.ApplyRefresh(again)), "deleted folder");
        });
        await Test("Cancelled refresh makes no request and keeps saved collections", async () =>
        {
            using var repository = Repository("collection-refresh-cancel"); var store = new CollectionStore(repository.DirectoryPath);
            store.Merge(original, original.Folder, "https://example.invalid/collection.json");
            string path = Path.Combine(repository.DirectoryPath, "collections.json"), saved = File.ReadAllText(path);
            using var cancellation = new CancellationTokenSource(); cancellation.Cancel();
            await Throws(() => store.Refresh(original.Folder, cancellation.Token, null), "cancelled refresh");
            Check(File.ReadAllText(path) == saved);
        });
        await Test("Collection icon URLs update by stable ID and reject unsafe sources", async () =>
        {
            using var repository = Repository("collection-icons-update");
            var store = new CollectionStore(repository.DirectoryPath);
            var value = DataFormat.Clone(original); value.Apps[0]["Icon"] = "https://images.example/game.png?v=1";
            store.Merge(value, value.Folder);
            Check(store.Recipes(value.Folder, [])[0].Icon == value.Apps[0]["Icon"]);
            value.Apps[0]["Icon"] = "https://images.example/game.png?v=2";
            Check(store.Preview(value, value.Folder) == new CollectionChanges(0, 1, 0));
            store.Merge(value, value.Folder); Check(store.Recipes(value.Folder, []).Count == 1);
            foreach (string icon in new[] { "file:///tmp/icon.png", "https://user:password@example/icon.png", "https://example/icon.png#fragment", "../icon.png" })
            {
                value.Apps[0]["Icon"] = icon;
                await Throws(() => Task.Run(() => value.Recipes()), "unsafe icon");
            }
        });
        await Test("Remote PNGs cache offline; invalid, oversized, and missing icons use fallback", async () =>
        {
            using var repository = Repository("collection-icons-cache");
            var cache = new CollectionIcons(repository.DirectoryPath);
            byte[] png = Convert.FromBase64String("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=");
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(15));
            var listener = new System.Net.Sockets.TcpListener(System.Net.IPAddress.Loopback, 0); listener.Start();
            string url = $"http://127.0.0.1:{((System.Net.IPEndPoint)listener.LocalEndpoint).Port}";
            var server = Task.Run(async () =>
            {
                for (int i = 0; i < 5; i++)
                {
                    using var client = await listener.AcceptTcpClientAsync(timeout.Token);
                    await using var stream = client.GetStream();
                    using var reader = new StreamReader(stream, leaveOpen: true);
                    string request = await reader.ReadLineAsync(timeout.Token) ?? "";
                    while (!string.IsNullOrEmpty(await reader.ReadLineAsync(timeout.Token))) { }
                    byte[] data = request.Contains("/invalid") ? Encoding.UTF8.GetBytes("not an image") : (byte[])png.Clone();
                    if (request.Contains("/dimensions")) { data[16] = 0; data[17] = 0; data[18] = 16; data[19] = 0; }
                    bool large = request.Contains("/large");
                    string status = request.Contains("/missing") ? "404 Not Found" : "200 OK";
                    await stream.WriteAsync(Encoding.ASCII.GetBytes($"HTTP/1.1 {status}\r\nContent-Length: {(large ? 1048577 : data.Length)}\r\nConnection: close\r\n\r\n"), timeout.Token);
                    if (!large) await stream.WriteAsync(data, timeout.Token);
                }
            });
            try
            {
                await cache.Fetch(url + "/icon.png");
                string path = cache.CachedPath(url + "/icon.png") ?? throw new Exception("PNG not cached");
                Check(File.ReadAllBytes(path).SequenceEqual(png));
                await cache.Fetch(url + "/icon.png"); // Reuse must not make a sixth request.
                foreach (string name in new[] { "invalid", "large", "dimensions", "missing" })
                {
                    await cache.Fetch(url + "/" + name);
                    Check(cache.CachedPath(url + "/" + name) == null);
                }
                await server;
                Check(cache.CachedPath(url + "/icon.png?v=2") == null);
                Check(Directory.GetFiles(Path.GetDirectoryName(path)!).Length == 1);
            }
            finally { listener.Stop(); }
            await cache.Fetch(url + "/icon.png");
            Check(cache.CachedPath(url + "/icon.png") != null);
        });
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
