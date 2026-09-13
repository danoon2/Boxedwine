// Exercise the real ZIP pipeline with a fake optimizer; no browser or compiler.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import vm from 'node:vm';
import { deflateRawSync, inflateRawSync } from 'node:zlib';

const here = dirname(fileURLToPath(import.meta.url));
const pipeline = join(here, 'boxedwine-wasm-jit-cache-pipeline.mjs');
const source = fs.readFileSync(pipeline, 'utf8');
const zip = vm.createContext({ Buffer, Uint32Array, deflateRawSync, inflateRawSync });
vm.runInContext(source.slice(source.indexOf('function readU16('), source.indexOf('function isWasm(')), zip);
const wasm = Buffer.from([0, 97, 115, 109, 1, 0, 0, 0,
    1, 5, 1, 96, 0, 1, 127,
    2, 15, 1, 3, 101, 110, 118, 6, 109, 101, 109, 111, 114, 121, 2, 0, 1,
    3, 2, 1, 0, 7, 6, 1, 2, 98, 48, 0, 0,
    10, 9, 1, 7, 0, 65, 0, 40, 2, 0, 11]);
const second = Buffer.from(wasm);
second[second.length - 5] = 4; // Read a different memory address.
const root = fs.mkdtempSync(join(tmpdir(), 'boxedwine-optimizer-pipeline-'));
try {
    const tool = join(root, 'fake-binaryen.mjs'), asset = join(root, 'optimizer-support.bin');
    const calls = join(root, 'optimizer-calls.txt');
    fs.writeFileSync(asset, 'support 1');
    fs.writeFileSync(tool, `import { appendFileSync, readFileSync } from 'node:fs';
export default function(options) {
    readFileSync(options.locateFile('optimizer-support.bin'));
    return { Features: { All: 123 }, setOptimizeLevel() {}, setShrinkLevel() {},
        readBinary(data) { appendFileSync(process.env.BW_TEST_OPTIMIZER_CALLS, 'call\\n');
            return { setFeatures() {}, runPasses() {}, validate() { return true; },
                emitBinary() { return data; }, dispose() {} }; } };
}`);
    const entries = [wasm, second].map((data, i) => ({ name: `v6-0000${i + 1}000-00000001.wasm`, data }));
    entries.push({ name: 'boxedwine-jit-manifest.json', data: Buffer.from(JSON.stringify({
        version: 1, cacheVersion: 'v6', runtime: { cpu: { blockInstructionCountOffset: 0, yieldOffset: 4 },
            scheduler: { contextTimeRemainingPtr: 0 } },
        entries: entries.map((entry, i) => ({ key: entry.name.slice(0, -5), eip: (i + 1) * 4096,
            blockHash: 1, opCount: 1, emulatedLen: 1 })) })) });
    const input = join(root, 'input.zip'), cache = join(root, 'cache');
    fs.writeFileSync(input, zip.buildZip(entries, 0));
    function callCount() { return fs.existsSync(calls) ? fs.readFileSync(calls, 'utf8').trim().split('\n').length : 0; }
    function run(name, options, expectedCalls, expectedCache) {
        const output = join(root, name + '.zip'), before = callCount();
        const result = spawnSync(process.execPath, [pipeline, '--binaryen-js', tool, ...options, input, output],
            { encoding: 'utf8', env: { ...process.env, BW_TEST_OPTIMIZER_CALLS: calls } });
        assert.equal(result.status, 0, result.stderr);
        assert.equal(callCount() - before, expectedCalls);
        assert.match(result.stdout, /OPTIMIZER_TIMING phase=load-and-identity milliseconds=/);
        assert.match(result.stdout, /OPTIMIZER_TIMING phase=modules milliseconds=/);
        if (expectedCache) assert.match(result.stdout, expectedCache);
        else assert(!result.stdout.includes('OPTIMIZER_CACHE'));
        return zip.parseZip(fs.readFileSync(output)).map(entry => {
            if (entry.name.endsWith('.json')) {
                const value = JSON.parse(entry.data);
                delete value.generatedAt;
                return { name: entry.name, value };
            }
            return { name: entry.name, bytes: entry.data.toString('hex') };
        });
    }
    const flat = run('flat-control', ['--flat'], 2);
    const cached = ['--optimizer-cache-dir', cache];
    assert.deepEqual(run('flat-cold', ['--flat', ...cached], 2, /hits=0 misses=2 invalid=0/), flat);
    assert.deepEqual(run('flat-warm', ['--flat', ...cached], 0, /hits=2 misses=0 invalid=0/), flat);
    const grouped = run('group-control', ['--no-direct-calls'], 2);
    assert.deepEqual(run('group-warm', ['--no-direct-calls', ...cached], 0, /hits=2 misses=0 invalid=0/), grouped);
    fs.writeFileSync(asset, 'support 2');
    assert.deepEqual(run('asset-changed', ['--flat', ...cached], 2, /hits=0 misses=2 invalid=0/), flat);
    fs.appendFileSync(tool, '\n// optimizer version changed\n');
    assert.deepEqual(run('tool-changed', ['--flat', ...cached], 2, /hits=0 misses=2 invalid=0/), flat);
    assert.deepEqual(run('disabled', ['--flat'], 2), flat);
    const invalid = spawnSync(process.execPath, [pipeline, '--optimizer-cache-dir=', input, join(root, 'bad.zip')],
        { encoding: 'utf8' });
    assert.equal(invalid.status, 1);
    assert.match(invalid.stderr, /requires a path/);
    const compatibility = spawnSync(process.execPath, [join(here, 'testJitCacheVersion.mjs'), tool],
        { encoding: 'utf8', env: { ...process.env, BW_TEST_OPTIMIZER_CALLS: calls } });
    assert.equal(compatibility.status, 0, compatibility.stderr);
    console.log('PASS: 8 pipeline runs preserve flat/grouped output and track cache/tool/asset changes (fake optimizer)');
    console.log(compatibility.stdout.trim());
} finally {
    assert(fs.realpathSync(root).startsWith(fs.realpathSync(tmpdir()) + sep));
    fs.rmSync(root, { recursive: true });
}
