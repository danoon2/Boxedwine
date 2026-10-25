const assert = require('node:assert/strict');
const { readFileSync } = require('node:fs');
const path = require('node:path');
const { test } = require('node:test');
const vm = require('node:vm');
const zlib = require('node:zlib');
const crypto = require('node:crypto');

const source = readFileSync(path.join(__dirname, 'boxedwine-shell.js'), 'utf8');

function launcher(url) {
    const elements = new Map();
    const mounts = [];
    const directories = [];
    const context = vm.createContext({
        window: { location: { href: url }, addEventListener() {} },
        document: {
            addEventListener() {},
            getElementById(id) {
                if (!elements.has(id)) {
                    elements.set(id, { style: {}, value: '', addEventListener() {}, getContext() {}, contains() { return false; } });
                }
                return elements.get(id);
            },
        },
        console: { log() {}, error() {}, warn() {} },
        IDBFS: {},
        FS: {
            mkdirTree(directory) { directories.push(directory); },
            mount(_type, options, directory) { mounts.push({ directory, autoPersist: options.autoPersist }); },
            syncfs(populate, callback) { assert.equal(populate, true); callback(null); },
        },
    });
    vm.runInContext(source, context);
    context.setConfiguration();
    let initialized = false;
    context.initBrowserFilesystem(() => { initialized = true; });
    assert.equal(initialized, true);
    const result = JSON.parse(vm.runInContext(
        'JSON.stringify({root: ROOT, drive: Config.d_drive, params: getEmulatorParams()})', context,
    ));
    assert.equal(result.params[result.params.indexOf('-root') + 1], result.root);
    assert.equal(result.params[result.params.indexOf('-mount_drive') + 1], result.drive);
    assert.deepEqual(directories, [result.root, result.drive]);
    return { ...result, mounts };
}

class Link {
    constructor(target) { this.target = target; }
}

// A DOM and file system just big enough for the dialogs; records every innerHTML write
// and every download. In `tree` objects are folders, strings or bytes are files and a
// Link is a symlink. Every file was last modified at fileDate.
const fileDate = new Date(2024, 5, 1, 12, 30, 10);
function dialogShell(tree, url = 'http://localhost/') {
    const byId = new Map();
    const htmlWrites = [];
    const blobs = new Map();
    const downloads = [];
    class Element {
        constructor() {
            this.children = [];
            this.style = {};
            this.value = '';
            this.listeners = {};
            this.text = '';
            this.attributes = {};
        }
        setAttribute(name, value) { this.attributes[name] = value; }
        click() {
            if (this.attributes.download) {
                downloads.push({ name: this.attributes.download, blob: blobs.get(this.attributes.href) });
            }
            return this.dispatch('click', {});
        }
        set id(id) { this.elementId = id; byId.set(id, this); }
        get id() { return this.elementId; }
        set textContent(text) { this.text = String(text); this.children = []; }
        get textContent() { return this.text + this.children.map((child) => child.textContent).join(''); }
        set innerHTML(html) { htmlWrites.push(String(html)); this.text = ''; this.children = []; }
        appendChild(child) { this.children.push(child); return child; }
        addEventListener(type, listener) { (this.listeners[type] ||= []).push(listener); }
        dispatch(type, event) { return Promise.all((this.listeners[type] || []).map((listener) => listener(event))); }
        all() { return [this, ...this.children.flatMap((child) => child.all())]; }
        getContext() {}
        contains() { return false; }
    }
    const DIR = 0o040000, FILE = 0o100000, LINK = 0o120000;
    const names = (path) => path.split('/').filter(Boolean);
    const lookup = (path, follow) => {
        const node = names(path).reduce((dir, name) => dir && dir[name], tree);
        return follow && node instanceof Link ? lookup(node.target, true) : node;
    };
    const mode = (node) => node instanceof Link ? LINK : typeof node === 'string' || ArrayBuffer.isView(node) ? FILE : DIR;
    const context = vm.createContext({
        window: {
            location: { href: url },
            URL: {
                createObjectURL(blob) {
                    const href = 'blob:' + blobs.size;
                    blobs.set(href, blob);
                    return href;
                },
                revokeObjectURL(href) { blobs.delete(href); },
            },
            listeners: {},
            addEventListener(type, listener) { this.listeners[type] = listener; },
        },
        history: { urls: [], replaceState(state, title, url) { this.urls.push(url); } },
        document: {
            addEventListener() {},
            createElement: () => new Element(),
            getElementById(id) {
                if (!byId.has(id)) new Element().id = id;
                return byId.get(id);
            },
            body: { appendChild() {}, removeChild() {} },
        },
        console: { log() {}, error() {}, warn() {} },
        TextEncoder, TextDecoder, Blob, Response, CompressionStream, DecompressionStream,
        IDBFS: {},
        FS: {
            readdir: (path) => ['.', '..', ...Object.keys(lookup(path, true))],
            readFile(path) {
                const node = lookup(path, true);
                if (mode(node) !== FILE) throw new Error('ENOENT ' + path);
                return new Uint8Array(Buffer.from(node));
            },
            stat: () => ({ mtime: fileDate }),
            lookupPath: (path, options) => ({ node: { mode: mode(lookup(path, options.follow)) } }),
            analyzePath: (path) => ({ exists: lookup(path, true) !== undefined }),
            isDir: (value) => value === DIR,
            isLink: (value) => value === LINK,
            mkdirTree: (path) => names(path).reduce((dir, name) => (dir[name] ||= {}), tree),
            writeFile(path, data) {
                const parts = names(path);
                lookup('/' + parts.slice(0, -1).join('/'), true)[parts.at(-1)] = data;
            },
        },
    });
    vm.runInContext(source, context);
    return { context, htmlWrites, byId, downloads };
}

// Reads a zip with Node's zlib, independently of the shell's own zip reader,
// checking every entry's CRC-32.
function readZip(bytes) {
    const zip = Buffer.from(bytes);
    const end = zip.lastIndexOf(Buffer.from([0x50, 0x4b, 0x05, 0x06]));
    let offset = zip.readUInt32LE(end + 16);
    const entries = {};
    for (let i = zip.readUInt16LE(end + 10); i > 0; i--) {
        assert.equal(zip.readUInt32LE(offset), 0x02014b50);
        const nameLength = zip.readUInt16LE(offset + 28);
        const name = zip.toString('utf8', offset + 46, offset + 46 + nameLength);
        const method = zip.readUInt16LE(offset + 10);
        const local = zip.readUInt32LE(offset + 42);
        const start = local + 30 + zip.readUInt16LE(local + 26) + zip.readUInt16LE(local + 28);
        const body = zip.subarray(start, start + zip.readUInt32LE(offset + 20));
        const data = method === 8 ? zlib.inflateRawSync(body) : body;
        assert.equal(zlib.crc32(data), zip.readUInt32LE(offset + 16), name);
        assert.equal(data.length, zip.readUInt32LE(offset + 24), name);
        entries[name] = { method, data, dosTime: zip.readUInt32LE(offset + 12), attributes: zip.readUInt32LE(offset + 38) };
        offset += 46 + nameLength + zip.readUInt16LE(offset + 30) + zip.readUInt16LE(offset + 32);
    }
    return entries;
}

// clicks the Get File(s) tree entry labelled text, then Get
async function getFromTree(context, byId, text) {
    byId.get('tree').all().find((element) => element.textContent === text && element.listeners.click).click();
    await context.extract();
}

// Browser drag and drop entries; file() and readEntries() answer asynchronously like the real ones.
const droppedFile = (fullPath, content) => ({
    isDirectory: false,
    fullPath,
    file: (resolve, reject) => setImmediate(() => content === null
        ? reject(new Error('NotReadableError'))
        : resolve({ arrayBuffer: async () => new Uint8Array(Buffer.from(content)).buffer })),
});

// Builds a zip: .txt files and folders are stored, everything else deflated. Each local
// header gets an extra field the central directory lacks, as real zip tools often write.
function makeZip(files) {
    const locals = [];
    const centrals = [];
    let offset = 0;
    for (const [name, content] of Object.entries(files)) {
        const nameBytes = Buffer.from(name);
        const raw = Buffer.from(content);
        const method = name.endsWith('/') || name.endsWith('.txt') ? 0 : 8;
        const body = method === 8 ? zlib.deflateRawSync(raw) : raw;
        const extra = Buffer.from([0xfe, 0xca, 0, 0]);
        const local = Buffer.alloc(30);
        local.writeUInt32LE(0x04034b50, 0);
        local.writeUInt16LE(20, 4);
        local.writeUInt16LE(0x800, 6);
        local.writeUInt16LE(method, 8);
        local.writeUInt32LE(zlib.crc32(raw), 14);
        local.writeUInt32LE(body.length, 18);
        local.writeUInt32LE(raw.length, 22);
        local.writeUInt16LE(nameBytes.length, 26);
        local.writeUInt16LE(extra.length, 28);
        const central = Buffer.alloc(46);
        central.writeUInt32LE(0x02014b50, 0);
        central.writeUInt16LE(20, 4);
        central.writeUInt16LE(20, 6);
        central.writeUInt16LE(0x800, 8);
        central.writeUInt16LE(method, 10);
        central.writeUInt32LE(zlib.crc32(raw), 16);
        central.writeUInt32LE(body.length, 20);
        central.writeUInt32LE(raw.length, 24);
        central.writeUInt16LE(nameBytes.length, 28);
        central.writeUInt32LE(offset, 42);
        locals.push(local, nameBytes, extra, body);
        centrals.push(central, nameBytes);
        offset += local.length + nameBytes.length + extra.length + body.length;
    }
    const directory = Buffer.concat(centrals);
    const end = Buffer.alloc(22);
    end.writeUInt32LE(0x06054b50, 0);
    end.writeUInt16LE(Object.keys(files).length, 8);
    end.writeUInt16LE(Object.keys(files).length, 10);
    end.writeUInt32LE(directory.length, 12);
    end.writeUInt32LE(offset, 16);
    return Buffer.concat([...locals, directory, end]);
}

async function dropOnto(tree, entries) {
    const { context, byId } = dialogShell(tree);
    vm.runInContext('Config.d_drive = "/d"; Config.isAutoRunSet = false;', context);
    await byId.get('dropzone').dispatch('drop', {
        preventDefault() {},
        dataTransfer: { items: entries.map((entry) => ({ webkitGetAsEntry: () => entry })) },
    });
    const listed = byId.get('items').all().filter((element) => element.listeners.click).map((element) => element.textContent);
    return { message: byId.get('message').textContent, listed };
}
const droppedFolder = (fullPath, batches) => ({
    isDirectory: true,
    fullPath,
    createReader() {
        const pending = [...batches, []];
        return { readEntries: (resolve) => setImmediate(() => resolve(pending.shift())) };
    },
});

test('file dialogs show hostile file names as text', () => {
    const file = "a'><img src=x onerror=alert(1)>.txt";
    const dir = "x');alert(1);('";
    const exe = '/game/<img src=x onerror=alert(2)>.exe';
    const { context, htmlWrites, byId } = dialogShell({ d: { [file]: 'data', [dir]: { 'inner.txt': 'data' } } });

    htmlWrites.length = 0; // ignore the page's own startup status text
    context.buildGetFilesModalForDrives({ 'd:': '/d' });
    context.populateModalExe([exe]);
    assert.deepEqual(htmlWrites.filter((html) => html !== ''), []);

    const shown = byId.get('tree').all().concat(byId.get('items').all());
    const span = (text) => shown.find((element) => element.textContent === text && element.listeners.click);
    assert.ok(shown.some((element) => element.textContent === exe));
    span(file).click();
    assert.equal(byId.get('selectedItem').value, 'd:/' + file);
    span('[' + dir + ']').click();
    assert.equal(byId.get('selectedItem').value, 'd:/' + dir + '/');

    // the folder still expands and collapses through its +/- links
    const folder = byId.get(span('[' + dir + ']').id.split('-')[0]);
    assert.equal(folder.style.display, 'none');
    byId.get(folder.id + '-expand').children[0].click();
    assert.equal(folder.style.display, '');
    assert.equal(byId.get('1').style.display, '');
});

test('Get File(s) lists files but does not follow symlinks', () => {
    const { context, byId } = dialogShell({ d: { 'save.dat': 'data', loop: new Link('/d'), saves: { 'slot1.dat': 'data' } } });
    context.buildGetFilesModalForDrives({ 'd:': '/d' });
    const labels = byId.get('tree').all().filter((element) => element.listeners.click && /-data$/.test(element.id))
        .map((element) => element.textContent);
    assert.deepEqual(labels.sort(), ['[d:]', '[saves]', 'save.dat', 'slot1.dat']);
});

test('a dropped folder is copied to d: before its programs are listed', async () => {
    const tree = { d: {} };
    const { context, byId } = dialogShell(tree);
    vm.runInContext('Config.d_drive = "/d"; Config.isAutoRunSet = false;', context);
    const game = droppedFolder('/game', [
        [droppedFile('/game/GAME.EXE', 'mz'), droppedFolder('/game/data', [[droppedFile('/game/data/pack.zip', 'zip')]])],
        [droppedFile('/game/run.bat', 'bat'), droppedFile('/game/broken.txt', null)],
    ]);
    await byId.get('dropzone').dispatch('drop', {
        preventDefault() {},
        dataTransfer: { items: [{ webkitGetAsEntry: () => game }, { webkitGetAsEntry: () => null }] },
    });

    const text = (bytes) => new TextDecoder().decode(bytes);
    assert.equal(text(tree.d.game['GAME.EXE']), 'mz');
    assert.equal(text(tree.d.game.data['pack.zip']), 'zip'); // zips keep their folder
    assert.equal(text(tree.d.game['run.bat']), 'bat');
    assert.equal(byId.get('message').textContent, '1 file or folder could not be copied to d:');
    const listed = byId.get('items').all().filter((element) => element.listeners.click).map((element) => element.textContent);
    assert.deepEqual(listed, ['/game/GAME.EXE', '/game/run.bat']);
});

test('a dropped zip is unzipped into a d: folder named after it', async () => {
    const tree = { d: {} };
    const game = makeZip({
        'game/': '',
        'game/GAME.EXE': 'mz'.repeat(500),
        'game/readme.txt': 'stored',
        'game/data/pack.zip': 'nested zip stays a file',
        '__MACOSX/game/._GAME.EXE': 'finder metadata',
    });
    const tools = makeZip({ 'run.bat': '@echo hi', 'bin/tool.exe': 'tool' });
    const { message, listed } = await dropOnto(tree, [droppedFile('/game.zip', game), droppedFile('/Tools.ZIP', tools)]);

    const text = (bytes) => new TextDecoder().decode(bytes);
    assert.equal(text(tree.d.game['GAME.EXE']), 'mz'.repeat(500)); // not d:/game/game
    assert.equal(text(tree.d.game['readme.txt']), 'stored');
    assert.equal(text(tree.d.game.data['pack.zip']), 'nested zip stays a file');
    assert.equal(text(tree.d.Tools['run.bat']), '@echo hi');
    assert.equal(text(tree.d.Tools.bin['tool.exe']), 'tool');
    assert.deepEqual(Object.keys(tree.d.game).sort(), ['GAME.EXE', 'data', 'readme.txt']);
    assert.deepEqual(Object.keys(tree.d).sort(), ['Tools', 'game']);
    assert.equal(message, '');
    assert.deepEqual(listed, ['/game/GAME.EXE', '/Tools/run.bat', '/Tools/bin/tool.exe']);
});

test('a dropped zip cannot write outside its folder', async () => {
    const tree = { d: {} };
    const evil = makeZip({
        '../../escape.exe': 'x',
        'a\\..\\..\\..\\escape2.exe': 'x',
        'evil/../../escape3.exe': 'x',
        '/rooted.txt': 'kept inside',
        'ok.txt': 'fine',
    });
    const { message } = await dropOnto(tree, [droppedFile('/evil.zip', evil), droppedFile('/broken.zip', 'not a zip')]);

    assert.deepEqual(Object.keys(tree), ['d']);
    assert.deepEqual(Object.keys(tree.d), ['evil']);
    assert.deepEqual(Object.keys(tree.d.evil).sort(), ['ok.txt', 'rooted.txt']);
    assert.equal(message, '4 files or folders could not be copied to d:. No executable files found');
});

test('Get File(s) downloads a folder as a zip of everything in it', async () => {
    const random = crypto.randomBytes(4000);
    const saves = { 'slot1.dat': random, 'notes.txt': 'hello '.repeat(200), 'ünï.txt': 'unicode', empty: {}, nested: { 'deep.txt': 'deep' } };
    const { context, byId, downloads } = dialogShell({ d: { saves, 'top.txt': 'top' } });
    context.buildGetFilesModalForDrives({ 'd:': '/d' });
    await getFromTree(context, byId, '[saves]');

    assert.deepEqual(downloads.map((download) => download.name), ['saves.zip']);
    const zip = readZip(new Uint8Array(await downloads[0].blob.arrayBuffer()));
    assert.deepEqual(Object.keys(zip).sort(), ['empty/', 'nested/', 'nested/deep.txt', 'notes.txt', 'slot1.dat', 'ünï.txt']);
    assert.deepEqual(zip['slot1.dat'].data, random);
    assert.equal(zip['slot1.dat'].method, 0); // random bytes don't shrink, so they are stored
    assert.equal(zip['notes.txt'].data.toString(), 'hello '.repeat(200));
    assert.equal(zip['notes.txt'].method, 8);
    assert.equal(zip['ünï.txt'].data.toString(), 'unicode');
    assert.equal(zip['empty/'].attributes, 0x10);
    assert.equal(zip['notes.txt'].dosTime, ((2024 - 1980) << 25 | 6 << 21 | 1 << 16 | 12 << 11 | 30 << 5 | 10 >> 1) >>> 0);
    assert.equal(byId.get('loadStatus').style.display, 'none');

    // the top of the tree is d: itself
    await getFromTree(context, byId, '[d:]');
    assert.equal(downloads[1].name, 'd.zip');
    assert.equal(readZip(new Uint8Array(await downloads[1].blob.arrayBuffer()))['saves/nested/deep.txt'].data.toString(), 'deep');

    // dropping a downloaded zip back onto the page restores the folder
    const tree = { d: {} };
    await dropOnto(tree, [droppedFile('/saves.zip', new Uint8Array(await downloads[0].blob.arrayBuffer()))]);
    assert.deepEqual(Object.keys(tree.d.saves).sort(), ['empty', 'nested', 'notes.txt', 'slot1.dat', 'ünï.txt']);
    assert.deepEqual(Buffer.from(tree.d.saves['slot1.dat']), random);
});

test('Get File(s) shows the files programs changed on c: next to d:', async () => {
    const driveC = { users: { username: { 'save.dat': 'progress' } }, windows: { 'win.ini': '[fonts]' } };
    const tree = { root: { home: { username: { '.wine': { drive_c: driveC } } } }, d: { 'notes.txt': 'notes' } };
    const { context, byId, downloads } = dialogShell(tree);
    vm.runInContext('Config.d_drive = "/d";', context);
    context.buildGetFilesModal(); // c: is ROOT's drive_c, ROOT is "/root" before configuration

    const tops = byId.get('tree').children[0].children.map((row) => row.textContent.match(/\[[cd]:\]/)[0]);
    assert.deepEqual(tops, ['[c:]', '[d:]']);
    assert.equal(byId.get('1').style.display, ''); // both drives start expanded
    await getFromTree(context, byId, 'save.dat');
    assert.equal(byId.get('selectedItem').value, 'c:/users/username/save.dat');
    await getFromTree(context, byId, '[c:]');
    assert.deepEqual(downloads.map((download) => download.name), ['save.dat', 'c.zip']);
    assert.deepEqual(Object.keys(readZip(new Uint8Array(await downloads[1].blob.arrayBuffer()))).sort(),
        ['users/', 'users/username/', 'users/username/save.dat', 'windows/', 'windows/win.ini']);
    await getFromTree(context, byId, 'notes.txt');
    assert.equal(byId.get('selectedItem').value, 'd:/notes.txt');

    // before any program has written to c: only d: is listed
    const fresh = dialogShell({ d: { 'notes.txt': 'notes' } });
    vm.runInContext('Config.d_drive = "/d";', fresh.context);
    fresh.context.buildGetFilesModal();
    assert.equal(fresh.byId.get('tree').children[0].children.length, 1);
    assert.equal(fresh.byId.get('tree').textContent, '+-[d:]notes.txt');
});

test('Save Overlay zips what Wine changed, without the emulator bookkeeping', async () => {
    const wine = { '.update-timestamp': '1700000000\n', 'user.reg': 'WINE REGISTRY', drive_c: { windows: { 'win.ini': '[fonts]' } } };
    const tree = { root: {
        home: { username: { '.wine': wine } },
        var: { cache: { fontconfig: { 'fonts.cache-7': 'cache' } } },
        tmp: { '.wine-1': { 'server-100-d77': { lock: '' } } },
        dev: {}, proc: {}, mnt: {},
        'boxedwine.zip.deleted': '/home/username/.wine/user.reg\n',
    } };
    const { context, downloads } = dialogShell(tree);
    await context.saveOverlay(); // ROOT is "/root" before configuration

    assert.deepEqual(downloads.map((download) => download.name), ['overlay.zip']);
    const zip = readZip(new Uint8Array(await downloads[0].blob.arrayBuffer()));
    assert.deepEqual(Object.keys(zip).sort(), [
        'home/', 'home/username/', 'home/username/.wine/', 'home/username/.wine/.update-timestamp',
        'home/username/.wine/drive_c/', 'home/username/.wine/drive_c/windows/', 'home/username/.wine/drive_c/windows/win.ini',
        'home/username/.wine/user.reg', 'var/', 'var/cache/', 'var/cache/fontconfig/', 'var/cache/fontconfig/fonts.cache-7',
    ]);
    assert.equal(zip['home/username/.wine/.update-timestamp'].data.toString(), '1700000000\n');
});

test('overlays are mounted before the root zip so their files win', () => {
    const { params } = launcher('http://localhost/?overlay=prefix.zip;fonts&p=notepad');
    const zips = params.filter((_, i) => params[i - 1] === '-zip');
    assert.deepEqual(zips, ['prefix.zip', 'fonts.zip', 'boxedwine.zip']);
});

test('zipping a folder reports a file it cannot read, then works again', async () => {
    const tree = { d: { saves: { 'a.txt': 'a', 'b.txt': 'b' } } };
    const { context, byId, downloads } = dialogShell(tree);
    context.buildGetFilesModalForDrives({ 'd:': '/d' });
    delete tree.d.saves['b.txt']; // deleted after the dialog listed it
    await getFromTree(context, byId, '[saves]');
    assert.deepEqual(downloads, []);
    assert.equal(byId.get('loadStatus').textContent, 'Unable to zip saves: ENOENT /d/saves/b.txt');
    assert.equal(byId.get('loadStatus').style.display, '');

    tree.d.saves['b.txt'] = 'b';
    await Promise.all([context.extract(), context.extract()]); // a double click zips once
    assert.deepEqual(downloads.map((download) => download.name), ['saves.zip']);
});

test('the JIT cache export uses createZip and still reads in the offline pipeline', async () => {
    const { context, downloads } = dialogShell({});
    const block = Buffer.concat([Buffer.from([0, 97, 115, 109, 1, 0, 0, 0]), Buffer.alloc(300, 7)]);
    const key = 'v6-00001000-00000001';
    context.Module.wasmJitCache = new Map([[key, new Uint8Array(block)], ['not-a-cache-key', new Uint8Array([1])]]);
    context.Module.wasmJitBlockMeta = new Map([[key, { key, exits: [] }]]);
    context.Module.wasmJitInteriorProfileLines = ['interior 1000 2'];
    await context.saveJitModules();

    assert.deepEqual(downloads.map((download) => download.name), ['wasm-jit-modules.zip']);
    const bytes = Buffer.from(await downloads[0].blob.arrayBuffer());
    const zip = readZip(bytes);
    const names = [key + '.wasm', 'boxedwine-jit-manifest.json', 'boxedwine-jit-profile.txt'];
    assert.deepEqual(Object.keys(zip), names);
    assert.deepEqual(zip[key + '.wasm'].data, block);
    assert.equal(zip[key + '.wasm'].method, 8);
    assert.equal(JSON.parse(zip['boxedwine-jit-manifest.json'].data).entries[0].key, key);
    assert.equal(zip['boxedwine-jit-profile.txt'].data.toString(), 'interior 1000 2\n');

    // the offline cache pipeline reads it with its own zip parser
    const pipeline = readFileSync(path.join(__dirname, 'boxedwine-wasm-jit-cache-pipeline.mjs'), 'utf8');
    const tools = vm.createContext({ Buffer, Uint32Array, deflateRawSync: zlib.deflateRawSync, inflateRawSync: zlib.inflateRawSync });
    vm.runInContext(pipeline.slice(pipeline.indexOf('function readU16('), pipeline.indexOf('function isWasm(')), tools);
    const entries = tools.parseZip(bytes);
    assert.deepEqual(Array.from(entries, (entry) => entry.name), names);
    assert.deepEqual(Buffer.from(entries[0].data), block);
});

test('createZip adds Zip64 end records past 65535 entries', async () => {
    const { context } = dialogShell({});
    const count = 0x10001;
    const zip = Buffer.from(await (await context.createZip(Array.from({ length: count }, (_, i) => ({ name: 'f' + i + '/' })))).arrayBuffer());
    const end = zip.length - 22;
    assert.equal(zip.readUInt32LE(end), 0x06054b50);
    assert.equal(zip.readUInt16LE(end + 10), 0xffff); // too many for 16 bits: see the Zip64 record
    const locator = end - 20;
    assert.equal(zip.readUInt32LE(locator), 0x07064b50);
    const record = Number(zip.readBigUInt64LE(locator + 8));
    assert.equal(zip.readUInt32LE(record), 0x06064b50);
    assert.equal(Number(zip.readBigUInt64LE(record + 32)), count);
    assert.equal(zip.readUInt32LE(Number(zip.readBigUInt64LE(record + 48))), 0x02014b50); // central directory
});

test('createZip records the exact time beside the MS-DOS local time', async () => {
    const { context } = dialogShell({});
    const date = new Date(Date.UTC(2026, 9, 13, 22, 19, 48));
    const zip = Buffer.from(await (await context.createZip([{ name: 'a.txt', data: new Uint8Array([1, 2, 3]), date }])).arrayBuffer());
    const extended = Buffer.from([0x55, 0x54, 5, 0, 1, 0x84, 0xae, 0xce, 0x6a]); // 1791929988 s
    assert.equal(zip.readUInt16LE(28), extended.length); // local header
    assert.deepEqual(zip.subarray(35, 44), extended);
    assert.deepEqual(zip.subarray(44, 47), Buffer.from([1, 2, 3])); // stored data follows it
    const central = zip.readUInt32LE(zip.length - 22 + 16);
    assert.equal(zip.readUInt32LE(central), 0x02014b50);
    assert.equal(zip.readUInt32LE(central + 12), context.dosDateTime(date)); // still local for other tools
    assert.equal(zip.readUInt16LE(central + 30), extended.length);
    assert.deepEqual(zip.subarray(central + 51, central + 60), extended);
});

test('closing a dialog removes #close from the address bar', () => {
    const { context } = dialogShell({});
    const page = { pathname: '/boxedwine.html', search: '?app=game&auto=false' };
    context.window.location = { ...page, hash: '#openModal' };
    context.window.listeners.hashchange();
    assert.deepEqual(context.history.urls, []); // an open dialog keeps its fragment
    context.window.location = { ...page, hash: '#close' };
    context.window.listeners.hashchange();
    assert.deepEqual(context.history.urls, ['/boxedwine.html?app=game&auto=false']);
});

test('auto=false shows Start and Get File(s), auto run shows neither', () => {
    for (const [query, shown] of [['auto=false', true], ['auto=true', false]]) {
        const { context } = dialogShell({}, 'http://localhost/?app=game&' + query);
        context.setConfiguration();
        context.start = () => {}; // auto run would start the emulator here
        context.buildBrowserFileSystem();
        const button = (id) => context.document.getElementById(id);
        assert.equal(button('startbtn').style.display === '', shown, query);
        assert.equal(button('downloadbtn').style.display === '', shown, query);
    }
});

test('auto=false with a CD-ROM shows Start disabled until the disc and the rest have downloaded', () => {
    const { context } = dialogShell({}, 'http://localhost/?app=game&auto=false&cdrom=e:disc1.iso;disc2.iso');
    context.setConfiguration();
    const start = context.document.getElementById('startbtn');
    const state = () => ({ shown: start.style.display === '', disabled: start.disabled, text: start.textContent });
    context.showLoadingStartButton('Downloading disc 1...');
    assert.deepEqual(state(), { shown: true, disabled: true, text: 'Downloading disc 1...' });
    context.updateHostFolderControls(); // as a remembered D: folder does while the disc downloads
    assert.deepEqual(state(), { shown: true, disabled: true, text: 'Downloading disc 1...' });
    context.showLoadingStartButton('Loading...');
    assert.deepEqual(state(), { shown: true, disabled: true, text: 'Loading...' });
    context.buildBrowserFileSystem();
    assert.deepEqual(state(), { shown: true, disabled: false, text: 'Start' });

    for (const query of ['auto=false', 'auto=true&cdrom=e:disc1.iso']) {
        const { context } = dialogShell({}, 'http://localhost/?app=game&' + query);
        context.setConfiguration();
        context.showLoadingStartButton('Downloading disc...');
        assert.equal(context.document.getElementById('startbtn').style.display === '', false, query);
    }
});

test('games mount separate persistent Wine roots and D drives', () => {
    const mech = launcher('http://localhost/demos/st/?app=mw3&p=mech3demo.exe');
    const pinball = launcher('http://localhost/demos/st/?app=pinball&p=DEMO.EXE');
    assert.equal(mech.root, '/root/app/mw3.zip');
    assert.equal(mech.drive, '/d_drive/app/mw3.zip');
    assert.notEqual(mech.root, pinball.root);
    assert.notEqual(mech.drive, pinball.drive);
    for (const game of [mech, pinball]) {
        assert.deepEqual(game.mounts, [
            { directory: game.root, autoPersist: true },
            { directory: game.drive, autoPersist: true },
        ]);
    }
});

test('game storage survives mode, build, root ZIP, and launcher changes', () => {
    const first = launcher('http://localhost/old/st/?root=old&app=mw3&p=mech3demo.exe');
    const next = launcher('http://localhost/new/mt-jit/?root=new&app=mw3.zip&overlay=fixes&p=cmd&args=/c%20mech3demo.exe');
    assert.equal(first.root, next.root);
    assert.equal(first.drive, next.drive);
});

test('overlay games are isolated and overlay lists cannot collide with filenames', () => {
    const caesar = launcher('http://localhost/?overlay=c3&p=cmd');
    const other = launcher('http://localhost/?overlay=other&p=cmd');
    assert.equal(caesar.root, '/root/overlay/c3.zip');
    assert.equal(caesar.drive, '/d_drive/overlay/c3.zip');
    assert.notEqual(caesar.root, other.root);
    assert.notEqual(caesar.root, launcher('http://localhost/?app=c3').root);
    const list = launcher('http://localhost/?overlay=one.zip;two.zip');
    const filename = launcher('http://localhost/?overlay=one.zip%2Btwo.zip');
    assert.notEqual(list.root, filename.root);
});

test('encoded names have stable storage keys without adding path components', () => {
    const plus = launcher('http://localhost/?app=My+Game');
    const percent = launcher('http://localhost/?app=My%20Game.zip');
    assert.equal(plus.root, percent.root);
    const nested = launcher('http://localhost/?app=..%2Fgame');
    assert.equal(nested.root, '/root/app/..%2Fgame.zip');
});

test('memory and regression launches do not mount persistent storage', () => {
    for (const query of ['storage=memory', 'regressionBuild=1']) {
        assert.deepEqual(launcher('http://localhost/?app=mw3&' + query).mounts, []);
    }
});

const gdiCommands = [
    'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'DirectDrawRenderer', '/t', 'REG_SZ', '/d', 'gdi', '/f', '&&',
    'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'renderer', '/t', 'REG_SZ', '/d', 'gdi', '/f', '&&',
];

test('registry presets launch the same command as hand-written reg add args', () => {
    const preset = launcher('http://localhost/?app=PERF_W95.EXE.zip&p=PERF_W95.EXE&bpp=8&resolution=640x480&registry=[gdi]');
    const manual = launcher('http://localhost/?app=PERF_W95.EXE.zip&p=cmd&bpp=8&resolution=640x480&args=/c%20reg%20add%20%22HKCU%5CSoftware%5CWine%5CDirect3D%22%20/v%20DirectDrawRenderer%20/t%20REG_SZ%20/d%20gdi%20/f%20%26%26%20reg%20add%20%22HKCU%5CSoftware%5CWine%5CDirect3D%22%20/v%20renderer%20/t%20REG_SZ%20/d%20gdi%20/f%20%26%26%20%22PERF_W95.EXE%22');
    assert.deepEqual(preset.params, manual.params);
});

test('registry presets stack in order and skip unknown names', () => {
    const wine = (url) => {
        const { params } = launcher(url);
        return params.slice(params.indexOf('/bin/wine') + 1);
    };
    const expected = [
        'cmd', '/c',
        ...gdiCommands,
        'reg', 'add', 'HKCU\\Software\\Wine', '/v', 'Version', '/t', 'REG_SZ', '/d', 'win98', '/f', '&&',
        'GAME.EXE', '-x',
    ];
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=[gdi,win98]'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=gdi;win98'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=%5Bgdi%3B%20bogus%3BWIN98%5D'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&registry=[bogus]'), ['GAME.EXE']);
});

test('registry version presets set the Windows version Wine reports', () => {
    for (const version of ['win95', 'win98', 'winme', 'win2k', 'winxp', 'win7', 'win10']) {
        const { params } = launcher('http://localhost/?app=game&p=GAME.EXE&registry=' + version);
        assert.deepEqual(params.slice(params.indexOf('/bin/wine') + 1), [
            'cmd', '/c', 'reg', 'add', 'HKCU\\Software\\Wine', '/v', 'Version', '/t', 'REG_SZ', '/d', version, '/f', '&&', 'GAME.EXE',
        ]);
    }
    const { params } = launcher('http://localhost/?app=game&p=GAME.EXE&registry=cdrom');
    assert.deepEqual(params.slice(params.indexOf('/bin/wine') + 1), ['GAME.EXE'], 'drive types come from cdrom= now');
});

test('registry renderer and mouse warp presets match the native UI settings', () => {
    const wine = (query) => {
        const { params } = launcher('http://localhost/?app=game&p=GAME.EXE&' + query);
        return params.slice(params.indexOf('/bin/wine') + 1);
    };
    assert.deepEqual(wine('registry=gl'), [
        'cmd', '/c',
        'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'DirectDrawRenderer', '/t', 'REG_SZ', '/d', 'opengl', '/f', '&&',
        'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'renderer', '/t', 'REG_SZ', '/d', 'gl', '/f', '&&',
        'GAME.EXE',
    ]);
    for (const mode of ['enable', 'disable', 'force']) {
        assert.deepEqual(wine('registry=mousewarp-' + mode), [
            'cmd', '/c', 'reg', 'add', 'HKCU\\Software\\Wine\\DirectInput', '/v', 'MouseWarpOverride', '/t', 'REG_SZ', '/d', mode, '/f', '&&',
            'GAME.EXE',
        ]);
    }
    assert.deepEqual(wine('registry=[MouseWarp-Disable;gdi]'), [
        'cmd', '/c', 'reg', 'add', 'HKCU\\Software\\Wine\\DirectInput', '/v', 'MouseWarpOverride', '/t', 'REG_SZ', '/d', 'disable', '/f', '&&',
        ...gdiCommands, 'GAME.EXE',
    ]);
});

test('registry presets run before a hand-written cmd /c chain', () => {
    const { params } = launcher('http://localhost/?app=game&p=cmd&args=/c%20reg%20add%20X%20%26%26%20GAME.EXE&registry=[gdi]');
    assert.deepEqual(params.slice(params.indexOf('/bin/wine') + 1), [
        'cmd', '/c', ...gdiCommands,
        'cmd', '/c', 'reg', 'add', 'X', '&&', 'GAME.EXE',
    ]);
});

test('preset params only run allow-listed values', () => {
    const tail = (query) => {
        const { params } = launcher('http://localhost/?app=game&p=GAME.EXE&' + query);
        return params.slice(params.indexOf('-env') >= 0 ? params.indexOf('-env') : params.indexOf('/bin/wine'));
    };
    const plain = tail('');
    for (const name of ['__proto__', 'constructor', 'toString', 'hasOwnProperty', 'valueOf']) {
        assert.deepEqual(tail('registry=' + name + '&wine-override=' + name), plain);
    }
    // operators, quotes and newlines stay inside an unknown name instead of reaching cmd
    for (const hostile of ['gdi%20%26%26%20calc', 'gdi%22%20%26%20calc', 'gdi%0Acalc', 'gdi%7Ccalc', '%22gdi%22']) {
        assert.deepEqual(tail('registry=' + hostile + '&wine-override=' + hostile), plain);
    }
});

test('working directory accepts drive shorthand like p', () => {
    const w = (query) => {
        const { params } = launcher('http://localhost/?app=game&p=GAME.EXE&' + query);
        return params[params.indexOf('-w') + 1];
    };
    assert.equal(w('w=c:/files/sub'), '/home/username/.wine/dosdevices/c:/files/sub');
    assert.equal(w('workingDir=D:%5Csaves%5Cslot1'), '/home/username/.wine/dosdevices/d:/saves/slot1');
    assert.equal(w('w=d:'), '/home/username/.wine/dosdevices/d:/');
    assert.equal(w('w=/home/username/game'), '/home/username/game');
});

test('wine-override presets pass a single WINEDLLOVERRIDES', () => {
    const env = (url) => {
        const { params } = launcher(url);
        return params.filter((_, i) => params[i - 1] === '-env');
    };
    assert.deepEqual(env('http://localhost/?app=game&p=GAME.EXE&wine-override=[nomono]'), ['WINEDLLOVERRIDES=mscoree,mshtml=']);
    assert.deepEqual(env('http://localhost/?app=game&p=GAME.EXE&wine-override=[bogus]'), []);
    assert.deepEqual(env('http://localhost/?app=game&p=GAME.EXE&env=%22WINEDLLOVERRIDES:ddraw=b%22&wine-override=nomono'),
        ['WINEDLLOVERRIDES=ddraw=b;mscoree,mshtml=']);
    assert.deepEqual(env('http://localhost/?app=game&p=GAME.EXE&env=%22WINEDEBUG:-all%22&wine-override=nomono'),
        ['WINEDEBUG=-all', 'WINEDLLOVERRIDES=mscoree,mshtml=']);
});

test('desktop launches without archives retain the existing storage', () => {
    const desktop = launcher('http://localhost/?root=TinyCore15Wine11.0');
    assert.equal(desktop.root, '/root');
    assert.equal(desktop.drive, '/d_drive');
});

// Loads disc 1 of ?cdrom= from a server holding `served` (name -> text), and reports
// the files it wrote, the names it asked for and the files kept for the disc.
async function loadDisc(url, served) {
    const files = new Map();
    const requests = [];
    const context = vm.createContext({
        window: { location: { href: url }, addEventListener() {} },
        document: { addEventListener() {}, getElementById() { return { style: {}, value: '', addEventListener() {}, getContext() {}, contains() { return false; } }; } },
        console: { log() {}, error() {}, warn() {} },
        TextDecoder,
        FS: {
            stat(file) { if (!files.has(file)) { throw new Error('ENOENT'); } return {}; },
            mkdirTree() {},
            readFile(file) { return files.get(file); },
            createDataFile(dir, name, bytes) { files.set(dir + name, Buffer.from(bytes)); },
        },
        fetch(name) {
            requests.push(name);
            const body = served[name];
            return Promise.resolve({
                status: body === undefined ? 404 : 200,
                arrayBuffer: async () => new TextEncoder().encode(body).buffer,
            });
        },
    });
    vm.runInContext(source, context);
    context.setConfiguration();
    const discFiles = JSON.parse(await vm.runInContext('loadCDRomDisc(0).then(() => JSON.stringify(cdromDiscFiles[0]))', context));
    return { written: [...files.keys()].sort(), requests, discFiles };
}

const gameCue = 'FILE "game.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n';

test('a BIN is downloaded with the CUE sheet beside it', async () => {
    const disc = await loadDisc('http://localhost/?cdrom=e:game.bin', { 'game.bin': 'data', 'game.cue': gameCue });
    assert.deepEqual(disc.written, ['/game.bin', '/game.cue']);
    assert.deepEqual(disc.discFiles, ['game.bin', 'game.cue']);
});

test('a BIN without a CUE sheet is downloaded alone', async () => {
    const disc = await loadDisc('http://localhost/?cdrom=e:game.bin', { 'game.bin': 'data' });
    assert.deepEqual(disc.written, ['/game.bin']);
    assert.deepEqual(disc.requests, ['game.cue', 'game.bin']);
    assert.deepEqual(disc.discFiles, ['game.bin']);
});

test('a page served for a missing CUE sheet is not taken for one', async () => {
    const disc = await loadDisc('http://localhost/?cdrom=e:game.bin', { 'game.bin': 'data', 'game.cue': '<html>Not found</html>' });
    assert.deepEqual(disc.written, ['/game.bin']);
});

test('an upper case BIN looks for an upper case CUE sheet, other images for none', async () => {
    const upper = await loadDisc('http://localhost/?cdrom=e:GAME.BIN', { 'GAME.BIN': 'data' });
    assert.deepEqual(upper.requests, ['GAME.CUE', 'GAME.BIN']);
    const iso = await loadDisc('http://localhost/?cdrom=e:game.iso', { 'game.iso': 'data' });
    assert.deepEqual(iso.requests, ['game.iso']);
});
