// Real-browser control for the optional observer; no Wine or game is loaded.
const report = {kind: 'game-performance-browser-control', checks: [], phases: [], errors: [], passed: false};
const delay = milliseconds => new Promise(resolve => setTimeout(resolve, milliseconds));
const query = new URL(location.href).search;
let observer;
function check(name, passed, details = {}) {
    report.checks.push({name, passed, ...details});
    if (!passed) throw Error('Control failed: ' + name);
}
function target() {
    const canvas = document.createElement('canvas');
    canvas.id = 'boxedwine-webgl-canvas-0';
    canvas.width = canvas.height = 64;
    canvas.tabIndex = 0;
    document.getElementById('frame').replaceChildren(canvas);
    canvas.focus();
    return canvas;
}
function pixels(canvas) {
    const copy = document.createElement('canvas');
    copy.width = copy.height = 64;
    const context = copy.getContext('2d');
    context.drawImage(canvas, 0, 0);
    return Array.from(context.getImageData(20, 20, 1, 1).data);
}
const correct = color => color.every((value, index) => Math.abs(value - [51, 102, 153, 255][index]) <= 1);
addEventListener('error', event => report.errors.push(String(event.message)));
addEventListener('unhandledrejection', event => report.errors.push(String(event.reason)));

try {
    const {installGamePerformance, summarizeGamePerformance} = await import('/gamePerformance.mjs' + query);
    check('module_uninstalled', window.__boxedwineGamePerformance === undefined);
    const source = new OffscreenCanvas(64, 64);
    const gl = source.getContext('webgl2', {antialias: false, preserveDrawingBuffer: true});
    if (!gl) throw Error('WebGL2 unavailable');
    const debug = gl.getExtension('WEBGL_debug_renderer_info');
    report.environment = {userAgent: navigator.userAgent, version: gl.getParameter(gl.VERSION),
        renderer: gl.getParameter(debug ? debug.UNMASKED_RENDERER_WEBGL : gl.RENDERER)};
    gl.clearColor(.2, .4, .6, 1); gl.clear(gl.COLOR_BUFFER_BIT);
    let canvas = target();
    let context = canvas.getContext('2d');
    context.drawImage(source, 0, 0);
    check('baseline_pixels', correct(pixels(canvas)));
    const original = {drawImage: CanvasRenderingContext2D.prototype.drawImage,
        putImageData: CanvasRenderingContext2D.prototype.putImageData,
        transferFromImageBitmap: ImageBitmapRenderingContext.prototype.transferFromImageBitmap};
    const memory = new WebAssembly.Memory({initial: 1, maximum: 2});
    window.Module = {HEAPU8: new Uint8Array(memory.buffer)};
    installGamePerformance(); observer = window.__boxedwineGamePerformance;

    observer.begin('exception');
    let exception = false;
    try {context.drawImage({});} catch (error) {exception = error instanceof TypeError;}
    check('native_exception', exception && observer.end().presentations.length === 0);

    async function measure(name, draw, grow = false) {
        observer.begin(name.toLowerCase());
        let calls = 0, timer, growth;
        const present = () => {draw(); calls++;};
        try {
            present();
            timer = setInterval(present, 25);
            if (grow) growth = setTimeout(() => {
                memory.grow(1); window.Module.HEAPU8 = new Uint8Array(memory.buffer);
            }, 1500);
            await delay(5100);
        } finally {clearInterval(timer); clearTimeout(growth);}
        const raw = observer.end();
        const summary = summarizeGamePerformance(raw, 5000);
        const color = pixels(canvas);
        check(name, summary.measurementValid && raw.presentations.length === calls
            && summary.wasmCapacity.maxBytes === 131072 && correct(color),
            {calls, color, summary});
        report.phases.push({name, raw, summary});
    }
    const crop = document.createElement('canvas');
    crop.width = crop.height = 64;
    const cropContext = crop.getContext('2d');
    await measure('drawImage', () => {
        // The intermediate HTML canvas uses the hooked prototype too.
        cropContext.drawImage(source, 0, 0);
        context.drawImage(crop, 0, 0);
    }, true);

    canvas = target(); context = canvas.getContext('2d');
    const data = new ImageData(64, 64);
    for (let index = 0; index < data.data.length; index += 4) data.data.set([51, 102, 153, 255], index);
    await measure('putImageData', () => context.putImageData(data, 0, 0));

    canvas = target(); context = canvas.getContext('bitmaprenderer');
    if (!context) throw Error('Bitmap renderer unavailable');
    const bitmapSource = new OffscreenCanvas(64, 64), bitmapContext = bitmapSource.getContext('2d');
    await measure('transferFromImageBitmap', () => {
        bitmapContext.putImageData(data, 0, 0);
        context.transferFromImageBitmap(null); // Clearing must not count as a submission.
        context.transferFromImageBitmap(bitmapSource.transferToImageBitmap());
    });

    canvas = target(); context = canvas.getContext('2d');
    observer.begin('hidden-parent');
    document.getElementById('frame').style.display = 'none';
    context.drawImage(source, 0, 0); context.drawImage(source, 0, 0);
    await delay(5100);
    const hidden = observer.end();
    const hiddenSummary = summarizeGamePerformance(hidden, 5000);
    check('hidden_parent', hiddenSummary.measurementValid === false
        && hiddenSummary.problems.includes('Page or canvas lost visibility/focus'), {summary: hiddenSummary});
    report.phases.push({name: 'hidden-parent', raw: hidden, summary: hiddenSummary});
    document.getElementById('frame').style.display = ''; canvas.focus();

    // Record paired CPU submission costs without claiming a game-overhead limit.
    observer.begin('overhead');
    const timings = [];
    for (let repeat = 0; repeat < 8; repeat++) {
        for (const kind of repeat % 2 ? ['observed', 'direct'] : ['direct', 'observed']) {
            const draw = kind === 'direct' ? original.drawImage : context.drawImage;
            const started = performance.now();
            for (let index = 0; index < 500; index++) draw.call(context, source, 0, 0);
            timings.push({repeat, kind, calls: 500, milliseconds: performance.now() - started});
            await delay(0);
        }
    }
    const overhead = observer.end();
    check('overhead_counts', overhead.presentations.length === 4000 && overhead.hooksIntact === true);
    report.overhead = {timings, raw: overhead,
        scope: 'Paired 64x64 canvas CPU submission costs, including sample collection. This is not game instrumentation overhead or a performance threshold.'};
    observer.dispose(); observer = null;
    check('restored', Object.entries(original).every(([name, method]) =>
        (name === 'transferFromImageBitmap' ? ImageBitmapRenderingContext : CanvasRenderingContext2D).prototype[name] === method)
        && window.__boxedwineGamePerformance === undefined);
    check('gl_error', gl.getError() === gl.NO_ERROR);
    report.passed = report.errors.length === 0;
} catch (error) {
    report.errors.push(String(error.stack || error));
} finally {
    observer?.dispose();
    document.getElementById('result').textContent = JSON.stringify({passed: report.passed, checks: report.checks, errors: report.errors}, null, 2);
    await fetch('/result' + query, {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(report)});
}
