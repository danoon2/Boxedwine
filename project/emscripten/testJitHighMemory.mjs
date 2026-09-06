// Exercise the actual emitted production group installer with high heap
// pointers. Stub only the surrounding broker bookkeeping, not the installer.
import fs from 'node:fs';
import vm from 'node:vm';
const file = process.argv[2];
if (!file) {
    throw new Error('Usage: node testJitHighMemory.mjs Build/MultiThreadedJit/boxedwine.js');
}
const source = fs.readFileSync(file, 'utf8');
const begin = source.indexOf('function boxedwine_wasm_instantiate_runtime_group_mt(');
const end = source.indexOf('function boxedwine_wasm_install_existing_group_mt(', begin);
if (begin < 0 || end < 0) {
    throw new Error('Compiled MT JIT installer functions not found');
}
const body = source.slice(begin, end);
const memory = new WebAssembly.Memory({initial: 32770, maximum: 49152, shared: true});
// (module (func (export "b0") (result i32) i32.const 42))
const bytes = new Uint8Array([
    0, 97, 115, 109, 1, 0, 0, 0,
    1, 5, 1, 96, 0, 1, 127,
    3, 2, 1, 0,
    7, 6, 1, 2, 98, 48, 0, 0,
    10, 6, 1, 4, 0, 65, 42, 11
]);
const table = new WebAssembly.Table({initial: 1, element: 'anyfunc'});
const environment = {
    WebAssembly, Uint8Array, Int32Array, Atomics, Map, console, Module: {},
    HEAPU8: new Uint8Array(memory.buffer),
    HEAP32: new Int32Array(memory.buffer),
    wasmMemory: memory,
    wasmTable: table,
    growMemViews() {},
    err(message) { console.error(message); },
    bwWasmJitMtGroupInstanceKey(id) { return 'm' + id; },
    bwWasmJitMtCanConstructFreshGroup() { return true; },
    bwWasmJitBrokerGetOrCompile(id, owner, incarnation, view) {
        return {module: new WebAssembly.Module(view), source: 1};
    },
    bwWasmJitMtRememberGroupInstance() {},
    bwWasmJitMtRecordGroupWorkerEvent() {},
    bwWasmJitMtForgetGroupInstance() {}
};
const context = vm.createContext(environment);
vm.runInContext(body, context);
for (const base of [0x10000, 0x80010000]) {
    const imports = base + 0x1000;
    const next = base + 0x2000;
    const output = base + 0x3000;
    const lookup = base + 0x4000;
    environment.HEAPU8.set(bytes, base);
    // Match signed i32 arguments delivered by WASM to the EM_JS import.
    const result = context.boxedwine_wasm_instantiate_runtime_group_mt(
        base | 0, bytes.length, imports | 0, 1, next | 0, 1, output | 0,
        2, base, base, 1, 1, 1, lookup | 0);
    const slot = environment.HEAP32[output >>> 2];
    if (result !== 1 || slot < 1 || table.get(slot)() !== 42 ||
            environment.HEAP32[lookup >>> 2] !== 1 ||
            environment.HEAP32[next >>> 2] !== slot + 1) {
        throw new Error('Production installation failed at 0x' + base.toString(16));
    }
    console.log('PASS module installation and execution at 0x' + base.toString(16));
}
