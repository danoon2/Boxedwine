// Actual Binaryen cache validation. Run separately from browser or compiler work.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import fs from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import vm from 'node:vm';
import { deflateRawSync, inflateRawSync } from 'node:zlib';

const repo = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const project = join(repo, 'project/emscripten');
if (process.argv.length < 3 || process.argv.length > 4 || process.argv[2] === '--help') {
    console.log('Usage: node validateOptimizerCache.mjs NEW_OUTPUT_DIRECTORY [BINARYEN_JS]');
    console.log('Retains flat/grouped cached/uncached output, execution checks and phase timings.');
    process.exit(process.argv[2] === '--help' ? 0 : 2);
}
const output = resolve(process.argv[2]);
const binaryen = resolve(process.argv[3] || join(project, 'binaryen_js.js'));
fs.mkdirSync(output);
const pipeline = join(project, 'boxedwine-wasm-jit-cache-pipeline.mjs');
const source = fs.readFileSync(pipeline, 'utf8');
const zip = vm.createContext({ Buffer, Uint32Array, deflateRawSync, inflateRawSync });
const start = source.indexOf('function readU16('), end = source.indexOf('function isWasm(');
assert(start > 0 && end > start);
vm.runInContext(source.slice(start, end), zip);
const sha256 = data => createHash('sha256').update(data).digest('hex');
function identity(path) {
    const data = fs.readFileSync(path);
    return { path, bytes: data.length, sha256: sha256(data) };
}
const report = {
    schema_version: 1, started_at: new Date().toISOString(), complete: false, passed: false,
    scope: 'Actual bundled Binaryen on 64 synthetic memory-reading modules; output/cache correctness and isolated pipeline timings, not game speed.',
    node: { path: process.execPath, versions: process.versions },
    tools: [fileURLToPath(import.meta.url), pipeline, binaryen,
        join(project, 'boxedwine-optimizer-cache.mjs')].map(identity),
    runs: [], problems: []
};
function save() { fs.writeFileSync(join(output, 'report.json'), JSON.stringify(report, null, 2) + '\n'); }
function uleb(value) {
    const result = [];
    do { const byte = value & 127; value >>>= 7; result.push(byte | (value ? 128 : 0)); } while (value);
    return result;
}
function sleb(value) {
    const result = [];
    while (true) {
        const byte = value & 127;
        value >>= 7;
        const done = value === 0 && !(byte & 64) || value === -1 && (byte & 64);
        result.push(byte | (done ? 0 : 128));
        if (done) return result;
    }
}
function section(id, bytes) { return [id, ...uleb(bytes.length), ...bytes]; }
function moduleBytes(address) {
    // (func (export "b0") (result i32) (i32.load (i32.const address)))
    // Redundant addition/multiplication makes optimization observable in bytes.
    const body = [0, 65, ...sleb(address), 40, 2, 0, 65, 0, 106, 65, 1, 108, 11];
    return Buffer.from([0, 97, 115, 109, 1, 0, 0, 0,
        ...section(1, [1, 96, 0, 1, 127]),
        ...section(2, [1, 3, 101, 110, 118, 6, 109, 101, 109, 111, 114, 121, 2, 0, 1]),
        ...section(3, [1, 0]), ...section(7, [1, 2, 98, 48, 0, 0]),
        ...section(10, [1, ...uleb(body.length), ...body])]);
}
const memory = new WebAssembly.Memory({ initial: 1 });
const heap = new Int32Array(memory.buffer);
const expected = new Map();
const modules = Array.from({ length: 64 }, (_, index) => {
    heap[index] = index * 65537 - 123456;
    const eip = 4096 * (index + 1), key = 'v6-' + eip.toString(16).padStart(8, '0') + '-00000001';
    const data = moduleBytes(index * 4);
    assert(WebAssembly.validate(data));
    assert.equal(new WebAssembly.Instance(new WebAssembly.Module(data), { env: { memory } }).exports.b0(), heap[index]);
    expected.set(key, heap[index]);
    return { key, eip, data };
});
const input = join(output, 'input.zip');
fs.writeFileSync(input, zip.buildZip(modules.map(item => ({ name: item.key + '.wasm', data: item.data })).concat([
    { name: 'boxedwine-jit-manifest.json', data: Buffer.from(JSON.stringify({
        version: 1, cacheVersion: 'v6', runtime: { cpu: { blockInstructionCountOffset: 0, yieldOffset: 4 },
            scheduler: { contextTimeRemainingPtr: 0 } },
        entries: modules.map(({ key, eip }) => ({ key, eip, blockHash: 1, opCount: 1, emulatedLen: 1 }))
    })) }
]), 0));
report.input = identity(input);
function normalized(entries) {
    return entries.map(entry => {
        if (!entry.name.endsWith('.json')) return { name: entry.name, sha256: sha256(entry.data) };
        const value = JSON.parse(entry.data);
        delete value.generatedAt;
        return { name: entry.name, value };
    }).sort((a, b) => a.name.localeCompare(b.name));
}
function verifyExecution(entries, flat) {
    const bytes = new Map(entries.map(entry => [entry.name, entry.data]));
    if (flat) {
        for (const [key, value] of expected) {
            const data = bytes.get(key + '.wasm');
            assert(data && WebAssembly.validate(data));
            const instance = new WebAssembly.Instance(new WebAssembly.Module(data), { env: { memory } });
            assert.equal(instance.exports.b0(), value, key);
        }
    } else {
        const manifest = JSON.parse(bytes.get('boxedwine-jit-grouped-manifest.json'));
        const seen = new Set();
        for (const group of manifest.groups) {
            const data = bytes.get(group.path);
            assert(data && WebAssembly.validate(data));
            const instance = new WebAssembly.Instance(new WebAssembly.Module(data), { env: { memory } });
            for (const entry of group.entries) {
                assert(expected.has(entry.key) && !seen.has(entry.key));
                assert.equal(instance.exports[entry.exportName](), expected.get(entry.key), entry.key);
                seen.add(entry.key);
            }
        }
        assert.equal(seen.size, expected.size);
    }
}
const references = new Map();
function run(name, flat, cacheState) {
    const target = join(output, name + '.zip'), logPath = join(output, name + '.log');
    const args = [pipeline, '--binaryen-js', binaryen, ...(flat ? ['--flat'] : []),
        ...(cacheState ? ['--optimizer-cache-dir', join(output, 'cache')] : []), input, target];
    const started = performance.now();
    const child = spawnSync(process.execPath, args, { encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 });
    fs.writeFileSync(logPath, child.stdout + child.stderr, { flag: 'wx' });
    const row = { name, command: [process.execPath, ...args], elapsed_ms: performance.now() - started,
        exit_code: child.status, log: identity(logPath), passed: false };
    report.runs.push(row); save();
    assert.equal(child.status, 0, child.stderr);
    assert(!/Warning:|WARNING:|validation error/i.test(child.stdout + child.stderr));
    const stats = /OPTIMIZER_CACHE hits=(\d+) misses=(\d+) invalid=(\d+)/.exec(child.stdout);
    row.cache = stats ? { hits: +stats[1], misses: +stats[2], invalid: +stats[3] } : null;
    assert.deepEqual(row.cache, cacheState === 'cold' ? { hits: 0, misses: 64, invalid: 0 }
        : cacheState === 'warm' ? { hits: 64, misses: 0, invalid: 0 } : null);
    row.phase_ms = Object.fromEntries([...child.stdout.matchAll(/OPTIMIZER_TIMING phase=([\w-]+) milliseconds=([\d.]+)/g)]
        .map(match => [match[1], +match[2]]));
    assert.deepEqual(Object.keys(row.phase_ms).sort(), ['load-and-identity', 'modules']);
    assert(Object.values(row.phase_ms).every(value => Number.isFinite(value) && value >= 0));
    const entries = zip.parseZip(fs.readFileSync(target));
    verifyExecution(entries, flat);
    row.executed_exports = 64;
    row.output = identity(target);
    const content = normalized(entries), mode = flat ? 'flat' : 'grouped';
    if (!references.has(mode)) references.set(mode, content);
    else assert.deepEqual(content, references.get(mode), name + ' changed optimized payload or manifest');
    if (flat) assert(modules.some(item => !entries.find(entry => entry.name === item.key + '.wasm').data.equals(item.data)),
        'Control did not exercise a real optimizer transformation');
    row.passed = true; save();
    console.log('PASS ' + name + ': 64 executed exports, ' + row.elapsed_ms.toFixed(1) + ' ms');
}
try {
    save();
    run('flat-uncached', true, null);
    run('flat-cold', true, 'cold');
    run('flat-warm', true, 'warm');
    run('grouped-uncached', false, null);
    run('grouped-warm', false, 'warm');
    for (let index = 0; index < 3; ++index) {
        run('flat-uncached-repeat-' + index, true, null);
        run('flat-warm-repeat-' + index, true, 'warm');
    }
    for (const item of report.tools) assert.deepEqual(identity(item.path), item);
    report.complete = report.passed = true;
} catch (error) {
    report.problems.push(String(error.stack || error));
    process.exitCode = 1;
} finally {
    report.finished_at = new Date().toISOString();
    save();
}
