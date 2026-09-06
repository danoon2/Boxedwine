// Run with: node testJitCacheVersion.mjs [path/to/binaryen_js.js]
// Exercise the production ZIP importer and offline pipeline with old/current
// flat and grouped caches. No browser or guest filesystem is required.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import vm from 'node:vm';
import { deflateRawSync, inflateRawSync } from 'node:zlib';

const here = dirname(fileURLToPath(import.meta.url));
const shell = fs.readFileSync(join(here, 'boxedwine-shell.js'), 'utf8');
const pipelinePath = join(here, 'boxedwine-wasm-jit-cache-pipeline.mjs');
const pipeline = fs.readFileSync(pipelinePath, 'utf8');
function between(source, begin, end) {
    const start = source.indexOf(begin);
    const stop = source.indexOf(end, start);
    assert(start >= 0 && stop > start, `Missing production code: ${begin}`);
    return source.slice(start, stop);
}
const zipContext = vm.createContext({ Buffer, Uint32Array, deflateRawSync, inflateRawSync });
vm.runInContext(between(pipeline, 'function readU16(', 'function isWasm('), zipContext);
const importer = between(shell, 'var BOXEDWINE_WASM_JIT_CACHE_VERSION', 'function getBase64Data(') +
    between(shell, 'async function importJitModulesFromBuffer(', 'async function saveJitModules(');

// (module (import "env" "memory" (memory 1))
//   (func (export "b0") (result i32) i32.const 0 i32.load))
const wasm = Buffer.from([
    0, 97, 115, 109, 1, 0, 0, 0,
    1, 5, 1, 96, 0, 1, 127,
    2, 15, 1, 3, 101, 110, 118, 6, 109, 101, 109, 111, 114, 121, 2, 0, 1,
    3, 2, 1, 0, 7, 6, 1, 2, 98, 48, 0, 0,
    10, 9, 1, 7, 0, 65, 0, 40, 2, 0, 11
]);
const memory = new WebAssembly.Memory({ initial: 1 });
new Int32Array(memory.buffer)[0] = 42;
function flatEntries(version) {
    const key = `${version}-00001000-00000001`;
    return [
        { name: `${key}.wasm`, data: wasm },
        { name: 'boxedwine-jit-manifest.json', data: Buffer.from(JSON.stringify({
            version: 1, cacheVersion: version,
            runtime: { cpu: { blockInstructionCountOffset: 0, yieldOffset: 4 },
                scheduler: { contextTimeRemainingPtr: 0 } },
            entries: [{ key, eip: 4096, blockHash: 1, opCount: 1, emulatedLen: 1 }]
        })) }
    ];
}

for (const version of ['v5', 'v6']) {
    for (const mt of [false, true]) {
        for (const compression of [0, 6]) {
            const Module = {
                wasmJitCache: new Map(), wasmJitCompiledCache: new Map(),
                wasmJitGroupModules: new Map(), wasmJitGroupEntryMap: new Map()
            };
            if (mt) Module._wasm_jit_mt_register = () => {};
            const warnings = [];
            const context = vm.createContext({
                Module, WebAssembly, Uint8Array, DataView, TextDecoder, Map, Set,
                console: { log() {}, warn(...args) { warnings.push(args.join(' ')); } },
                inflateRaw: async (data) => inflateRawSync(data)
            });
            vm.runInContext(importer, context);
            const key = `${version}-00001000-00000001`;
            const entries = flatEntries(version).concat([
                { name: 'groups/group-0000.wasm', data: wasm },
                { name: 'boxedwine-jit-grouped-manifest.json', data: Buffer.from(JSON.stringify({
                    format: 'boxedwine-wasm-jit-grouped-cache', cacheVersion: version, mt,
                    groups: [{ path: 'groups/group-0000.wasm', entries: [{ key, exportName: 'b0' }] }]
                })) }
            ]);
            await context.importJitModulesFromBuffer(zipContext.buildZip(entries, compression));
            assert(!warnings.some((warning) => /import failed|precompile failed/.test(warning)), warnings.join('\n'));
            if (version === 'v5') {
                assert.equal(Module.wasmJitCache.size, 0, 'old flat cache must be rejected');
                assert.equal(Module.wasmJitCompiledCache.size, 0);
                assert.equal(Module.wasmJitGroupedManifest, null, 'old grouped cache must be rejected');
                assert.equal(Module.wasmJitGroupEntryMap.size, 0);
                assert.equal(Module.wasmJitMtPendingGroups, undefined);
            } else {
                assert.equal(Module.wasmJitCache.size, 1, 'current flat cache must be accepted');
                assert.equal(Module.wasmJitGroupedManifest.cacheVersion, 'v6');
                if (mt) {
                    assert.equal(Module.wasmJitMtPendingGroups.length, 1);
                } else {
                    assert.equal(new WebAssembly.Instance(Module.wasmJitCompiledCache.get(key), { env: { memory } }).exports.b0(), 42);
                    assert.equal(Module.wasmJitGroupEntryMap.get(key).exportName, 'b0');
                }
            }
        }
    }
}

const temp = fs.mkdtempSync(join(tmpdir(), 'boxedwine-cache-version-'));
try {
    const oldZip = join(temp, 'old.zip');
    const currentZip = join(temp, 'current.zip');
    const flatZip = join(temp, 'flat.zip');
    const groupedZip = join(temp, 'grouped.zip');
    fs.writeFileSync(oldZip, zipContext.buildZip(flatEntries('v5'), 0));
    fs.writeFileSync(currentZip, zipContext.buildZip(flatEntries('v6'), 0));
    function runPipeline(...args) {
        return spawnSync(process.execPath, [pipelinePath, '--binaryen-js',
            process.argv[2] || join(here, 'binaryen_js.js'), ...args], { encoding: 'utf8' });
    }
    const rejected = runPipeline('--flat', oldZip, flatZip);
    assert.equal(rejected.status, 1);
    assert.match(rejected.stderr, /not v6; record a new cache/);
    assert(!fs.existsSync(flatZip), 'old modules must not be relabeled as current');
    const flat = runPipeline('--flat', currentZip, flatZip);
    assert.equal(flat.status, 0, flat.stderr);
    const flatManifest = zipContext.parseZip(fs.readFileSync(flatZip))
        .find((entry) => entry.name === 'boxedwine-jit-manifest.json');
    assert.equal(JSON.parse(flatManifest.data).cacheVersion, 'v6');
    // Flat output must retain its version so it can be fed back into grouping.
    const grouped = runPipeline(flatZip, groupedZip);
    assert.equal(grouped.status, 0, grouped.stderr);
    const groupedManifest = zipContext.parseZip(fs.readFileSync(groupedZip))
        .find((entry) => entry.name === 'boxedwine-jit-grouped-manifest.json');
    assert.equal(JSON.parse(groupedManifest.data).cacheVersion, 'v6');
} finally {
    assert(fs.realpathSync(temp).startsWith(fs.realpathSync(tmpdir()) + sep));
    fs.rmSync(temp, { recursive: true });
}
console.log('PASS: old WASM caches rejected; v6 flat/grouped caches import and round-trip');
