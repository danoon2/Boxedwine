// Report tests only: no browser, graphics context, or compiler is started.
import assert from 'node:assert/strict';
import test from 'node:test';
import {readGLCounters, diffGLCounters, groupGLCounters} from './webgl-counters.mjs';

const buildId = 'ab'.repeat(32);
const instanceId = '01234567-89ab-4cde-8fab-0123456789ab';
const max = (1n << 64n) - 1n;
const snapshot = (counters = [], extra = {}) => ({version: 1, buildId, instanceId,
    count: 5000, startedMs: 0, finishedMs: 1, counters, ...extra});
const row = (index, name, value) => ({index, name, value: String(value)});
const mock = (values, names = []) => ({
    _boxedwine_gl_counter_version: () => 1,
    _boxedwine_gl_counter_count: () => values.length,
    _boxedwine_gl_counter_get: index => values[index],
    _boxedwine_gl_counter_name: () => 0,
    ccall(name, result, types, args) {
        assert.deepEqual([name, result, types], ['boxedwine_gl_counter_name', 'string', ['number']]);
        return names[args[0]] ?? null;
    },
});

test('reads sparse exact values, preserves JSON precision and identifies each module instance', () => {
    const module = mock([0n, (1n << 53n) + 7n, 0n, 42n], ['', 'glDrawArrays']);
    const result = readGLCounters(module, buildId, () => 10);
    assert.deepEqual(result.counters, [row(1, 'glDrawArrays', '9007199254740999'), row(3, 'int99_3', 42)]);
    assert.deepEqual(JSON.parse(JSON.stringify(result)), result);
    assert.equal(readGLCounters(module, buildId, () => 11).instanceId, result.instanceId);
    assert.notEqual(readGLCounters(mock([0n]), buildId).instanceId, result.instanceId);
});

test('handles the actual signed WebAssembly i64 boundary without losing the unsigned high bit', () => {
    // () -> i64 returning -1. This literal module needs no external toolchain.
    const bytes = Uint8Array.from([0, 97, 115, 109, 1, 0, 0, 0,
        1, 5, 1, 96, 0, 1, 126, 3, 2, 1, 0,
        7, 7, 1, 3, 103, 101, 116, 0, 0, 10, 6, 1, 4, 0, 66, 127, 11]);
    const get = new WebAssembly.Instance(new WebAssembly.Module(bytes)).exports.get;
    assert.equal(get(), -1n);
    const module = mock([get(), -(1n << 63n), (1n << 63n) - 1n]);
    assert.deepEqual(readGLCounters(module, buildId).counters.map(r => r.value),
        [max.toString(), '9223372036854775808', '9223372036854775807']);
});

test('requires native diagnostic exports and rejects malformed native results', () => {
    for (const module of [null, undefined, {}, 1]) assert.throws(() => readGLCounters(module, buildId));
    for (const field of Object.keys(mock([0n]))) {
        const module = mock([0n]);
        delete module[field];
        assert.throws(() => readGLCounters(module, buildId), /COUNTERS/);
    }
    for (const value of [0, '1', null, undefined, max + 1n, -(1n << 63n) - 1n]) {
        assert.throws(() => readGLCounters(mock([value]), buildId), /64-bit/);
    }
    for (const count of [0, -1, 65537, NaN, 1.5, '1']) {
        assert.throws(() => readGLCounters({...mock([0n]), _boxedwine_gl_counter_count: () => count}, buildId), /schema/);
    }
    assert.throws(() => readGLCounters({...mock([0n]), _boxedwine_gl_counter_version: () => 2}, buildId), /schema/);
    assert.throws(() => readGLCounters(mock([1n], [123]), buildId), /name/);
    assert.throws(() => readGLCounters(mock([1n, 2n], ['glDrawArrays', 'glDrawArrays']), buildId), /duplicate/);
});

test('validates recorded identities, clock intervals and every raw row before reporting', () => {
    const validRow = row(0, 'glDrawArrays', 1);
    for (const value of [null, {}, snapshot([], {version: 2}), snapshot([], {buildId: 'bad'}),
        snapshot([], {instanceId: ''}), snapshot([], {count: 0}), snapshot([], {count: 1.5}),
        snapshot([], {startedMs: NaN}), snapshot([], {finishedMs: Infinity}),
        snapshot([], {startedMs: 2}), snapshot([null]), snapshot([], {counters: {}})]) {
        assert.throws(() => groupGLCounters(value));
    }
    for (const change of [{index: -1}, {index: 5000}, {index: 0.5}, {name: ''}, {name: 1},
        {value: 1}, {value: '01'}, {value: '-1'}, {value: '1e1'}, {value: (max + 1n).toString()}]) {
        assert.throws(() => groupGLCounters(snapshot([{...validRow, ...change}])));
    }
    assert.throws(() => groupGLCounters(snapshot([validRow, validRow])), /duplicate/);
    assert.throws(() => groupGLCounters(snapshot([validRow, {...validRow, index: 1}])), /duplicate/);
    for (const time of [NaN, Infinity, -Infinity]) assert.throws(() => readGLCounters(mock([0n]), buildId, () => time), /time/);
    let time = 10;
    assert.throws(() => readGLCounters(mock([0n]), buildId, () => time--), /time/);
});

test('computes exact deltas, includes new counters and omits unchanged counters', () => {
    const before = snapshot([row(0, 'glDrawArrays', 1n << 53n), row(1, 'glFinish', 4)]);
    const after = snapshot([row(0, 'glDrawArrays', (1n << 53n) + 3n), row(1, 'glFinish', 4),
        row(2, 'glReadPixels', 8)], {startedMs: 21, finishedMs: 23});
    const result = diffGLCounters(before, after);
    assert.deepEqual(result.counters, [row(0, 'glDrawArrays', 3), row(2, 'glReadPixels', 8)]);
    assert.deepEqual(result.samplingMilliseconds, {before: 1, after: 2});
    assert.equal(result.instanceId, instanceId);
    assert.equal(groupGLCounters(result).callbacks, '11');
    assert.equal(before.counters[0].value, '9007199254740992');
    assert.deepEqual(diffGLCounters(snapshot([row(0, 'glDrawArrays', 0)]), snapshot([], {startedMs: 1})).counters, []);
});

test('refuses cross-build, restarted, reset, wrapped, renamed or overlapping intervals', () => {
    const before = snapshot([row(0, 'glDrawArrays', 10)]);
    const after = snapshot([row(0, 'glDrawArrays', 12)], {startedMs: 2, finishedMs: 3});
    for (const change of [{buildId: 'cd'.repeat(32)}, {count: 4999},
        {instanceId: '01234567-89ab-4cde-8fab-0123456789ac'}, {startedMs: 0.5},
        {counters: []}, {counters: [row(0, 'glDrawArrays', 9)]}, {counters: [row(0, 'glDrawElements', 12)]}]) {
        assert.throws(() => diffGLCounters(before, {...after, ...change}));
    }
    assert.throws(() => diffGLCounters(snapshot([row(0, 'glFinish', max)]),
        snapshot([row(0, 'glFinish', 0)], {startedMs: 1})), /decreased/);
});

test('groups actual GL aliases by requested work, keeping host counters separate', () => {
    const categories = {
        drawRequests: ['glDrawArrays', 'glDrawElementsInstanced', 'glDrawRangeElements', 'glMultiDrawElementsEXT', 'glBegin', 'glDrawPixels', 'glBitmap'],
        shaderCompileRequests: ['glCompileShader', 'glCompileShaderARB', 'glCompileShaderIncludeARB'],
        programLinkRequests: ['glLinkProgram', 'glLinkProgramARB'],
        bufferDataRequests: ['glBufferData', 'glBufferSubDataARB', 'glNamedBufferStorageEXT'],
        bufferMapRequests: ['glMapBuffer', 'glMapBufferRange', 'glMapNamedBufferRangeEXT'],
        bufferUnmapRequests: ['glUnmapBuffer', 'glUnmapNamedBufferEXT'],
        textureDataRequests: ['glTexImage2D', 'glCompressedTexSubImage3DARB', 'glMultiTexImage2DEXT', 'glTextureSubImage2D'],
        readbackRequests: ['glReadPixels', 'glReadnPixelsARB', 'glGetTexImage', 'glGetnCompressedTexImageARB',
            'glGetTextureSubImage', 'glGetMultiTexImageEXT', 'glGetCompressedMultiTexImageEXT', 'glGetNamedBufferSubDataEXT'],
        swapRequests: ['glXSwapBuffers', 'eglSwapBuffers'],
        bindingRequests: ['glXMakeCurrent', 'glXMakeContextCurrent', 'eglMakeCurrent'],
        finishRequests: ['glFinish'], flushRequests: ['glFlush'],
        clientWaitRequests: ['glClientWaitSync', 'eglClientWaitSync', 'eglWaitGL', 'eglWaitClient', 'eglWaitNative'],
        serverWaitRequests: ['glWaitSync', 'eglWaitSync'],
    };
    for (const [category, names] of Object.entries(categories)) {
        for (const name of names) {
            const result = groupGLCounters(snapshot([row(0, name, 3)]));
            assert.equal(result[category], '3', name);
            assert.equal(result.callbacks, '3', name);
            for (const key of Object.keys(result).filter(k => ![category, 'callbacks'].includes(k))) {
                assert.equal(result[key], '0', name + ' must not count as ' + key);
            }
        }
    }
    const result = groupGLCounters(snapshot([row(0, 'hostContextChanges', 7), row(1, 'mainThreadDispatches', 8),
        row(2, 'glDrawArrays', max), row(3, 'glDrawElements', max)]));
    assert.equal(result.callbacks, (2n * max).toString());
    assert.equal(result.drawRequests, (2n * max).toString());
    assert.equal(result.hostContextChanges, '7');
    assert.equal(result.mainThreadDispatches, '8');
});

test('does not misclassify similarly named state or metadata calls as draws, waits or readbacks', () => {
    for (const name of ['glDrawBuffer', 'glDrawBuffersARB', 'glReadBuffer', 'glBeginQuery', 'glBeginTransformFeedback',
        'glGetImageHandleARB', 'glGetTextureHandleARB', 'glGetTexLevelParameteriv', 'glGetBufferParameteriv', 'glFlushMappedBufferRange',
        'glGetSynciv', 'glFenceSync', 'glXSwapIntervalEXT', 'glBindTexture', 'glTexStorage2D', 'int99_4999']) {
        const result = groupGLCounters(snapshot([row(0, name, 2)]));
        assert.equal(result.callbacks, '2');
        for (const key of Object.keys(result).filter(k => k !== 'callbacks')) assert.equal(result[key], '0', name + ' as ' + key);
    }
    // An unrecognized future name must stay in raw callbacks, not become a readback by prefix alone.
    assert.equal(groupGLCounters(snapshot([row(0, 'glGetTexImageHandleARB', 2)])).readbackRequests, '0');
});
