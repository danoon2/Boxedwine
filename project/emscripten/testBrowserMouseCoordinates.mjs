import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';

// Exercise the production EM_ASM coordinate conversion with browser geometry.
const source = readFileSync(process.argv[2] || new URL('../../platform/sdl/knativeinputSDL.cpp', import.meta.url), 'utf8');
const start = source.indexOf('    function canvasPoint(');
const end = source.indexOf('    function boxedwineButton(', start);
assert(start >= 0 && end > start);
const code = source.slice(start, end);
const rect = {left: 185, top: 37, width: 880, height: 660, right: 1065, bottom: 697};
const input = {width: 800, height: 600};
const displayed = {width: 640, height: 480, getBoundingClientRect: () => rect};
const document = {pointerLockElement: null};
let relativeMode = null;
const context = vm.createContext({canvas: input, document, capturingMouse: false,
    _boxedwineEmscriptenPointerLock: enabled => {relativeMode = enabled;},
    activeCanvasRect: () => displayed});
vm.runInContext(code, context);
const point = (x, y, outside = false) => JSON.parse(JSON.stringify(context.canvasPoint(
    {clientX: rect.left + x * rect.width, clientY: rect.top + y * rect.height}, outside)));
assert.deepEqual(point(470 / 640, 428 / 480), {x: 470, y: 428});
assert.deepEqual(point(0, 0), {x: 0, y: 0});
assert.equal(point(-0.1, 0.5), null);
assert.equal(point(1, 0.5), null);
assert.deepEqual(point(1.2, -0.2, true), {x: 639, y: 0});

// Locked events have fixed client coordinates, even as movementX/Y change.
// Leave them to SDL, which maintains the virtual cursor and guest warps.
document.pointerLockElement = input;
context.capturingMouse = true;
assert.equal(point(0.5, 0.5), null, 'Locked motion must not become an absolute event');
assert.equal(point(0.5, 0.5, true), null, 'Locked button release must reach SDL');
assert.equal(context.capturingMouse, false, 'Discard obsolete absolute capture state');
assert.equal(relativeMode, 1, 'SDL relative mode follows browser pointer lock');
document.pointerLockElement = null;
assert.deepEqual(point(0.5, 0.5), {x: 320, y: 240}, 'Absolute input resumes after unlock');
assert.equal(relativeMode, 0, 'SDL relative mode ends after browser unlock');
displayed.width = 1024; displayed.height = 768;
assert.deepEqual(point(0.5, 0.5), {x: 512, y: 384});
displayed.width = 0;
assert.equal(point(0.5, 0.5), null);
console.log('PASS browser mouse coordinates: displayed GL/GDI pixels, CSS scaling, bounds and pointer capture.');
