import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import assert from 'node:assert/strict';

const source = readFileSync(new URL('./emscripten-automation-client.js', import.meta.url), 'utf8');

function harness() {
    const requests = [];
    const events = {};
    let sync;
    const context = vm.createContext({
        AUTOMATION_CONFIG: {token: 'test', cwd: '/files', script: '/automation/install/script.txt',
            program: 'setup.exe', options: ['-bpp', '16'], args: ['with spaces']},
        Module: {}, ROOT: '/root', Config: {extraZipFiles: ['app.zip'], rootZipFile: 'filesystem.zip'},
        console: {log() {}, warn() {}, error() {}},
        FS: {syncfs(populate, callback) { assert.equal(populate, false); sync = callback; }},
        IDBFS: {quit() { throw new Error('database connection closed before sync'); }},
        fetch: async (path, options) => { requests.push({path, ...options}); return {ok: true, json: async () => ({})}; },
        setInterval() { return 1; }, clearInterval() {}, addEventListener(name, callback) { events[name] = callback; },
        getEmulatorParams: null, emrun_file_dump: null,
    });
    context.failArchiveSetup = () => { context.Config.archiveSetupFailed = true; };
    vm.runInContext(source, context);
    return {context, requests, events, finishSync: error => sync(error)};
}

test('completion waits for durable filesystem and screenshot delivery', async () => {
    const h = harness();
    h.context.Module.onRuntimeInitialized();
    h.context.emrun_file_dump('failed.bmp', new Uint8Array([1, 2]));
    h.context.console.log('final output');
    h.context.Module.onExit(111);
    h.context.IDBFS.quit();
    assert.equal(h.requests.filter(r => r.path.endsWith('/result')).length, 0);
    await h.finishSync(null);
    assert.equal(JSON.parse(h.requests.at(-1).body).code, 111);
    assert.equal(JSON.parse(h.requests.at(-1).body).syncError, null);
    assert.equal(h.requests[0].path, '/automation/test/file/failed.bmp');
    assert.deepEqual(JSON.parse(h.requests[1].body), ['final output', 'Automation guest exited: 111; flushing filesystem']);
    h.context.Module.onExit(111);
    assert.equal(h.requests.filter(r => r.path.endsWith('/result')).length, 1);
});

test('archive setup failure reports promptly without starting or syncing the guest', async () => {
    const h = harness();
    h.context.FS.syncfs = () => assert.fail('guest filesystem does not exist yet');
    h.context.console.error('TypeError: Failed to fetch');
    h.context.failArchiveSetup(new Error('filesystem transfer failed'));
    await new Promise(resolve => setImmediate(resolve));
    assert.equal(h.context.Config.archiveSetupFailed, true);
    const results = h.requests.filter(r => r.path.endsWith('/result'));
    assert.equal(results.length, 1);
    const failure = JSON.parse(results[0].body);
    assert.equal(failure.reason, 'archive_setup_failed');
    assert.match(failure.browserErrors[0], /filesystem transfer failed/);
    assert.equal(failure.code, undefined);
    assert.deepEqual(JSON.parse(h.requests[0].body), ['TypeError: Failed to fetch']);
    h.context.failArchiveSetup(new Error('second failure'));
    h.context.Module.onExit(111);
    await new Promise(resolve => setImmediate(resolve));
    assert.equal(h.requests.filter(r => r.path.endsWith('/result')).length, 1);
});

test('filesystem flush failure cannot be reported as success', async () => {
    const h = harness();
    h.context.Module.onExit(111);
    await h.finishSync(new Error('disk full'));
    assert.match(JSON.parse(h.requests.at(-1).body).syncError, /disk full/);
});

test('errors retain their stacks and still fail a successful guest exit', async () => {
    const h = harness();
    h.events.error({message: 'short message', error: {stack: 'Error: full stack\n    at guest (boxedwine.js:1:42)'}});
    h.events.unhandledrejection({reason: {stack: 'Error: rejected\n    at cleanup (boxedwine.js:1:84)'}});
    h.events.error({message: 'worker without an Error object'});
    h.context.Module.onExit(111);
    await h.finishSync(null);
    const result = JSON.parse(h.requests.at(-1).body);
    assert.equal(result.code, 111);
    assert.deepEqual(result.browserErrors, [
        'Error: full stack\n    at guest (boxedwine.js:1:42)',
        'Error: rejected\n    at cleanup (boxedwine.js:1:84)',
        'worker without an Error object',
    ]);
});

test('native options and argv retain boundaries', () => {
    const h = harness();
    assert.deepEqual(Array.from(h.context.getEmulatorParams()), [
        '-root', '/root', '-zip', 'app.zip', '-zip', 'filesystem.zip',
        '-play', '/automation/install/script.txt', '-w', '/files', '-bpp', '16',
        '-env', 'WINE_D3D_CONFIG=webgl=0,webgl_glsl_es=0',
        '-nosound', '-novideo', '/bin/wine', 'setup.exe', 'with spaces',
    ]);
});
