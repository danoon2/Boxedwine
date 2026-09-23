// Real browser input and delayed Win32 cursor queries. This does not certify
// GDI presentation, relative input, or complete game compatibility.
import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import {createHash} from 'node:crypto';
import {readFile, writeFile} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {dirname, join, resolve} from 'node:path';
import {createInterface} from 'node:readline';
import {fileURLToPath, pathToFileURL} from 'node:url';

export function checkCursorTrace(text) {
    assert(text.includes('INPUT_READY mode=GL'), 'Missing GL ready marker');
    assert(!text.includes('INPUT_ERROR'), 'Probe reported an API error');
    const rows = [];
    for (const line of text.slice(text.indexOf('INPUT_READY mode=GL')).split('\n')) {
        const match = /^INPUT_(MOVE|DOWN|UP|POLL) generation=(\d+) mode=(GL|GDI) ok=(\d+),(\d+),(\d+) error=(\d+) message=(-?\d+),(-?\d+) screen=(-?\d+),(-?\d+) client=(-?\d+),(-?\d+) origin=(-?\d+),(-?\d+) buttons=(\d+),(\d+) capture=(\d+)$/.exec(line.trim());
        if (!match) {
            assert(!/^INPUT_(MOVE|DOWN|UP|POLL) /.test(line), 'Malformed cursor record');
            continue;
        }
        const [, kind, generation, mode, ...values] = match;
        const [get, clientOk, originOk, error, mx, my, sx, sy, cx, cy, ox, oy, asyncButton, button, capture] = values.map(Number);
        rows.push({kind, generation: Number(generation), mode, get, clientOk, originOk, error,
            mx, my, sx, sy, cx, cy, ox, oy, asyncButton, button, capture});
    }
    const down = rows.findIndex(row => row.kind === 'DOWN' && row.mode === 'GL');
    assert(down >= 0, 'Missing real mouse down');
    const tested = rows.slice(down).filter(row => row.mode === 'GL');
    assert(tested.some(row => row.kind === 'UP'), 'Missing mouse release');
    const held = tested.filter(row => row.kind === 'POLL' && row.capture === 1);
    const released = tested.filter(row => row.kind === 'POLL' && row.capture === 0);
    assert(held.length && released.length, 'Missing delayed held/released queries');
    for (const row of tested) {
        assert(row.get && row.clientOk && row.originOk, 'Cursor query returned false: ' + JSON.stringify(row));
        assert.equal(row.sx - row.ox, row.cx, 'Screen/client X mismatch');
        assert.equal(row.sy - row.oy, row.cy, 'Screen/client Y mismatch');
        // SDL truncates scaled browser coordinates, whereas the custom message
        // path rounds them. Retain and report this one-pixel discrepancy.
        assert(Math.abs(row.cx - row.mx) <= 1 && Math.abs(row.cy - row.my) <= 1,
            'Polled cursor diverges from mouse event: ' + JSON.stringify(row));
    }
    assert(held.every(row => row.asyncButton === 1 && row.button === 1), 'Held button state lost');
    assert(released.every(row => row.asyncButton === 0 && row.button === 0), 'Released button state stuck');
    return {passed: true, rows: tested.length, heldPolls: held.length, releasedPolls: released.length,
        maxMessageDelta: Math.max(...tested.flatMap(row => [Math.abs(row.cx - row.mx), Math.abs(row.cy - row.my)])),
        click: [tested[0].mx, tested[0].my]};
}

async function checkInputs(config) {
    for (const item of config.inputs) {
        const data = await readFile(item.path);
        assert.equal(data.length, item.bytes, 'Changed input size: ' + item.path);
        assert.equal(createHash('sha256').update(data).digest('hex'), item.sha256, 'Changed input: ' + item.path);
    }
}

export function checkLockedCursor(text, expected, held) {
    assert(!text.includes('INPUT_ERROR'), 'Probe reported an API error');
    const rows = text.split('\n').filter(line => line.startsWith('INPUT_POLL ')).slice(-2);
    assert.equal(rows.length, 2, 'Missing two delayed locked cursor polls');
    for (const row of rows) {
        assert(row.includes('ok=1,1,1 error=0'), 'Locked cursor query failed');
        const message = row.match(/ message=(-?\d+),(-?\d+) /);
        const client = row.match(/ client=(-?\d+),(-?\d+) /);
        assert(message && client, 'Malformed locked cursor coordinates');
        for (let axis = 0; axis < 2; ++axis) {
            assert(Math.abs(Number(client[axis + 1]) - expected[axis]) <= 1, 'Locked cursor did not follow relative motion: ' + row);
            assert(Math.abs(Number(message[axis + 1]) - Number(client[axis + 1])) <= 1, 'Locked event/query mismatch: ' + row);
        }
        assert(row.endsWith(`buttons=${held ? '1,1' : '0,0'} capture=${held ? 1 : 0}`), 'Locked button state mismatch: ' + row);
    }
    return {passed: true, expected, held, polls: rows};
}

export function checkWarpTrace(text) {
    const start = text.lastIndexOf('INPUT_WARP client=320,240 ');
    assert(start >= 0, 'Missing guest warp');
    const rows = text.slice(start).split('\n').filter(line => line.startsWith('INPUT_WARP_QUERY '));
    assert(rows.length >= 2, 'Missing delayed guest warp queries');
    for (const line of rows) {
        const match = /^INPUT_WARP_QUERY elapsed=(\d+) ok=(\d+),(\d+) client=(-?\d+),(-?\d+)$/.exec(line);
        assert(match, 'Malformed warp query');
        const [elapsed, get, convert, x, y] = match.slice(1).map(Number);
        assert(elapsed >= 1000 && get === 1 && convert === 1, 'Invalid delayed warp query');
        assert(x === 320 && y === 240, 'Stale cursor after guest warp: ' + line);
    }
    return {passed: true, rows};
}

export async function run(configPath, gdiCycles = 0, pointerLock = false) {
    assert(Number.isInteger(gdiCycles) && gdiCycles >= 0 && gdiCycles <= 10, 'Invalid GDI cycle count');
    const config = JSON.parse(await readFile(configPath, 'utf8'));
    await checkInputs(config);
    const output = resolve(config.output);
    const result = {startedAt: new Date().toISOString(), passed: false, errors: [],
        scope: 'Absolute mouse events, delayed GetCursorPos and held/released button state in a GL client.'};
    const runnerSource = await readFile(fileURLToPath(import.meta.url));
    result.runner = {path: fileURLToPath(import.meta.url), bytes: runnerSource.length,
        sha256: createHash('sha256').update(runnerSource).digest('hex')};
    const child = spawn(process.execPath, [join(dirname(fileURLToPath(import.meta.url)), 'captureGame.mjs'), resolve(configPath)],
        {stdio: ['pipe', 'pipe', 'pipe'], windowsHide: true});
    const messages = [], waiters = [], transcript = [], stderr = [];
    let ended = false;
    const completion = new Promise(resolveExit => child.once('close', code => {
        ended = true; resolveExit(code);
        for (const waiter of waiters.splice(0)) waiter({error: 'Capture driver closed'});
    }));
    child.once('error', error => {result.errors.push(String(error));});
    child.stderr.on('data', data => stderr.push(data.toString()));
    createInterface({input: child.stdout}).on('line', line => {
        transcript.push(line);
        let value;
        try {value = JSON.parse(line);} catch {value = {error: 'Non-JSON driver output: ' + line};}
        if (waiters.length) waiters.shift()(value); else messages.push(value);
    });
    async function receive(ms = 30000) {
        if (messages.length) return messages.shift();
        if (ended) throw Error('Capture driver ended');
        return new Promise((yes, no) => {
            const waiter = value => {clearTimeout(timer); yes(value);};
            const timer = setTimeout(() => {waiters.splice(waiters.indexOf(waiter), 1); no(Error('Driver response timeout'));}, ms);
            waiters.push(waiter);
        });
    }
    async function action(type, values = {}, timeout = 30000) {
        const request = {type, ...values};
        child.stdin.write(JSON.stringify(request) + '\n');
        const reply = await receive(timeout);
        assert(!reply.error, reply.error);
        if (type === 'close') assert.equal(reply.closed, 'requested');
        else assert.deepEqual(reply.action, request);
        return reply.result;
    }
    let browser;
    try {
        assert.equal((await receive(60000)).ready, true);
        await writeFile(join(output, 'runCursorQueryProbe.mjs'), runnerSource, {flag: 'wx'});
        const port = Number((await readFile(join(output, 'chrome-profile/DevToolsActivePort'), 'utf8')).split('\n')[0]);
        const {chromium} = createRequire(import.meta.url)(config.playwrightModule);
        browser = await chromium.connectOverCDP('http://127.0.0.1:' + port);
        const pages = browser.contexts()[0].pages().filter(page => page.url().includes('app=CursorQueryFile.zip'));
        assert.equal(pages.length, 1, 'Expected exactly one owned cursor probe page');
        const page = pages[0];
        async function trace() {
            return page.evaluate(() => {
                const state = window.__readGameCapture();
                const path = state.lifecyclePath.replace(/boxedwine-game-lifecycle\.log$/, 'cursor-query.log');
                const fs = typeof FS !== 'undefined' ? FS : Module.FS;
                try {return {path, log: fs.readFile(path, {encoding: 'utf8'}), state};}
                catch {return {path, log: '', state};}
            });
        }
        const deadline = Date.now() + 600000;
        let ready = false;
        while (Date.now() < deadline) {
            const snapshot = await trace();
            if (snapshot.log.includes('INPUT_ERROR')) throw Error(snapshot.log);
            if (snapshot.log.includes('INPUT_READY mode=GL') && snapshot.state.frameCanvasSize?.join(',') === '640,480') {
                await writeFile(join(output, 'guest-ready.json'), JSON.stringify(snapshot, null, 2), {flag: 'wx'});
                ready = true; break;
            }
            await action('wait', {milliseconds: 2000});
        }
        assert(ready, 'Probe startup timed out');
        // Enter at a different location first. Jumping straight to the click
        // hides stale SDL coordinates left behind by the browser handlers.
        await action('move', {x: 0.02, y: 0.02});
        await action('move', {x: 470 / 640, y: 428 / 480});
        await action('click', {x: 470 / 640, y: 428 / 480, holdMs: 1800});
        await action('wait', {milliseconds: 2000});
        const snapshot = await trace();
        await writeFile(join(output, 'guest-clicked.json'), JSON.stringify(snapshot, null, 2), {flag: 'wx'});
        result.cursor = checkCursorTrace(snapshot.log);
        if (pointerLock) {
            async function pollLocked(label, check) {
                const deadline = Date.now() + 15000;
                let locked, checked, lastError, attempt = 0;
                do {
                    await action('wait', {milliseconds: 1000});
                    locked = await trace();
                    await writeFile(join(output, `locked-${label}-attempt-${attempt++}.json`), JSON.stringify(locked, null, 2), {flag: 'wx'});
                    assert.equal(locked.state.pointerLock, 'canvas', 'Pointer lock was lost');
                    try {checked = check(locked.log); break;}
                    catch (error) {lastError = error;}
                } while (Date.now() < deadline);
                await writeFile(join(output, 'locked-' + label + '.json'), JSON.stringify(locked, null, 2), {flag: 'wx'});
                if (!checked) throw lastError;
                return checked;
            }
            result.pointerLock = [];
            await action('pointer-lock', {enabled: true});
            // Acquire the real lock by clicking inside the GL image. Acquiring
            // it at the checkbox leaves fixed clientY outside the canvas and
            // would miss the old absolute-handler interception regression.
            await page.evaluate(() => document.exitPointerLock());
            await page.waitForFunction(() => !document.pointerLockElement);
            const lockBox = await page.locator('#canvas').boundingBox();
            await page.mouse.move(lockBox.x + 100, lockBox.y + 100);
            // Keep lock acquisition, button events and relative motion on one
            // Playwright mouse. Another connection has its own last position.
            result.pointerLockAcquisition = {box: lockBox,
                click: [lockBox.x + lockBox.width / 2, lockBox.y + lockBox.height / 2],
                at: new Date().toISOString()};
            await page.mouse.click(...result.pointerLockAcquisition.click, {delay: 100});
            await page.waitForFunction(() => document.pointerLockElement?.id === 'canvas');
            await action('key', {key: 'r'});
            result.warp = await pollLocked('warp', checkWarpTrace);
            await page.mouse.down();
            result.warpHeld = await pollLocked('warp-held', text => checkLockedCursor(text, [320, 240], true));
            await page.mouse.up();
            const box = await page.locator('#canvas').boundingBox();
            // SDL scales movement in CSS pixels to its guest-sized window.
            // A quarter of the displayed canvas is exactly (160,120) pixels;
            // allow only the retained fractional-coordinate rounding pixel.
            for (const [label, x, y, held, expected] of [
                ['move', .75, .25, false, [480, 120]],
                ['held', .625, .375, true, [400, 180]],
                ['released', .5, .5, false, [320, 240]],
            ]) {
                if (label === 'held') await page.mouse.down();
                if (label === 'released') await page.mouse.up();
                await page.mouse.move(box.x + x * box.width, box.y + y * box.height);
                result.pointerLock.push(await pollLocked(label, text => checkLockedCursor(text, expected, held)));
            }
            await action('pointer-lock', {enabled: false});
            await action('click', {x: 470 / 640, y: 428 / 480, holdMs: 1800});
            await action('wait', {milliseconds: 1500});
            const unlocked = await trace();
            await writeFile(join(output, 'unlocked.json'), JSON.stringify(unlocked, null, 2), {flag: 'wx'});
            assert.equal(unlocked.state.pointerLock, '');
            result.unlocked = checkLockedCursor(unlocked.log, [470, 428], false);
        }
        if (gdiCycles) {
            result.gdiFrames = [];
            for (let cycle = 0; cycle < gdiCycles; ++cycle) {
                for (const mode of ['GDI', 'GL']) {
                    await action('focus');
                    await action('key', {key: 'g', holdMs: 500});
                    const modeDeadline = Date.now() + 30000;
                    let snapshot;
                    do {
                        await action('wait', {milliseconds: 1000});
                        snapshot = await trace();
                        if (snapshot.log.match(/INPUT_READY mode=(GL|GDI)/g)?.at(-1) === 'INPUT_READY mode=' + mode
                                && snapshot.state.frameCanvasSize?.join(',') === (mode === 'GL' ? '640,480' : '1024,768')) break;
                    } while (Date.now() < modeDeadline);
                    const name = `transition-${cycle}-${mode.toLowerCase()}`;
                    await writeFile(join(output, 'guest-' + name + '.json'), JSON.stringify(snapshot, null, 2), {flag: 'wx'});
                    await action('native-capture', {name});
                    result.gdiFrames.push({mode, name, size: snapshot.state.frameCanvasSize});
                    assert.equal(snapshot.log.match(/INPUT_READY mode=(GL|GDI)/g)?.at(-1), 'INPUT_READY mode=' + mode);
                    assert.deepEqual(snapshot.state.frameCanvasSize, mode === 'GL' ? [640, 480] : [1024, 768], 'Presentation did not switch modes');
                    assert(!snapshot.log.includes('INPUT_ERROR'), 'Transition reported an API error');
                }
            }
            // These are compositor captures, not pixel acceptance. An external
            // audit must check expected clear/fill colors in the saved PNGs.
            result.gdiPixelsAudited = false;
        }
    } catch (error) {
        result.errors.push(String(error));
    } finally {
        if (!ended) {
            try {
                if (pointerLock) {
                    await action('mouse-button', {down: false});
                    await action('pointer-lock', {enabled: false});
                }
                await action('key', {key: 'Escape'});
                await action('observe-exit', {timeoutSeconds: 60}, 90000);
                result.exitObserved = true;
            } catch (error) {result.errors.push('Exit: ' + error);}
            try {await action('close');} catch (error) {result.errors.push('Close: ' + error);}
        }
        result.driverExit = await completion;
        // The owning capture driver closed its browser. Release this observer.
        if (browser?.isConnected()) await browser.close();
    }
    try {
        await checkInputs(config);
        const capture = JSON.parse(await readFile(join(output, 'capture.json'), 'utf8'));
        assert.equal(result.driverExit, 0);
        assert.deepEqual(capture.errors, []);
        assert(capture.applicationExitObserved && capture.cleanupObserved && capture.cleanupWaitSatisfied);
        assert(capture.cleanupObservationMilliseconds >= 15000);
        result.buildId = config.buildId;
        result.lifecycle = {applicationExitObserved: capture.applicationExitObserved, cleanupObserved: capture.cleanupObserved,
            cleanupObservationMilliseconds: capture.cleanupObservationMilliseconds};
    } catch (error) {result.errors.push('Final audit: ' + error);}
    result.passed = result.errors.length === 0;
    for (const [name, value] of [['cursor-driver.jsonl', transcript.join('\n') + '\n'], ['cursor-driver.stderr', stderr.join('')],
        ['cursor-result.json', JSON.stringify(result, null, 2) + '\n']]) {
        await writeFile(join(output, name), value, {flag: 'wx'});
    }
    console.log(JSON.stringify(result));
    return result;
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
    run(process.argv[2], Number(process.argv[3] || 0), process.argv[4] === '--pointer-lock').then(result => {process.exitCode = result.passed ? 0 : 1;}).catch(error => {console.error(error); process.exitCode = 1;});
}
