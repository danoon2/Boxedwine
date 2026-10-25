/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "audiocapture.h"

#ifdef BOXEDWINE_LOCAL_SERVER
#include <map>

std::atomic<bool> AudioCapture::enabled(false);

// SDL_AudioFormat bits, repeated here so this file does not need SDL.
static const U32 FORMAT_BITSIZE_MASK = 0xFF;
static const U32 FORMAT_FLOAT = 0x100;
static const U32 FORMAT_BIG_ENDIAN = 0x1000;
static const U32 FORMAT_SIGNED = 0x8000;

// A voice keeps at most this much converted audio waiting for the mixer.
static const U32 MAX_QUEUED_FRAMES = AudioCapture::RATE;
// Chunks a subscriber may fall behind by before the oldest are dropped (about a second at 20 ms).
static const size_t MAX_SUBSCRIBER_CHUNKS = 50;
static const U32 MIX_INTERVAL_MS = 20;

namespace {
    struct Voice {
        U32 format = 0;
        U32 freq = 0;
        U32 channels = 0;
        // Decoded stereo frames not yet resampled. The first one is the frame
        // at floor(pos), kept so interpolation spans writes.
        std::vector<float> src;
        double pos = 0;
        std::deque<float> queue;
        bool closing = false;
    };

    std::mutex mutex;
    std::map<U32, Voice> voices;
    std::vector<std::shared_ptr<AudioCapture::Subscriber>> subscribers;
    U32 peak = 0;
    U64 framesMixed = 0;
    U64 soundFrames = 0;
    bool heardSound = false;
    U32 lastSoundTime = 0;

    // Never destroyed: exit() (kpanic does it under -novideo) runs static
    // destructors, and destroying a joinable std::thread calls std::terminate.
    std::thread& mixerThread = *new std::thread();
    std::mutex mixerMutex;
    std::condition_variable mixerCond;
    bool mixerRunning = false;

    float decodeSample(const U8* p, U32 format) {
        U32 bits = format & FORMAT_BITSIZE_MASK;
        bool bigEndian = (format & FORMAT_BIG_ENDIAN) != 0;
        bool isSigned = (format & FORMAT_SIGNED) != 0;
        if (bits == 8) {
            return isSigned ? (S8)p[0] / 128.0f : ((S32)p[0] - 128) / 128.0f;
        }
        if (bits == 16) {
            U32 v = bigEndian ? ((U32)p[0] << 8) | p[1] : ((U32)p[1] << 8) | p[0];
            return isSigned ? (S16)v / 32768.0f : ((S32)v - 32768) / 32768.0f;
        }
        if (bits == 32) {
            U32 v = bigEndian ? ((U32)p[0] << 24) | ((U32)p[1] << 16) | ((U32)p[2] << 8) | p[3]
                : ((U32)p[3] << 24) | ((U32)p[2] << 16) | ((U32)p[1] << 8) | p[0];
            if (format & FORMAT_FLOAT) {
                float f;
                memcpy(&f, &v, 4);
                return f;
            }
            return (float)((S32)v / 2147483648.0);
        }
        return 0.0f;
    }

    void stopMixerThread() {
        {
            std::lock_guard<std::mutex> lock(mixerMutex);
            mixerRunning = false;
        }
        mixerCond.notify_all();
        if (mixerThread.joinable()) {
            mixerThread.join();
        }
    }
}

bool AudioCapture::Subscriber::pop(std::vector<S16>& chunk, U32 timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() {
        return stopped || !chunks.empty();
    });
    if (chunks.empty()) {
        return false;
    }
    chunk.swap(chunks.front());
    chunks.pop_front();
    return true;
}

void AudioCapture::write(U32 voiceId, U32 format, U32 freq, U32 channels, const U8* data, U32 len) {
    if (!enabled || !channels || !freq) {
        return;
    }
    U32 bytesPerSample = (format & FORMAT_BITSIZE_MASK) / 8;
    if (bytesPerSample != 1 && bytesPerSample != 2 && bytesPerSample != 4) {
        return;
    }
    U32 frameSize = bytesPerSample * channels;
    U32 frames = len / frameSize;

    std::lock_guard<std::mutex> lock(mutex);
    Voice& voice = voices[voiceId];
    if (voice.format != format || voice.freq != freq || voice.channels != channels) {
        voice = Voice();
        voice.format = format;
        voice.freq = freq;
        voice.channels = channels;
    }
    voice.closing = false;
    for (U32 i = 0; i < frames; i++) {
        const U8* frame = data + (size_t)i * frameSize;
        float left = decodeSample(frame, format);
        float right = channels > 1 ? decodeSample(frame + bytesPerSample, format) : left;
        voice.src.push_back(left);
        voice.src.push_back(right);
    }
    // Linear resampling to RATE.
    double step = (double)freq / RATE;
    size_t srcFrames = voice.src.size() / 2;
    while (voice.pos + 1 < srcFrames) {
        size_t index = (size_t)voice.pos;
        float frac = (float)(voice.pos - index);
        for (U32 c = 0; c < 2; c++) {
            float a = voice.src[index * 2 + c];
            float b = voice.src[index * 2 + 2 + c];
            voice.queue.push_back(a + (b - a) * frac);
        }
        voice.pos += step;
    }
    size_t consumed = std::min((size_t)voice.pos, srcFrames);
    voice.src.erase(voice.src.begin(), voice.src.begin() + consumed * 2);
    voice.pos -= consumed;
    if (voice.queue.size() > MAX_QUEUED_FRAMES * 2) {
        voice.queue.erase(voice.queue.begin(), voice.queue.begin() + (voice.queue.size() - MAX_QUEUED_FRAMES * 2));
    }
}

void AudioCapture::closeVoice(U32 voiceId) {
    if (!enabled) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    auto it = voices.find(voiceId);
    if (it != voices.end()) {
        // Keep it until the mixer has played what is queued, as the device does.
        it->second.closing = true;
    }
}

void AudioCapture::mix(U32 frames, std::vector<S16>& out) {
    out.assign((size_t)frames * 2, 0);
    std::vector<float> sum((size_t)frames * 2, 0.0f);
    std::vector<std::shared_ptr<Subscriber>> targets;
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto it = voices.begin(); it != voices.end();) {
            Voice& voice = it->second;
            size_t count = std::min(voice.queue.size(), sum.size());
            for (size_t i = 0; i < count; i++) {
                sum[i] += voice.queue[i];
            }
            voice.queue.erase(voice.queue.begin(), voice.queue.begin() + count);
            if (voice.closing && voice.queue.empty()) {
                it = voices.erase(it);
            } else {
                ++it;
            }
        }
        for (U32 f = 0; f < frames; f++) {
            bool sound = false;
            for (U32 c = 0; c < 2; c++) {
                float v = sum[f * 2 + c] * 32768.0f;
                S32 s = (S32)std::max(-32768.0f, std::min(32767.0f, v));
                out[f * 2 + c] = (S16)s;
                U32 magnitude = (U32)std::min(32767, std::abs(s));
                if (magnitude) {
                    sound = true;
                    peak = std::max(peak, magnitude);
                }
            }
            if (sound) {
                soundFrames++;
                heardSound = true;
                lastSoundTime = KSystem::getMilliesSinceStart();
            }
        }
        framesMixed += frames;
        targets = subscribers;
    }
    for (auto& subscriber : targets) {
        {
            std::lock_guard<std::mutex> lock(subscriber->mutex);
            subscriber->chunks.push_back(out);
            while (subscriber->chunks.size() > MAX_SUBSCRIBER_CHUNKS) {
                subscriber->chunks.pop_front();
            }
        }
        subscriber->cond.notify_all();
    }
}

void AudioCapture::start() {
    stopMixerThread();
    enabled = true;
    mixerRunning = true;
    mixerThread = std::thread([]() {
        auto start = std::chrono::steady_clock::now();
        U64 emitted = 0;
        std::vector<S16> chunk;
        std::unique_lock<std::mutex> lock(mixerMutex);
        while (mixerRunning) {
            mixerCond.wait_for(lock, std::chrono::milliseconds(MIX_INTERVAL_MS));
            if (!mixerRunning) {
                break;
            }
            U64 elapsedUs = (U64)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
            U64 due = elapsedUs * RATE / 1000000;
            if (due <= emitted) {
                continue;
            }
            // After a long stall (a debugger, a suspended host) don't send a burst.
            U64 frames = std::min<U64>(due - emitted, RATE / 4);
            emitted = due;
            lock.unlock();
            mix((U32)frames, chunk);
            lock.lock();
        }
    });
}

void AudioCapture::stop() {
    stopMixerThread();
    enabled = false;
    std::vector<std::shared_ptr<Subscriber>> targets;
    {
        std::lock_guard<std::mutex> lock(mutex);
        targets = subscribers;
    }
    for (auto& subscriber : targets) {
        {
            std::lock_guard<std::mutex> lock(subscriber->mutex);
            subscriber->stopped = true;
        }
        subscriber->cond.notify_all();
    }
}

std::shared_ptr<AudioCapture::Subscriber> AudioCapture::subscribe() {
    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>();
    std::lock_guard<std::mutex> lock(mutex);
    subscribers.push_back(subscriber);
    return subscriber;
}

void AudioCapture::unsubscribe(const std::shared_ptr<Subscriber>& subscriber) {
    std::lock_guard<std::mutex> lock(mutex);
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
}

AudioCapture::Status AudioCapture::status(bool resetPeak) {
    std::lock_guard<std::mutex> lock(mutex);
    Status result;
    result.voices = (U32)voices.size();
    result.peak = peak;
    result.framesMixed = framesMixed;
    result.soundFrames = soundFrames;
    result.msSinceSound = heardSound ? (S64)(U32)(KSystem::getMilliesSinceStart() - lastSoundTime) : -1;
    if (resetPeak) {
        peak = 0;
    }
    return result;
}

void AudioCapture::reset() {
    std::lock_guard<std::mutex> lock(mutex);
    voices.clear();
    subscribers.clear();
    peak = 0;
    framesMixed = 0;
    soundFrames = 0;
    heardSound = false;
    lastSoundTime = 0;
}

#endif
