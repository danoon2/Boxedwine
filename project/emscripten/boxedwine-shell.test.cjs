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
                    elements.set(id, { style: {}, value: '', addEventListener() {}, getContext() {} });
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

test('desktop launches without archives retain the existing storage', () => {
    const desktop = launcher('http://localhost/?root=TinyCore15Wine11.0');
    assert.equal(desktop.root, '/root');
    assert.equal(desktop.drive, '/d_drive');
});
