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

#include "encoding.h"
#include <set>
#include <cctype>
#include <cstdlib>
#include <cmath>

#ifdef WIN32
#undef BOOL
#include <winsock2.h>
#include <ws2tcpip.h>
#define BOOL unsigned int
#pragma comment(lib, "Ws2_32.lib")
#undef min
#undef max
typedef SOCKET ServerSocket;
static const ServerSocket NO_SOCKET = INVALID_SOCKET;
static void closeServerSocket(ServerSocket s) { closesocket(s); }
static const int SHUTDOWN_BOTH = SD_BOTH;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int ServerSocket;
static const ServerSocket NO_SOCKET = -1;
static void closeServerSocket(ServerSocket s) { close(s); }
static const int SHUTDOWN_BOTH = SHUT_RDWR;
#endif

#ifdef MSG_NOSIGNAL
static const int SEND_FLAGS = MSG_NOSIGNAL;
#else
static const int SEND_FLAGS = 0;
#endif

static const size_t MAX_HEAD_SIZE = 64 * 1024;
static const size_t MAX_BODY_SIZE = 1024 * 1024;
static const size_t MAX_WS_CLIENT_FRAME = 64 * 1024;

namespace {
    struct Route {
        std::string method;
        std::string path;
        std::string description;
        LocalServer::Handler handler;
    };

    std::mutex routesMutex;
    std::vector<Route> routes;

    std::mutex stateMutex;
    std::condition_variable stateCond;
    ServerSocket listenSocket = NO_SOCKET;
    // Never destroyed: exit() (kpanic does it under -novideo) runs static
    // destructors, and destroying a joinable std::thread calls std::terminate.
    std::thread& acceptThread = *new std::thread();
    std::set<ServerSocket> clientSockets;
    U32 activeConnections = 0;
    std::atomic<bool> running(false);
    std::atomic<bool> stopping(false);
    U32 boundPort = 0;

    bool sendAll(ServerSocket s, const U8* data, size_t len) {
        while (len) {
            int sent = send(s, (const char*)data, (int)std::min<size_t>(len, 1024 * 1024), SEND_FLAGS);
            if (sent <= 0) {
                return false;
            }
            data += sent;
            len -= sent;
        }
        return true;
    }

    // Waits up to timeoutMs for the socket to become readable. Returns 1 if
    // readable, 0 on timeout, -1 on error.
    int waitReadable(ServerSocket s, U32 timeoutMs) {
#ifdef WIN32
        fd_set set;
        FD_ZERO(&set);
        FD_SET(s, &set);
        timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        int result = select((int)(s + 1), &set, nullptr, nullptr, &tv);
#else
        // poll, because FD_SET overflows the fd_set once the emulator's open
        // host files push socket numbers past FD_SETSIZE.
        pollfd pfd = {};
        pfd.fd = s;
        pfd.events = POLLIN;
        int result = poll(&pfd, 1, (int)timeoutMs);
#endif
        if (result < 0) {
            return -1;
        }
        return result > 0 ? 1 : 0;
    }

    bool recvExact(ServerSocket s, U8* data, size_t len) {
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

    const char* statusText(U32 status) {
        switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 426: return "Upgrade Required";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "Unknown";
        }
    }

    std::string toLower(std::string text) {
        for (char& c : text) {
            c = (char)tolower((unsigned char)c);
        }
        return text;
    }

    std::string trim(const std::string& text) {
        size_t start = text.find_first_not_of(" \t");
        if (start == std::string::npos) {
            return std::string();
        }
        size_t end = text.find_last_not_of(" \t\r");
        return text.substr(start, end - start + 1);
    }

    // host[:port] from a Host header or an Origin URL names this machine.
    bool isLoopbackHost(std::string host) {
        size_t scheme = host.find("://");
        if (scheme != std::string::npos) {
            host = host.substr(scheme + 3);
        }
        host = toLower(host.substr(0, host.find('/')));
        if (!host.empty() && host[0] == '[') {
            return host.compare(0, 5, "[::1]") == 0;
        }
        host = host.substr(0, host.find(':'));
        return host == "127.0.0.1" || host == "localhost";
    }

    class SocketConnection : public LocalServer::Connection {
    public:
        SocketConnection(ServerSocket s) : s(s) {}

        bool sendResponse(U32 status, const std::string& contentType, const U8* body, size_t len, const std::map<std::string, std::string>& extraHeaders) override {
            if (responded) {
                return false;
            }
            responded = true;
            std::string head = "HTTP/1.1 " + std::to_string(status) + " " + statusText(status) + "\r\n";
            head += "Content-Type: " + contentType + "\r\n";
            head += "Content-Length: " + std::to_string(len) + "\r\n";
            head += "Cache-Control: no-store\r\n";
            head += "Connection: close\r\n";
            for (auto& header : extraHeaders) {
                head += header.first + ": " + header.second + "\r\n";
            }
            head += "\r\n";
            return sendAll(s, (const U8*)head.data(), head.size()) && (!len || sendAll(s, body, len));
        }

        bool acceptWebSocket(const LocalServer::Request& request) override {
            if (responded || !request.isWebSocketUpgrade()) {
                return false;
            }
            auto key = request.headers.find("sec-websocket-key");
            if (key == request.headers.end()) {
                sendError(400, "missing Sec-WebSocket-Key");
                return false;
            }
            responded = true;
            std::string head = "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: " + ServerEncoding::webSocketAccept(trim(key->second)) + "\r\n\r\n";
            webSocket = sendAll(s, (const U8*)head.data(), head.size());
            return webSocket;
        }

        bool wsSendText(const std::string& text) override {
            return sendFrame(0x1, (const U8*)text.data(), text.size());
        }

        bool wsSendBinary(const U8* data, size_t len) override {
            return sendFrame(0x2, data, len);
        }

        bool wsService(U32 timeoutMs) override {
            if (!webSocket || closed || stopping) {
                return false;
            }
            int ready = waitReadable(s, timeoutMs);
            if (ready < 0) {
                closed = true;
                return false;
            }
            if (ready == 0) {
                return !stopping;
            }
            // Read one client frame. Clients mask their frames (RFC 6455 5.3).
            U8 header[2];
            if (!recvExact(s, header, 2)) {
                closed = true;
                return false;
            }
            U32 opcode = header[0] & 0x0F;
            bool masked = (header[1] & 0x80) != 0;
            U64 len = header[1] & 0x7F;
            if (len == 126) {
                U8 ext[2];
                if (!recvExact(s, ext, 2)) {
                    closed = true;
                    return false;
                }
                len = ((U64)ext[0] << 8) | ext[1];
            } else if (len == 127) {
                U8 ext[8];
                if (!recvExact(s, ext, 8)) {
                    closed = true;
                    return false;
                }
                len = 0;
                for (int i = 0; i < 8; i++) {
                    len = (len << 8) | ext[i];
                }
            }
            if (len > MAX_WS_CLIENT_FRAME) {
                closed = true;
                return false;
            }
            U8 mask[4] = { 0, 0, 0, 0 };
            if (masked && !recvExact(s, mask, 4)) {
                closed = true;
                return false;
            }
            std::vector<U8> payload((size_t)len);
            if (len && !recvExact(s, payload.data(), (size_t)len)) {
                closed = true;
                return false;
            }
            for (size_t i = 0; i < payload.size(); i++) {
                payload[i] ^= mask[i % 4];
            }
            if (opcode == 0x8) {
                sendFrame(0x8, payload.data(), std::min<size_t>(payload.size(), 2));
                closed = true;
                return false;
            }
            if (opcode == 0x9) {
                sendFrame(0xA, payload.data(), payload.size());
            }
            return !stopping;
        }

        void finish() {
            if (webSocket && !closed) {
                // 1001: going away.
                U8 code[2] = { 0x03, 0xE9 };
                sendFrame(0x8, code, 2);
            }
        }

        bool responded = false;

    private:
        bool sendFrame(U32 opcode, const U8* data, size_t len) {
            if (!webSocket || closed) {
                return false;
            }
            U8 header[10];
            size_t headerLen = 2;
            header[0] = (U8)(0x80 | opcode);
            if (len < 126) {
                header[1] = (U8)len;
            } else if (len <= 0xFFFF) {
                header[1] = 126;
                header[2] = (U8)(len >> 8);
                header[3] = (U8)len;
                headerLen = 4;
            } else {
                header[1] = 127;
                for (int i = 0; i < 8; i++) {
                    header[2 + i] = (U8)((U64)len >> ((7 - i) * 8));
                }
                headerLen = 10;
            }
            if (!sendAll(s, header, headerLen) || (len && !sendAll(s, data, len))) {
                closed = true;
                return false;
            }
            return true;
        }

        ServerSocket s;
        bool webSocket = false;
        bool closed = false;
    };

    void sendIndex(LocalServer::Connection& connection) {
        std::string text = "Boxedwine local server\n\n";
        std::lock_guard<std::mutex> lock(routesMutex);
        for (auto& route : routes) {
            text += route.method + " " + route.path + "\n    " + route.description + "\n";
        }
        connection.sendResponse(200, "text/plain; charset=utf-8", text);
    }

    void handleConnection(ServerSocket s) {
        SocketConnection connection(s);
        std::string data;
        size_t headEnd = std::string::npos;
        char buffer[4096];
        while (headEnd == std::string::npos) {
            if (data.size() > MAX_HEAD_SIZE) {
                connection.sendError(413, "request head too large");
                return;
            }
            int ready = waitReadable(s, 10000);
            if (ready <= 0 || stopping) {
                return;
            }
            int received = recv(s, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                return;
            }
            data.append(buffer, received);
            headEnd = data.find("\r\n\r\n");
        }
        LocalServer::Request request;
        if (!LocalServer::parseRequestHead(data.substr(0, headEnd), request)) {
            connection.sendError(400, "malformed request");
            return;
        }
        request.body = data.substr(headEnd + 4);
        auto contentLength = request.headers.find("content-length");
        if (contentLength != request.headers.end()) {
            size_t len = (size_t)strtoull(contentLength->second.c_str(), nullptr, 10);
            if (len > MAX_BODY_SIZE) {
                connection.sendError(413, "request body too large");
                return;
            }
            while (request.body.size() < len) {
                int ready = waitReadable(s, 10000);
                if (ready <= 0 || stopping) {
                    return;
                }
                int received = recv(s, buffer, (int)std::min<size_t>(sizeof(buffer), len - request.body.size()), 0);
                if (received <= 0) {
                    return;
                }
                request.body.append(buffer, received);
            }
            request.body.resize(len);
        }

        // Browsers send Origin with every WebSocket and cross-site request, and
        // WebSockets skip CORS, so without this any web page could read the
        // screen. Checking Host also stops DNS rebinding. Command-line tools
        // send a loopback Host and no Origin.
        auto host = request.headers.find("host");
        auto origin = request.headers.find("origin");
        if ((host != request.headers.end() && !isLoopbackHost(host->second)) || (origin != request.headers.end() && !isLoopbackHost(origin->second))) {
            connection.sendError(403, "only requests from this machine's loopback address are served");
            return;
        }
        if (request.path == "/" && request.method == "GET") {
            sendIndex(connection);
            return;
        }
        LocalServer::Handler handler;
        bool pathFound = false;
        {
            std::lock_guard<std::mutex> lock(routesMutex);
            for (auto& route : routes) {
                if (route.path == request.path) {
                    pathFound = true;
                    if (route.method == request.method) {
                        handler = route.handler;
                        break;
                    }
                }
            }
        }
        if (!handler) {
            connection.sendError(pathFound ? 405 : 404, pathFound ? "method not allowed" : "no such endpoint, see /");
            return;
        }
        handler(request, connection);
        connection.finish();
        if (!connection.responded) {
            connection.sendError(500, "handler sent no response");
        }
    }

    void acceptLoop() {
        while (!stopping) {
            ServerSocket listening;
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                listening = listenSocket;
            }
            if (listening == NO_SOCKET) {
                break;
            }
            int ready = waitReadable(listening, 200);
            if (ready < 0) {
                break;
            }
            if (ready == 0) {
                continue;
            }
            ServerSocket client = accept(listening, nullptr, nullptr);
            if (client == NO_SOCKET) {
                continue;
            }
            int noDelay = 1;
            setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));
#ifdef SO_NOSIGPIPE
            int noSigPipe = 1;
            setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
#endif
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                if (stopping) {
                    closeServerSocket(client);
                    break;
                }
                clientSockets.insert(client);
                activeConnections++;
            }
            std::thread([client]() {
                handleConnection(client);
                std::lock_guard<std::mutex> lock(stateMutex);
                clientSockets.erase(client);
                closeServerSocket(client);
                activeConnections--;
                stateCond.notify_all();
            }).detach();
        }
    }
}

bool LocalServer::Request::hasParam(const std::string& name) const {
    return query.find(name) != query.end();
}

std::string LocalServer::Request::param(const std::string& name, const std::string& defaultValue) const {
    auto it = query.find(name);
    return it == query.end() ? defaultValue : it->second;
}

double LocalServer::Request::paramDouble(const std::string& name, double defaultValue) const {
    auto it = query.find(name);
    if (it == query.end() || it->second.empty()) {
        return defaultValue;
    }
    char* end = nullptr;
    double value = strtod(it->second.c_str(), &end);
    // NaN would slip through every range check a handler makes.
    return (end && !*end && std::isfinite(value)) ? value : defaultValue;
}

bool LocalServer::Request::isWebSocketUpgrade() const {
    auto upgrade = headers.find("upgrade");
    return method == "GET" && upgrade != headers.end() && toLower(upgrade->second) == "websocket";
}

bool LocalServer::Connection::sendError(U32 status, const std::string& message) {
    return sendJson(status, "{\"error\":\"" + LocalServer::jsonEscape(message) + "\"}");
}

std::string LocalServer::urlDecode(const std::string& text) {
    std::string result;
    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '+') {
            result.push_back(' ');
        } else if (c == '%' && i + 2 < text.size() && isxdigit((unsigned char)text[i + 1]) && isxdigit((unsigned char)text[i + 2])) {
            result.push_back((char)strtol(text.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        } else {
            result.push_back(c);
        }
    }
    return result;
}

std::string LocalServer::jsonEscape(const std::string& text) {
    std::string result;
    for (char c : text) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if ((unsigned char)c < 0x20) {
                char buffer[8];
                snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                result += buffer;
            } else {
                result.push_back(c);
            }
        }
    }
    return result;
}

bool LocalServer::parseRequestHead(const std::string& head, Request& request) {
    size_t lineEnd = head.find("\r\n");
    std::string requestLine = head.substr(0, lineEnd);
    size_t space1 = requestLine.find(' ');
    size_t space2 = requestLine.find(' ', space1 == std::string::npos ? 0 : space1 + 1);
    if (space1 == std::string::npos || space2 == std::string::npos || requestLine.compare(space2 + 1, 5, "HTTP/") != 0) {
        return false;
    }
    request.method = requestLine.substr(0, space1);
    std::string target = requestLine.substr(space1 + 1, space2 - space1 - 1);
    if (target.empty() || target[0] != '/') {
        return false;
    }
    size_t question = target.find('?');
    request.path = urlDecode(target.substr(0, question));
    if (question != std::string::npos) {
        std::string query = target.substr(question + 1);
        size_t pos = 0;
        while (pos <= query.size()) {
            size_t amp = query.find('&', pos);
            std::string pair = query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
            if (!pair.empty()) {
                size_t equals = pair.find('=');
                std::string name = urlDecode(pair.substr(0, equals));
                std::string value = equals == std::string::npos ? std::string() : urlDecode(pair.substr(equals + 1));
                request.query[name] = value;
            }
            if (amp == std::string::npos) {
                break;
            }
            pos = amp + 1;
        }
    }
    while (lineEnd != std::string::npos) {
        size_t start = lineEnd + 2;
        lineEnd = head.find("\r\n", start);
        std::string line = head.substr(start, lineEnd == std::string::npos ? std::string::npos : lineEnd - start);
        if (line.empty()) {
            continue;
        }
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            return false;
        }
        request.headers[toLower(trim(line.substr(0, colon)))] = trim(line.substr(colon + 1));
    }
    return true;
}

void LocalServer::addRoute(const std::string& method, const std::string& path, const std::string& description, Handler handler) {
    std::lock_guard<std::mutex> lock(routesMutex);
    for (auto& route : routes) {
        if (route.method == method && route.path == path) {
            route.description = description;
            route.handler = handler;
            return;
        }
    }
    routes.push_back({ method, path, description, handler });
}

bool LocalServer::start(U32 port, std::string& error) {
    if (running) {
        error = "already running";
        return false;
    }
    {
        // stop() gives up on handlers after 2 seconds. Clearing stopping under
        // them would keep them running, and their exit would underflow the count.
        std::lock_guard<std::mutex> lock(stateMutex);
        if (activeConnections) {
            error = "connections from the previous run are still closing";
            return false;
        }
    }
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0202, &wsaData) != 0) {
        error = "WSAStartup failed";
        return false;
    }
#endif
    ServerSocket s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == NO_SOCKET) {
        error = "socket() failed";
        return false;
    }
#ifdef WIN32
    // Another process must not be able to take the port over.
    BOOL exclusive = 1;
    setsockopt(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&exclusive, sizeof(exclusive));
#else
    int reuse = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((U16)port);
    if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
        closeServerSocket(s);
        error = "could not bind 127.0.0.1:" + std::to_string(port);
        return false;
    }
    if (listen(s, 16) != 0) {
        closeServerSocket(s);
        error = "listen() failed";
        return false;
    }
    sockaddr_in bound = {};
    socklen_t boundLen = sizeof(bound);
    getsockname(s, (sockaddr*)&bound, &boundLen);
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        listenSocket = s;
        boundPort = ntohs(bound.sin_port);
        stopping = false;
        running = true;
    }
    acceptThread = std::thread(acceptLoop);
    return true;
}

void LocalServer::stop() {
    if (!running) {
        return;
    }
    stopping = true;
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    std::unique_lock<std::mutex> lock(stateMutex);
    closeServerSocket(listenSocket);
    listenSocket = NO_SOCKET;
    for (ServerSocket client : clientSockets) {
        shutdown(client, SHUTDOWN_BOTH);
    }
    // Handlers notice stopping within their next wait. Don't hang shutdown on a stuck one.
    stateCond.wait_for(lock, std::chrono::seconds(2), []() {
        return activeConnections == 0;
    });
    running = false;
}

bool LocalServer::isRunning() {
    return running;
}

bool LocalServer::isStopping() {
    return stopping;
}

U32 LocalServer::port() {
    return boundPort;
}

#endif
