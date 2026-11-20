// Usage: node runThreadWebGLPresentationTest.mjs CHROME PLAYWRIGHT_MODULE OUTPUT [SOURCE]
// Exercises the production EM_JS helper in a real worker/WebGL context. SOURCE
// permits a different presentation helper. BW_PRESENT_BLOCKED=1 keeps the
// stress worker inside Wasm-style blocking waits to reproduce the original leak.
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {mkdir, readFile, writeFile} from 'node:fs/promises';
import {createServer} from 'node:http';
import {createRequire} from 'node:module';
import {resolve, join} from 'node:path';

const [chrome, playwrightModule, directory, sourcePath] = process.argv.slice(2);
if (!chrome || !playwrightModule || !directory) throw Error('Expected CHROME PLAYWRIGHT_MODULE OUTPUT [SOURCE]');
const blocked = process.env.BW_PRESENT_BLOCKED === '1';
const output = resolve(directory);
await mkdir(output, {recursive: true});
const source = await readFile(sourcePath || new URL('../../../source/opengl/sdl/sdlgl.cpp', import.meta.url), 'utf8');
const body = source.match(/EM_JS\(void, boxedwinePresentCurrentWebGLFrame,[^\n]*\{([\s\S]*?)\r?\n\}\);/)?.[1];
assert.ok(body, 'Production presentation helper');
const threadSource = await readFile(new URL('../../../source/emulation/cpu/normal/normalPlatformMultiThreaded.cpp', import.meta.url), 'utf8');
const scheduleBody = threadSource.match(/EM_JS\(void, boxedwineInitThreadEventLoop,[^\n]*\{([\s\S]*?)\r?\n\}\);/)?.[1];
assert.ok(scheduleBody, 'Production worker event-loop helper');
const {chromium} = createRequire(import.meta.url)(playwrightModule);
const server = createServer((request, response) => {
    response.writeHead(200, {'Content-Type': 'text/html', 'Cross-Origin-Opener-Policy': 'same-origin',
        'Cross-Origin-Embedder-Policy': 'require-corp'});
    response.end('<!doctype html><title>Threaded WebGL presentation regression</title>');
});
await new Promise(done => server.listen(0, '127.0.0.1', done));
const report = {sourceSha256: createHash('sha256').update(source).digest('hex'),
    scope: 'Production presentation helper; worker pixels, GL state and retained GPU frame images', passed: false};
let browser, page;
try {
    browser = await chromium.launch({executablePath: chrome, headless: true,
        args: ['--disable-background-timer-throttling', '--disable-renderer-backgrounding']});
    report.browserVersion = browser.version();
    page = await browser.newPage();
    await page.goto(`http://127.0.0.1:${server.address().port}/`);
    report.cases = await page.evaluate(async ({body, scheduleBody}) => {
        function workerMain(body, scheduleBody) {
            onmessage = async ({data: options}) => {
                try {
                    const stress = options.stress, size = stress ? 256 : 8;
                    const canvas = new OffscreenCanvas(size, size);
                    const gl = canvas.getContext(options.version, {alpha: false, antialias: false, preserveDrawingBuffer: true});
                    if (!gl) throw Error(options.version + ' unavailable');
                    const webgl2 = options.version === 'webgl2';
                    self.GL = {currentContext: {GLctx: gl}};
                    self.Module = {__boxedwinePresentState: options.control};
                    self.bwCallHandlerCmd = 'present';
                    const present = new Function('requestedWidth', 'requestedHeight', body);
                    if (stress) {
                        self.MainLoop = {};
                        new Function(scheduleBody)();
                        const yieldToBrowser = () => new Promise(resolve => MainLoop.setImmediate(resolve));
                        for (let i = 0; i < 300; ++i) {
                            gl.clearColor(i % 2, 0.5, 1, 1);
                            gl.clear(gl.COLOR_BUFFER_BIT);
                            present(size, size);
                            if (Module.__boxedwinePresentErrorLogged) throw Error('Presentation failed');
                            while (Atomics.load(options.control, 0)) Atomics.wait(options.control, 0, 1, 100);
                            if (!options.blocked) await yieldToBrowser();
                        }
                        postMessage({kind: 'held', frames: 300});
                        while (!Atomics.load(options.control, 1)) Atomics.wait(options.control, 1, 0, 100);
                        return;
                    }
                    gl.enable(gl.SCISSOR_TEST);
                    for (let y = 0; y < size; ++y) for (let x = 0; x < size; ++x) {
                        gl.scissor(x, y, 1, 1);
                        gl.clearColor(x * 31 / 255, y * 29 / 255, (x + y) * 11 / 255, 1);
                        gl.clear(gl.COLOR_BUFFER_BIT);
                    }
                    gl.disable(gl.SCISSOR_TEST);
                    gl.viewport(...options.viewport);
                    if (options.screen) [Module.__boxedwineWebGLPresentWidth, Module.__boxedwineWebGLPresentHeight] = options.screen;
                    function makeFbo() {
                        const fb = gl.createFramebuffer(), texture = gl.createTexture();
                        gl.bindTexture(gl.TEXTURE_2D, texture);
                        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 8, 8, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
                        gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
                        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
                        return fb;
                    }
                    let pbo;
                    if (options.hostile) {
                        if (webgl2) gl.readBuffer(gl.NONE);
                        const read = makeFbo(), draw = makeFbo();
                        if (webgl2) {
                            gl.bindFramebuffer(gl.READ_FRAMEBUFFER, read);
                            gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, draw);
                            pbo = gl.createBuffer();
                            gl.bindBuffer(gl.PIXEL_PACK_BUFFER, pbo);
                            gl.bufferData(gl.PIXEL_PACK_BUFFER, new Uint8Array(4096).fill(91), gl.STREAM_READ);
                            gl.pixelStorei(gl.PACK_ROW_LENGTH, 31);
                            gl.pixelStorei(gl.PACK_SKIP_ROWS, 2);
                            gl.pixelStorei(gl.PACK_SKIP_PIXELS, 3);
                        }
                        gl.pixelStorei(gl.PACK_ALIGNMENT, 8);
                    }
                    function state() {
                        return [gl.getParameter(gl.FRAMEBUFFER_BINDING), gl.getParameter(gl.PACK_ALIGNMENT),
                            ...gl.getParameter(gl.VIEWPORT), gl.isEnabled(gl.SCISSOR_TEST),
                            ...(webgl2 ? [gl.getParameter(gl.READ_FRAMEBUFFER_BINDING), gl.getParameter(gl.READ_BUFFER),
                                gl.getParameter(gl.PIXEL_PACK_BUFFER_BINDING), gl.getParameter(gl.PACK_ROW_LENGTH),
                                gl.getParameter(gl.PACK_SKIP_ROWS), gl.getParameter(gl.PACK_SKIP_PIXELS)] : [])];
                    }
                    const before = state();
                    if (gl.getError()) throw Error('Invalid test setup');
                    present(...options.requested);
                    if (options.throttle) for (let i = 0; i < 5; ++i) present(...options.requested);
                    const after = state(), restored = before.every((value, i) => value === after[i]);
                    let pboUntouched = true, defaultReadBufferRestored = true;
                    if (pbo) {
                        const bytes = new Uint8Array(4096);
                        gl.getBufferSubData(gl.PIXEL_PACK_BUFFER, 0, bytes);
                        pboUntouched = bytes.every(value => value === 91);
                        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, null);
                        defaultReadBufferRestored = gl.getParameter(gl.READ_BUFFER) === gl.NONE;
                    }
                    postMessage({kind: 'result', restored, pboUntouched, defaultReadBufferRestored,
                        error: gl.getError(), presentError: !!Module.__boxedwinePresentErrorLogged});
                } catch (error) { postMessage({kind: 'error', message: String(error.stack || error)}); }
            };
        }
        const workerUrl = URL.createObjectURL(new Blob([`(${workerMain.toString()})(${JSON.stringify(body)}, ${JSON.stringify(scheduleBody)})`], {type: 'text/javascript'}));
        window.runPresentation = options => new Promise((resolve, reject) => {
            const control = new Int32Array(new SharedArrayBuffer(8));
            const worker = new Worker(workerUrl), canvas = document.createElement('canvas');
            const context = canvas.getContext('2d', {willReadFrequently: true});
            let frames = 0, frame;
            const timeout = setTimeout(() => {worker.terminate(); reject(Error('Worker timeout'));}, 120000);
            function finish() {clearTimeout(timeout); worker.terminate();}
            worker.onerror = event => {finish(); reject(Error(event.message));};
            worker.onmessage = ({data}) => {
                if (data.cmd === 'present') {
                    ++frames;
                    let width, height;
                    if (data.handler === 'boxedwinePresentFrame') {
                        const [bitmap, , x, y, w, h] = data.args;
                        width = canvas.width = w; height = canvas.height = h;
                        context.drawImage(bitmap, x, y, w, h, 0, 0, w, h);
                        bitmap.close();
                    } else if (data.handler === 'boxedwinePresentSoftwareFrame') {
                        const [w, h, buffer] = data.args;
                        width = canvas.width = w; height = canvas.height = h;
                        context.putImageData(new ImageData(new Uint8ClampedArray(buffer), w, h), 0, 0);
                    } else {finish(); reject(Error('Unknown presentation handler')); return;}
                    if (!options.stress) frame = {width, height, pixels: Array.from(context.getImageData(0, 0, width, height).data)};
                    if (!options.throttle) {Atomics.store(control, 0, 0); Atomics.notify(control, 0);}
                } else if (data.kind === 'result') {
                    finish(); resolve({...data, frames, frame});
                } else if (data.kind === 'held') {
                    clearTimeout(timeout);
                    window.releasePresentation = () => {Atomics.store(control, 1, 1); Atomics.notify(control, 1); worker.terminate();};
                    resolve({frames});
                } else if (data.kind === 'error') {finish(); reject(Error(data.message));}
            };
            worker.postMessage({...options, control});
        });
        const results = [];
        for (const version of ['webgl', 'webgl2']) {
            for (const spec of [
                {name: 'whole canvas', requested: [8, 8], crop: [0, 0, 8, 8]},
                {name: 'odd-height requested crop', requested: [4, 5], crop: [0, 0, 4, 5], hostile: true},
                {name: 'offset viewport crop', requested: [0, 0], viewport: [2, 1, 4, 5], crop: [2, 1, 4, 5], hostile: true},
                {name: 'screen size limits request', requested: [8, 8], screen: [3, 5], crop: [0, 0, 3, 5]},
                {name: 'one frame in flight', requested: [8, 8], crop: [0, 0, 8, 8], throttle: true}
            ]) {
                const result = await window.runPresentation({version, viewport: [0, 0, 8, 8], ...spec});
                const [sx, sy, width, height] = spec.crop;
                let differences = 0;
                for (let y = 0; y < height; ++y) for (let x = 0; x < width; ++x) {
                    const gx = sx + x, gy = sy + height - 1 - y;
                    const expected = [gx * 31, gy * 29, (gx + gy) * 11, 255];
                    for (let c = 0; c < 4; ++c) if (Math.abs(result.frame.pixels[(y * width + x) * 4 + c] - expected[c]) > 1) ++differences;
                }
                results.push({name: version + ': ' + spec.name, differences, restored: result.restored,
                    pboUntouched: result.pboUntouched, defaultReadBufferRestored: result.defaultReadBufferRestored,
                    error: result.error, frames: result.frames,
                    passed: differences === 0 && result.restored && result.pboUntouched && result.defaultReadBufferRestored
                        && result.error === 0 && !result.presentError && result.frames === 1
                        && result.frame.width === width && result.frame.height === height});
            }
        }
        return results;
    }, {body, scheduleBody});
    console.log(JSON.stringify({cases: report.cases}));
    report.stress = await page.evaluate(blocked => window.runPresentation({version: 'webgl2', stress: true, blocked}), blocked);
    report.stress.yieldBetweenFrames = !blocked;
    const cdp = await browser.newBrowserCDPSession();
    const complete = new Promise(done => cdp.once('Tracing.tracingComplete', done));
    await cdp.send('Tracing.start', {transferMode: 'ReturnAsStream', traceConfig: {recordMode: 'recordUntilFull',
        includedCategories: ['disabled-by-default-memory-infra'], memoryDumpConfig: {allowedDumpModes: ['detailed'], triggers: []}}});
    // The blocked worker cannot answer its dump request, so overall success can
    // be false. Require the separate GPU-process dump below, where the retained
    // images live, rather than treating a missing dump as zero retained images.
    report.stress.dump = await cdp.send('Tracing.requestMemoryDump', {deterministic: false, levelOfDetail: 'detailed'});
    await cdp.send('Tracing.end');
    const {stream} = await complete;
    let trace = '';
    for (;;) {
        const chunk = await cdp.send('IO.read', {handle: stream});
        trace += chunk.base64Encoded ? Buffer.from(chunk.data, 'base64').toString('utf8') : chunk.data;
        if (chunk.eof) break;
    }
    await cdp.send('IO.close', {handle: stream});
    await writeFile(join(output, 'memory-trace.json'), trace);
    const images = new Map();
    let hasGpuDump = false;
    for (const event of JSON.parse(trace).traceEvents) {
        for (const [name, allocation] of Object.entries(event.args?.dumps?.allocators || {})) {
            if (name === 'gpu') hasGpuDump = true;
            if (name.startsWith('gpu/shared_images/') && name.includes('/mailbox_') && !name.includes('/element_')
                    && allocation.attrs?.dimensions?.value === '256x256') images.set(name, allocation);
        }
    }
    Object.assign(report.stress, {hasGpuDump, retainedFrameImages: images.size,
        retainedBytes: [...images.values()].reduce((sum, image) => sum + parseInt(image.attrs.size.value, 16), 0),
        passed: hasGpuDump && images.size <= 16 && report.stress.frames === 300});
    report.passed = report.cases.every(result => result.passed) && report.stress.passed;
    console.log(JSON.stringify({stress: report.stress, passed: report.passed}));
    assert.ok(report.passed, 'Presentation regression checks');
} catch (error) {
    report.error = String(error.stack || error);
    process.exitCode = 1;
} finally {
    await page?.evaluate(() => window.releasePresentation?.()).catch(() => {});
    await browser?.close();
    await new Promise(done => server.close(done));
    await writeFile(join(output, 'report.json'), JSON.stringify(report, null, 2) + '\n');
}
