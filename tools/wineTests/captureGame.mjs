// Discovery driver: retain compositor frames and inputs without claiming acceptance.
import { createServer } from 'node:http';
import { createReadStream, existsSync, openSync, closeSync } from 'node:fs';
import { mkdir, readFile, writeFile, realpath, stat, appendFile } from 'node:fs/promises';
import { resolve, relative, join, extname, sep, isAbsolute } from 'node:path';
import { createHash } from 'node:crypto';
import { spawn, spawnSync } from 'node:child_process';
import { createRequire } from 'node:module';
import { createInterface } from 'node:readline';
import { setTimeout as delay } from 'node:timers/promises';
import { parseLifecycle, nativePixelStyle, measureCanvasStack, nativeClip } from './gameCaptureChecks.mjs';
import { installGamePerformance, summarizeGamePerformance } from './gamePerformance.mjs';

const config = JSON.parse(await readFile(process.argv[2], 'utf8'));
const require = createRequire(import.meta.url);
const { chromium } = require(config.playwrightModule);
const output = resolve(config.output);
await mkdir(output, { recursive: false });
await writeFile(join(output, 'config.json'), JSON.stringify(config, null, 2));
const record = { startedAt: new Date().toISOString(), scope: 'Discovery capture; no game acceptance inferred.',
    config, actions: [], errors: [], inputHashes: [], sourceHashes: [],
    applicationExitObserved: false, cleanupObserved: false, cleanupWaitSatisfied: false };
const site = await realpath(config.site);
let browser, page, child, server, lines, timer, chromeFd;
let closing = false;
let sequence = 0;
const checksum = data => createHash('sha256').update(data).digest('hex');
async function jsonFile(name, data) {
    await writeFile(join(output, name), JSON.stringify(data, null, 2) + '\n', { flag: 'wx' });
}
async function log(name, value) {
    await appendFile(join(output, name), JSON.stringify({ at: new Date().toISOString(), ...value }) + '\n');
}
const wrapper = `<script>
(function() {
  const original = getEmulatorParams;
  const quote = value => "'" + String(value).replace(/'/g, "'\\\\''") + "'";
  getEmulatorParams = function() {
    const params = original();
    const index = params.indexOf('/bin/wine');
    if (index < 0) throw Error('Game capture cannot find Wine launch arguments');
    const rootIndex = params.indexOf('-root');
    if (rootIndex < 0 || rootIndex + 1 >= index || !params[rootIndex + 1].startsWith('/'))
      throw Error('Game capture cannot find the guest writable root');
    window.__gameCaptureLifecyclePath = params[rootIndex + 1].replace(/\\/$/, '')
      + '/tmp/boxedwine-game-lifecycle.log';
    const game = params.slice(index).map(quote).join(' ');
    const command = game + '; game_status=$?; echo BW_GAME_EXIT:$game_status >> /tmp/boxedwine-game-lifecycle.log; '
      + '/opt/wine/bin/wineserver -k; cleanup_status=$?; '
      + 'echo BW_GAME_CLEANUP:$cleanup_status >> /tmp/boxedwine-game-lifecycle.log; exit $game_status';
    params.splice(index, params.length - index, '/bin/sh', '-c', command);
    window.__gameCaptureCommand = command;
    console.log('Game capture command: ' + command);
    return params;
  };
  window.__readGameCapture = function() {
    let lifecycle = '', heapBytes = null;
    try {
      const fs = typeof FS !== 'undefined' ? FS : Module.FS;
      lifecycle = new TextDecoder().decode(fs.readFile(window.__gameCaptureLifecyclePath));
    } catch (_) {}
    try { heapBytes = typeof HEAPU8 !== 'undefined' ? HEAPU8.byteLength : Module.HEAPU8.byteLength; } catch (_) {}
    return { output: document.getElementById('output').value, lifecycle, heapBytes,
      focused: document.hasFocus(), activeElement: document.activeElement?.id,
      visibility: document.visibilityState,
      command: window.__gameCaptureCommand || '', lifecyclePath: window.__gameCaptureLifecyclePath || '',
      pointerLock: document.pointerLockElement?.id || '',
      fullscreen: document.fullscreenElement?.id || '',
      frameCanvasSize: (() => {
        const frame = document.querySelector('.emscripten_border');
        if (!frame) return null;
        const style = getComputedStyle(frame);
        return ['--boxedwine-canvas-width', '--boxedwine-canvas-height']
          .map(name => Number(style.getPropertyValue(name)));
      })(),
      status: document.getElementById('status')?.textContent || '',
      canvases: Array.from(document.querySelectorAll('canvas')).map(canvas => ({
        id: canvas.id, width: canvas.width, height: canvas.height,
        display: getComputedStyle(canvas).display, visibility: getComputedStyle(canvas).visibility,
        rect: canvas.getBoundingClientRect().toJSON()
      })) };
  };
})();
</script>`;
const insertion = /(<script\b[^>]*\bsrc\s*=\s*(?:["']boxedwine-shell\.js["']|boxedwine-shell\.js)[^>]*>\s*<\/script\s*>)/i;

async function state() {
    return page.evaluate(() => typeof window.__readGameCapture === 'function' ? window.__readGameCapture()
        : { status: document.getElementById('status')?.textContent || 'observer not loaded' });
}
async function capture(name) {
    if (!/^[a-z0-9_-]+$/.test(name)) throw Error('Invalid capture name');
    const snapshot = await state();
    await jsonFile(name + '.json', snapshot);
    const full = await page.screenshot({ fullPage: true, timeout: 15000 });
    await writeFile(join(output, name + '-page.png'), full, { flag: 'wx' });
    const frame = await page.locator('.emscripten_border').screenshot({ timeout: 15000 });
    await writeFile(join(output, name + '-canvas.png'), frame, { flag: 'wx' });
    return { name, canvasSha256: checksum(frame), heapBytes: snapshot.heapBytes,
        lifecycle: snapshot.lifecycle, pointerLock: snapshot.pointerLock };
}
async function captureNativePixels(name) {
    // Preserve the real presentation before applying the explicitly recorded test layout.
    const presentation = await capture(name + '-presentation');
    const style = await page.addStyleTag({content: nativePixelStyle});
    try {
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
        const before = await page.evaluate(measureCanvasStack);
        await jsonFile(name + '-native-layout-before.json', before);
        const geometry = nativeClip(before);
        const image = await page.screenshot({clip: geometry.clip, timeout: 15000});
        await writeFile(join(output, name + '-native.png'), image, {flag: 'wx'});
        const after = await page.evaluate(measureCanvasStack);
        await jsonFile(name + '-native-layout-after.json', after);
        if (JSON.stringify(before) !== JSON.stringify(after)) throw Error('Canvas layout changed during native capture');
        const result = {presentation, ...geometry, sha256: checksum(image), nativePixelStyle,
            scope: 'Browser compositor at one CSS/device pixel per game pixel; no reference match inferred.'};
        await jsonFile(name + '-native.json', result);
        return result;
    } finally {
        await style.evaluate(element => element.remove());
    }
}
async function close(reason) {
    if (closing) return;
    closing = true;
    clearTimeout(timer);
    lines?.close();
    record.closeReason = reason;
    try { if (page) await jsonFile('final-state.json', await state()); }
    catch (error) { record.errors.push(String(error)); }
    if (browser) await browser.close().catch(error => record.errors.push(String(error)));
    if (child && child.exitCode === null) {
        if (process.platform === 'win32') spawnSync('taskkill', ['/PID', String(child.pid), '/T', '/F'], { windowsHide: true, stdio: 'ignore' });
        else child.kill('SIGTERM');
    }
    if (chromeFd !== undefined) closeSync(chromeFd);
    if (server) { server.closeAllConnections(); await new Promise(done => server.close(done)); }
    record.finishedAt = new Date().toISOString();
    await jsonFile('capture.json', record);
    console.log(JSON.stringify({ closed: reason, cleanupObserved: record.cleanupObserved,
        applicationExitObserved: record.applicationExitObserved,
        cleanupWaitSatisfied: record.cleanupWaitSatisfied, errors: record.errors, output }));
    process.exitCode = record.errors.length ? 1 : 0;
}
try {
    // Retain the exact driver and checks alongside each run, before browser startup.
    for (const name of ['captureGame.mjs', 'gameCaptureChecks.mjs', 'gamePerformance.mjs']) {
        const bytes = await readFile(new URL('./' + name, import.meta.url));
        await writeFile(join(output, name), bytes, {flag: 'wx'});
        record.sourceHashes.push({name, bytes: bytes.length, sha256: checksum(bytes)});
    }
    for (const input of config.inputs) {
        const data = await readFile(input.path);
        if (checksum(data) !== input.sha256) throw Error('Input hash mismatch: ' + input.path);
        record.inputHashes.push({ path: input.path, bytes: data.length, sha256: checksum(data) });
    }
    server = createServer(async (request, response) => {
        try {
            const url = new URL(request.url, 'http://localhost');
            let path = await realpath(join(site, decodeURIComponent(url.pathname).replace(/^\/+/, '')));
            const rel = relative(site, path);
            if (rel === '..' || rel.startsWith('..' + sep) || isAbsolute(rel)) throw Error('Outside site');
            if (!(await stat(path)).isFile()) throw Error('Not a file');
            const headers = { 'Cache-Control': 'no-store', 'Cross-Origin-Opener-Policy': 'same-origin',
                'Cross-Origin-Embedder-Policy': 'require-corp', 'Cross-Origin-Resource-Policy': 'same-origin',
                'Content-Type': ({ '.html': 'text/html', '.js': 'application/javascript', '.wasm': 'application/wasm',
                    '.css': 'text/css', '.zip': 'application/zip', '.png': 'image/png' })[extname(path)] || 'application/octet-stream' };
            if (path.endsWith('boxedwine.html')) {
                const html = await readFile(path, 'utf8');
                if (!insertion.test(html)) throw Error('No shell insertion point');
                response.writeHead(200, headers);
                response.end(html.replace(insertion, match => match + wrapper));
            } else {
                response.writeHead(200, { ...headers, 'Content-Length': (await stat(path)).size });
                createReadStream(path).on('error', () => response.destroy()).pipe(response);
            }
            await log('server.jsonl', { url: request.url, status: 200 });
        } catch (error) {
            response.writeHead(404); response.end('Not found');
            await log('server.jsonl', { url: request.url, status: 404, error: String(error) });
        }
    });
    await new Promise(done => server.listen(0, '127.0.0.1', done));
    const port = server.address().port;
    record.launchUrl = `http://127.0.0.1:${port}${config.launchPath}`;
    const profile = join(output, 'chrome-profile');
    await mkdir(profile);
    const args = [...config.chromeArgs, '--user-data-dir=' + profile, '--remote-debugging-port=0',
        '--remote-debugging-address=127.0.0.1', '--force-device-scale-factor=1', 'about:blank'];
    record.chromeCommand = [config.chrome, ...args];
    chromeFd = openSync(join(output, 'chrome.log'), 'wx');
    child = spawn(config.chrome, args, { windowsHide: true, stdio: ['ignore', chromeFd, chromeFd] });
    child.on('error', error => record.errors.push(String(error)));
    const endpoint = join(profile, 'DevToolsActivePort');
    const deadline = Date.now() + 30000;
    while (!existsSync(endpoint)) {
        if (child.exitCode !== null || Date.now() >= deadline) throw Error('Chrome did not create a debugging endpoint');
        await delay(100);
    }
    const debugPort = Number((await readFile(endpoint, 'utf8')).split('\n')[0]);
    browser = await chromium.connectOverCDP('http://127.0.0.1:' + debugPort);
    record.browserVersion = browser.version();
    const browserSession = await browser.newBrowserCDPSession();
    record.systemInfo = await browserSession.send('SystemInfo.getInfo');
    page = browser.contexts()[0].pages()[0];
    page.setDefaultTimeout(15000);
    page.on('console', message => log('console.jsonl', { type: message.type(), text: message.text() }));
    page.on('pageerror', error => { record.errors.push(String(error)); log('errors.jsonl', { message: String(error), stack: error.stack }); });
    page.on('crash', () => record.errors.push('page crashed'));
    page.on('dialog', dialog => { record.errors.push('unexpected browser dialog: ' + dialog.message()); dialog.dismiss(); });
    if (config.performance !== undefined && typeof config.performance !== 'boolean') throw Error('Performance option must be Boolean');
    if (config.performance === true) await page.addInitScript(installGamePerformance);
    await page.goto(record.launchUrl, { waitUntil: 'domcontentloaded', timeout: 30000 });
    await page.bringToFront();
    await jsonFile('started.json', record);
    console.log(JSON.stringify({ ready: true, url: record.launchUrl, output }));
    timer = setTimeout(() => { record.errors.push('discovery deadline expired'); close('deadline'); }, (config.timeoutSeconds || 1200) * 1000);
    lines = createInterface({ input: process.stdin, terminal: false });
    for await (const line of lines) {
        if (closing) break;
        let action;
        try {
            action = JSON.parse(line);
            const index = ++sequence;
            record.actions.push({ index, at: new Date().toISOString(), action });
            let result;
            switch (action.type) {
                case 'wait': {
                    if (!Number.isInteger(action.milliseconds) || action.milliseconds < 0 || action.milliseconds > 5000) throw Error('Wait requires 0..5000 milliseconds');
                    await delay(action.milliseconds); result = await state(); break;
                }
                case 'state': result = await state(); break;
                case 'measure-performance': {
                    if (config.performance !== true) throw Error('Performance observer is not enabled');
                    if (!/^[a-z0-9_-]+$/.test(action.name)) throw Error('Invalid performance phase name');
                    if (!Number.isInteger(action.milliseconds) || action.milliseconds < 5000 || action.milliseconds > 60000)
                        throw Error('Performance phase requires 5000..60000 milliseconds');
                    await page.evaluate(name => window.__boxedwineGamePerformance.begin(name), action.name);
                    // No screenshots, state polling or input commands during this interval.
                    await delay(action.milliseconds);
                    const raw = await page.evaluate(() => window.__boxedwineGamePerformance.end());
                    await jsonFile(action.name + '-performance-raw.json', raw);
                    result = summarizeGamePerformance(raw, action.milliseconds);
                    await jsonFile(action.name + '-performance.json', result);
                    break;
                }
                case 'capture': result = await capture(action.name); break;
                case 'native-capture': result = await captureNativePixels(action.name); break;
                case 'focus': await page.bringToFront(); await page.locator('#canvas').focus(); result = await state(); break;
                case 'key': await page.keyboard.press(action.key, { delay: action.holdMs || 100 }); result = await state(); break;
                case 'move': {
                    if (![action.x, action.y].every(value => Number.isFinite(value) && value >= 0 && value <= 1)) throw Error('Move requires normalized canvas coordinates');
                    const box = await page.locator('#canvas').boundingBox();
                    if (!box) throw Error('Input canvas is not visible');
                    // A click does not emit mousemove. Relative-input games need
                    // this actual motion event before their button event.
                    await page.mouse.move(box.x + action.x * box.width, box.y + action.y * box.height);
                    result = await state(); break;
                }
                case 'mouse-button': {
                    if (typeof action.down !== 'boolean') throw Error('Mouse button requires a Boolean down state');
                    if (action.down) await page.mouse.down(); else await page.mouse.up();
                    result = await state(); break;
                }
                case 'click': {
                    if (![action.x, action.y].every(value => Number.isFinite(value) && value >= 0 && value <= 1)) throw Error('Click requires normalized canvas coordinates');
                    const holdMs = action.holdMs ?? 100;
                    if (!Number.isInteger(holdMs) || holdMs < 0 || holdMs > 5000) throw Error('Invalid click hold duration');
                    await page.bringToFront();
                    const box = await page.locator('#canvas').boundingBox();
                    if (!box) throw Error('Input canvas is not visible');
                    await page.mouse.click(box.x + action.x * box.width, box.y + action.y * box.height, {delay: holdMs});
                    result = await state(); break;
                }
                case 'pointer-lock': {
                    // Locked clicks are delivered to the canvas, so release
                    // browser capture before interacting with the checkbox.
                    if (action.enabled === false) {
                        await page.evaluate(() => document.exitPointerLock());
                        await page.waitForFunction(() => !document.pointerLockElement);
                    }
                    await page.locator('#pointerLock').setChecked(action.enabled === true);
                    result = await state(); break;
                }
                case 'observe-exit': {
                    const limit = Date.now() + (action.timeoutSeconds || 180) * 1000;
                    while (Date.now() < limit) {
                        result = await state();
                        if (parseLifecycle(result.lifecycle).complete) {
                            record.applicationExitObserved = true;
                            record.cleanupObserved = true;
                            const started = performance.now();
                            record.cleanupObservationStartedAt = new Date().toISOString();
                            await delay(15000);
                            result = await state();
                            if (!parseLifecycle(result.lifecycle).complete) throw Error('Lifecycle evidence changed during cleanup observation');
                            record.cleanupObservationMilliseconds = performance.now() - started;
                            if (record.cleanupObservationMilliseconds < 15000) throw Error('Cleanup observation was too short');
                            record.cleanupWaitSatisfied = true;
                            break;
                        }
                        await delay(1000);
                    }
                    if (!record.cleanupWaitSatisfied) throw Error('Game cleanup did not complete');
                    break;
                }
                case 'close': await close('requested'); break;
                default: throw Error('Unknown capture action: ' + action.type);
            }
            if (!closing) {
                await jsonFile(`action-${index}.json`, { action, result });
                console.log(JSON.stringify({ index, action, result }));
            }
        } catch (error) {
            record.errors.push(String(error));
            console.log(JSON.stringify({ action, error: String(error) }));
        }
    }
    if (!closing) await close('stdin closed');
} catch (error) {
    record.errors.push(String(error));
    await close('startup error');
}
