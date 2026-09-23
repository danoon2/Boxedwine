import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';

const source = readFileSync(process.argv[2] || new URL('../../platform/sdl/knativeinputSDL.cpp', import.meta.url), 'utf8');
const start = source.indexOf('    var capturingMouse =');
const end = source.indexOf('    })());', start);
assert(start >= 0 && end > start);

function browser(threaded) {
    // Run the production handlers with only DOM geometry and the WASM boundary
    // stubbed. Test both preprocessor branches used by the browser builds.
    const code = source.slice(start, end).replace(
        /#if !defined\(BOXEDWINE_MULTI_THREADED\)([\s\S]*?)#endif/g,
        (_, body) => threaded ? '' : body);
    const windowHandlers = new Map();
    const documentHandlers = new Map();
    const events = [];
    const canvas = {
        width: 640, height: 480, focus() {}, addEventListener() {},
        getBoundingClientRect: () => ({left: 100, top: 50, right: 740, bottom: 530, width: 640, height: 480})
    };
    const document = {
        pointerLockElement: null,
        getElementById: () => null,
        addEventListener: (type, callback) => documentHandlers.set(type, callback)
    };
    const context = vm.createContext({canvas, document,
        window: {addEventListener: (type, callback) => windowHandlers.set(type, callback)},
        _boxedwineEmscriptenMouseButton: (down, button, x, y) => events.push({down, button, x, y}),
        _boxedwineEmscriptenMouseMove() {}, _boxedwineEmscriptenPointerLock() {},
        _boxedwineEmscriptenKey() {}
    });
    vm.runInContext(code, context);
    return {events, context,
        send(type, button = 0, clientX = 300, clientY = 250) {
            let stopped = false;
            windowHandlers.get(type)({button, clientX, clientY,
                preventDefault() {}, stopImmediatePropagation() {stopped = true;}});
            return stopped;
        },
        lock(enabled) {
            document.pointerLockElement = enabled ? canvas : null;
            documentHandlers.get('pointerlockchange')();
        }
    };
}

for (const threaded of [false, true]) {
    const mode = threaded ? 'MT' : 'ST';
    test(`${mode}: a held button produces no release during motion`, () => {
        const b = browser(threaded);
        assert.equal(b.send('mousedown'), true);
        for (let i = 0; i < 100; i++) b.send('mousemove', 0, 300 + i, 250);
        assert.deepEqual(b.events, [{down: 1, button: 0, x: 200, y: 200}]);
        b.send('mouseup');
        assert.deepEqual(b.events.at(-1), {down: 0, button: 0, x: 200, y: 200});
    });

    for (const first of [0, 2]) {
        test(`${mode}: both flippers release independently, ${first} first`, () => {
            const b = browser(threaded);
            b.send('mousedown', 0);
            b.send('mousedown', 2);
            b.send('mouseup', first);
            assert.equal(b.send('mousemove', 0, 900, 700), true, 'Remaining button keeps capture outside canvas');
            assert.equal(b.send('mouseup', 2 - first, 900, 700), true);
            assert.deepEqual(b.events.map(e => [e.down, e.button]), [[1, 0], [1, 1], [0, first / 2], [0, (2 - first) / 2]]);
            assert.deepEqual(b.events.at(-1), {down: 0, button: (2 - first) / 2, x: 639, y: 479});
            assert.equal(b.send('mouseup', 2 - first), false, 'Duplicate release is ignored');
        });
    }

    test(`${mode}: middle and side buttons retain their identities`, () => {
        const b = browser(threaded);
        for (const button of [1, 3, 4]) {
            b.send('mousedown', button);
            b.send('mouseup', button);
        }
        assert.deepEqual(b.events.map(e => [e.down, e.button]), [[1, 2], [0, 2], [1, 3], [0, 3], [1, 4], [0, 4]]);
        assert.equal(b.send('mousedown', 5), false);
        assert.equal(b.send('mousedown', 0, 90, 250), false);
        assert.equal(b.events.length, 6);
    });

    test(`${mode}: losing focus releases each captured button once`, () => {
        const b = browser(threaded);
        b.send('mousedown', 0);
        b.send('mousedown', 2);
        b.send('mousemove', 0, 400, 300);
        b.send('blur');
        b.send('blur');
        assert.deepEqual(b.events.slice(2), [
            {down: 0, button: 0, x: 300, y: 250},
            {down: 0, button: 1, x: 300, y: 250}
        ]);
        assert.equal(b.send('mouseup'), false);
    });

    test(`${mode}: pointer lock releases absolute capture and passes input to SDL`, () => {
        const b = browser(threaded);
        b.send('mousedown');
        b.lock(true);
        assert.deepEqual(b.events.map(e => e.down), [1, 0]);
        assert.equal(b.send('mousedown'), false);
        assert.equal(b.send('mousemove'), false);
        assert.equal(b.send('mouseup'), false);
        b.lock(false);
        b.send('mousedown');
        b.send('mouseup');
        assert.deepEqual(b.events.map(e => e.down), [1, 0, 1, 0]);
    });
}
