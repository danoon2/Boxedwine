import assert from 'node:assert/strict';
import {parseLifecycle, nativeClip} from '../gameCaptureChecks.mjs';

assert.equal(parseLifecycle('').complete, false);
assert.equal(parseLifecycle('command: echo BW_GAME_EXIT:0; echo BW_GAME_CLEANUP:0').complete, false);
assert.equal(parseLifecycle('BW_GAME_EXIT:0\n').complete, false);
assert.equal(parseLifecycle('BW_GAME_EXIT:0\r\nBW_GAME_CLEANUP:0\r\n').complete, true);
for (const text of ['BW_GAME_EXIT:5', 'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:1',
    'BW_GAME_CLEANUP:0', 'BW_GAME_CLEANUP:0\nBW_GAME_EXIT:0',
    'BW_GAME_EXIT:0\nBW_GAME_EXIT:0\nBW_GAME_CLEANUP:0',
    'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:0\nBW_GAME_CLEANUP:0',
    'BW_GAME_EXIT:0 trailing', 'BW_GAME_EXIT:-1', 'BW_GAME_EXIT:00']) assert.throws(() => parseLifecycle(text));

const sample = {devicePixelRatio: 1, viewport: {width: 1280, height: 960},
    frame: {x: 8, y: 32, width: 802, height: 602}, canvases: [
        {id: 'canvas', width: 800, height: 600, order: 0, visible: true, transformed: false,
            zIndex: 0, rect: {x: 9, y: 33, width: 800, height: 600}},
        {id: 'boxedwine-webgl-canvas-0', width: 640, height: 480, order: 1, visible: false,
            transformed: false, zIndex: 1, rect: {x: 9, y: 33, width: 800, height: 600}}]};
assert.equal(nativeClip(sample).topCanvas, 'canvas'); // A hidden GL overlay must not hide the GDI oracle.
const gl = structuredClone(sample);
gl.canvases[1].visible = true;
assert.throws(() => nativeClip(gl), /backing size/); // A stale or stretched overlay remains a failure.
gl.frame.width = 642; gl.frame.height = 482;
for (const canvas of gl.canvases) {canvas.rect.width = 640; canvas.rect.height = 480;}
assert.equal(nativeClip(gl).topCanvas, 'boxedwine-webgl-canvas-0');
assert.deepEqual(nativeClip(gl).clip, {x: 9, y: 33, width: 640, height: 480});
for (const change of [
    value => {value.devicePixelRatio = 2;},
    value => {value.canvases[0].rect.x += .5;},
    value => {value.canvases[0].transformed = true;},
    value => {value.viewport.height = 480;},
    value => {value.canvases[0].visible = false;},
    value => {value.canvases[0].id = 'unknown';},
    value => {value.frame.width = 700;},
]) {const value = structuredClone(sample); change(value); assert.throws(() => nativeClip(value));}
console.log('PASS: strict lifecycle markers and native compositor crop validation (no browser launched).');

import { readFile } from 'node:fs/promises';
import { runInNewContext } from 'node:vm';
const source = await readFile(new URL('../captureGame.mjs', import.meta.url), 'utf8');
const start = source.indexOf('const wrapper = ') + 'const wrapper = '.length;
const end = source.indexOf(';\nconst insertion', start);
assert(start > 0 && end > start);
const wrapper = runInNewContext(source.slice(start, end));
const input = ['-root', '/root/overlay/c3.zip', '-zip', 'root.zip', '-mount', 'app.zip', '/files', '-w', '/files',
    '/bin/wine', 'cmd', '/c', 'reg', 'add', 'HKCU\\Software\\Wine\\Direct3D',
    '/v', 'DirectDrawRenderer', '/d', 'gdi', '&&', "folder with spaces/game's.exe", '-arg=$NO_SUBSTITUTION'];
const context = { getEmulatorParams: () => [...input], window: {}, console: { log() {} } };
runInNewContext(wrapper.replace(/^<script>\n|\n<\/script>$/g, ''), context);
const actual = context.getEmulatorParams();
assert.deepEqual(actual.slice(0, 9), input.slice(0, 9));
assert.deepEqual(actual.slice(9, 11), ['/bin/sh', '-c']);
assert(actual[11].includes('BW_GAME_EXIT:$game_status'));
assert(actual[11].includes('BW_GAME_CLEANUP:$cleanup_status'));
const lifecycle = 'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:0\n';
context.TextDecoder = TextDecoder;
const frame = {};
context.document = { getElementById: () => ({value: 'game output', textContent: ''}),
    querySelector: selector => { assert.equal(selector, '.emscripten_border'); return frame; },
    querySelectorAll: () => [], pointerLockElement: null, hasFocus: () => true, visibilityState: 'visible' };
context.getComputedStyle = element => {
    assert.equal(element, frame);
    return {getPropertyValue: name => ({'--boxedwine-canvas-width': '640', '--boxedwine-canvas-height': '480'})[name]};
};
for (const root of ['/root', '/root/overlay/c3.zip', '/root/app/mw3.zip', '/root/']) {
    input[1] = root;
    context.getEmulatorParams();
    const expected = root.replace(/\/$/, '') + '/tmp/boxedwine-game-lifecycle.log';
    for (const useModule of [false, true]) {
        delete context.FS;
        delete context.Module;
        const fs = { readFile(path) { assert.equal(path, expected); return new TextEncoder().encode(lifecycle); } };
        if (useModule) context.Module = { FS: fs };
        else context.FS = fs;
        const snapshot = context.window.__readGameCapture();
        assert.equal(snapshot.lifecyclePath, expected);
        assert.equal(snapshot.lifecycle, lifecycle);
        assert.deepEqual(Array.from(snapshot.frameCanvasSize), [640, 480]);
    }
}
input[1] = '/root/overlay/c3.zip';
context.document.querySelector = () => null;
assert.equal(context.window.__readGameCapture().frameCanvasSize, null);
console.log('PASS: launch argument preservation and eight guest-root lifecycle cases.');
