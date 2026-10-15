const assert = require('node:assert/strict');
const { readFileSync } = require('node:fs');
const path = require('node:path');
const { test } = require('node:test');
const vm = require('node:vm');

const source = readFileSync(path.join(__dirname, 'boxedwine-shell.js'), 'utf8');

function launcher(url) {
    const elements = new Map();
    const mounts = [];
    const directories = [];
    const context = vm.createContext({
        window: { location: { href: url } },
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

test('registry presets launch the same command as hand-written reg add args', () => {
    const preset = launcher('http://localhost/?app=PERF_W95.EXE.zip&p=PERF_W95.EXE&bpp=8&resolution=640x480&registry=[gdi]');
    const manual = launcher('http://localhost/?app=PERF_W95.EXE.zip&p=cmd&bpp=8&resolution=640x480&args=/c%20reg%20add%20%22HKCU%5CSoftware%5CWine%5CDirect3D%22%20/v%20DirectDrawRenderer%20/t%20REG_SZ%20/d%20gdi%20/f%20%26%26%20%22PERF_W95.EXE%22');
    assert.deepEqual(preset.params, manual.params);
});

test('registry presets stack in order and skip unknown names', () => {
    const wine = (url) => {
        const { params } = launcher(url);
        return params.slice(params.indexOf('/bin/wine') + 1);
    };
    const expected = [
        'cmd', '/c',
        'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'DirectDrawRenderer', '/t', 'REG_SZ', '/d', 'gdi', '/f', '&&',
        'reg', 'add', 'HKLM\\Software\\Wine\\Drives', '/v', 'd:', '/d', 'cdrom', '/f', '&&',
        'GAME.EXE', '-x',
    ];
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=[gdi,cdrom]'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=gdi;cdrom'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&args=-x&registry=%5Bgdi%3B%20bogus%3BCDROM%5D'), expected);
    assert.deepEqual(wine('http://localhost/?app=game&p=GAME.EXE&registry=[bogus]'), ['GAME.EXE']);
});

test('registry presets run before a hand-written cmd /c chain', () => {
    const { params } = launcher('http://localhost/?app=game&p=cmd&args=/c%20reg%20add%20X%20%26%26%20GAME.EXE&registry=[gdi]');
    assert.deepEqual(params.slice(params.indexOf('/bin/wine') + 1), [
        'cmd', '/c', 'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D', '/v', 'DirectDrawRenderer', '/t', 'REG_SZ', '/d', 'gdi', '/f', '&&',
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
