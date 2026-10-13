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
#include "localserver.h"

#ifdef BOXEDWINE_LOCAL_SERVER

#include "screencapture.h"
#include "audiocapture.h"
#include "encoding.h"

static const U32 MAX_WAIT_MS = 60000;

static std::string frameJson(const ScreenCapture::Frame& frame) {
    U32 now = KSystem::getMilliesSinceStart();
    return std::string("{\"width\":") + std::to_string(frame.width)
        + ",\"height\":" + std::to_string(frame.height)
        + ",\"hash\":\"" + ServerEncoding::hex64(frame.hash) + "\""
        + ",\"change_seq\":" + std::to_string(frame.changeSeq)
        + ",\"changed_ms_ago\":" + std::to_string((U32)(now - frame.changedAt))
        + ",\"presents\":" + std::to_string(ScreenCapture::presentCount())
        + ",\"gl_presenting\":" + (ScreenCapture::glPresenting() ? "true" : "false");
}

static std::map<std::string, std::string> frameHeaders(const ScreenCapture::Frame& frame) {
    return {
        { "X-Frame-Hash", ServerEncoding::hex64(frame.hash) },
        { "X-Frame-Change-Seq", std::to_string(frame.changeSeq) },
    };
}

static void handleScreenshot(const LocalServer::Request& request, LocalServer::Connection& connection) {
    ScreenCapture::Frame frame;
    if (!ScreenCapture::getFrame(frame)) {
        connection.sendError(503, "nothing has been presented yet");
        return;
    }
    U32 x = 0;
    U32 y = 0;
    U32 w = frame.width;
    U32 h = frame.height;
    bool crop = request.hasParam("x") || request.hasParam("y") || request.hasParam("w") || request.hasParam("h");
    if (crop) {
        double px = request.paramDouble("x", 0);
        double py = request.paramDouble("y", 0);
        double pw = request.paramDouble("w", (double)frame.width - px);
        double ph = request.paramDouble("h", (double)frame.height - py);
        if (px < 0 || py < 0 || pw < 1 || ph < 1 || px + pw > frame.width || py + ph > frame.height) {
            connection.sendError(400, "x, y, w and h must describe a non-empty rectangle inside the " + std::to_string(frame.width) + "x" + std::to_string(frame.height) + " screen");
            return;
        }
        x = (U32)px;
        y = (U32)py;
        w = (U32)pw;
        h = (U32)ph;
    }
    U32 pitch = frame.width * 4;
    std::vector<U8> png = ServerEncoding::encodePng(frame.pixels.data() + (size_t)y * pitch + (size_t)x * 4, w, h, pitch);
    if (png.empty()) {
        connection.sendError(500, "PNG encoding failed");
        return;
    }
    connection.sendResponse(200, "image/png", png.data(), png.size(), frameHeaders(frame));
}

static void handleScreen(const LocalServer::Request& request, LocalServer::Connection& connection) {
    double waitParam = request.paramDouble("wait", 0);
    U32 waitMs = (U32)std::max(0.0, std::min((double)MAX_WAIT_MS, waitParam));
    std::string since = request.param("since");
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitMs);

    ScreenCapture::Frame frame;
    bool have = ScreenCapture::getInfo(frame);
    // Long poll: wait for a picture whose hash differs from since, or for the
    // first picture when there is none yet.
    while (waitMs && (!have || (!since.empty() && ServerEncoding::hex64(frame.hash) == since))) {
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline || LocalServer::isStopping()) {
            break;
        }
        U32 remaining = (U32)std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
        ScreenCapture::waitForChange(have ? frame.changeSeq : 0, std::min<U32>(remaining, 500));
        have = ScreenCapture::getInfo(frame);
    }
    if (!have) {
        connection.sendError(503, "nothing has been presented yet");
        return;
    }
    std::string json = frameJson(frame);
    if (!since.empty()) {
        json += std::string(",\"changed\":") + (ServerEncoding::hex64(frame.hash) != since ? "true" : "false");
    }
    json += "}";
    connection.sendJson(200, json);
}

static void handleScreenStream(const LocalServer::Request& request, LocalServer::Connection& connection) {
    if (!request.isWebSocketUpgrade()) {
        connection.sendError(426, "this endpoint is a WebSocket");
        return;
    }
    double fps = std::max(0.1, std::min(60.0, request.paramDouble("fps", 5)));
    std::string mode = request.param("mode", "change");
    if (mode != "change" && mode != "all") {
        connection.sendError(400, "mode must be change or all");
        return;
    }
    if (!connection.acceptWebSocket(request)) {
        return;
    }
    auto interval = std::chrono::microseconds((S64)(1000000.0 / fps));
    auto next = std::chrono::steady_clock::now();
    U64 lastSeq = 0;
    bool sentAny = false;
    while (true) {
        auto now = std::chrono::steady_clock::now();
        if (now < next) {
            U32 waitMs = (U32)std::chrono::duration_cast<std::chrono::milliseconds>(next - now).count();
            if (!connection.wsService(std::max<U32>(waitMs, 1))) {
                return;
            }
            continue;
        }
        next += interval;
        if (next < now) {
            next = now + interval; // fell behind, skip ticks
        }
        ScreenCapture::Frame info;
        if (!ScreenCapture::getInfo(info)) {
            continue;
        }
        if (mode == "change" && sentAny && info.changeSeq == lastSeq) {
            continue;
        }
        ScreenCapture::Frame frame;
        if (!ScreenCapture::getFrame(frame, false)) {
            continue;
        }
        std::vector<U8> png = ServerEncoding::encodePng(frame.pixels.data(), frame.width, frame.height, frame.width * 4);
        if (png.empty()) {
            continue;
        }
        if (!connection.wsSendBinary(png.data(), png.size())) {
            return;
        }
        lastSeq = frame.changeSeq;
        sentAny = true;
    }
}

static void handleAudio(const LocalServer::Request& request, LocalServer::Connection& connection) {
    AudioCapture::Status status = AudioCapture::status(true);
    std::string json = std::string("{\"voices\":") + std::to_string(status.voices)
        + ",\"peak\":" + std::to_string(status.peak)
        + ",\"frames_mixed\":" + std::to_string(status.framesMixed)
        + ",\"sound_frames\":" + std::to_string(status.soundFrames)
        + ",\"ms_since_sound\":" + (status.msSinceSound < 0 ? std::string("null") : std::to_string(status.msSinceSound))
        + ",\"sample_rate\":" + std::to_string(AudioCapture::RATE)
        + ",\"channels\":" + std::to_string(AudioCapture::CHANNELS)
        + ",\"format\":\"s16le\"}";
    connection.sendJson(200, json);
}

static void handleAudioStream(const LocalServer::Request& request, LocalServer::Connection& connection) {
    if (!request.isWebSocketUpgrade()) {
        connection.sendError(426, "this endpoint is a WebSocket");
        return;
    }
    if (!connection.acceptWebSocket(request)) {
        return;
    }
    std::string header = "{\"type\":\"audio_format\",\"sample_rate\":" + std::to_string(AudioCapture::RATE)
        + ",\"channels\":" + std::to_string(AudioCapture::CHANNELS) + ",\"format\":\"s16le\"}";
    if (!connection.wsSendText(header)) {
        return;
    }
    std::shared_ptr<AudioCapture::Subscriber> subscriber = AudioCapture::subscribe();
    std::vector<S16> chunk;
    while (connection.wsService(0)) {
        if (!subscriber->pop(chunk, 50)) {
            continue;
        }
        // s16le: samples are already little endian on every host Boxedwine runs on.
        if (!connection.wsSendBinary((const U8*)chunk.data(), chunk.size() * sizeof(S16))) {
            break;
        }
    }
    AudioCapture::unsubscribe(subscriber);
}

void startMediaCapture() {
    ScreenCapture::reset();
    ScreenCapture::enabled = true;
    AudioCapture::reset();
    AudioCapture::start();

    LocalServer::addRoute("GET", "/screenshot",
        "The current picture as PNG. Optional x, y, w, h crop it. Works with -novideo. "
        "X-Frame-Hash and X-Frame-Change-Seq headers identify the picture.",
        handleScreenshot);
    LocalServer::addRoute("GET", "/screen",
        "JSON with the picture's size, hash, change_seq, changed_ms_ago and presents, without pixels. "
        "With since=<hash>&wait=<ms> it waits up to wait ms for a picture whose hash differs.",
        handleScreen);
    LocalServer::addRoute("GET", "/ws/screen",
        "WebSocket of binary PNG frames, at most fps a second (default 5, 0.1 to 60). "
        "mode=change (default) sends only changed pictures, mode=all sends one every tick.",
        handleScreenStream);
    LocalServer::addRoute("GET", "/audio",
        "JSON audio status: open voices, peak (loudest sample since the previous call, 0 is silence), "
        "frames mixed, frames with sound and ms_since_sound.",
        handleAudio);
    LocalServer::addRoute("GET", "/ws/audio",
        "WebSocket of the mixed guest audio: a JSON format header, then binary s16le 44100 Hz stereo chunks.",
        handleAudioStream);
}

void stopMediaCapture() {
    AudioCapture::stop();
    ScreenCapture::enabled = false;
}

#endif
