// Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
// The PCM ring is separate from Wasm memory so the same sink also works in
// single-threaded builds. Only the producer runs on the browser main thread;
// the worklet never calls the emulator, allocates PCM blocks, or waits for it.
addToLibrary({
  $BoxedwineAudio: {
    devices: new Map(),
    nextId: 1,
    context: null,
    loading: null,
    failed: false,
    timer: null,
    resume: null,

    // Shared control words: read, write, stop, starvation frames, starvation
    // episodes, rendered frames, render quantum, maximum queued frames,
    // nonzero output samples, drained. Stop: 0=running, 1=abort, 2=drain.
    processorSource: function() {
      class BoxedwinePcmProcessor extends AudioWorkletProcessor {
        constructor(options) {
          super();
          const p = options.processorOptions;
          this.control = new Int32Array(p.control);
          this.pcm = new Float32Array(p.pcm);
          this.channels = p.channels;
          this.capacity = this.pcm.length / this.channels;
          this.ratio = p.rate / sampleRate;
          this.phase = 0;
          this.history = new Float32Array(32 * this.channels);
          this.historyIndex = 0;
          this.tail = 16;
          this.lastWrite = 0;
          this.idleFrames = 0;
          this.primeFrames = p.primeFrames || 0;
          this.primeWait = 0;
          this.primed = false;
          // A 32-tap windowed-sinc filter needs only 16 input frames of
          // lookahead (1.45 ms at 11025 Hz), with phase carried across blocks.
          this.filter = new Float32Array(1024 * 32);
          const cutoff = Math.min(1, 1 / this.ratio);
          for (let phase = 0; phase < 1024; ++phase) {
            let total = 0;
            for (let tap = 0; tap < 32; ++tap) {
              const x = tap - 15 - phase / 1024;
              const sinc = Math.abs(x) < 1e-9 ? cutoff : Math.sin(Math.PI * x * cutoff) / (Math.PI * x);
              const value = sinc * (0.5 + 0.5 * Math.cos(Math.PI * x / 16));
              this.filter[phase * 32 + tap] = value;
              total += value;
            }
            for (let tap = 0; tap < 32; ++tap) this.filter[phase * 32 + tap] /= total;
          }
          this.started = false;
          this.starved = false;
        }
        process(inputs, outputs) {
          const control = this.control;
          const stop = Atomics.load(control, 2);
          if (stop === 1) return false;
          const closing = stop === 2;
          const output = outputs[0];
          const frames = output[0].length;
          let read = Atomics.load(control, 0) >>> 0;
          const write = Atomics.load(control, 1) >>> 0;
          let available = (write - read) >>> 0;
          if (write !== this.lastWrite) {
            if (this.tail === 0) {
              this.phase = 0;
              this.tail = 16;
            }
            this.lastWrite = write;
            this.idleFrames = 0;
          } else {
            this.idleFrames += frames;
          }
          // WaveOut waits for its final frames before it closes the device.
          // After an idle period, zero-pad the filter's right edge so a short
          // stream can finish without requiring another write or explicit close.
          const draining = closing || (available <= 16 && this.idleFrames >= sampleRate * 0.02);
          let count = 0;
          let nonzero = 0;
          // Collect real PCM before first playback/recovery. Do not prepend
          // silence, and never wait indefinitely for a short packet to fill it.
          if (!this.primed && available) this.primeWait += frames;
          if (draining || available >= this.primeFrames || this.primeWait >= sampleRate * 0.02) this.primed = true;
          if (!this.primed) {
            // The output below is silence until there is a scheduling cushion.
          } else if (this.ratio === 1) {
            count = Math.min(frames, available);
            for (let c = 0; c < output.length; ++c) {
              for (let i = 0; i < count; ++i) {
                const value = this.pcm[((read + i) & (this.capacity - 1)) * this.channels + c];
                output[c][i] = value;
                if (value !== 0) ++nonzero;
              }
            }
            read = (read + count) >>> 0;
            available -= count;
            this.tail = 0;
          } else {
            while (count < frames && (available > 16 || (draining && (available || this.tail)))) {
              const filterOffset = Math.min(1023, Math.floor(this.phase * 1024)) * 32;
              for (let c = 0; c < output.length; ++c) {
                let value = 0;
                for (let tap = 0; tap < 32; ++tap) {
                  const offset = tap - 15;
                  const sample = offset < 0 ? this.history[((this.historyIndex + offset) & 31) * this.channels + c] :
                    offset < available ? this.pcm[((read + offset) & (this.capacity - 1)) * this.channels + c] : 0;
                  value += sample * this.filter[filterOffset + tap];
                }
                output[c][count] = value;
                if (value !== 0) ++nonzero;
              }
              ++count;
              this.phase += this.ratio;
              while (this.phase >= 1) {
                for (let c = 0; c < this.channels; ++c) {
                  this.history[this.historyIndex * this.channels + c] = available ?
                    this.pcm[(read & (this.capacity - 1)) * this.channels + c] : 0;
                }
                this.historyIndex = (this.historyIndex + 1) & 31;
                if (available) { read = (read + 1) >>> 0; --available; }
                else if (this.tail) --this.tail;
                this.phase -= 1;
              }
            }
          }
          for (const channel of output) channel.fill(0, count);
          // Publish only after the PCM copy; the producer may reuse these slots.
          Atomics.store(control, 0, read | 0);
          if (count) this.started = true;
          if (this.started && count < frames && !draining) {
            Atomics.add(control, 3, frames - count);
            if (!this.starved) Atomics.add(control, 4, 1);
            this.starved = true;
            if (this.primed) {
              this.primed = false;
              this.primeWait = 0;
            }
          } else if (count) {
            this.starved = false;
          }
          Atomics.add(control, 5, frames);
          Atomics.store(control, 6, frames);
          Atomics.add(control, 8, nonzero);
          const done = closing && available === 0 && this.tail === 0;
          if (done) Atomics.store(control, 9, 1);
          return !done;
        }
      }
      registerProcessor('boxedwine-pcm', BoxedwinePcmProcessor);
    },

    available: function() {
      return !BoxedwineAudio.failed && typeof window !== 'undefined' &&
        typeof AudioContext !== 'undefined' && typeof AudioWorkletNode !== 'undefined' &&
        typeof SharedArrayBuffer !== 'undefined' && globalThis.crossOriginIsolated === true &&
        new URLSearchParams(window.location.search).get('audioBackend') !== 'sdl';
    },

    initialize: function() {
      const audio = BoxedwineAudio;
      if (audio.context) return;
      const context = audio.context = new AudioContext({latencyHint: 'interactive'});
      Module['boxedwineAudio'] = audio;
      const url = URL.createObjectURL(new Blob([
        '(' + audio.processorSource.toString() + ')();'
      ], {type: 'text/javascript'}));
      audio.loading = context.audioWorklet.addModule(url).catch(function(error) {
        audio.failed = true;
        err('AudioWorklet unavailable; falling back to SDL audio: ' + error);
      }).finally(function() { URL.revokeObjectURL(url); });
      audio.resume = function() {
        if (context.state === 'suspended') context.resume().catch(function() {});
      };
      // Capture runs before the absolute input handlers consume these events.
      for (const event of ['pointerdown', 'mousedown', 'keydown', 'touchstart']) {
        window.addEventListener(event, audio.resume, true);
      }
      if (navigator.userActivation && navigator.userActivation.hasBeenActive) audio.resume();
      audio.timer = setInterval(function() { audio.tick(); }, 10);
    },

    open: function(channels, rate, targetFrames) {
      const audio = BoxedwineAudio;
      if (!audio.available() || channels < 1 || channels > 8 || rate <= 0 || !targetFrames) return 0;
      try {
        audio.initialize();
        // Physical space is not the latency target. Normal writes use the
        // fixed limit negotiated in C++ before Wine's first buffer query.
        let capacity = 1;
        while (capacity < rate / 2) capacity *= 2;
        const id = audio.nextId++;
        const device = {
          id: id, channels: channels, rate: rate, capacity: capacity, node: null,
          control: new Int32Array(new SharedArrayBuffer(40)),
          pcm: new Float32Array(new SharedArrayBuffer(capacity * channels * 4)),
          closing: false, lastDrain: performance.now(), drainFraction: 0,
          queueFailures: 0, targetFrames: targetFrames
        };
        audio.devices.set(id, device);
        audio.loading.then(function() {
          if (!audio.devices.has(id) || audio.failed) return;
          try {
            device.node = new AudioWorkletNode(audio.context, 'boxedwine-pcm', {
              numberOfInputs: 0, numberOfOutputs: 1, outputChannelCount: [channels],
              processorOptions: {control: device.control.buffer, pcm: device.pcm.buffer,
                channels: channels, rate: rate, primeFrames: Math.ceil(rate * 0.02)}
            });
            device.node.onprocessorerror = function() {
              audio.failed = true;
              err('AudioWorklet processor failed; falling back to SDL audio');
            };
            device.node.connect(audio.context.destination);
          } catch (error) {
            audio.failed = true;
            err('AudioWorklet node failed; falling back to SDL audio: ' + error);
          }
        });
        return id;
      } catch (error) {
        audio.failed = true;
        err('AudioWorklet initialization failed; falling back to SDL audio: ' + error);
        return 0;
      }
    },

    queued: function(device) {
      const read = Atomics.load(device.control, 0) >>> 0;
      const write = Atomics.load(device.control, 1) >>> 0;
      return (write - read) >>> 0;
    },

    queue: function(id, pointer, frames) {
      const device = BoxedwineAudio.devices.get(id);
      if (!device || device.closing || BoxedwineAudio.failed) return 0;
      const queued = BoxedwineAudio.queued(device);
      if (frames > device.capacity - queued) {
        ++device.queueFailures;
        return 0;
      }
      const write = Atomics.load(device.control, 1) >>> 0;
      // Read HEAPF32 afresh: memory growth can replace the Wasm memory view.
      let source = pointer >>> 2;
      // Guest writes can begin at unaligned x86 addresses. Format conversion
      // produces aligned storage, but float PCM may reach this path directly.
      const unaligned = pointer & 3 ? new DataView(HEAPF32.buffer) : null;
      for (let i = 0; i < frames; ++i) {
        const destination = ((write + i) & (device.capacity - 1)) * device.channels;
        for (let c = 0; c < device.channels; ++c) {
          device.pcm[destination + c] = unaligned ?
            unaligned.getFloat32((pointer >>> 0) + (i * device.channels + c) * 4, true) : HEAPF32[source++];
        }
      }
      Atomics.store(device.control, 1, (write + frames) | 0);
      Atomics.store(device.control, 7, Math.max(Atomics.load(device.control, 7), queued + frames));
      return frames;
    },

    dispose: function(device) {
      Atomics.store(device.control, 2, 1);
      if (device.node) device.node.disconnect();
      BoxedwineAudio.devices.delete(device.id);
    },

    close: function(id, drain) {
      const device = BoxedwineAudio.devices.get(id);
      if (!device) return;
      if (drain && !BoxedwineAudio.failed && (device.node || BoxedwineAudio.queued(device))) {
        device.closing = true;
        Atomics.store(device.control, 2, 2);
      }
      else BoxedwineAudio.dispose(device);
    },

    tick: function() {
      const audio = BoxedwineAudio;
      const now = performance.now();
      for (const device of audio.devices.values()) {
        // Match SDL's silent drain while autoplay is blocked or the worklet is
        // loading. Wine must make progress even without a running audio device.
        if (!device.node || audio.context.state !== 'running') {
          const elapsedFrames = (now - device.lastDrain) * device.rate / 1000 + device.drainFraction;
          const frames = Math.floor(elapsedFrames);
          device.drainFraction = elapsedFrames - frames;
          const read = Atomics.load(device.control, 0);
          const count = Math.min(frames, audio.queued(device));
          // Never move the reader backwards if audio resumed concurrently.
          Atomics.compareExchange(device.control, 0, read, (read + count) | 0);
        }
        device.lastDrain = now;
        if (device.closing && (Atomics.load(device.control, 9) ||
            ((!device.node || audio.context.state !== 'running') && !audio.queued(device)))) audio.dispose(device);
      }
    },

    snapshot: function() {
      const audio = BoxedwineAudio;
      const context = audio.context;
      return {
        backend: 'audio-worklet', failed: audio.failed, state: context && context.state,
        sampleRate: context && context.sampleRate,
        baseLatencyMs: context && context.baseLatency * 1000,
        outputLatencyMs: context && context.outputLatency * 1000,
        devices: Array.from(audio.devices.values(), function(device) {
          const c = device.control;
          return {id: device.id, channels: device.channels, inputRate: device.rate, ready: !!device.node,
            targetQueueMs: device.targetFrames * 1000 / device.rate,
            queuedFrames: audio.queued(device), queuedMs: audio.queued(device) * 1000 / device.rate,
            consumedFrames: Atomics.load(c, 0) >>> 0, writtenFrames: Atomics.load(c, 1) >>> 0,
            starvedFrames: Atomics.load(c, 3) >>> 0, starvationEpisodes: Atomics.load(c, 4) >>> 0,
            renderedFrames: Atomics.load(c, 5) >>> 0, quantumFrames: Atomics.load(c, 6),
            nonzeroSamples: Atomics.load(c, 8) >>> 0,
            maxQueuedMs: Atomics.load(c, 7) * 1000 / device.rate,
            queueFailures: device.queueFailures, closing: device.closing};
        })
      };
    },

    shutdown: function() {
      const audio = BoxedwineAudio;
      for (const device of audio.devices.values()) audio.dispose(device);
      if (audio.timer !== null) clearInterval(audio.timer);
      for (const event of ['pointerdown', 'mousedown', 'keydown', 'touchstart']) {
        window.removeEventListener(event, audio.resume, true);
      }
      if (audio.context) audio.context.close().catch(function() {});
      audio.context = audio.loading = audio.timer = audio.resume = null;
    }
  },

  bw_audio_available__deps: ['$BoxedwineAudio'],
  bw_audio_available__proxy: 'sync',
  bw_audio_available: () => BoxedwineAudio.available() ? 1 : 0,
  bw_audio_open__deps: ['$BoxedwineAudio'],
  bw_audio_open__proxy: 'sync',
  bw_audio_open: (channels, rate, targetFrames) => BoxedwineAudio.open(channels, rate, targetFrames),
  bw_audio_status__deps: ['$BoxedwineAudio'],
  bw_audio_status__proxy: 'sync',
  bw_audio_status: (id) => {
    if (BoxedwineAudio.failed) return -1;
    const device = BoxedwineAudio.devices.get(id);
    // Preserve one pthread round trip while distinguishing empty from failed.
    return device ? BoxedwineAudio.queued(device) : -1;
  },
  bw_audio_queued__deps: ['$BoxedwineAudio'],
  bw_audio_queued__proxy: 'sync',
  bw_audio_queued: (id) => {
    // A failed processor cannot consume a full ring. Wake the guest writer so
    // its next write can reopen on SDL instead of waiting forever for space.
    if (BoxedwineAudio.failed) return 0;
    const device = BoxedwineAudio.devices.get(id);
    return device ? BoxedwineAudio.queued(device) : 0;
  },
  bw_audio_queue__deps: ['$BoxedwineAudio'],
  bw_audio_queue__proxy: 'sync',
  bw_audio_queue: (id, pointer, frames) => BoxedwineAudio.queue(id, pointer, frames),
  bw_audio_close__deps: ['$BoxedwineAudio'],
  bw_audio_close__proxy: 'sync',
  bw_audio_close: (id, drain) => BoxedwineAudio.close(id, drain),
  bw_audio_shutdown__deps: ['$BoxedwineAudio'],
  bw_audio_shutdown__proxy: 'sync',
  bw_audio_shutdown: () => BoxedwineAudio.shutdown()
});
