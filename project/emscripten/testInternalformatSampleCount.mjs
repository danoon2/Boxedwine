// Execute the production EM_JS body against the WebGL query contract. This
// checks translation and writes only; native/browser integration is separate.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';

const source = readFileSync(new URL('../../source/opengl/glcommon.cpp', import.meta.url), 'utf8');
const match = source.match(/EM_JS\(void,\s*boxedwine_get_internalformat_sample_count_js,\s*\([^)]*\),\s*\{([\s\S]*?)\n\s*\}\);/);
assert.ok(match, 'production sample-count bridge body');
const SAMPLES = 0x80a9, NUM_SAMPLE_COUNTS = 0x9380;
const RENDERBUFFER = 0x8d41, RGBA8 = 0x8058, INVALID_ENUM = 0x0500, INVALID_VALUE = 0x0501;
const sentinel = 0x12345678;

function fixture(samples = [8, 4, 2], extra = {}) {
    const heap = new Int32Array(16).fill(sentinel);
    const errors = [], calls = [];
    const globals = {
        HEAP32: heap,
        GL: {recordError(error) { errors.push(error); }},
        GLctx: {
            getError() { throw Error('A capability query must not consume errors.'); },
            getInternalformatParameter(target, format, pname) {
                calls.push([target, format, pname]);
                // WebGL 2 accepts SAMPLES here and returns an Int32Array.
                if (target !== RENDERBUFFER || format !== RGBA8 || pname !== SAMPLES) {
                    errors.push(INVALID_ENUM);
                    return null;
                }
                return new Int32Array(samples);
            },
        },
        ...extra,
    };
    const context = vm.createContext(globals);
    const query = vm.runInContext(`(function(target, internalformat, bufSize, params) {${match[1]}\n})`, context);
    return {heap, errors, calls, query, context};
}

test('returns the number of reported counts, not the largest count', () => {
    const f = fixture();
    f.query(RENDERBUFFER, RGBA8, 1, 8);
    assert.equal(f.heap[2], 3);
    assert.deepEqual(f.calls, [[RENDERBUFFER, RGBA8, SAMPLES]]);
    assert.deepEqual(f.errors, []);
});
test('an empty sample list produces a valid zero', () => {
    const f = fixture([]);
    f.query(RENDERBUFFER, RGBA8, 1, 8);
    assert.equal(f.heap[2], 0);
    assert.deepEqual(f.errors, []);
});
test('one count is written even when the output buffer is larger', () => {
    const f = fixture([4]);
    f.query(RENDERBUFFER, RGBA8, 8, 8);
    assert.deepEqual(Array.from(f.heap), Array.from({length: 16}, (_, i) => i === 2 ? 1 : sentinel));
});
test('zero buffer size validates the query without writing', () => {
    const f = fixture();
    f.query(RENDERBUFFER, RGBA8, 0, 0);
    assert.deepEqual(Array.from(f.heap), Array(16).fill(sentinel));
    assert.equal(f.calls.length, 1);
    assert.deepEqual(f.errors, []);
});
test('an invalid target preserves the whole destination and error', () => {
    const f = fixture();
    f.query(0x0de1, RGBA8, 4, 8);
    assert.deepEqual(Array.from(f.heap), Array(16).fill(sentinel));
    assert.deepEqual(f.errors, [INVALID_ENUM]);
});
test('an invalid format is still validated with a zero-sized output', () => {
    const f = fixture();
    f.query(RENDERBUFFER, 0xdead, 0, 0);
    assert.deepEqual(f.errors, [INVALID_ENUM]);
    assert.deepEqual(Array.from(f.heap), Array(16).fill(sentinel));
});
test('an earlier GL error is not consumed or cleared', () => {
    const f = fixture();
    f.errors.push(0x0502);
    f.query(RENDERBUFFER, RGBA8, 1, 8);
    assert.equal(f.heap[2], 3);
    assert.deepEqual(f.errors, [0x0502]);
});
test('negative buffer sizes fail without a query or write', () => {
    const f = fixture();
    f.query(RENDERBUFFER, RGBA8, -1, 8);
    assert.deepEqual(f.errors, [INVALID_VALUE]);
    assert.deepEqual(f.calls, []);
    assert.deepEqual(Array.from(f.heap), Array(16).fill(sentinel));
});
test('missing positive-sized destination is rejected', () => {
    const f = fixture();
    f.query(RENDERBUFFER, RGBA8, 1, 0);
    assert.deepEqual(f.errors, [INVALID_VALUE]);
    assert.deepEqual(f.calls, []);
});
test('a signed wasm pointer above 2 GB uses the unsigned heap index', () => {
    const writes = [];
    const HEAP32 = new Proxy({}, {set(_, key, value) { writes.push([key, value]); return true; }});
    const f = fixture([8, 4], {HEAP32});
    f.query(RENDERBUFFER, RGBA8, 1, 0x80000004 | 0);
    assert.deepEqual(writes, [['536870913', 2]]);
    assert.deepEqual(f.errors, []);
});

// Optional retained-runtime control. It invokes the original generated SDK
// function against exactly the same WebGL behavior, rather than reconstructing
// the failing implementation in this test.
const originalArg = process.argv.indexOf('--original-runtime');
if (originalArg !== -1) {
    const original = readFileSync(process.argv[originalArg + 1], 'utf8');
    const start = original.indexOf('function _emscripten_glGetInternalformativ(');
    assert.ok(start >= 0, 'original generated function');
    const end = original.indexOf('}function ', start);
    assert.ok(end > start, 'original generated function end');
    const originalFunction = original.slice(start, end + 1);
    test('retained SDK forwards NUM_SAMPLE_COUNTS and fails the same contract', () => {
        const f = fixture();
        const query = vm.runInContext(`(${originalFunction})`, f.context);
        query(RENDERBUFFER, RGBA8, NUM_SAMPLE_COUNTS, 1, 8);
        assert.deepEqual(f.calls, [[RENDERBUFFER, RGBA8, NUM_SAMPLE_COUNTS]]);
        assert.deepEqual(f.errors, [INVALID_ENUM]);
        assert.equal(f.heap[2], sentinel);
    });
}
