// Exercise the shipped producer and worklet against a deterministic audio clock.
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import test from 'node:test';
import vm from 'node:vm';

const source = await readFile(new URL('./boxedwine-audio-worklet.js', import.meta.url), 'utf8');

function fixture({channels = 2, capacity = 512, moduleFailure = false, rate = 48000, outputRate = 48000, primeFrames = 0} = {}) {
    let library, Processor, now = 0;
    const errors = [], listeners = new Map();
    const context = vm.createContext({
        addToLibrary(value) { library = value; },
        SharedArrayBuffer, Int32Array, Float32Array, Atomics, Map, Math, Array,
        URL: Object.assign(class extends URL {}, {createObjectURL: () => 'blob:test', revokeObjectURL() {}}),
        URLSearchParams, Blob, Module: {}, HEAPF32: new Float32Array(4096),
        performance: {now: () => now},
        navigator: {userActivation: {hasBeenActive: false}},
        crossOriginIsolated: true,
        window: {location: {search: ''},
            addEventListener(name, callback) { listeners.set(name, callback); },
            removeEventListener(name) { listeners.delete(name); }},
        setInterval() { return 1; }, clearInterval() {},
        err(message) { errors.push(message); },
        AudioContext: class {
            sampleRate = 48000;
            state = 'suspended';
            destination = {};
            audioWorklet = {addModule: () => moduleFailure ? Promise.reject(Error('blocked module')) : Promise.resolve()};
            resume() { this.state = 'running'; return Promise.resolve(); }
            close() { this.state = 'closed'; return Promise.resolve(); }
        },
        AudioWorkletNode: class { connect() {} disconnect() { this.disconnected = true; } }
    });
    vm.runInContext(source, context);
    context.BoxedwineAudio = library.$BoxedwineAudio;
    const audio = context.BoxedwineAudio;
    vm.runInNewContext('(' + audio.processorSource.toString() + ')()', {
        AudioWorkletProcessor: class {}, Int32Array, Float32Array, Atomics, Math, sampleRate: outputRate,
        registerProcessor(name, value) { assert.equal(name, 'boxedwine-pcm'); Processor = value; }
    });
    const control = new Int32Array(new SharedArrayBuffer(40));
    const pcm = new Float32Array(new SharedArrayBuffer(capacity * channels * 4));
    const device = {id: 1, channels, capacity, rate, control, pcm, node: {disconnect() {}}, closing: false,
        lastDrain: 0, drainFraction: 0, queueFailures: 0, targetFrames: Math.floor(rate * 0.03)};
    audio.devices.set(1, device);
    audio.context = {state: 'running', sampleRate: 48000};
    const processor = new Processor({processorOptions: {control: control.buffer, pcm: pcm.buffer, channels, rate, primeFrames}});
    function render(frames = 128) {
        const output = Array.from({length: channels}, () => new Float32Array(frames).fill(99));
        const running = processor.process([], [output]);
        return {output, running};
    }
    function queue(values) {
        context.HEAPF32.set(values);
        return audio.queue(1, 0, values.length / channels);
    }
    return {audio, context, device, control, processor, render, queue, errors, listeners, library,
        advance(ms) { now += ms; audio.tick(); }};
}

test('stereo samples retain channel order across physical ring wrap', () => {
    const f = fixture({capacity: 256});
    f.control[0] = f.control[1] = 220;
    const samples = Array.from({length: 200}, (_, i) => (i % 2 ? -1 : 1) * (i + 1) / 256);
    assert.equal(f.queue(samples), 100);
    const {output} = f.render();
    for (let i = 0; i < 100; ++i) {
        assert.equal(output[0][i], samples[2 * i]);
        assert.equal(output[1][i], samples[2 * i + 1]);
    }
    assert(output.every(channel => channel.slice(100).every(x => x === 0)));
    assert.equal(f.audio.queued(f.device), 0);
    assert.equal(f.control[0], 320);
});

test('counter rollover preserves FIFO ordering and real consumption', () => {
    const f = fixture({channels: 1});
    f.control[0] = f.control[1] = -64;
    f.queue(Array.from({length: 256}, (_, i) => i));
    assert.equal(f.audio.queued(f.device), 256);
    assert.deepEqual(Array.from(f.render().output[0]), Array.from({length: 128}, (_, i) => i));
    assert.equal(f.audio.queued(f.device), 128);
    assert.deepEqual(Array.from(f.render().output[0]), Array.from({length: 128}, (_, i) => i + 128));
    assert.equal(f.audio.queued(f.device), 0);
    assert.equal(f.control[0], 192);
});

test('unaligned guest float writes preserve samples and channel order', () => {
    const f = fixture();
    const data = new DataView(f.context.HEAPF32.buffer);
    const samples = [0.25, -0.5, 0.75, -1];
    samples.forEach((value, index) => data.setFloat32(3 + index * 4, value, true));
    assert.equal(f.audio.queue(1, 3, 2), 2);
    const {output} = f.render();
    assert.deepEqual(Array.from(output[0].slice(0, 2)), [0.25, 0.75]);
    assert.deepEqual(Array.from(output[1].slice(0, 2)), [-0.5, -1]);
});

test('a full ring rejects the entire write without overwriting unplayed audio', () => {
    const f = fixture({channels: 1, capacity: 128});
    assert.equal(f.queue(Array(128).fill(0.5)), 128);
    assert.equal(f.queue([0.75]), 0);
    assert.equal(f.device.queueFailures, 1);
    assert(f.render().output[0].every(x => x === 0.5));
});

test('one pthread status query distinguishes queued, empty, and failed output', () => {
    const f = fixture({channels: 1});
    f.queue(Array(256).fill(0.5));
    let status = f.library.bw_audio_status(1);
    assert.equal(status, 256);
    f.render();
    status = f.library.bw_audio_status(1);
    assert.equal(status, 128);
    f.render();
    assert.equal(f.library.bw_audio_status(1), 0);
    f.audio.failed = true;
    assert.equal(f.library.bw_audio_status(1), -1);
    f.audio.failed = false;
    f.audio.close(1, false);
    assert.equal(f.library.bw_audio_status(1), -1);
});

test('underruns output silence without consuming future frames or replaying old samples', () => {
    const f = fixture({channels: 1});
    f.queue([0.25, 0.5]);
    f.render();
    const consumed = f.control[0];
    assert(f.render().output[0].every(x => x === 0));
    assert.equal(f.control[0], consumed);
    assert.equal(f.control[4], 1);
    f.queue(Array(128).fill(0.75));
    assert(f.render().output[0].every(x => x === 0.75));
    f.render();
    assert.equal(f.control[4], 2);
});

test('render quantum is taken from the output buffer, not hardcoded to 128', () => {
    const f = fixture({channels: 1});
    f.queue(Array(512).fill(0.5));
    f.render(256);
    assert.equal(f.control[6], 256);
    assert.equal(f.audio.queued(f.device), 256);
});

test('producer reacquires the Wasm heap after memory growth', () => {
    const f = fixture({channels: 1});
    f.queue([0.25]);
    f.context.HEAPF32 = new Float32Array([0.75]);
    assert.equal(f.audio.queue(1, 0, 1), 1);
    assert.deepEqual(Array.from(f.render().output[0].slice(0, 2)), [0.25, 0.75]);
});

test('running queue follows the consumer, not elapsed wall time', () => {
    const f = fixture({channels: 1});
    f.queue(Array(256).fill(0.25));
    f.advance(1000);
    assert.equal(f.audio.queued(f.device), 256);
    f.render();
    assert.equal(f.audio.queued(f.device), 128);
});

test('blocked autoplay drains silently and stops emulating consumption on resume', () => {
    const f = fixture({channels: 1});
    f.audio.context.state = 'suspended';
    f.queue(Array(512).fill(0.25));
    f.advance(5);
    assert.equal(f.audio.queued(f.device), 272);
    f.audio.context.state = 'running';
    f.advance(100);
    assert.equal(f.audio.queued(f.device), 272);
    f.render();
    assert.equal(f.audio.queued(f.device), 144);
});

test('draining close keeps the tail alive and then releases the node', () => {
    const f = fixture({channels: 1});
    f.device.node = {disconnect() { this.disconnected = true; }};
    f.queue([0.5]);
    f.audio.close(1, true);
    assert(f.audio.devices.has(1));
    assert.equal(f.queue([0.75]), 0);
    assert.equal(f.render().output[0][0], 0.5);
    f.advance(10);
    assert.equal(f.audio.devices.size, 0);
    assert.equal(f.device.node.disconnected, true);
    assert.equal(f.render().running, false);
});

test('unsupported or explicitly selected SDL bypasses worklet initialization', () => {
    const f = fixture();
    f.context.window.location.search = '?audioBackend=sdl';
    assert.equal(f.audio.available(), false);
    f.context.window.location.search = '';
    f.context.crossOriginIsolated = false;
    assert.equal(f.audio.available(), false);
    assert.equal(f.audio.open(2), 0);
});

test('processor failure releases backpressure so a blocked writer can select SDL', () => {
    const f = fixture({channels: 1, capacity: 128});
    f.queue(Array(128).fill(0.5));
    assert.equal(f.library.bw_audio_queued(1), 128);
    f.audio.failed = true;
    assert.equal(f.library.bw_audio_queued(1), 0);
    assert.equal(f.audio.available(), false);
});

test('asynchronous module failure selects fallback and shutdown removes listeners', async () => {
    const f = fixture({moduleFailure: true});
    f.audio.devices.clear();
    f.audio.context = null;
    const id = f.audio.open(2, 48000, 1440);
    assert(id);
    await f.audio.loading;
    assert.equal(f.audio.available(), false);
    assert(f.errors.some(message => message.includes('blocked module')));
    assert.equal(f.listeners.size, 4);
    f.audio.shutdown();
    assert.equal(f.listeners.size, 0);
    assert.equal(f.audio.devices.size, 0);
});

test('11025 Hz resampling starts with a small packet and preserves pitch/amplitude', () => {
    const f = fixture({channels: 1, capacity: 4096, rate: 11025});
    const input = Array.from({length: 2048}, (_, i) => 0.5 * Math.sin(2 * Math.PI * 1000 * i / 11025));
    f.queue(input.slice(0, 128));
    const first = f.render().output[0];
    assert(first.some(x => Math.abs(x) > 0.4), 'A small packet must reach output without 512-frame staging');
    assert.equal(f.control[0], 29);
    f.queue(input.slice(128));
    const output = Array.from(first);
    for (let i = 0; i < 40; ++i) output.push(...f.render().output[0]);
    let error = 0;
    for (let i = 128; i < output.length; ++i) {
        error += (output[i] - 0.5 * Math.sin(2 * Math.PI * 1000 * i / 48000)) ** 2;
    }
    assert(Math.sqrt(error / (output.length - 128)) < 0.001);
    assert.equal(f.control[4], 0, 'Well-fed resampling must not starve');
});

test('resampling is continuous across callback sizes, ring wrap, and counter rollover', () => {
    const a = fixture({channels: 1, capacity: 4096, rate: 11025});
    const b = fixture({channels: 1, capacity: 4096, rate: 11025});
    b.control[0] = b.control[1] = -100;
    const input = Array.from({length: 2048}, (_, i) => Math.sin(i * 0.4));
    a.queue(input);
    b.queue(input);
    const actual = [], expected = [];
    for (let i = 0; i < 20; ++i) actual.push(...a.render(256).output[0]);
    for (let i = 0; i < 40; ++i) expected.push(...b.render(128).output[0]);
    assert.deepEqual(actual, expected);
    assert.equal(a.audio.queued(a.device), b.audio.queued(b.device));
});

test('downsampling filters frequencies above the output Nyquist frequency', () => {
    function rms(frequency) {
        const f = fixture({channels: 1, capacity: 4096, rate: 48000, outputRate: 16000});
        f.queue(Array.from({length: 3072}, (_, i) => Math.sin(2 * Math.PI * frequency * i / 48000)));
        const output = [];
        for (let i = 0; i < 7; ++i) output.push(...f.render().output[0]);
        return Math.sqrt(output.slice(128).reduce((sum, x) => sum + x * x, 0) / (output.length - 128));
    }
    assert(rms(1000) > 0.7);
    assert(rms(12000) < 0.01);
});

test('draining resampling emits the filter tail before releasing the node', () => {
    const f = fixture({channels: 1, rate: 11025});
    f.device.node = {disconnect() {}};
    f.queue([1]);
    f.audio.close(1, true);
    const rendered = f.render();
    assert(rendered.output[0].some(x => x !== 0));
    assert.equal(f.control[0], 1);
    assert.equal(f.control[9], 1);
    assert.equal(rendered.running, false);
    f.advance(10);
    assert.equal(f.audio.devices.size, 0);
});

test('a final short packet is consumed without waiting for the guest to close', () => {
    const f = fixture({channels: 1, rate: 11025});
    f.queue([1, 0, 0]);
    let nonzero = 0;
    for (let i = 0; i < 20; ++i) {
        const rendered = f.render();
        assert.equal(rendered.running, true);
        nonzero += rendered.output[0].filter(x => x !== 0).length;
    }
    assert(nonzero > 0);
    assert.equal(f.audio.queued(f.device), 0, 'WaveOut must be able to observe its final frames consumed');
    assert.equal(f.control[0], 3);
    assert.equal(f.control[9], 0, 'An idle stream remains open for the next sound');
    f.queue([0.5]);
    for (let i = 0; i < 20; ++i) f.render();
    assert.equal(f.control[0], 4);
    assert.equal(f.processor.tail, 0);
});

test('startup collects real PCM and short packets cannot wait forever', () => {
    const f = fixture({channels: 1, primeFrames: 960});
    f.queue([0.5]);
    for (let i = 0; i < 7; ++i) assert(f.render().output[0].every(x => x === 0));
    assert.equal(f.render().output[0][0], 0.5);
    assert.equal(f.control[0], 1);
    // Recovery must also time out instead of restarting its timer each quantum.
    f.queue([0.75]);
    for (let i = 0; i < 7; ++i) assert(f.render().output[0].every(x => x === 0));
    assert.equal(f.render().output[0][0], 0.75);
});

test('repeated starvation cannot change the published queue limit', () => {
    const f = fixture({channels: 1, rate: 11025});
    const initial = f.audio.snapshot().devices[0].targetQueueMs;
    for (let i = 1; i <= 6; ++i) {
        f.control[5] = i * 48000;
        f.control[1] = i * 11000;
        f.control[0] = f.control[1] - 220;
        f.control[3] = i * 1000;
        f.control[4] = i * 4;
        f.advance(1000);
        assert.equal(f.audio.snapshot().devices[0].targetQueueMs, initial);
        assert.equal(f.audio.queued(f.device), 220, 'Underrun telemetry must not consume queued PCM');
    }
});

test('initialization and reopening use the capacity negotiated by C++', async () => {
    const f = fixture();
    f.audio.devices.clear();
    f.audio.context = null;
    const first = f.library.bw_audio_open(1, 11025, 330);
    await f.audio.loading;
    assert.equal(f.audio.snapshot().devices[0].targetQueueMs, 330 * 1000 / 11025);
    f.audio.close(first, false);
    const second = f.library.bw_audio_open(2, 22050, 661);
    await f.audio.loading;
    assert.equal(f.audio.snapshot().devices[0].targetQueueMs, 661 * 1000 / 22050);
    assert.equal(f.library.bw_audio_status(second), 0);
});
