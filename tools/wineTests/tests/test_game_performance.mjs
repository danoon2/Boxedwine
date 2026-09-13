import {test} from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import {installGamePerformance, summarizeGamePerformance} from '../gamePerformance.mjs';

function browser() {
    let at = 0, tick;
    const calls = [], events = new Map();
    const canvas = {id: 'boxedwine-webgl-canvas-0', width: 640, height: 480,
        getClientRects: () => [{width: 640, height: 480}]};
    class Context {
        constructor(target = canvas) {this.canvas = target;}
        drawImage(...args) {if (args[0] === 'throw') throw Error('Native failure'); calls.push([this, args]); return 42;}
        putImageData(...args) {calls.push([this, args]);}
    }
    class Bitmap {constructor() {this.canvas = canvas;} transferFromImageBitmap(...args) {calls.push([this, args]);}}
    const originals = [Context.prototype.drawImage, Context.prototype.putImageData, Bitmap.prototype.transferFromImageBitmap];
    const document = {visibilityState: 'visible', hasFocus: () => true, getElementById: () => canvas,
        addEventListener: (name, fn) => events.set(name, fn), removeEventListener: name => events.delete(name)};
    const window = {CanvasRenderingContext2D: Context, ImageBitmapRenderingContext: Bitmap,
        Module: {HEAPU8: new Uint8Array(65536)},
        addEventListener: (name, fn) => events.set(name, fn), removeEventListener: name => events.delete(name)};
    const context = vm.createContext({window, document, performance: {now: () => at, timeOrigin: 12345},
        getComputedStyle: () => ({display: 'block', visibility: 'visible'}),
        setInterval: fn => {tick = fn; return 1;}, clearInterval: () => {tick = null;}});
    vm.runInContext('(' + installGamePerformance.toString() + ')()', context);
    const observer = window.__boxedwineGamePerformance;
    return {observer, Context, Bitmap, calls, canvas, document, window, context,
        events, originals, setTime: value => {at = value;}, tick: () => tick?.(),
        // Remove cross-realm prototypes, as Playwright's page.evaluate does.
        end: () => JSON.parse(JSON.stringify(observer.end()))};
}

function raw() {
    return {schemaVersion: 1, name: 'steady', startedAt: 1000, finishedAt: 7000,
        firstPresentationMilliseconds: {drawImage: 300}, hooksIntact: true, overflow: false,
        presentations: [1100, 1110, 1130, 1160, 1260].map(at => ({at, kind: 'drawImage', width: 640, height: 480})),
        memory: [{at: 1000, wasmCapacityBytes: 65536}, {at: 7000, wasmCapacityBytes: 131072}],
        visibility: [1000, 7000].map(at => ({at, visibility: 'visible', focused: true, canvasVisible: true}))};
}

test('observer records only completed presentation-canvas methods and preserves native behavior', () => {
    const b = browser(), ctx = new b.Context(), crop = new b.Context({id: '', width: 640, height: 480});
    const args = [{source: true}, 0, 0, 640, 480];
    b.setTime(12); assert.equal(ctx.drawImage(...args), 42);
    b.observer.begin('steady');
    b.setTime(20); crop.drawImage(...args);
    assert.throws(() => ctx.drawImage('throw'), /Native failure/);
    b.setTime(30); ctx.drawImage(...args);
    b.setTime(40); ctx.putImageData(...args);
    const bitmap = new b.Bitmap();
    b.setTime(45); bitmap.transferFromImageBitmap(null);
    b.setTime(50); bitmap.transferFromImageBitmap({bitmap: true});
    b.setTime(100); const result = b.end();
    assert.equal(b.calls[0][0], ctx);
    assert.deepEqual(b.calls[0][1], args);
    assert.deepEqual(result.presentations.map(row => [row.at, row.kind]),
        [[30, 'drawImage'], [40, 'putImageData'], [50, 'transferFromImageBitmap']]);
    assert.deepEqual(result.firstPresentationMilliseconds,
        {drawImage: 12, putImageData: 40, transferFromImageBitmap: 50});
    assert.equal(result.hooksIntact, true);
});

test('memory growth, focus loss and hooks changed by another observer are retained', () => {
    const b = browser();
    b.observer.begin('steady');
    b.setTime(1000); b.window.Module.HEAPU8 = new Uint8Array(131072); b.tick();
    b.document.visibilityState = 'hidden'; b.events.get('visibilitychange')();
    b.Context.prototype.drawImage = () => {};
    b.setTime(6000); const result = b.end();
    assert.deepEqual(result.memory.map(row => row.wasmCapacityBytes), [65536, 131072, 131072]);
    assert.equal(result.visibility[1].visibility, 'hidden');
    assert.equal(result.hooksIntact, false);
    const replacement = b.Context.prototype.drawImage;
    b.observer.dispose();
    assert.equal(b.Context.prototype.drawImage, replacement);
    assert.equal(b.Context.prototype.putImageData, b.originals[1]);
    assert.equal(b.Bitmap.prototype.transferFromImageBitmap, b.originals[2]);
    assert.equal(b.window.__boxedwineGamePerformance, undefined);
    assert.equal(b.events.size, 0);
});

test('phase lifecycle rejects overlapping and missing measurements', () => {
    const b = browser();
    assert.throws(() => b.observer.end(), /No active/);
    assert.throws(() => b.observer.begin('../output'), /Invalid/);
    b.observer.begin('first');
    assert.throws(() => b.observer.begin('second'), /already active/);
    b.setTime(100); b.end();
    b.observer.begin('second');
    b.setTime(200); assert.equal(b.end().presentations.length, 0);
    assert.throws(() => vm.runInContext('(' + installGamePerformance.toString() + ')()', b.context), /already installed/);
});

test('an ancestor-hidden canvas cannot pass the visible-output measurement gate', () => {
    const b = browser(), ctx = new b.Context();
    // A display:none ancestor leaves the canvas's own computed display as block.
    b.canvas.getClientRects = () => [];
    b.observer.begin('hidden');
    b.setTime(100); ctx.drawImage({});
    b.setTime(200); ctx.drawImage({});
    b.setTime(6000); const result = b.end();
    assert.equal(result.visibility[0].canvasVisible, false);
    assert.equal(summarizeGamePerformance(result, 6000).measurementValid, false);
});

test('bounded collection marks overflow instead of silently accepting truncated timing', () => {
    const b = browser(), ctx = new b.Context();
    b.observer.begin('long');
    for (let index = 0; index < 120001; index++) {b.setTime(index); ctx.drawImage({});}
    const result = b.end();
    assert.equal(result.presentations.length, 120000);
    assert.equal(result.overflow, true);
});

test('summary retains long trailing stalls and distinguishes submission and memory metrics', () => {
    const report = summarizeGamePerformance(raw(), 6000);
    assert.equal(report.measurementValid, true);
    assert.equal(report.performanceAcceptance, false);
    assert.deepEqual(report.intervalMilliseconds, {samples: 4, median: 20, p95: 100, p99: 100, max: 100, over50: 1, over100: 0});
    assert.equal(report.leadingSilenceMilliseconds, 100);
    assert.equal(report.trailingSilenceMilliseconds, 5740);
    assert.equal(report.submissionsPerSecond, 5 / 6);
    assert.deepEqual(report.wasmCapacity, {samples: 2, maxBytes: 131072});
});

test('blank, resized, backgrounded, incomplete and unavailable-memory samples cannot be valid', () => {
    for (const change of [
        data => {data.presentations = [];},
        data => {data.presentations[1].width = 800;},
        data => {data.visibility[0].focused = false;},
        data => {data.visibility = [];},
        data => {data.memory.forEach(row => row.wasmCapacityBytes = null);},
        data => {data.overflow = true;},
        data => {delete data.hooksIntact;}
    ]) {
        const data = raw(); change(data);
        assert.equal(summarizeGamePerformance(data, 6000).measurementValid, false);
    }
});

test('short observation, nonfinite times and out-of-order/foreign samples are rejected', () => {
    for (const change of [
        data => {data.finishedAt = 6999;},
        data => {data.finishedAt = NaN;},
        data => {data.presentations[1].at = 1001;},
        data => {data.presentations[0].at = 999;},
        data => {data.presentations[0].kind = 'requestAnimationFrame';},
        data => {data.memory[0].at = 7001;},
        data => {data.memory[0].wasmCapacityBytes = -1;}
    ]) {
        const data = raw(); change(data);
        assert.throws(() => summarizeGamePerformance(data, 6000), /Invalid|short/);
    }
});
