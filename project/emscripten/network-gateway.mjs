import { createHash, scryptSync, timingSafeEqual } from "node:crypto";
import { createSocket as createDatagramSocket } from "node:dgram";
import { readFileSync } from "node:fs";
import { createServer as createHttpServer } from "node:http";
import { connect as connectTcp, createServer as createTcpServer } from "node:net";

let host = "127.0.0.1";
let port = 8001;
let path = "/boxedwine-network";
let debug = false;
let dashboardAuthSpec = process.env.BOXEDWINE_GATEWAY_AUTH || "";
const allowRules = [
    { host: "127.0.0.1", port: "*" },
    { host: "localhost", port: "*" },
];
let roomSubnet = "10.0.3.";
let nextRoomHost = 2;
const mdnsMulticastHost = "224.0.0.251";
const mdnsPort = 5353;
const maxSignalPayloadBytes = 64 * 1024;
const roomPeers = new Set();
const roomPeersByIp = new Map();
const roomTcpListeners = new Map();
const startedAt = Date.now();
const maxActivityEntries = 200;
const activity = [];
const dashboardHtml = readFileSync(new URL("./network-gateway/index.html", import.meta.url));
const gatewayStats = {
    hostTcpConnections: 0,
    hostTcpBytesFromPeer: 0,
    hostTcpBytesToPeer: 0,
    hostUdpDatagramsFromPeer: 0,
    hostUdpBytesFromPeer: 0,
    hostUdpDatagramsToPeer: 0,
    hostUdpBytesToPeer: 0,
    roomTcpConnections: 0,
    roomTcpBytes: 0,
    roomUdpDatagrams: 0,
    roomUdpBytes: 0,
    roomMdnsDatagrams: 0,
    roomMdnsBytes: 0,
    roomSignalsRelayed: 0,
    roomSignalBytes: 0,
    roomSignalErrors: 0,
    directTcpBytes: 0,
    directUdpBytes: 0,
};

function recordActivity(type, message, detail = {}) {
    activity.push({
        at: new Date().toISOString(),
        type,
        message,
        detail,
    });
    if (activity.length > maxActivityEntries) {
        activity.splice(0, activity.length - maxActivityEntries);
    }
}

function resetStats() {
    for (const key of Object.keys(gatewayStats)) {
        gatewayStats[key] = 0;
    }
}

for (let i = 2; i < process.argv.length; i += 1) {
    const argument = process.argv[i];
    if (argument === "--host") {
        host = process.argv[++i];
    } else if (argument === "--port") {
        port = Number(process.argv[++i]);
    } else if (argument === "--path") {
        path = process.argv[++i];
    } else if (argument === "--allow") {
        allowRules.push(parseAllowRule(process.argv[++i]));
    } else if (argument === "--allow-all") {
        allowRules.push({ host: "*", port: "*" });
    } else if (argument === "--room-subnet") {
        roomSubnet = process.argv[++i];
    } else if (argument === "--debug") {
        debug = true;
    } else if (argument === "--auth") {
        dashboardAuthSpec = process.argv[++i] || "";
    } else {
        throw new Error(`Unknown argument: ${argument}`);
    }
}

if (!/^\d+\.\d+\.\d+\.$/.test(roomSubnet)) {
    throw new Error(`Invalid room subnet "${roomSubnet}", expected dotted /24 prefix such as 10.0.3.`);
}

const dashboardAuth = parseDashboardAuth(dashboardAuthSpec);

function parseAllowRule(rule) {
    const split = rule.lastIndexOf(":");
    if (split <= 0 || split === rule.length - 1) {
        throw new Error(`Invalid allow rule "${rule}", expected host:port`);
    }
    const allowedHost = rule.substring(0, split);
    const allowedPort = rule.substring(split + 1);
    if (allowedPort !== "*" && !/^\d+$/.test(allowedPort)) {
        throw new Error(`Invalid allow rule port in "${rule}"`);
    }
    return { host: allowedHost, port: allowedPort === "*" ? "*" : Number(allowedPort) };
}

function isAllowed(targetHost, targetPort) {
    return allowRules.some((rule) => {
        const hostMatches = rule.host === "*" || rule.host === targetHost;
        const portMatches = rule.port === "*" || rule.port === targetPort;
        return hostMatches && portMatches;
    });
}

function isRoomHost(targetHost) {
    return typeof targetHost === "string" && targetHost.startsWith(roomSubnet);
}

function roomBroadcastHost() {
    return `${roomSubnet}255`;
}

function isRoomBroadcastHost(targetHost) {
    return targetHost === "255.255.255.255" || targetHost === roomBroadcastHost();
}

function isRoomMdnsMulticast(targetHost, targetPort) {
    return targetHost === mdnsMulticastHost && targetPort === mdnsPort;
}

function errnoForNodeError(error) {
    switch (error && error.code) {
        case "EACCES":
        case "EPERM":
            return 13;
        case "ETIMEDOUT":
            return 110;
        case "ECONNREFUSED":
            return 111;
        case "EHOSTUNREACH":
            return 113;
        case "ENETUNREACH":
            return 101;
        case "ECONNRESET":
            return 104;
        default:
            return 5;
    }
}

function debugLog(message) {
    if (debug) {
        console.log(message);
    }
}

function writeJson(response, status, payload) {
    const body = JSON.stringify(payload);
    response.writeHead(status, {
        "content-type": "application/json; charset=utf-8",
        "cache-control": "no-store",
        "content-length": Buffer.byteLength(body),
    });
    response.end(body);
}

function writeText(response, status, body, contentType = "text/plain; charset=utf-8") {
    response.writeHead(status, {
        "content-type": contentType,
        "cache-control": "no-store",
        "content-length": Buffer.byteLength(body),
    });
    response.end(body);
}

function parseDashboardAuth(spec) {
    if (!spec) {
        return null;
    }
    const parts = String(spec).split(":");
    if (parts.length !== 5 || !parts[0] || parts[1] !== "scrypt" || parts[2] !== "v1") {
        throw new Error("Invalid dashboard auth format. Expected username:scrypt:v1:<salt-base64>:<hash-base64>");
    }
    const salt = Buffer.from(parts[3], "base64");
    const hash = Buffer.from(parts[4], "base64");
    if (salt.length < 16 || hash.length < 32) {
        throw new Error("Invalid dashboard auth hash. Regenerate it with network-gateway-password-hash.mjs");
    }
    return {
        username: parts[0],
        salt,
        hash,
    };
}

function verifyDashboardAuth(header) {
    if (!dashboardAuth) {
        return true;
    }
    if (!header || !header.startsWith("Basic ")) {
        return false;
    }
    let decoded;
    try {
        decoded = Buffer.from(header.substring(6), "base64").toString("utf8");
    } catch {
        return false;
    }
    const split = decoded.indexOf(":");
    if (split <= 0) {
        return false;
    }
    const username = decoded.substring(0, split);
    const password = decoded.substring(split + 1);
    if (username !== dashboardAuth.username) {
        return false;
    }
    let candidate;
    try {
        candidate = scryptSync(password, dashboardAuth.salt, dashboardAuth.hash.length, {
            N: 16384,
            r: 8,
            p: 1,
        });
    } catch {
        return false;
    }
    return candidate.length === dashboardAuth.hash.length && timingSafeEqual(candidate, dashboardAuth.hash);
}

function requireDashboardAuth(request, response) {
    if (verifyDashboardAuth(request.headers.authorization || "")) {
        return true;
    }
    response.writeHead(401, {
        "www-authenticate": 'Basic realm="Boxedwine Network Gateway", charset="UTF-8"',
        "content-type": "text/plain; charset=utf-8",
        "cache-control": "no-store",
    });
    response.end("Authentication required");
    return false;
}

function encodeWebSocketFrame(opcode, payload) {
    const body = Buffer.isBuffer(payload) ? payload : Buffer.from(payload);
    let header;
    if (body.length < 126) {
        header = Buffer.alloc(2);
        header[1] = body.length;
    } else if (body.length <= 0xffff) {
        header = Buffer.alloc(4);
        header[1] = 126;
        header.writeUInt16BE(body.length, 2);
    } else {
        header = Buffer.alloc(10);
        header[1] = 127;
        header.writeBigUInt64BE(BigInt(body.length), 2);
    }
    header[0] = 0x80 | opcode;
    return Buffer.concat([header, body]);
}

function makeDataFrame(socketId, payload) {
    const header = Buffer.alloc(5);
    header[0] = 1;
    header.writeUInt32BE(socketId >>> 0, 1);
    return Buffer.concat([header, payload]);
}

function ipv4BytesToString(buffer, offset) {
    return `${buffer[offset]}.${buffer[offset + 1]}.${buffer[offset + 2]}.${buffer[offset + 3]}`;
}

function ipv4StringToBytes(host) {
    const parts = host.split(".").map((part) => Number(part));
    if (parts.length !== 4 || parts.some((part) => !Number.isInteger(part) || part < 0 || part > 255)) {
        return null;
    }
    return parts;
}

function makeUdpFrame(socketId, host, port, payload) {
    const hostBytes = ipv4StringToBytes(host);
    if (!hostBytes) {
        return null;
    }
    const header = Buffer.alloc(11);
    header[0] = 2;
    header.writeUInt32BE(socketId >>> 0, 1);
    for (let i = 0; i < 4; i += 1) {
        header[5 + i] = hostBytes[i];
    }
    header.writeUInt16BE(port & 0xffff, 9);
    return Buffer.concat([header, payload]);
}

function registerRoomPeer(peer) {
    while (nextRoomHost < 255) {
        const ip = `${roomSubnet}${nextRoomHost}`;
        nextRoomHost += 1;
        if (!roomPeersByIp.has(ip)) {
            peer.virtualIp = ip;
            roomPeers.add(peer);
            roomPeersByIp.set(ip, peer);
            debugLog(`Room peer joined ip=${ip}`);
            recordActivity("peer", `Peer joined ${ip}`, { ip });
            broadcastRoomState();
            return;
        }
    }
    throw new Error("No room virtual IPs available");
}

function unregisterRoomPeer(peer) {
    if (!peer.virtualIp || !roomPeers.has(peer)) {
        return;
    }
    debugLog(`Room peer left ip=${peer.virtualIp}`);
    recordActivity("peer", `Peer left ${peer.virtualIp}`, { ip: peer.virtualIp });
    roomPeers.delete(peer);
    roomPeersByIp.delete(peer.virtualIp);
    peer.virtualIp = null;
    broadcastRoomState();
}

function broadcastRoomState() {
    const peers = [...roomPeers]
        .filter((peer) => peer.virtualIp)
        .map((peer) => ({ ip: peer.virtualIp }));
    for (const peer of roomPeers) {
        peer.sendControl({ type: "room", ip: peer.virtualIp, peers });
    }
}

function roomTcpListenerKey(ip, port) {
    return `${ip}:${port}`;
}

function snapshotStats() {
    return {
        ...gatewayStats,
        roomPeers: roomPeers.size,
        roomTcpListeners: roomTcpListeners.size,
    };
}

function snapshotPeers() {
    return [...roomPeers]
        .filter((peer) => peer.virtualIp)
        .map((peer) => ({
            ip: peer.virtualIp,
            tcpSockets: peer.tcpSockets.size,
            virtualTcpLinks: peer.virtualTcpLinks.size,
            tcpServers: peer.tcpServers.size,
            pendingTcpAccepts: peer.pendingTcpAccepts.size,
            udpSockets: peer.udpSockets.size,
            udpBindings: [...peer.udpBindings.entries()].map(([socketId, binding]) => ({
                socketId,
                host: binding.host,
                port: binding.port,
            })),
        }));
}

function snapshotStatus() {
    return {
        ok: true,
        host,
        port,
        path,
        websocketUrl: `ws://${host}:${port}${path}`,
        httpUrl: `http://${host}:${port}/`,
        debug,
        authEnabled: !!dashboardAuth,
        authUser: dashboardAuth ? dashboardAuth.username : "",
        roomSubnet,
        roomBroadcast: roomBroadcastHost(),
        roomMdnsMulticast: `${mdnsMulticastHost}:${mdnsPort}`,
        uptimeMs: Date.now() - startedAt,
        startedAt: new Date(startedAt).toISOString(),
        allowRules,
        stats: snapshotStats(),
        peers: snapshotPeers(),
        activity: activity.slice(-80).reverse(),
    };
}

function resetGateway(reason = "manual") {
    recordActivity("reset", `Gateway reset requested (${reason})`);
    for (const peer of [...roomPeers]) {
        peer.closeAll();
        peer.socket.destroy();
    }
    roomPeers.clear();
    roomPeersByIp.clear();
    roomTcpListeners.clear();
    nextRoomHost = 2;
    resetStats();
    recordActivity("reset", "Gateway state cleared", { reason });
}

function readDataFrameSocketId(payload) {
    if (payload.length < 5 || payload[0] !== 1) {
        return null;
    }
    return readFrameSocketId(payload);
}

function readFrameSocketId(payload) {
    if (payload.length < 5) {
        return null;
    }
    return payload.readUInt32BE(1);
}

class WebSocketPeer {
    constructor(socket) {
        this.socket = socket;
        this.buffer = Buffer.alloc(0);
        this.virtualIp = null;
        this.closed = false;
        this.tcpSockets = new Map();
        this.virtualTcpLinks = new Map();
        this.tcpServers = new Map();
        this.pendingTcpAccepts = new Map();
        this.udpSockets = new Map();
        this.udpBindings = new Map();
        this.nextAcceptToken = 1;

        socket.on("data", (chunk) => this.onData(chunk));
        socket.on("close", () => this.closeAll());
        socket.on("error", () => this.closeAll());
        setImmediate(() => {
            if (!this.closed) {
                registerRoomPeer(this);
            }
        });
    }

    sendControl(message) {
        return this.socket.write(encodeWebSocketFrame(1, JSON.stringify(message)));
    }

    sendBinary(payload) {
        return this.socket.write(encodeWebSocketFrame(2, payload));
    }

    onData(chunk) {
        this.buffer = Buffer.concat([this.buffer, chunk]);
        while (this.buffer.length >= 2) {
            const first = this.buffer[0];
            const second = this.buffer[1];
            const opcode = first & 0x0f;
            const masked = (second & 0x80) !== 0;
            let payloadLength = second & 0x7f;
            let offset = 2;

            if (payloadLength === 126) {
                if (this.buffer.length < offset + 2) {
                    return;
                }
                payloadLength = this.buffer.readUInt16BE(offset);
                offset += 2;
            } else if (payloadLength === 127) {
                if (this.buffer.length < offset + 8) {
                    return;
                }
                const bigLength = this.buffer.readBigUInt64BE(offset);
                if (bigLength > BigInt(Number.MAX_SAFE_INTEGER)) {
                    this.socket.destroy();
                    return;
                }
                payloadLength = Number(bigLength);
                offset += 8;
            }

            const maskLength = masked ? 4 : 0;
            if (this.buffer.length < offset + maskLength + payloadLength) {
                return;
            }

            let payload = this.buffer.subarray(offset + maskLength, offset + maskLength + payloadLength);
            if (masked) {
                const mask = this.buffer.subarray(offset, offset + 4);
                payload = Buffer.from(payload);
                for (let i = 0; i < payload.length; i += 1) {
                    payload[i] ^= mask[i & 3];
                }
            }
            this.buffer = this.buffer.subarray(offset + maskLength + payloadLength);

            if (opcode === 0x1) {
                this.onControl(JSON.parse(payload.toString("utf8")));
            } else if (opcode === 0x2) {
                this.onBinary(payload);
            } else if (opcode === 0x8) {
                this.socket.end(encodeWebSocketFrame(0x8, Buffer.alloc(0)));
                this.closeAll();
                return;
            } else if (opcode === 0x9) {
                this.socket.write(encodeWebSocketFrame(0xa, payload));
            }
        }
    }

    onControl(message) {
        if (message.type === "open") {
            this.openTcp(message);
        } else if (message.type === "close") {
            this.closeSocket(message.id);
        } else if (message.type === "shutdown") {
            this.shutdownTcp(message.id, message.how);
        } else if (message.type === "udp-open") {
            this.openUdp(message);
        } else if (message.type === "udp-bind") {
            this.bindUdp(message);
        } else if (message.type === "listen") {
            this.listenTcp(message);
        } else if (message.type === "accept") {
            this.acceptTcp(message);
        } else if (message.type === "signal") {
            this.relaySignal(message);
        } else if (message.type === "stats") {
            this.sendControl({ type: "stats", id: message.id, stats: snapshotStats() });
        } else if (message.type === "trace") {
            this.logTrace(message);
        }
    }

    onBinary(payload) {
        if (payload.length < 5) {
            return;
        }
        if (payload[0] === 2) {
            this.onUdpDatagram(payload);
            return;
        }
        if (payload[0] !== 1) {
            return;
        }
        const socketId = readFrameSocketId(payload);
        if (socketId == null) {
            return;
        }
        const tcpSocket = this.tcpSockets.get(socketId);
        if (tcpSocket) {
            const body = payload.subarray(5);
            gatewayStats.hostTcpBytesFromPeer += body.length;
            tcpSocket.write(body);
            return;
        }
        const virtualTcpLink = this.virtualTcpLinks.get(socketId);
        if (virtualTcpLink) {
            const body = payload.subarray(5);
            gatewayStats.roomTcpBytes += body.length;
            virtualTcpLink.peer.sendBinary(makeDataFrame(virtualTcpLink.socketId, body));
        }
    }

    onUdpDatagram(payload) {
        if (payload.length < 11) {
            return;
        }
        const socketId = readFrameSocketId(payload);
        const udpSocket = this.udpSockets.get(socketId);
        if (!udpSocket) {
            return;
        }
        const targetHost = ipv4BytesToString(payload, 5);
        const targetPort = payload.readUInt16BE(9);
        const virtualTarget = roomPeersByIp.get(targetHost);
        if (virtualTarget) {
            this.forwardRoomUdp(socketId, virtualTarget, targetPort, payload.subarray(11));
            return;
        }
        if (isRoomMdnsMulticast(targetHost, targetPort)) {
            this.forwardRoomMdns(socketId, payload.subarray(11));
            return;
        }
        if (isRoomBroadcastHost(targetHost)) {
            this.forwardRoomUdpBroadcast(socketId, targetHost, targetPort, payload.subarray(11));
            return;
        }
        if (isRoomHost(targetHost)) {
            debugLog(`Dropping room UDP to departed peer from=${this.virtualIp} socket=${socketId} target=${targetHost}:${targetPort} len=${payload.length - 11}`);
            recordActivity("udp", `Dropped UDP to departed peer ${targetHost}:${targetPort}`, {
                from: this.virtualIp,
                socketId,
                bytes: payload.length - 11,
            });
            return;
        }
        if (!isAllowed(targetHost, targetPort)) {
            console.warn(`Rejecting UDP send id=${socketId} host=${targetHost} port=${targetPort}: not allowed`);
            recordActivity("error", `Rejected UDP to ${targetHost}:${targetPort}`, { socketId, reason: "not allowed" });
            this.sendControl({ type: "error", id: socketId, status: -13 });
            return;
        }
        const body = payload.subarray(11);
        console.log(`Sending UDP id=${socketId} host=${targetHost} port=${targetPort} len=${body.length}`);
        recordActivity("udp", `UDP to host ${targetHost}:${targetPort}`, { from: this.virtualIp, socketId, bytes: body.length });
        gatewayStats.hostUdpDatagramsFromPeer += 1;
        gatewayStats.hostUdpBytesFromPeer += body.length;
        udpSocket.send(body, targetPort, targetHost, (error) => {
            if (error) {
                console.warn(`UDP send error id=${socketId} host=${targetHost} port=${targetPort} code=${error && error.code ? error.code : "unknown"}`);
                recordActivity("error", `UDP send error ${targetHost}:${targetPort}`, { socketId, code: error && error.code ? error.code : "unknown" });
                this.sendControl({ type: "error", id: socketId, status: -errnoForNodeError(error) });
            }
        });
    }

    forwardRoomUdpBroadcast(sourceSocketId, targetHost, targetPort, body) {
        let deliveredPeers = 0;
        for (const targetPeer of roomPeers) {
            if (targetPeer === this || targetPeer.closed || !targetPeer.virtualIp) {
                continue;
            }
            deliveredPeers += 1;
            this.forwardRoomUdp(sourceSocketId, targetPeer, targetPort, body);
        }
        debugLog(`Room UDP broadcast from=${this.virtualIp} socket=${sourceSocketId} target=${targetHost}:${targetPort} peers=${deliveredPeers} len=${body.length}`);
        recordActivity("udp", `Room UDP broadcast to ${targetHost}:${targetPort}`, {
            from: this.virtualIp,
            socketId: sourceSocketId,
            peers: deliveredPeers,
            bytes: body.length,
        });
    }

    forwardRoomMdns(sourceSocketId, body) {
        let deliveredPeers = 0;
        for (const targetPeer of roomPeers) {
            if (targetPeer === this || targetPeer.closed || !targetPeer.virtualIp) {
                continue;
            }
            deliveredPeers += 1;
            this.forwardRoomUdp(sourceSocketId, targetPeer, mdnsPort, body);
        }
        debugLog(`Room mDNS from=${this.virtualIp} socket=${sourceSocketId} target=${mdnsMulticastHost}:${mdnsPort} peers=${deliveredPeers} len=${body.length}`);
        recordActivity("udp", `Room mDNS multicast to ${mdnsMulticastHost}:${mdnsPort}`, {
            from: this.virtualIp,
            socketId: sourceSocketId,
            peers: deliveredPeers,
            bytes: body.length,
        });
        gatewayStats.roomMdnsDatagrams += 1;
        gatewayStats.roomMdnsBytes += body.length;
    }

    forwardRoomUdp(sourceSocketId, targetPeer, targetPort, body) {
        const sourceBinding = this.udpBindings.get(sourceSocketId);
        const sourcePort = sourceBinding && sourceBinding.port ? sourceBinding.port : targetPort;
        const boundTargetSocketIds = [];
        for (const [targetSocketId, binding] of targetPeer.udpBindings.entries()) {
            if (binding.port === targetPort && targetPeer.udpSockets.has(targetSocketId)) {
                boundTargetSocketIds.push(targetSocketId);
            }
        }
        const targetSocketIds = boundTargetSocketIds.length > 0
            ? boundTargetSocketIds
            : [...targetPeer.udpSockets.keys()].filter((targetSocketId) => !targetPeer.udpBindings.has(targetSocketId));
        if (targetSocketIds.length === 0) {
            debugLog(`Room UDP target unavailable from=${this.virtualIp} sourceSocket=${sourceSocketId} target=${targetPeer.virtualIp}:${targetPort}`);
            recordActivity("udp", `Room UDP target unavailable ${targetPeer.virtualIp}:${targetPort}`, {
                from: this.virtualIp,
                socketId: sourceSocketId,
            });
            return;
        }
        for (const targetSocketId of targetSocketIds) {
            const frame = makeUdpFrame(targetSocketId, this.virtualIp || "0.0.0.0", sourcePort, body);
            if (frame) {
                debugLog(`Delivering room UDP from=${this.virtualIp}:${sourcePort} sourceSocket=${sourceSocketId} to=${targetPeer.virtualIp}:${targetPort} targetSocket=${targetSocketId} frameLen=${frame.length} writable=${targetPeer.socket.writable ? 1 : 0} buffered=${targetPeer.socket.writableLength}`);
                const accepted = targetPeer.sendBinary(frame);
                if (!accepted) {
                    debugLog(`Room UDP delivery queued target=${targetPeer.virtualIp} targetSocket=${targetSocketId} buffered=${targetPeer.socket.writableLength}`);
                    targetPeer.socket.once("drain", () => {
                        debugLog(`Room UDP delivery drained target=${targetPeer.virtualIp} targetSocket=${targetSocketId}`);
                    });
                }
                gatewayStats.roomUdpDatagrams += 1;
                gatewayStats.roomUdpBytes += body.length;
            }
        }
        debugLog(`Room UDP from=${this.virtualIp}:${sourcePort} socket=${sourceSocketId} to=${targetPeer.virtualIp}:${targetPort} sockets=${targetSocketIds.join(",")} len=${body.length}`);
        recordActivity("udp", `Room UDP ${this.virtualIp}:${sourcePort} -> ${targetPeer.virtualIp}:${targetPort}`, {
            sourceSocketId,
            targetSocketIds,
            bytes: body.length,
        });
    }

    openTcp(message) {
        const socketId = Number(message.id);
        const targetHost = String(message.host || "");
        const targetPort = Number(message.port);

        if (!Number.isInteger(socketId) || !Number.isInteger(targetPort) || targetPort <= 0 || targetPort > 65535) {
            console.warn(`Rejecting TCP open id=${socketId} host=${targetHost} port=${targetPort}: invalid target`);
            this.sendControl({ type: "open", id: socketId, status: -22 });
            return;
        }
        const virtualTarget = roomPeersByIp.get(targetHost);
        if (virtualTarget) {
            this.openRoomTcp(socketId, virtualTarget, targetPort);
            return;
        }
        if (!isAllowed(targetHost, targetPort)) {
            console.warn(`Rejecting TCP open id=${socketId} host=${targetHost} port=${targetPort}: not allowed`);
            recordActivity("error", `Rejected TCP open to ${targetHost}:${targetPort}`, { socketId, reason: "not allowed" });
            this.sendControl({ type: "open", id: socketId, status: -13 });
            return;
        }

        console.log(`Opening TCP id=${socketId} host=${targetHost} port=${targetPort}`);
        recordActivity("tcp", `Opening TCP ${targetHost}:${targetPort}`, { from: this.virtualIp, socketId });
        const tcpSocket = connectTcp({ host: targetHost, port: targetPort });
        this.attachTcpSocket(socketId, tcpSocket, `host=${targetHost} port=${targetPort}`);
        tcpSocket.on("connect", () => {
            gatewayStats.hostTcpConnections += 1;
            console.log(`Opened TCP id=${socketId} host=${targetHost} port=${targetPort}`);
            recordActivity("tcp", `Opened TCP ${targetHost}:${targetPort}`, { from: this.virtualIp, socketId });
            this.sendControl({ type: "open", id: socketId, status: 0 });
        });
    }

    openRoomTcp(socketId, targetPeer, targetPort) {
        const listener = roomTcpListeners.get(roomTcpListenerKey(targetPeer.virtualIp, targetPort));
        if (!listener || listener.peer !== targetPeer) {
            console.warn(`Rejecting room TCP open from=${this.virtualIp} id=${socketId} target=${targetPeer.virtualIp}:${targetPort}: no listener`);
            recordActivity("error", `Rejected room TCP to ${targetPeer.virtualIp}:${targetPort}`, { from: this.virtualIp, socketId, reason: "no listener" });
            this.sendControl({ type: "open", id: socketId, status: -111 });
            return;
        }
        const token = targetPeer.nextAcceptToken++;
        targetPeer.pendingTcpAccepts.set(token, {
            kind: "virtual",
            listenerId: listener.socketId,
            openerPeer: this,
            openerSocketId: socketId,
            host: this.virtualIp || "0.0.0.0",
            port: 0,
        });
        console.log(`Pending room TCP accept listener=${listener.socketId} token=${token} from=${this.virtualIp} socket=${socketId} target=${targetPeer.virtualIp}:${targetPort}`);
        recordActivity("tcp", `Pending room TCP ${this.virtualIp} -> ${targetPeer.virtualIp}:${targetPort}`, { socketId, listenerSocketId: listener.socketId });
        targetPeer.sendControl({ type: "pending", id: listener.socketId, token, host: this.virtualIp || "0.0.0.0", port: 0 });
    }

    attachTcpSocket(socketId, tcpSocket, context) {
        this.tcpSockets.set(socketId, tcpSocket);
        tcpSocket.setNoDelay(true);
        tcpSocket.on("data", (chunk) => {
            gatewayStats.hostTcpBytesToPeer += chunk.length;
            this.sendBinary(makeDataFrame(socketId, chunk));
        });
        tcpSocket.on("end", () => {
            this.sendControl({ type: "close", id: socketId });
        });
        tcpSocket.on("close", () => {
            this.tcpSockets.delete(socketId);
            this.sendControl({ type: "close", id: socketId });
        });
        tcpSocket.on("error", (error) => {
            console.warn(`TCP error id=${socketId} ${context} code=${error && error.code ? error.code : "unknown"}`);
            recordActivity("error", `TCP error ${context}`, { socketId, code: error && error.code ? error.code : "unknown" });
            this.tcpSockets.delete(socketId);
            this.sendControl({ type: "error", id: socketId, status: -errnoForNodeError(error) });
        });
    }

    shutdownTcp(socketId, how) {
        const tcpSocket = this.tcpSockets.get(Number(socketId));
        if (!tcpSocket) {
            return;
        }
        if (how === 1) {
            tcpSocket.end();
        } else {
            tcpSocket.destroy();
        }
    }

    closeTcp(socketId) {
        socketId = Number(socketId);
        const virtualTcpLink = this.virtualTcpLinks.get(socketId);
        if (virtualTcpLink) {
            this.virtualTcpLinks.delete(socketId);
            virtualTcpLink.peer.virtualTcpLinks.delete(virtualTcpLink.socketId);
            virtualTcpLink.peer.sendControl({ type: "close", id: virtualTcpLink.socketId });
        }
        const tcpSocket = this.tcpSockets.get(socketId);
        if (tcpSocket) {
            tcpSocket.destroy();
            this.tcpSockets.delete(socketId);
        }
    }

    listenTcp(message) {
        const socketId = Number(message.id);
        const listenHost = String(message.host || "");
        const listenPort = Number(message.port);
        const backlog = Number(message.backlog || 1);

        if (!Number.isInteger(socketId) || !Number.isInteger(listenPort) || listenPort <= 0 || listenPort > 65535) {
            console.warn(`Rejecting TCP listen id=${socketId} host=${listenHost} port=${listenPort}: invalid target`);
            this.sendControl({ type: "listen", id: socketId, status: -22 });
            return;
        }
        const isRoomListen = listenHost === this.virtualIp || listenHost === "0.0.0.0";
        if (isRoomListen) {
            const key = roomTcpListenerKey(this.virtualIp, listenPort);
            roomTcpListeners.set(key, { peer: this, socketId, host: this.virtualIp, port: listenPort, backlog });
            console.log(`Listening room TCP id=${socketId} local=${this.virtualIp}:${listenPort}`);
            recordActivity("tcp", `Listening room TCP ${this.virtualIp}:${listenPort}`, { socketId, backlog });
            this.sendControl({ type: "listen", id: socketId, status: 0, host: this.virtualIp, port: listenPort });
            return;
        }
        if (listenHost !== "127.0.0.1" && listenHost !== "localhost") {
            console.warn(`Rejecting TCP listen id=${socketId} host=${listenHost} port=${listenPort}: unsupported listen host`);
            this.sendControl({ type: "listen", id: socketId, status: -97 });
            return;
        }
        if (this.tcpServers.has(socketId)) {
            this.sendControl({ type: "listen", id: socketId, status: 0, host: listenHost, port: listenPort });
            return;
        }

        const server = createTcpServer((tcpSocket) => {
            const token = this.nextAcceptToken++;
            const remoteHost = tcpSocket.remoteAddress && tcpSocket.remoteAddress.startsWith("::ffff:")
                ? tcpSocket.remoteAddress.substring(7)
                : (tcpSocket.remoteAddress || "0.0.0.0");
            const remotePort = tcpSocket.remotePort || 0;
            tcpSocket.pause();
            this.pendingTcpAccepts.set(token, {
                listenerId: socketId,
                socket: tcpSocket,
                host: remoteHost,
                port: remotePort,
            });
            debugLog(`Pending TCP accept listener=${socketId} token=${token} from=${remoteHost}:${remotePort}`);
            this.sendControl({ type: "pending", id: socketId, token, host: remoteHost, port: remotePort });
        });

        server.on("error", (error) => {
            console.warn(`TCP listen error id=${socketId} host=${listenHost} port=${listenPort} code=${error && error.code ? error.code : "unknown"}`);
            this.tcpServers.delete(socketId);
            this.sendControl({ type: "listen", id: socketId, status: -errnoForNodeError(error) });
        });

        this.tcpServers.set(socketId, server);
        server.listen({ host: listenHost, port: listenPort, backlog }, () => {
            const address = server.address();
            debugLog(`Listening TCP id=${socketId} local=${address.address}:${address.port}`);
            recordActivity("tcp", `Listening TCP ${address.address}:${address.port}`, { socketId, backlog });
            this.sendControl({ type: "listen", id: socketId, status: 0, host: address.address, port: address.port });
        });
    }

    acceptTcp(message) {
        const listenerId = Number(message.id);
        const token = Number(message.token);
        const acceptedId = Number(message.acceptedId);
        const pending = this.pendingTcpAccepts.get(token);
        if (!pending || pending.listenerId !== listenerId || !Number.isInteger(acceptedId)) {
            console.warn(`Rejecting TCP accept listener=${listenerId} token=${token} accepted=${acceptedId}: no pending connection`);
            this.sendControl({ type: "accept", id: listenerId, acceptedId, status: -11 });
            return;
        }

        this.pendingTcpAccepts.delete(token);
        if (pending.kind === "virtual") {
            this.virtualTcpLinks.set(acceptedId, { peer: pending.openerPeer, socketId: pending.openerSocketId });
            pending.openerPeer.virtualTcpLinks.set(pending.openerSocketId, { peer: this, socketId: acceptedId });
            pending.openerPeer.sendControl({ type: "open", id: pending.openerSocketId, status: 0 });
            gatewayStats.roomTcpConnections += 1;
            console.log(`Accepted room TCP listener=${listenerId} token=${token} accepted=${acceptedId} opener=${pending.openerPeer.virtualIp} socket=${pending.openerSocketId}`);
            recordActivity("tcp", `Accepted room TCP from ${pending.openerPeer.virtualIp}`, { listenerId, acceptedId, openerSocketId: pending.openerSocketId });
            this.sendControl({ type: "accept", id: listenerId, acceptedId, status: 0, host: pending.host, port: pending.port });
            return;
        }
        this.attachTcpSocket(acceptedId, pending.socket, `accepted listener=${listenerId} token=${token}`);
        pending.socket.resume();
        debugLog(`Accepted TCP listener=${listenerId} token=${token} accepted=${acceptedId}`);
        this.sendControl({ type: "accept", id: listenerId, acceptedId, status: 0, host: pending.host, port: pending.port });
    }

    relaySignal(message) {
        const targetIp = String(message.to || "");
        const targetPeer = roomPeersByIp.get(targetIp);
        if (!this.virtualIp || !targetPeer) {
            console.warn(`Rejecting room signal from=${this.virtualIp || "unassigned"} to=${targetIp}: no target`);
            recordActivity("error", `Rejected room signal to ${targetIp}`, { from: this.virtualIp || "unassigned", reason: "no target" });
            gatewayStats.roomSignalErrors += 1;
            this.sendControl({ type: "signal-error", to: targetIp, status: -113 });
            return;
        }
        if (targetPeer === this) {
            gatewayStats.roomSignalErrors += 1;
            this.sendControl({ type: "signal-error", to: targetIp, status: -22 });
            return;
        }

        const payload = message.payload === undefined ? null : message.payload;
        const payloadText = JSON.stringify(payload);
        if (Buffer.byteLength(payloadText, "utf8") > maxSignalPayloadBytes) {
            console.warn(`Rejecting room signal from=${this.virtualIp} to=${targetIp}: payload too large`);
            gatewayStats.roomSignalErrors += 1;
            this.sendControl({ type: "signal-error", to: targetIp, status: -90 });
            return;
        }

        const signalType = String(message.signalType || "signal");
        targetPeer.sendControl({
            type: "signal",
            from: this.virtualIp,
            signalType,
            payload,
        });
        const payloadBytes = Buffer.byteLength(payloadText, "utf8");
        gatewayStats.roomSignalsRelayed += 1;
        gatewayStats.roomSignalBytes += payloadBytes;
        debugLog(`Relayed room signal from=${this.virtualIp} to=${targetIp} signalType=${signalType} bytes=${payloadBytes}`);
        recordActivity("signal", `Relayed ${signalType} ${this.virtualIp} -> ${targetIp}`, { bytes: payloadBytes });
    }

    logTrace(message) {
        const event = String(message.event || "unknown");
        const detail = String(message.detail || "");
        debugLog(`Peer trace ip=${this.virtualIp || "unassigned"} event=${event}${detail ? ` ${detail}` : ""}`);
    }

    openUdp(message) {
        const socketId = Number(message.id);
        if (!Number.isInteger(socketId)) {
            this.sendControl({ type: "error", id: socketId, status: -22 });
            return;
        }
        if (this.udpSockets.has(socketId)) {
            return;
        }

        const udpSocket = createDatagramSocket("udp4");
        this.udpSockets.set(socketId, udpSocket);

        udpSocket.on("message", (messageBuffer, rinfo) => {
            const frame = makeUdpFrame(socketId, rinfo.address, rinfo.port, messageBuffer);
            if (frame) {
                gatewayStats.hostUdpDatagramsToPeer += 1;
                gatewayStats.hostUdpBytesToPeer += messageBuffer.length;
                this.sendBinary(frame);
            }
        });
        udpSocket.on("error", (error) => {
            console.warn(`UDP error id=${socketId} code=${error && error.code ? error.code : "unknown"}`);
            recordActivity("error", "UDP socket error", { socketId, code: error && error.code ? error.code : "unknown" });
            this.udpSockets.delete(socketId);
            udpSocket.close();
            this.sendControl({ type: "error", id: socketId, status: -errnoForNodeError(error) });
        });
        udpSocket.bind(0, () => {
            const address = udpSocket.address();
            debugLog(`Opened UDP id=${socketId} local=${address.address}:${address.port}`);
            recordActivity("udp", `Opened UDP socket ${socketId}`, { ip: this.virtualIp, address: address.address, port: address.port });
            this.sendControl({ type: "udp-open", id: socketId, status: 0 });
        });
    }

    bindUdp(message) {
        const socketId = Number(message.id);
        const bindHost = String(message.host || "0.0.0.0");
        const bindPort = Number(message.port || 0);
        if (!Number.isInteger(socketId) || !Number.isInteger(bindPort) || bindPort < 0 || bindPort > 65535) {
            this.sendControl({ type: "udp-bind", id: socketId, status: -22 });
            return;
        }
        if (!this.udpSockets.has(socketId)) {
            this.sendControl({ type: "udp-bind", id: socketId, status: -107 });
            return;
        }
        this.udpBindings.set(socketId, { host: bindHost, port: bindPort });
        debugLog(`Bound UDP id=${socketId} virtual=${this.virtualIp || "0.0.0.0"} local=${bindHost}:${bindPort}`);
        recordActivity("udp", `Bound UDP ${this.virtualIp || "0.0.0.0"}:${bindPort}`, { socketId, host: bindHost, port: bindPort });
        this.sendControl({ type: "udp-bind", id: socketId, status: 0, host: bindHost, port: bindPort });
    }

    closeUdp(socketId) {
        const udpSocket = this.udpSockets.get(Number(socketId));
        if (udpSocket) {
            udpSocket.close();
            this.udpSockets.delete(Number(socketId));
        }
        this.udpBindings.delete(Number(socketId));
    }

    closeSocket(socketId) {
        this.closeTcp(socketId);
        this.closeUdp(socketId);
        const server = this.tcpServers.get(Number(socketId));
        if (server) {
            server.close();
            this.tcpServers.delete(Number(socketId));
        }
        for (const [key, listener] of roomTcpListeners.entries()) {
            if (listener.peer === this && listener.socketId === Number(socketId)) {
                roomTcpListeners.delete(key);
            }
        }
        for (const [token, pending] of this.pendingTcpAccepts.entries()) {
            if (pending.listenerId === Number(socketId)) {
                if (pending.socket) {
                    pending.socket.destroy();
                }
                this.pendingTcpAccepts.delete(token);
            }
        }
    }

    closeAll() {
        if (this.closed) {
            return;
        }
        this.closed = true;
        unregisterRoomPeer(this);
        for (const tcpSocket of this.tcpSockets.values()) {
            tcpSocket.destroy();
        }
        this.tcpSockets.clear();
        for (const [socketId, virtualTcpLink] of this.virtualTcpLinks.entries()) {
            virtualTcpLink.peer.virtualTcpLinks.delete(virtualTcpLink.socketId);
            virtualTcpLink.peer.sendControl({ type: "close", id: virtualTcpLink.socketId });
            this.virtualTcpLinks.delete(socketId);
        }
        for (const server of this.tcpServers.values()) {
            server.close();
        }
        this.tcpServers.clear();
        for (const [key, listener] of roomTcpListeners.entries()) {
            if (listener.peer === this) {
                roomTcpListeners.delete(key);
            }
        }
        for (const pending of this.pendingTcpAccepts.values()) {
            if (pending.socket) {
                pending.socket.destroy();
            }
        }
        this.pendingTcpAccepts.clear();
        for (const udpSocket of this.udpSockets.values()) {
            udpSocket.close();
        }
        this.udpSockets.clear();
        this.udpBindings.clear();
    }
}

const server = createHttpServer((request, response) => {
    const { pathname } = new URL(request.url || "/", `http://${host}:${port}`);
    if (request.method === "GET" && (pathname === "/" || pathname === "/index.html")) {
        if (!requireDashboardAuth(request, response)) {
            return;
        }
        response.writeHead(200, {
            "content-type": "text/html; charset=utf-8",
            "cache-control": "no-store",
            "content-length": dashboardHtml.length,
        });
        response.end(dashboardHtml);
        return;
    }
    if (request.method === "GET" && pathname === "/api/status") {
        if (!requireDashboardAuth(request, response)) {
            return;
        }
        writeJson(response, 200, snapshotStatus());
        return;
    }
    if (request.method === "POST" && pathname === "/api/reset") {
        if (!requireDashboardAuth(request, response)) {
            return;
        }
        resetGateway("dashboard");
        writeJson(response, 200, snapshotStatus());
        return;
    }
    writeText(response, 404, "Boxedwine network gateway");
});

server.on("upgrade", (request, socket) => {
    const { pathname } = new URL(request.url || "/", "http://localhost");
    if (pathname !== path) {
        socket.destroy();
        return;
    }
    const key = request.headers["sec-websocket-key"];
    if (!key) {
        socket.destroy();
        return;
    }
    const accept = createHash("sha1")
        .update(`${key}258EAFA5-E914-47DA-95CA-C5AB0DC85B11`)
        .digest("base64");

    socket.write([
        "HTTP/1.1 101 Switching Protocols",
        "Upgrade: websocket",
        "Connection: Upgrade",
        `Sec-WebSocket-Accept: ${accept}`,
        "",
        "",
    ].join("\r\n"));
    new WebSocketPeer(socket);
});

server.listen(port, host, () => {
    const allowText = allowRules.map((rule) => `${rule.host}:${rule.port}`).join(", ");
    console.log(`Boxedwine network gateway listening at ws://${host}:${port}${path}`);
    console.log(`Dashboard available at http://${host}:${port}/`);
    if (dashboardAuth) {
        console.log(`Dashboard authentication enabled for user: ${dashboardAuth.username}`);
    } else {
        console.warn("Dashboard authentication disabled. Set BOXEDWINE_GATEWAY_AUTH to protect it.");
    }
    console.log(`Allowed TCP/UDP destinations: ${allowText}`);
});
