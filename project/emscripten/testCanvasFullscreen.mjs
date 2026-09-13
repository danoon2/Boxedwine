// Run with node --test testCanvasFullscreen.mjs. The browser regression also
// checks actual fullscreen layering; these controls cover API forwarding.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';

const shell = readFileSync(new URL('./boxedwine-shell.js', import.meta.url), 'utf8');
const begin = shell.indexOf('function setupCanvasFullscreen(');
const end = shell.indexOf('function toggleSound(', begin);
assert(begin >= 0 && end > begin);
const source = shell.slice(begin, end);

function install(canvas, target) {
    const context = vm.createContext({document:{getElementById:id => id === 'dropzone' ? target : null}});
    vm.runInContext(source, context);
    context.setupCanvasFullscreen(canvas);
}

test('fullscreen forwards options and promise to the common ancestor', async () => {
    const canvas = {requestFullscreen() {throw Error('canvas fullscreen hides its sibling');}};
    const options = {navigationUI:'hide'};
    const result = Promise.resolve('entered');
    const target = {contains:value => value === canvas,
        requestFullscreen(value) {assert.equal(this, target); assert.equal(value, options); return result;}};
    install(canvas, target);
    assert.equal(canvas.requestFullscreen(options), result);
    assert.equal(await result, 'entered');
});

test('request failure stays visible to the caller', async () => {
    const canvas = {requestFullscreen(){}};
    const target = {contains:() => true, requestFullscreen() {return Promise.reject(Error('denied'));}};
    install(canvas, target);
    await assert.rejects(canvas.requestFullscreen(), /denied/);
});

test('legacy methods retain their API and receiver', () => {
    for (const name of ['webkitRequestFullscreen', 'webkitRequestFullScreen']) {
        const canvas = {[name]() {throw Error('wrong target');}};
        const target = {contains:() => true, [name](option) {assert.equal(this,target); return option;}};
        install(canvas, target);
        assert.equal(canvas[name](17), 17);
    }
});

test('missing containers and unrelated canvases keep their original method', () => {
    for (const target of [null, {contains:() => false}, {contains:() => true}]) {
        const original = () => 'original';
        const canvas = {requestFullscreen:original};
        install(canvas, target);
        assert.equal(canvas.requestFullscreen, original);
    }
});

test('installing twice does not recurse or redirect pointer lock', () => {
    const pointerLock = () => 'pointer lock';
    let calls = 0;
    const canvas = {requestFullscreen(){}, requestPointerLock:pointerLock};
    const target = {contains:() => true, requestFullscreen() {calls++;}};
    install(canvas, target);
    install(canvas, target);
    canvas.requestFullscreen();
    assert.equal(calls, 1);
    assert.equal(canvas.requestPointerLock, pointerLock);
});
