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
#include "screencapture.h"
#include "encoding.h"

#ifdef BOXEDWINE_LOCAL_SERVER

std::atomic<bool> ScreenCapture::enabled(false);

// Same colour KNativeScreenSDL::clear() paints the desktop with.
static const U32 DESKTOP_COLOR = (58 << 16) | (110 << 8) | 165;
// How long a consumer's request keeps GL readback on.
static const U32 DEMAND_MS = 2000;
// GL frames are read at most this often, about 60 a second.
static const U32 GL_READ_INTERVAL_MS = 15;
// A GL window that swapped within this long counts as presenting.
static const U32 GL_PRESENTING_MS = 1000;
static const U32 GL_FRESH_FRAME_WAIT_MS = 250;

namespace {
    struct GlLayer {
        bool valid = false;
        U32 id = 0;
        S32 x = 0;
        S32 y = 0;
        U32 width = 0;
        U32 height = 0;
        std::vector<U32> pixels; // top-down
    };

    std::mutex mutex;
    std::condition_variable cond;

    std::vector<U32> work;
    U32 workWidth = 0;
    U32 workHeight = 0;

    std::vector<U32> gdi;
    U32 gdiWidth = 0;
    U32 gdiHeight = 0;

    GlLayer gl;
    U64 glFrames = 0;

    std::vector<U32> composed;
    std::vector<U32> current;
    U32 currentWidth = 0;
    U32 currentHeight = 0;
    bool haveFrame = false;
    U64 changeSeq = 0;
    U32 changedAt = 0;
    U64 hash = 0;
    bool hashValid = false;

    std::atomic<U32> demandUntil(0);
    std::atomic<U32> lastGlRead(0);
    std::atomic<U32> lastGlSwap(0);
    std::atomic<bool> glSwapSeen(false);
    std::atomic<U64> presents(0);

    bool timeBefore(U32 a, U32 b) {
        return (S32)(a - b) < 0;
    }

    // Builds the picture from the last GDI frame and the GL layer, and
    // publishes it if it differs from the current one. Caller holds mutex.
    void publish() {
        U32 width = gdiWidth;
        U32 height = gdiHeight;
        if (!width || !height) {
            if (!gl.valid) {
                return;
            }
            width = gl.width;
            height = gl.height;
        }
        composed.resize((size_t)width * height);
        if (gdiWidth == width && gdiHeight == height && gdi.size() == composed.size()) {
            memcpy(composed.data(), gdi.data(), composed.size() * 4);
        } else {
            std::fill(composed.begin(), composed.end(), 0);
        }
        if (gl.valid) {
            S32 left = std::max<S32>(gl.x, 0);
            S32 top = std::max<S32>(gl.y, 0);
            S32 right = std::min<S32>(gl.x + (S32)gl.width, (S32)width);
            S32 bottom = std::min<S32>(gl.y + (S32)gl.height, (S32)height);
            // A window entirely off the left edge has left - gl.x past its row,
            // so only index the layer when something overlaps.
            if (right > left) {
                for (S32 y = top; y < bottom; y++) {
                    const U32* src = &gl.pixels[(size_t)(y - gl.y) * gl.width + (left - gl.x)];
                    memcpy(&composed[(size_t)y * width + left], src, (size_t)(right - left) * 4);
                }
            }
        }
        if (haveFrame && width == currentWidth && height == currentHeight && !memcmp(composed.data(), current.data(), composed.size() * 4)) {
            return;
        }
        current.swap(composed);
        currentWidth = width;
        currentHeight = height;
        haveFrame = true;
        changeSeq++;
        changedAt = KSystem::getMilliesSinceStart();
        hashValid = false;
        cond.notify_all();
    }

    void fillInfo(ScreenCapture::Frame& frame) {
        if (!hashValid) {
            hash = ServerEncoding::hashFrame((const U8*)current.data(), currentWidth, currentHeight, currentWidth * 4);
            hashValid = true;
        }
        frame.width = currentWidth;
        frame.height = currentHeight;
        frame.changeSeq = changeSeq;
        frame.hash = hash;
        frame.changedAt = changedAt;
    }
}

void ScreenCapture::beginComposite(U32 width, U32 height) {
    if (!enabled || !width || !height) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    workWidth = width;
    workHeight = height;
    work.assign((size_t)width * height, DESKTOP_COLOR);
}

void ScreenCapture::blit(const U8* bits, U32 pitch, S32 dstX, S32 dstY, U32 width, U32 height) {
    if (!enabled || !bits) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    if (work.empty()) {
        return;
    }
    S32 left = std::max<S32>(dstX, 0);
    S32 top = std::max<S32>(dstY, 0);
    S32 right = std::min<S32>(dstX + (S32)width, (S32)workWidth);
    S32 bottom = std::min<S32>(dstY + (S32)height, (S32)workHeight);
    if (right <= left || bottom <= top) {
        return;
    }
    for (S32 y = top; y < bottom; y++) {
        const U32* src = (const U32*)(bits + (size_t)(y - dstY) * pitch) + (left - dstX);
        U32* dst = &work[(size_t)y * workWidth + left];
        for (S32 x = left; x < right; x++) {
            *dst++ = *src++ & 0xFFFFFF;
        }
    }
}

void ScreenCapture::endComposite() {
    if (!enabled) {
        return;
    }
    presents++;
    std::lock_guard<std::mutex> lock(mutex);
    if (work.empty()) {
        return;
    }
    gdi.swap(work);
    gdiWidth = workWidth;
    gdiHeight = workHeight;
    work.clear();
    publish();
}

bool ScreenCapture::onGlSwap() {
    if (!enabled) {
        return false;
    }
    presents++;
    U32 now = KSystem::getMilliesSinceStart();
    lastGlSwap = now;
    glSwapSeen = true;
    if (!timeBefore(now, demandUntil)) {
        return false;
    }
    if (now - lastGlRead < GL_READ_INTERVAL_MS) {
        return false;
    }
    lastGlRead = now;
    return true;
}

void ScreenCapture::setGlFrame(U32 drawableId, S32 screenX, S32 screenY, U32 width, U32 height, const U8* rows) {
    if (!enabled || !width || !height) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    gl.valid = true;
    gl.id = drawableId;
    gl.x = screenX;
    gl.y = screenY;
    gl.width = width;
    gl.height = height;
    gl.pixels.resize((size_t)width * height);
    for (U32 y = 0; y < height; y++) {
        const U32* src = (const U32*)(rows + (size_t)(height - 1 - y) * width * 4);
        U32* dst = &gl.pixels[(size_t)y * width];
        for (U32 x = 0; x < width; x++) {
            dst[x] = src[x] & 0xFFFFFF;
        }
    }
    glFrames++;
    publish();
}

void ScreenCapture::clearGlFrame(U32 drawableId) {
    if (!enabled) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    if (gl.valid && gl.id == drawableId) {
        gl.valid = false;
        gl.pixels.clear();
        publish();
    }
}

void ScreenCapture::noteDemand() {
    demandUntil = KSystem::getMilliesSinceStart() + DEMAND_MS;
}

bool ScreenCapture::glPresenting() {
    return glSwapSeen && KSystem::getMilliesSinceStart() - lastGlSwap < GL_PRESENTING_MS;
}

// Without recent demand the GL layer can be seconds old, so wait for a frame
// read after the request. Caller holds lock.
static void waitForFreshGlFrame(std::unique_lock<std::mutex>& lock) {
    if (!ScreenCapture::glPresenting()) {
        return;
    }
    U64 startFrames = glFrames;
    cond.wait_for(lock, std::chrono::milliseconds(GL_FRESH_FRAME_WAIT_MS), [startFrames]() {
        return glFrames != startFrames;
    });
}

bool ScreenCapture::getFrame(Frame& frame, bool freshGl) {
    noteDemand();
    std::unique_lock<std::mutex> lock(mutex);
    if (freshGl) {
        waitForFreshGlFrame(lock);
    }
    if (!haveFrame) {
        return false;
    }
    fillInfo(frame);
    frame.pixels.resize(current.size() * 4);
    memcpy(frame.pixels.data(), current.data(), frame.pixels.size());
    return true;
}

bool ScreenCapture::getInfo(Frame& frame, bool freshGl) {
    noteDemand();
    std::unique_lock<std::mutex> lock(mutex);
    if (freshGl) {
        waitForFreshGlFrame(lock);
    }
    if (!haveFrame) {
        return false;
    }
    fillInfo(frame);
    return true;
}

U64 ScreenCapture::waitForChange(U64 sinceSeq, U32 timeoutMs) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    std::unique_lock<std::mutex> lock(mutex);
    while (changeSeq == sinceSeq) {
        // Waiting counts as demand, or a GL game would never be read.
        noteDemand();
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            break;
        }
        cond.wait_for(lock, std::min<std::chrono::steady_clock::duration>(deadline - now, std::chrono::milliseconds(500)));
    }
    return changeSeq;
}

U64 ScreenCapture::presentCount() {
    return presents;
}

void ScreenCapture::reset() {
    std::lock_guard<std::mutex> lock(mutex);
    work.clear();
    workWidth = workHeight = 0;
    gdi.clear();
    gdiWidth = gdiHeight = 0;
    gl = GlLayer();
    glFrames = 0;
    composed.clear();
    current.clear();
    currentWidth = currentHeight = 0;
    haveFrame = false;
    changeSeq = 0;
    changedAt = 0;
    hash = 0;
    hashValid = false;
    demandUntil = 0;
    lastGlRead = 0;
    lastGlSwap = 0;
    glSwapSeen = false;
    presents = 0;
}

#endif
