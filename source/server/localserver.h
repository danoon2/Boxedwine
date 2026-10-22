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

#ifndef __LOCAL_SERVER_H__
#define __LOCAL_SERVER_H__

// The browser build has no listening sockets.
#if !defined(__EMSCRIPTEN__)
#define BOXEDWINE_LOCAL_SERVER
#endif

#ifdef BOXEDWINE_LOCAL_SERVER

#include <string>
#include <map>
#include <vector>
#include <functional>

// A small HTTP/1.1 and WebSocket server on 127.0.0.1, started by -localServer,
// for tools and agents that drive Boxedwine without a window. Each connection
// gets its own thread and carries one request; the response closes it.
// Endpoints register with addRoute(), so features beyond screen and audio
// can share the server.
class LocalServer {
public:
    struct Request {
        std::string method;
        std::string path;
        std::map<std::string, std::string> query;
        // Header names are lower case.
        std::map<std::string, std::string> headers;
        std::string body;

        bool hasParam(const std::string& name) const;
        std::string param(const std::string& name, const std::string& defaultValue = std::string()) const;
        double paramDouble(const std::string& name, double defaultValue) const;
        bool isWebSocketUpgrade() const;
    };

    class Connection {
    public:
        virtual ~Connection() {}

        virtual bool sendResponse(U32 status, const std::string& contentType, const U8* body, size_t len, const std::map<std::string, std::string>& extraHeaders = {}) = 0;
        bool sendResponse(U32 status, const std::string& contentType, const std::string& body, const std::map<std::string, std::string>& extraHeaders = {}) {
            return sendResponse(status, contentType, (const U8*)body.data(), body.size(), extraHeaders);
        }
        bool sendJson(U32 status, const std::string& json) {
            return sendResponse(status, "application/json", json);
        }
        // {"error": message} with the given status.
        bool sendError(U32 status, const std::string& message);

        // Completes the WebSocket handshake. After it, only the ws* calls apply.
        virtual bool acceptWebSocket(const Request& request) = 0;
        virtual bool wsSendText(const std::string& text) = 0;
        virtual bool wsSendBinary(const U8* data, size_t len) = 0;
        // Waits up to timeoutMs for client frames and handles them: answers
        // pings and close frames, ignores data. Returns false once the
        // connection is closed or the server is stopping.
        virtual bool wsService(U32 timeoutMs) = 0;
    };

    typedef std::function<void(const Request& request, Connection& connection)> Handler;

    // path matches exactly. description shows in the index at "/".
    static void addRoute(const std::string& method, const std::string& path, const std::string& description, Handler handler);

    // port 0 picks a free port. Returns false and fills error on failure.
    static bool start(U32 port, std::string& error);
    static void stop();
    static bool isRunning();
    static bool isStopping();
    static U32 port();

    // Exposed for tests.
    static bool parseRequestHead(const std::string& head, Request& request);
    static std::string urlDecode(const std::string& text);
    static std::string jsonEscape(const std::string& text);
};

// Turns on screen and audio capture and registers /screenshot, /screen,
// /ws/screen, /audio and /ws/audio.
void startMediaCapture();
void stopMediaCapture();

#endif

#endif
