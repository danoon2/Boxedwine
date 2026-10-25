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

#ifndef __AUDIO_CAPTURE_H__
#define __AUDIO_CAPTURE_H__

#include "localserver.h"

#ifdef BOXEDWINE_LOCAL_SERVER

#include <vector>
#include <deque>
#include <atomic>
#include <memory>

// Mixes the PCM the guest writes to every open /dev/dsp voice into one
// 44100 Hz stereo s16le stream for the local server. Each voice is converted
// and resampled (linear) when written, then a mixer thread pulls from all
// voices at wall-clock rate, so voices that start, stop or underrun line up
// the way the host's own mixer would play them.
//
// Works with -nosound too: the DSP device paces writes the same way.
class AudioCapture {
public:
    static const U32 RATE = 44100;
    static const U32 CHANNELS = 2;

    // A consumer of mixed chunks. The mixer drops the oldest chunks when a
    // consumer falls more than about a second behind.
    class Subscriber {
    public:
        // Waits up to timeoutMs for a chunk. Returns false on timeout or stop.
        bool pop(std::vector<S16>& chunk, U32 timeoutMs);

        std::mutex mutex;
        std::condition_variable cond;
        std::deque<std::vector<S16>> chunks;
        bool stopped = false;
    };

    struct Status {
        U32 voices = 0;
        // Loudest sample (0 to 32767) mixed since the previous status call that reset it.
        U32 peak = 0;
        U64 framesMixed = 0;
        // Mixed frames with any non-zero sample.
        U64 soundFrames = 0;
        // Milliseconds since the last non-zero sample, or -1 if there was none.
        S64 msSinceSound = -1;
    };

    static std::atomic<bool> enabled;

    // Called by the DSP device with the bytes it accepted. format is an
    // SDL_AudioFormat value (bit size, float, big endian and signed flags).
    static void write(U32 voiceId, U32 format, U32 freq, U32 channels, const U8* data, U32 len);
    static void closeVoice(U32 voiceId);

    // Starts and stops the mixer thread, and sets enabled.
    static void start();
    static void stop();

    static std::shared_ptr<Subscriber> subscribe();
    static void unsubscribe(const std::shared_ptr<Subscriber>& subscriber);
    static Status status(bool resetPeak);

    // Mixes frames from the queued voice audio into out (frames * 2 samples)
    // and hands the chunk to subscribers. The mixer thread calls it; tests
    // call it directly without starting the thread.
    static void mix(U32 frames, std::vector<S16>& out);
    static void reset();
};

#endif

#endif
