// Usage: node runRGB565ReadbackTest.mjs CHROME PLAYWRIGHT_MODULE OUTPUT [SOURCE]
// Runs the exact production EM_JS helpers on a real WebGL2 context, including
// the client-memory and temporary-buffer paths. SOURCE permits a failing control.
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {mkdir, readFile, writeFile} from 'node:fs/promises';
import {createServer} from 'node:http';
import {createRequire} from 'node:module';
import {resolve, join} from 'node:path';

const [chrome, playwrightModule, directory, sourcePath] = process.argv.slice(2);
if (!chrome || !playwrightModule || !directory) throw Error('Expected CHROME PLAYWRIGHT_MODULE OUTPUT [SOURCE]');
const output = resolve(directory);
await mkdir(output, {recursive: true});
const source = await readFile(sourcePath || new URL('../../source/opengl/glcommon.cpp', import.meta.url), 'utf8');
const bodies = {};
for (const name of ['blit_begin', 'blit_end', 'client', 'temp_rgba8']) {
    const match = source.match(new RegExp('EM_JS\\([^\\n]*boxedwine_read_pixels_' + name + '[^\\n]*\\{([\\s\\S]*?)\\n\\}\\);'));
    assert.ok(match, `Production ${name} helper`);
    bodies[name] = match[1];
}
const {chromium} = createRequire(import.meta.url)(playwrightModule);
// SharedArrayBuffer exercises the multi-threaded heap and requires isolation.
const server = createServer((request, response) => {
    response.writeHead(200, {'Content-Type': 'text/html', 'Cross-Origin-Opener-Policy': 'same-origin',
        'Cross-Origin-Embedder-Policy': 'require-corp'});
    response.end('<!doctype html><title>RGB565 readback regression</title>');
});
await new Promise(done => server.listen(0, '127.0.0.1', done));
let browser;
const report = {sourceSha256: createHash('sha256').update(source).digest('hex'),
    scope: 'Production EM_JS helpers on WebGL2; no full Boxedwine/Wine runtime', passed: false};
try {
    browser = await chromium.launch({executablePath: chrome, headless: true});
    report.browserVersion = browser.version();
    const page = await browser.newPage();
    await page.goto(`http://127.0.0.1:${server.address().port}/`);
    Object.assign(report, await page.evaluate(bodies => {
        const gl = document.createElement('canvas').getContext('webgl2');
        if (!gl) throw Error('WebGL2 unavailable');
        window.GLctx = gl;
        window.Module = {};
        window.heapObjectForWebGLType = () => window.HEAPU8;
        const begin = new Function('x', 'y', 'width', 'height', 'format', 'type', 'byteLength', bodies.blit_begin);
        const end = new Function('pixels', bodies.blit_end);
        const client = new Function('x', 'y', 'width', 'height', 'format', 'type', 'pixels', 'byteLength', bodies.client);
        const temp = new Function('x', 'y', 'width', 'height', 'format', 'type', 'pixels', 'byteLength', bodies.temp_rgba8);
        const results = [];
        const textures = [];
        function textureFbo(width, height, internalformat = gl.RGB565) {
            const texture = gl.createTexture(), fbo = gl.createFramebuffer();
            textures.push(texture);
            gl.bindTexture(gl.TEXTURE_2D, texture);
            const data = internalformat === gl.RGB565
                ? Uint16Array.from({length: width * height}, (_, i) => i & 65535)
                : Uint32Array.from({length: width * height}, (_, i) => (i * 65537) >>> 0);
            gl.texImage2D(gl.TEXTURE_2D, 0, internalformat, width, height, 0,
                internalformat === gl.RGB565 ? gl.RGB : gl.RGBA,
                internalformat === gl.RGB565 ? gl.UNSIGNED_SHORT_5_6_5 : gl.UNSIGNED_INT_2_10_10_10_REV, data);
            gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
            if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) throw Error('Incomplete test framebuffer');
            return fbo;
        }
        function pack(rowLength = 0, skipPixels = 0, skipRows = 0, alignment = 4) {
            for (const [pname, value] of [[gl.PACK_ROW_LENGTH, rowLength], [gl.PACK_SKIP_PIXELS, skipPixels],
                    [gl.PACK_SKIP_ROWS, skipRows], [gl.PACK_ALIGNMENT, alignment]]) gl.pixelStorei(pname, value);
        }
        function state() {
            const active = gl.getParameter(gl.ACTIVE_TEXTURE);
            const activeBinding = gl.getParameter(gl.TEXTURE_BINDING_2D);
            gl.activeTexture(gl.TEXTURE0);
            const texture0 = gl.getParameter(gl.TEXTURE_BINDING_2D);
            gl.activeTexture(active);
            return [gl.getParameter(gl.READ_FRAMEBUFFER_BINDING), gl.getParameter(gl.DRAW_FRAMEBUFFER_BINDING),
                gl.getParameter(gl.RENDERBUFFER_BINDING), active, activeBinding, texture0,
                gl.getParameter(gl.READ_BUFFER), gl.isEnabled(gl.SCISSOR_TEST), gl.isEnabled(gl.RASTERIZER_DISCARD),
                ...gl.getParameter(gl.COLOR_WRITEMASK), gl.getParameter(gl.PACK_ALIGNMENT),
                gl.getParameter(gl.PACK_ROW_LENGTH), gl.getParameter(gl.PACK_SKIP_PIXELS), gl.getParameter(gl.PACK_SKIP_ROWS)];
        }
        function check(name, {x = 0, y = 0, width = 256, height = 256, length, mode = 'client', shared = false,
                expectedUsed = true, expectedError = gl.NO_ERROR, pendingError = false} = {}) {
            const rowLength = gl.getParameter(gl.PACK_ROW_LENGTH) || Math.max(width, 0);
            const alignment = gl.getParameter(gl.PACK_ALIGNMENT);
            const stride = Math.ceil(rowLength * 4 / alignment) * alignment;
            const offset = gl.getParameter(gl.PACK_SKIP_ROWS) * stride + gl.getParameter(gl.PACK_SKIP_PIXELS) * 4;
            length ??= offset + Math.max(height - 1, 0) * stride + Math.max(width, 0) * 4;
            const pointer = 32;
            const heap = new Uint8Array(shared ? new SharedArrayBuffer(pointer + length + 32) : new ArrayBuffer(pointer + length + 32));
            for (let i = 0; i < heap.length; ++i) heap[i] = (i * 37 + 17) & 255;
            const expected = heap.slice();
            gl.readPixels(x, y, width, height, gl.RGBA, gl.UNSIGNED_BYTE, expected.subarray(pointer, pointer + length));
            const directError = gl.getError();
            const before = state();
            window.HEAPU8 = heap;
            if (pendingError) gl.enable(0xdead);
            const used = begin(x, y, width, height, gl.RGBA, gl.UNSIGNED_BYTE, length);
            (mode === 'temp' ? temp : client)(used ? 0 : x, used ? 0 : y, width, height,
                gl.RGBA, gl.UNSIGNED_BYTE, pointer, length);
            if (used) end(pointer);
            const actualError = gl.getError(), after = state();
            let differences = 0, firstDifference;
            for (let i = 0; i < heap.length; ++i) {
                if (heap[i] !== expected[i]) {
                    ++differences;
                    firstDifference ??= {offset: i - pointer, expected: expected[i], actual: heap[i]};
                }
            }
            const restored = before.every((value, index) => value === after[index]);
            const passed = !!used === expectedUsed && directError === expectedError && differences === 0 && restored
                && actualError === (pendingError ? gl.INVALID_ENUM : expectedError);
            results.push({name, passed, used: !!used, differences, firstDifference, restored, directError, actualError});
        }
        const source = textureFbo(512, 512), other = textureFbo(8, 8);
        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, source);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, other);
        gl.activeTexture(gl.TEXTURE5);
        gl.bindTexture(gl.TEXTURE_2D, textures[0]);
        // 512x128 covers each of the 65,536 RGB565 codes exactly once.
        for (const mode of ['client', 'temp']) {
            for (const shared of [false, true]) {
                check(`all RGB565 colors, ${mode}, ${shared ? 'shared' : 'ordinary'} heap`, {width: 512, height: 128, mode, shared});
            }
        }
        check('one pixel bypasses shortcut', {x: 3, width: 1, height: 1, expectedUsed: false});
        gl.enable(gl.SCISSOR_TEST);
        gl.scissor(1, 1, 1, 1);
        gl.enable(gl.RASTERIZER_DISCARD);
        gl.colorMask(false, true, false, false);
        check('offset read and hostile draw state', {x: 19, y: 11});
        check('existing error remains pending', {pendingError: true});
        for (const alignment of [1, 2, 4, 8]) {
            pack(263, 3, 2, alignment);
            check(`pack skips, row padding and alpha preserved, alignment ${alignment}`, {width: 257, height: 257, mode: 'temp', shared: true});
        }
        pack();
        check('short destination is unchanged', {length: 100, expectedUsed: false, expectedError: gl.INVALID_OPERATION});
        check('negative width and height', {width: -256, height: -256, length: 128, expectedUsed: false, expectedError: gl.INVALID_VALUE});
        check('scratch framebuffer grows', {width: 512, height: 512});
        check('scratch framebuffer is reused for a smaller read');
        gl.readBuffer(gl.NONE);
        check('missing read buffer', {expectedUsed: false, expectedError: gl.INVALID_OPERATION});
        gl.readBuffer(gl.COLOR_ATTACHMENT0);
        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, gl.createFramebuffer());
        check('incomplete source', {expectedUsed: false, expectedError: gl.INVALID_FRAMEBUFFER_OPERATION});
        textureFbo(256, 256, gl.RGB10_A2);
        check('RGB10_A2 retains direct conversion', {expectedUsed: false});
        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, source);
        const pbo = gl.createBuffer();
        gl.bindBuffer(gl.PIXEL_PACK_BUFFER, pbo);
        results.push({name: 'PBO read bypasses shortcut', passed: begin(0, 0, 256, 256, gl.RGBA, gl.UNSIGNED_BYTE, 262144) === 0});
        gl.bindBuffer(gl.PIXEL_PACK_BUFFER, null);
        gl.disable(gl.SCISSOR_TEST);
        gl.disable(gl.RASTERIZER_DISCARD);
        gl.colorMask(true, true, true, true);
        for (const samples of [0, 4]) {
            const rb = gl.createRenderbuffer(), fbo = gl.createFramebuffer();
            gl.bindRenderbuffer(gl.RENDERBUFFER, rb);
            gl.renderbufferStorageMultisample(gl.RENDERBUFFER, samples, gl.RGB565, 256, 256);
            gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
            gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.RENDERBUFFER, rb);
            gl.clearColor(0, 0, 3 / 31, 1);
            gl.clear(gl.COLOR_BUFFER_BIT);
            check(`${samples}-sample renderbuffer`, {expectedUsed: !samples, expectedError: samples ? gl.INVALID_OPERATION : gl.NO_ERROR});
        }
        const timings = [];
        for (const [width, height] of [[640, 480], [1280, 720]]) {
            textureFbo(width, height);
            const length = width * height * 4, pointer = 32;
            window.HEAPU8 = new Uint8Array(new SharedArrayBuffer(pointer + length));
            const direct = new Uint8Array(length);
            const samples = {direct: [], corrected: []};
            for (let i = 0; i < 70; ++i) {
                for (const name of (i & 1 ? ['corrected', 'direct'] : ['direct', 'corrected'])) {
                    gl.clearColor((i % 9) / 9, 0.4, 0.7, 1);
                    gl.clear(gl.COLOR_BUFFER_BIT);
                    const t = performance.now();
                    if (name === 'direct') gl.readPixels(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, direct);
                    else {
                        const used = begin(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, length);
                        if (!used) throw Error('Benchmark bypassed the fast path');
                        temp(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, pointer, length);
                        end(pointer);
                    }
                    const elapsed = performance.now() - t;
                    if (i >= 10) samples[name].push(elapsed);
                }
            }
            const times = {};
            for (const [name, values] of Object.entries(samples)) {
                values.sort((a, b) => a - b);
                times[name] = {medianMs: values[30], p90Ms: values[54]};
            }
            timings.push({width, height, ...times, speedup: times.direct.medianMs / times.corrected.medianMs});
        }
        const debug = gl.getExtension('WEBGL_debug_renderer_info');
        return {renderer: debug && gl.getParameter(debug.UNMASKED_RENDERER_WEBGL),
            cases: results, timings, finalError: gl.getError(), passed: results.every(result => result.passed)};
    }, bodies));
    assert.ok(report.passed, `Failed cases: ${report.cases.filter(result => !result.passed).map(result => result.name).join(', ')}`);
    assert.equal(report.finalError, 0, 'No unexpected GL errors');
} catch (error) {
    report.passed = false;
    report.error = String(error);
    process.exitCode = 1;
} finally {
    if (browser) await browser.close();
    await new Promise(done => server.close(done));
    await writeFile(join(output, 'readpixels-result.json'), JSON.stringify(report, null, 2) + '\n');
    console.log(JSON.stringify(report, null, 2));
}
