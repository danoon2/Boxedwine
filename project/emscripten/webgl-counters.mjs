// Optional diagnostic builds export cumulative counters at the guest GL boundary.
const MAX_U64 = (1n << 64n) - 1n;
const MIN_I64 = -(1n << 63n);
const HOST_COUNTERS = new Set(['hostContextChanges', 'mainThreadDispatches']);
const instances = new WeakMap();

function checkInstanceId(instanceId) {
    if (typeof instanceId !== 'string' || !/^[a-f0-9]{8}(?:-[a-f0-9]{4}){3}-[a-f0-9]{12}$/.test(instanceId)) {
        throw Error('Invalid GL counter instance identity');
    }
}

function checkBuildId(buildId) {
    if (typeof buildId !== 'string' || !/^[a-f0-9]{64}$/.test(buildId)) {
        throw Error('A SHA-256 build identity is required for GL counters');
    }
}

export function readGLCounters(module, buildId, now = () => performance.now()) {
    checkBuildId(buildId);
    if (!module || typeof module !== 'object') throw Error('BOXEDWINE_WEBGL_COUNTERS module required');
    for (const name of ['_boxedwine_gl_counter_version', '_boxedwine_gl_counter_count',
        '_boxedwine_gl_counter_get', '_boxedwine_gl_counter_name', 'ccall']) {
        if (typeof module[name] !== 'function') throw Error('BOXEDWINE_WEBGL_COUNTERS build required: ' + name);
    }
    const version = module._boxedwine_gl_counter_version();
    const count = module._boxedwine_gl_counter_count();
    if (version !== 1 || !Number.isInteger(count) || count < 1 || count > 65536) {
        throw Error('Unsupported GL counter schema');
    }
    const startedMs = now();
    const counters = [];
    for (let index = 0; index < count; ++index) {
        const exportedValue = module._boxedwine_gl_counter_get(index);
        if (typeof exportedValue !== 'bigint' || exportedValue < MIN_I64 || exportedValue > MAX_U64) {
            throw Error('GL counters require 64-bit BigInt exports');
        }
        // The WebAssembly JS boundary represents i64 as signed even for C++ uint64_t.
        const value = BigInt.asUintN(64, exportedValue);
        if (value) {
            const name = module.ccall('boxedwine_gl_counter_name', 'string', ['number'], [index]);
            if (name !== null && typeof name !== 'string') throw Error('Invalid GL counter name');
            counters.push({index, name: name || 'int99_' + index, value: value.toString()});
        }
    }
    const finishedMs = now();
    if (![startedMs, finishedMs].every(Number.isFinite) || finishedMs < startedMs) {
        throw Error('Invalid GL counter sampling time');
    }
    if (!instances.has(module)) instances.set(module, crypto.randomUUID());
    const snapshot = {version, buildId, instanceId: instances.get(module), count, startedMs, finishedMs, counters,
        scope: 'Cumulative dispatched guest GL calls and observed host context/dispatch changes. '
            + 'Each counter is atomic; collection does not pause all workers. Counts are requests, not successful GPU operations.'};
    entries(snapshot);
    return snapshot;
}

function entries(snapshot) {
    if (!snapshot || typeof snapshot !== 'object') throw Error('Invalid GL counter snapshot');
    checkBuildId(snapshot.buildId);
    checkInstanceId(snapshot.instanceId);
    if (snapshot.version !== 1 || !Number.isInteger(snapshot.count) || snapshot.count < 1
        || snapshot.count > 65536 || !Array.isArray(snapshot.counters)
        || ![snapshot.startedMs, snapshot.finishedMs].every(Number.isFinite)
        || snapshot.finishedMs < snapshot.startedMs) throw Error('Invalid GL counter snapshot');
    const result = new Map();
    const names = new Set();
    for (const row of snapshot.counters) {
        if (!row || !Number.isInteger(row.index) || row.index < 0 || row.index >= snapshot.count || result.has(row.index)
            || typeof row.name !== 'string' || !row.name || names.has(row.name) || typeof row.value !== 'string'
            || !/^(0|[1-9][0-9]*)$/.test(row.value) || BigInt(row.value) > MAX_U64) {
            throw Error('Invalid or duplicate GL counter entry');
        }
        result.set(row.index, row);
        names.add(row.name);
    }
    return result;
}

export function diffGLCounters(before, after) {
    const previous = entries(before);
    const current = entries(after);
    if (before.buildId !== after.buildId || before.instanceId !== after.instanceId || before.count !== after.count
        || before.version !== after.version || after.startedMs < before.finishedMs) {
        throw Error('GL counter snapshots use different builds/instances/schemas or overlap in time');
    }
    for (const [index, row] of previous) {
        if (!current.has(index) && BigInt(row.value) !== 0n) throw Error('GL counters reset between snapshots');
    }
    const counters = [];
    for (const [index, row] of current) {
        const prior = previous.get(index);
        if (prior && prior.name !== row.name) throw Error('GL counter identity changed');
        const difference = BigInt(row.value) - BigInt(prior?.value || '0');
        if (difference < 0n) throw Error('GL counter decreased between snapshots');
        if (difference) counters.push({...row, value: difference.toString()});
    }
    return {version: 1, buildId: after.buildId, instanceId: after.instanceId, count: after.count, counters,
        startedMs: before.startedMs, finishedMs: after.finishedMs,
        samplingMilliseconds: {before: before.finishedMs - before.startedMs, after: after.finishedMs - after.startedMs},
        scope: 'Per-counter deltas between cumulative snapshots; no global snapshot barrier or GPU completion is implied.'};
}

export function groupGLCounters(snapshot) {
    const raw = entries(snapshot);
    const groups = Object.fromEntries(['callbacks', 'drawRequests', 'shaderCompileRequests', 'programLinkRequests',
        'bufferDataRequests', 'bufferMapRequests', 'bufferUnmapRequests', 'textureDataRequests', 'readbackRequests',
        'swapRequests', 'bindingRequests', 'finishRequests', 'flushRequests', 'clientWaitRequests', 'serverWaitRequests',
        ...HOST_COUNTERS].map(name => [name, 0n]));
    for (const {name, value} of raw.values()) {
        const amount = BigInt(value);
        if (HOST_COUNTERS.has(name)) {groups[name] += amount; continue;}
        groups.callbacks += amount;
        if (/^gl(?:(?:Multi)?Draw(?:Arrays|Elements|RangeElements|TransformFeedback)|Begin$|DrawPixels$|Bitmap$)/.test(name)) groups.drawRequests += amount;
        if (/^glCompileShader(?:ARB|IncludeARB)?$/.test(name)) groups.shaderCompileRequests += amount;
        if (/^glLinkProgram(?:ARB)?$/.test(name)) groups.programLinkRequests += amount;
        if (/^gl(?:Named)?Buffer(?:Data|SubData|Storage)(?:ARB|EXT)?$/.test(name)) groups.bufferDataRequests += amount;
        if (/^glMap(?:Named)?Buffer(?:Range)?(?:ARB|EXT)?$/.test(name)) groups.bufferMapRequests += amount;
        if (/^glUnmap(?:Named)?Buffer(?:ARB|EXT)?$/.test(name)) groups.bufferUnmapRequests += amount;
        if (/^gl(?:Compressed)?(?:Tex|MultiTex|Texture)(?:Sub)?Image[123]D(?:ARB|EXT|OES)?$/.test(name)) groups.textureDataRequests += amount;
        if (/^glReadn?Pixels(?:ARB|EXT|NV)?$/.test(name) || /^glGetn?(?:Compressed)?(?:Tex|MultiTex|Texture)(?:Sub)?Image(?:ARB|EXT)?$/.test(name)
            || /^glGet(?:Named)?BufferSubData(?:ARB|EXT)?$/.test(name)) groups.readbackRequests += amount;
        if (['glXSwapBuffers', 'eglSwapBuffers'].includes(name)) groups.swapRequests += amount;
        if (['glXMakeCurrent', 'glXMakeContextCurrent', 'eglMakeCurrent'].includes(name)) groups.bindingRequests += amount;
        if (name === 'glFinish') groups.finishRequests += amount;
        if (name === 'glFlush') groups.flushRequests += amount;
        if (['glClientWaitSync', 'eglClientWaitSync', 'eglWaitGL', 'eglWaitClient', 'eglWaitNative'].includes(name)) groups.clientWaitRequests += amount;
        if (['glWaitSync', 'eglWaitSync'].includes(name)) groups.serverWaitRequests += amount;
    }
    return Object.fromEntries(Object.entries(groups).map(([name, value]) => [name, value.toString()]));
}
