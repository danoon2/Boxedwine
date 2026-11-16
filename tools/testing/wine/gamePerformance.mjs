// Optional capture-harness observer. Normal game launches do not install it.
export function installGamePerformance() {
    if (window.__boxedwineGamePerformance) throw Error('Performance observer already installed');
    const limit = 120000;
    const first = {};
    let phase = null;
    const now = () => performance.now();
    function sampleMemory() {
        if (!phase) return;
        let bytes = null;
        try {
            const heap = typeof HEAPU8 !== 'undefined' ? HEAPU8 : window.Module?.HEAPU8;
            if (heap) bytes = heap.byteLength;
        } catch (_) {}
        if (phase.memory.length < limit) phase.memory.push({at: now(), wasmCapacityBytes: bytes});
        else phase.overflow = true;
    }
    function presentation(context, kind) {
        const canvas = context.canvas;
        if (!canvas || canvas.id !== 'boxedwine-webgl-canvas-0' || !canvas.width || !canvas.height) return;
        const at = now();
        first[kind] ??= at;
        if (!phase) return;
        if (phase.presentations.length < limit) {
            phase.presentations.push({at, kind, width: canvas.width, height: canvas.height});
        } else phase.overflow = true;
    }
    const hooks = [];
    function hook(prototype, name) {
        if (!prototype || typeof prototype[name] !== 'function') return;
        const original = prototype[name];
        const wrapped = function(...args) {
            // Preserve arguments, receiver, return value and native exceptions.
            const value = Reflect.apply(original, this, args);
            // A null bitmap clears the canvas rather than presenting a frame.
            if (name !== 'transferFromImageBitmap' || args[0] != null) presentation(this, name);
            return value;
        };
        prototype[name] = wrapped;
        hooks.push({prototype, name, original, wrapped});
    }
    hook(window.CanvasRenderingContext2D?.prototype, 'drawImage');
    hook(window.CanvasRenderingContext2D?.prototype, 'putImageData');
    hook(window.ImageBitmapRenderingContext?.prototype, 'transferFromImageBitmap');
    function pageState() {
        const canvas = document.getElementById('boxedwine-webgl-canvas-0');
        const style = canvas ? getComputedStyle(canvas) : null;
        return {visibility: document.visibilityState, focused: document.hasFocus(),
            canvasVisible: !!canvas && canvas.width > 0 && canvas.height > 0
                && canvas.getClientRects().length > 0
                && style.display !== 'none' && style.visibility === 'visible'};
    }
    function visibility() {
        if (phase) phase.visibility.push({at: now(), ...pageState()});
    }
    document.addEventListener('visibilitychange', visibility);
    window.addEventListener('focus', visibility);
    window.addEventListener('blur', visibility);
    const timer = setInterval(sampleMemory, 1000);
    window.__boxedwineGamePerformance = {
        begin(name) {
            if (phase) throw Error('Performance phase already active');
            if (!/^[a-z0-9_-]+$/.test(name)) throw Error('Invalid performance phase name');
            if (!hooks.length) throw Error('No supported presentation methods');
            phase = {schemaVersion: 1, name, startedAt: now(), timeOrigin: performance.timeOrigin,
                firstPresentationMilliseconds: {...first}, hooks: hooks.map(row => row.name),
                presentations: [], memory: [], visibility: [], overflow: false};
            visibility(); sampleMemory();
        },
        end() {
            if (!phase) throw Error('No active performance phase');
            visibility(); sampleMemory();
            const result = phase;
            result.finishedAt = now();
            result.firstPresentationMilliseconds = {...first};
            result.hooksIntact = hooks.every(row => row.prototype[row.name] === row.wrapped);
            phase = null;
            return result;
        },
        dispose() {
            clearInterval(timer);
            document.removeEventListener('visibilitychange', visibility);
            window.removeEventListener('focus', visibility);
            window.removeEventListener('blur', visibility);
            for (const row of hooks) {
                if (row.prototype[row.name] === row.wrapped) row.prototype[row.name] = row.original;
            }
            phase = null;
            delete window.__boxedwineGamePerformance;
        }
    };
}

export function summarizeGamePerformance(raw, requestedMilliseconds) {
    const finite = value => typeof value === 'number' && Number.isFinite(value);
    if (!raw || raw.schemaVersion !== 1 || !finite(raw.startedAt) || !finite(raw.finishedAt)
            || !finite(requestedMilliseconds) || requestedMilliseconds < 5000
            || raw.finishedAt - raw.startedAt < requestedMilliseconds
            || !Array.isArray(raw.presentations) || !Array.isArray(raw.memory)
            || !Array.isArray(raw.visibility)) throw Error('Invalid or short performance observation');
    const problems = [];
    if (raw.overflow !== false) problems.push('Presentation or memory samples overflowed');
    if (raw.hooksIntact !== true) problems.push('Presentation hooks changed during measurement');
    if (raw.presentations.length < 2) problems.push('Fewer than two presentation submissions');
    if (raw.visibility.length < 2 || raw.visibility.some(row => row.visibility !== 'visible'
            || row.focused !== true || row.canvasVisible !== true)) problems.push('Page or canvas lost visibility/focus');
    const inside = at => finite(at) && at >= raw.startedAt && at <= raw.finishedAt;
    const intervals = [], dimensions = new Set(), kinds = {};
    let previous = raw.startedAt;
    for (const [index, row] of raw.presentations.entries()) {
        if (!inside(row.at) || row.at < previous || !['drawImage', 'putImageData', 'transferFromImageBitmap'].includes(row.kind)
                || !Number.isInteger(row.width) || row.width < 1 || !Number.isInteger(row.height) || row.height < 1)
            throw Error('Invalid presentation sample');
        if (index) intervals.push(row.at - previous);
        previous = row.at;
        dimensions.add(row.width + 'x' + row.height);
        kinds[row.kind] = (kinds[row.kind] || 0) + 1;
    }
    if (dimensions.size > 1) problems.push('Canvas size changed during measurement');
    const memory = [];
    for (const row of raw.memory) {
        if (!inside(row.at)) throw Error('Invalid memory sample time');
        if (row.wasmCapacityBytes === null) continue;
        if (!Number.isSafeInteger(row.wasmCapacityBytes) || row.wasmCapacityBytes <= 0)
            throw Error('Invalid WebAssembly capacity sample');
        memory.push(row.wasmCapacityBytes);
    }
    if (!memory.length) problems.push('WebAssembly memory capacity unavailable');
    const sorted = [...intervals].sort((a, b) => a - b);
    const percentile = p => sorted.length ? sorted[Math.max(0, Math.ceil(sorted.length * p) - 1)] : null;
    return {schemaVersion: 1, name: raw.name, measurementValid: !problems.length, problems,
        scope: 'Successful canvas presentation submissions, not GPU completion or physical display refresh. Memory is sampled WebAssembly capacity, not browser process working set. Instrumentation overhead is uncalibrated.',
        durationMilliseconds: raw.finishedAt - raw.startedAt,
        presentations: raw.presentations.length, presentationKinds: kinds,
        canvasSizes: [...dimensions],
        submissionsPerSecond: raw.presentations.length * 1000 / (raw.finishedAt - raw.startedAt),
        intervalMilliseconds: {samples: intervals.length, median: percentile(.5), p95: percentile(.95),
            p99: percentile(.99), max: sorted.at(-1) ?? null,
            over50: intervals.filter(value => value > 50).length,
            over100: intervals.filter(value => value > 100).length},
        // Leading/trailing silence matters even when the inter-frame intervals look healthy.
        leadingSilenceMilliseconds: raw.presentations.length ? raw.presentations[0].at - raw.startedAt : null,
        trailingSilenceMilliseconds: raw.presentations.length ? raw.finishedAt - raw.presentations.at(-1).at : null,
        wasmCapacity: {samples: memory.length, maxBytes: memory.length ? memory.reduce((a, b) => Math.max(a, b)) : null},
        firstPresentationMilliseconds: raw.firstPresentationMilliseconds,
        startupScope: 'First observed write to the presentation canvas since navigation; it can be a dialog or startup image, not the first 3D game frame.',
        performanceAcceptance: false};
}
