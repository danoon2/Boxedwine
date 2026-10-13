/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "../../server/localserver.h"

#ifdef BOXEDWINE_LOCAL_SERVER

#include "../../server/encoding.h"
#include "../../server/screencapture.h"
#include "../../server/audiocapture.h"
#include "../../sdl/startupArgs.h"
#include "../cpu/testCPU.h"
#include "../../util/stb_image.h"

#ifdef WIN32
#undef BOOL
#include <winsock2.h>
#include <ws2tcpip.h>
#define BOOL unsigned int
#undef min
#undef max
typedef SOCKET TestSocket;
static void closeTestSocket(TestSocket s) { closesocket(s); }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int TestSocket;
static void closeTestSocket(TestSocket s) { close(s); }
#endif

namespace {
    const U32 RED = 0xFF0000;
    const U32 GREEN = 0x00FF00;
    const U32 BLUE = 0x0000FF;
    const U32 DESKTOP = (58 << 16) | (110 << 8) | 165;

    U32 pixelAt(const ScreenCapture::Frame& frame, U32 x, U32 y) {
        return ((const U32*)frame.pixels.data())[y * frame.width + x];
    }

    // Presents one GDI frame of the given size with a single window.
    void presentWindow(U32 screenWidth, U32 screenHeight, const std::vector<U32>& window, U32 windowWidth, S32 x, S32 y) {
        ScreenCapture::beginComposite(screenWidth, screenHeight);
        ScreenCapture::blit((const U8*)window.data(), windowWidth * 4, x, y, windowWidth, (U32)(window.size() / windowWidth));
        ScreenCapture::endComposite();
    }

    TestSocket connectTo(U32 port) {
        TestSocket s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons((U16)port);
        if (connect(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
            closeTestSocket(s);
            return (TestSocket)-1;
        }
#ifdef WIN32
        DWORD timeout = 5000;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        timeval timeout = { 5, 0 };
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
        return s;
    }

    std::string readAll(TestSocket s) {
        std::string result;
        char buffer[4096];
        while (true) {
            int received = recv(s, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                break;
            }
            result.append(buffer, received);
        }
        return result;
    }

    // Sends a request and returns the whole response, which the server ends by closing.
    std::string httpGet(U32 port, const std::string& path, const std::string& headers = "Host: localhost\r\n") {
        TestSocket s = connectTo(port);
        if (s == (TestSocket)-1) {
            return std::string();
        }
        std::string request = "GET " + path + " HTTP/1.1\r\n" + headers + "\r\n";
        send(s, request.data(), (int)request.size(), 0);
        std::string response = readAll(s);
        closeTestSocket(s);
        return response;
    }

    std::string responseBody(const std::string& response) {
        size_t end = response.find("\r\n\r\n");
        return end == std::string::npos ? std::string() : response.substr(end + 4);
    }

    bool recvExactly(TestSocket s, U8* data, size_t len) {
        while (len) {
            int received = recv(s, (char*)data, (int)len, 0);
            if (received <= 0) {
                return false;
            }
            data += received;
            len -= received;
        }
        return true;
    }

    // Reads one unmasked server frame.
    bool readWsFrame(TestSocket s, U32& opcode, std::vector<U8>& payload) {
        U8 header[2];
        if (!recvExactly(s, header, 2)) {
            return false;
        }
        opcode = header[0] & 0x0F;
        U64 len = header[1] & 0x7F;
        if (len == 126) {
            U8 ext[2];
            if (!recvExactly(s, ext, 2)) {
                return false;
            }
            len = ((U64)ext[0] << 8) | ext[1];
        } else if (len == 127) {
            U8 ext[8];
            if (!recvExactly(s, ext, 8)) {
                return false;
            }
            len = 0;
            for (int i = 0; i < 8; i++) {
                len = (len << 8) | ext[i];
            }
        }
        payload.resize((size_t)len);
        return !len || recvExactly(s, payload.data(), (size_t)len);
    }
}

void testServerEncodings() {
    const char* abc = "abc";
    std::vector<U8> digest = ServerEncoding::sha1((const U8*)abc, 3);
    std::string hex;
    for (U8 b : digest) {
        char buffer[3];
        snprintf(buffer, sizeof(buffer), "%02x", b);
        hex += buffer;
    }
    if (hex != "a9993e364706816aba3e25717850c26c9cd0d89d") {
        testFail("SHA-1 of abc was %s", hex.c_str());
    }
    // A message spanning two blocks.
    std::string longText = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    digest = ServerEncoding::sha1((const U8*)longText.data(), longText.size());
    if (ServerEncoding::base64(digest.data(), digest.size()) != "hJg+RBw70m66rkqh+VEp5eVGcPE=") {
        testFail("SHA-1 of the two-block FIPS vector was wrong");
    }
    if (ServerEncoding::base64((const U8*)"ab", 2) != "YWI=" || ServerEncoding::base64((const U8*)"a", 1) != "YQ==") {
        testFail("base64 padding was wrong");
    }
    // The example handshake from RFC 6455 section 1.3.
    std::string accept = ServerEncoding::webSocketAccept("dGhlIHNhbXBsZSBub25jZQ==");
    if (accept != "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") {
        testFail("Sec-WebSocket-Accept was %s", accept.c_str());
    }
}

void testServerPngRoundTrip() {
    // 3x2 with a pitch wider than the row, to check padding is skipped.
    const U32 pitchPixels = 4;
    U32 pixels[pitchPixels * 2] = {
        RED, GREEN, BLUE, 0xDEADBEEF,
        0x123456, 0xFFFFFF, 0x000000, 0xDEADBEEF,
    };
    std::vector<U8> png = ServerEncoding::encodePng((const U8*)pixels, 3, 2, pitchPixels * 4);
    int w = 0, h = 0, components = 0;
    U8* decoded = stbi_load_from_memory(png.data(), (int)png.size(), &w, &h, &components, 3);
    if (!decoded) {
        testFail("PNG did not decode: %s", stbi_failure_reason());
        return;
    }
    if (w != 3 || h != 2) {
        testFail("PNG decoded as %dx%d", w, h);
    } else {
        for (U32 y = 0; y < 2; y++) {
            for (U32 x = 0; x < 3; x++) {
                U32 expected = pixels[y * pitchPixels + x] & 0xFFFFFF;
                const U8* p = decoded + (y * 3 + x) * 3;
                U32 actual = ((U32)p[0] << 16) | ((U32)p[1] << 8) | p[2];
                if (actual != expected) {
                    testFail("PNG pixel %u,%u was %06x, expected %06x", x, y, actual, expected);
                }
            }
        }
    }
    stbi_image_free(decoded);
}

void testServerRequestParsing() {
    LocalServer::Request request;
    std::string head = "GET /screen%2Fx?wait=250&since=ab%20cd&flag HTTP/1.1\r\nHost: localhost\r\nUpgrade:  WebSocket \r\nSec-WebSocket-Key: abc";
    if (!LocalServer::parseRequestHead(head, request)) {
        testFail("valid request head was rejected");
        return;
    }
    if (request.method != "GET" || request.path != "/screen/x") {
        testFail("method or path parsed as %s %s", request.method.c_str(), request.path.c_str());
    }
    if (request.param("wait") != "250" || request.param("since") != "ab cd" || !request.hasParam("flag") || request.hasParam("other")) {
        testFail("query parameters were parsed wrong");
    }
    if (request.paramDouble("wait", 0) != 250 || request.paramDouble("since", 7) != 7) {
        testFail("numeric query parameters were parsed wrong");
    }
    if (request.headers["host"] != "localhost" || !request.isWebSocketUpgrade()) {
        testFail("headers were parsed wrong");
    }
    LocalServer::Request bad;
    if (LocalServer::parseRequestHead("GARBAGE", bad) || LocalServer::parseRequestHead("GET noslash HTTP/1.1", bad)) {
        testFail("malformed request lines were accepted");
    }
    if (LocalServer::jsonEscape("a\"b\\c\n\x01") != "a\\\"b\\\\c\\n\\u0001") {
        testFail("JSON escaping was wrong");
    }
}

void testScreenCaptureComposite() {
    ScreenCapture::reset();
    ScreenCapture::enabled = true;
    if (ScreenCapture::onGlSwap()) {
        testFail("GL readback was requested without demand");
    }
    ScreenCapture::reset();
    ScreenCapture::Frame frame;
    if (ScreenCapture::getFrame(frame)) {
        testFail("a frame was reported before any present");
    }

    // A 2x2 window hanging off the top left corner of a 4x3 screen.
    std::vector<U32> window = { RED, GREEN, BLUE, 0xFF000000 | RED };
    presentWindow(4, 3, window, 2, -1, -1);
    if (!ScreenCapture::getFrame(frame) || frame.width != 4 || frame.height != 3 || frame.changeSeq != 1) {
        testFail("first present was not captured");
        ScreenCapture::enabled = false;
        return;
    }
    if (pixelAt(frame, 0, 0) != RED || pixelAt(frame, 1, 0) != DESKTOP || pixelAt(frame, 3, 2) != DESKTOP) {
        testFail("window was not clipped onto the desktop, top left is %06x", pixelAt(frame, 0, 0));
    }
    U64 firstHash = frame.hash;

    // The same picture again does not count as a change.
    presentWindow(4, 3, window, 2, -1, -1);
    ScreenCapture::getInfo(frame);
    if (frame.changeSeq != 1 || frame.hash != firstHash || ScreenCapture::presentCount() != 2) {
        testFail("an identical present changed the picture");
    }

    // Moving the window changes it, clipped at the bottom right.
    presentWindow(4, 3, window, 2, 3, 2);
    ScreenCapture::getFrame(frame);
    if (frame.changeSeq != 2 || frame.hash == firstHash || pixelAt(frame, 3, 2) != RED || pixelAt(frame, 0, 0) != DESKTOP) {
        testFail("moved window was not captured");
    }

    // A GL window at 1,1 covers part of it. GL rows arrive bottom-up as BGRA.
    U32 glRows[2 * 2] = { BLUE, BLUE, GREEN, GREEN }; // bottom row blue, top row green
    // getFrame above counted as demand.
    if (!ScreenCapture::onGlSwap()) {
        testFail("GL readback was not requested after demand");
    }
    ScreenCapture::setGlFrame(42, 1, 1, 2, 2, (const U8*)glRows);
    ScreenCapture::getFrame(frame, false);
    if (pixelAt(frame, 1, 1) != GREEN || pixelAt(frame, 2, 2) != BLUE || pixelAt(frame, 3, 2) != RED) {
        testFail("GL layer was not laid over the GDI picture at its position");
    }
    // A later GDI present keeps the GL layer on top.
    presentWindow(4, 3, window, 2, 3, 2);
    ScreenCapture::getFrame(frame, false);
    if (pixelAt(frame, 2, 2) != BLUE) {
        testFail("GDI present dropped the GL layer");
    }
    ScreenCapture::clearGlFrame(7);
    ScreenCapture::getFrame(frame, false);
    if (pixelAt(frame, 2, 2) != BLUE) {
        testFail("clearing another drawable removed the GL layer");
    }
    ScreenCapture::clearGlFrame(42);
    ScreenCapture::getFrame(frame, false);
    if (pixelAt(frame, 2, 2) != DESKTOP) {
        testFail("hidden GL window stayed in the picture");
    }

    // waitForChange returns at once when the sequence already moved on, and
    // after a present on another thread otherwise.
    U64 seq = frame.changeSeq;
    if (ScreenCapture::waitForChange(seq - 1, 1000) != seq) {
        testFail("waitForChange did not return the current sequence");
    }
    std::thread presenter([&window]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        presentWindow(4, 3, window, 2, 0, 0);
    });
    U64 changed = ScreenCapture::waitForChange(seq, 5000);
    presenter.join();
    if (changed == seq) {
        testFail("waitForChange missed a present");
    }

    ScreenCapture::enabled = false;
    ScreenCapture::reset();
}

void testAudioCaptureMix() {
    AudioCapture::reset();
    AudioCapture::enabled = true;
    const U32 S16LSB = 0x8010;
    const U32 U8_FORMAT = 0x0008;

    // Mono 22050 Hz at half scale doubles to 44100 Hz stereo.
    std::vector<S16> mono(2205, 16384);
    AudioCapture::write(1, S16LSB, 22050, 1, (const U8*)mono.data(), (U32)(mono.size() * 2));
    std::vector<S16> out;
    AudioCapture::mix(4000, out);
    if (out.size() != 8000 || out[0] != 16384 || out[1] != 16384 || out[2 * 4000 - 2] != 16384) {
        testFail("mono 22050 Hz was not resampled to stereo 44100 Hz, first sample %d", out.empty() ? 0 : out[0]);
    }
    AudioCapture::Status status = AudioCapture::status(true);
    if (status.voices != 1 || status.peak != 16384 || status.framesMixed != 4000 || status.soundFrames != 4000 || status.msSinceSound < 0) {
        testFail("audio status was wrong: voices %u peak %u", status.voices, status.peak);
    }
    if (AudioCapture::status(false).peak != 0) {
        testFail("peak was not reset by the previous status");
    }
    // Underrun: the voice ran out, so the rest is silence.
    AudioCapture::mix(1000, out);
    if (out[2 * 999] != 0) {
        testFail("an underrun was not silent");
    }

    // Two voices sum: stereo 44100 U8 at +64 and S16 at -8192 on the left only.
    std::vector<U8> unsignedPcm(441 * 2, 128 + 64);
    std::vector<S16> signedPcm(441 * 2, 0);
    for (size_t i = 0; i < signedPcm.size(); i += 2) {
        signedPcm[i] = -8192;
    }
    AudioCapture::write(2, U8_FORMAT, 44100, 2, unsignedPcm.data(), (U32)unsignedPcm.size());
    AudioCapture::write(3, S16LSB, 44100, 2, (const U8*)signedPcm.data(), (U32)(signedPcm.size() * 2));
    AudioCapture::mix(400, out);
    if (out[0] != 16384 - 8192 || out[1] != 16384) {
        testFail("two voices did not mix, got %d %d", out[0], out[1]);
    }

    // A closed voice plays out what it queued, then goes away.
    AudioCapture::closeVoice(2);
    AudioCapture::closeVoice(3);
    AudioCapture::closeVoice(1);
    if (AudioCapture::status(true).voices != 3) {
        testFail("closed voices were dropped before their audio played");
    }
    AudioCapture::mix(400, out);
    if (AudioCapture::status(true).voices != 0) {
        testFail("closed voices stayed after their audio played");
    }

    // Subscribers get each mixed chunk.
    std::shared_ptr<AudioCapture::Subscriber> subscriber = AudioCapture::subscribe();
    AudioCapture::mix(10, out);
    std::vector<S16> chunk;
    if (!subscriber->pop(chunk, 1000) || chunk.size() != 20) {
        testFail("subscriber did not receive the mixed chunk");
    }
    AudioCapture::unsubscribe(subscriber);
    AudioCapture::enabled = false;
    AudioCapture::reset();
}

void testLocalServerEndToEnd() {
    startMediaCapture();
    std::string error;
    if (!LocalServer::start(0, error)) {
        testFail("server did not start: %s", error.c_str());
        stopMediaCapture();
        return;
    }
    U32 port = LocalServer::port();
    if (!port) {
        testFail("server reported no port");
    }

    std::string response = httpGet(port, "/screenshot");
    if (response.find("HTTP/1.1 503") != 0) {
        testFail("screenshot before any present was not 503: %s", response.substr(0, 40).c_str());
    }
    std::vector<U32> window(16 * 8, GREEN);
    presentWindow(32, 16, window, 16, 4, 4);

    response = httpGet(port, "/screenshot");
    std::string body = responseBody(response);
    int w = 0, h = 0, components = 0;
    U8* decoded = stbi_load_from_memory((const U8*)body.data(), (int)body.size(), &w, &h, &components, 3);
    if (response.find("HTTP/1.1 200") != 0 || response.find("Content-Type: image/png") == std::string::npos || !decoded || w != 32 || h != 16) {
        testFail("screenshot was not a 32x16 PNG");
    } else if (decoded[(5 * 32 + 5) * 3 + 1] != 0xFF || decoded[1] != 110) {
        testFail("screenshot pixels were wrong");
    }
    if (decoded) {
        stbi_image_free(decoded);
    }

    response = httpGet(port, "/screenshot?x=4&y=4&w=16&h=8");
    body = responseBody(response);
    decoded = stbi_load_from_memory((const U8*)body.data(), (int)body.size(), &w, &h, &components, 3);
    if (!decoded || w != 16 || h != 8 || decoded[0] != 0 || decoded[1] != 0xFF) {
        testFail("cropped screenshot was wrong");
    }
    if (decoded) {
        stbi_image_free(decoded);
    }
    if (httpGet(port, "/screenshot?x=30&w=8").find("HTTP/1.1 400") != 0) {
        testFail("crop outside the screen was not rejected");
    }

    body = responseBody(httpGet(port, "/screen"));
    size_t hashPos = body.find("\"hash\":\"");
    if (body.find("\"width\":32") == std::string::npos || hashPos == std::string::npos) {
        testFail("/screen JSON was wrong: %s", body.c_str());
    } else {
        std::string hash = body.substr(hashPos + 8, 16);
        // Long poll returns when the picture changes.
        std::thread presenter([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::vector<U32> other(16 * 8, RED);
            presentWindow(32, 16, other, 16, 4, 4);
        });
        body = responseBody(httpGet(port, "/screen?wait=5000&since=" + hash));
        presenter.join();
        if (body.find("\"changed\":true") == std::string::npos || body.find(hash) != std::string::npos) {
            testFail("/screen long poll did not report the change: %s", body.c_str());
        }
    }

    // Only loopback hosts and origins, so web pages can't read the screen.
    std::string localHost = "Host: 127.0.0.1:" + std::to_string(port) + "\r\n";
    if (httpGet(port, "/screen", localHost + "Origin: http://localhost:3000\r\n").find("HTTP/1.1 200") != 0
            || httpGet(port, "/screen", "Host: [::1]:" + std::to_string(port) + "\r\n").find("HTTP/1.1 200") != 0
            || httpGet(port, "/screen", "").find("HTTP/1.1 200") != 0) {
        testFail("a loopback request was refused");
    }
    if (httpGet(port, "/screen", "Host: rebound.example.com\r\n").find("HTTP/1.1 403") != 0
            || httpGet(port, "/screen", localHost + "Origin: https://example.com\r\n").find("HTTP/1.1 403") != 0
            || httpGet(port, "/screen", localHost + "Origin: null\r\n").find("HTTP/1.1 403") != 0
            || httpGet(port, "/screen", "Host: localhost.example.com\r\n").find("HTTP/1.1 403") != 0) {
        testFail("a request from a web page was served");
    }

    if (httpGet(port, "/nope").find("HTTP/1.1 404") != 0) {
        testFail("unknown path was not 404");
    }
    if (httpGet(port, "/").find("/ws/audio") == std::string::npos) {
        testFail("index did not list the routes");
    }
    if (responseBody(httpGet(port, "/audio")).find("\"sample_rate\":44100") == std::string::npos) {
        testFail("/audio did not report the format");
    }
    if (httpGet(port, "/ws/audio").find("HTTP/1.1 426") != 0) {
        testFail("plain GET of a WebSocket endpoint was not 426");
    }

    // WebSocket: handshake, the format header, then PCM.
    TestSocket s = connectTo(port);
    std::string upgrade = "GET /ws/audio HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
    send(s, upgrade.data(), (int)upgrade.size(), 0);
    std::string handshake;
    char c;
    while (handshake.find("\r\n\r\n") == std::string::npos && recv(s, &c, 1, 0) == 1) {
        handshake.push_back(c);
    }
    if (handshake.find("101 Switching Protocols") == std::string::npos || handshake.find("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") == std::string::npos) {
        testFail("WebSocket handshake failed: %s", handshake.c_str());
    } else {
        U32 opcode = 0;
        std::vector<U8> payload;
        if (!readWsFrame(s, opcode, payload) || opcode != 1 || std::string(payload.begin(), payload.end()).find("\"format\":\"s16le\"") == std::string::npos) {
            testFail("WebSocket audio header was missing");
        }
        // The mixer thread sends silence every 20 ms even with no voices.
        if (!readWsFrame(s, opcode, payload) || opcode != 2 || payload.empty() || payload.size() % 4) {
            testFail("WebSocket audio chunk was missing");
        }
        // Close handshake: masked close frame from the client.
        U8 closeFrame[6] = { 0x88, 0x80, 0, 0, 0, 0 };
        send(s, (const char*)closeFrame, sizeof(closeFrame), 0);
    }
    closeTestSocket(s);

    LocalServer::stop();
    stopMediaCapture();
    if (LocalServer::isRunning()) {
        testFail("server still running after stop");
    }
    TestSocket after = connectTo(port);
    if (after != (TestSocket)-1) {
        closeTestSocket(after);
        testFail("server still accepted connections after stop");
    }
    ScreenCapture::reset();
    AudioCapture::reset();
}

void testStartupArgsLocalServerOption() {
    StartUpArgs startupArgs;
    const char* argv[] = { "boxedwine", "-localServer", "8123" };
    if (!startupArgs.parseStartupArgs(3, argv) || startupArgs.localServerPort != 8123) {
        testFail("-localServer was not parsed");
        return;
    }
    std::vector<BString> childArgs = startupArgs.buildArgs();
    bool found = false;
    for (size_t i = 0; i + 1 < childArgs.size(); i++) {
        if (childArgs[i] == "-localServer" && childArgs[i + 1] == "8123") {
            found = true;
        }
    }
    if (!found) {
        testFail("-localServer was not propagated to a child process");
    }
    StartUpArgs bad;
    const char* badArgv[] = { "boxedwine", "-localServer", "70000" };
    if (bad.parseStartupArgs(3, badArgv)) {
        testFail("an out of range -localServer port was accepted");
    }
}

#endif

#endif
