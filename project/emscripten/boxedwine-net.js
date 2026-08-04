addToLibrary({
  $BoxedWineNetwork: {
    POLLIN: 0x001,
    POLLOUT: 0x004,
    POLLERR: 0x008,
    POLLHUP: 0x010,
    EAGAIN: 11,
    EIO: 5,
    EINVAL: 22,
    EAFNOSUPPORT: 97,
    EADDRINUSE: 98,
    ENETUNREACH: 101,
    ECONNRESET: 104,
    EISCONN: 106,
    ENOTCONN: 107,
    ESHUTDOWN: 108,
    EDESTADDRREQ: 89,
    EALREADY: 114,
    EINPROGRESS: 115,
    MSG_PEEK: 0x2,

    nextSocket: 1,
    sockets: {},
    cachedTransport: null,

    config: function () {
      return Module["boxedwineNetworking"];
    },

    debugEnabled: function () {
      var config = BoxedWineNetwork.config();
      return !!(config && config["debug"]);
    },

    previewBytes: function (bytes) {
      var limit = Math.min(bytes.length, 32);
      var parts = [];
      for (var i = 0; i < limit; i++) {
        parts.push((bytes[i] < 16 ? "0" : "") + bytes[i].toString(16));
      }
      if (bytes.length > limit) {
        parts.push("...");
      }
      return parts.join(" ");
    },

    debugLog: function (message) {
      if (BoxedWineNetwork.debugEnabled()) {
        console.log("[boxedwine-net] " + message);
      }
    },

    enabled: function () {
      var config = BoxedWineNetwork.config();
      return config === true || !!(config && config["enabled"]);
    },

    transport: function () {
      var config = BoxedWineNetwork.config();
      if (!config) {
        return null;
      }
      if (config["transport"]) {
        return config["transport"];
      }
      if (config["gatewayUrl"]) {
        if (!BoxedWineNetwork.cachedTransport || BoxedWineNetwork.cachedTransport.gatewayUrl !== config["gatewayUrl"]) {
          BoxedWineNetwork.cachedTransport = BoxedWineNetwork.createWebSocketTcpTransport(config["gatewayUrl"]);
        }
        return BoxedWineNetwork.cachedTransport;
      }
      return null;
    },

    allocateSocket: function (domain, type, protocol) {
      var handle = BoxedWineNetwork.nextSocket++;
      BoxedWineNetwork.sockets[handle] = {
        domain: domain,
        type: type,
        protocol: protocol,
        events: 0,
        error: 0,
        readableBytes: 0,
      };
      return handle;
    },

    socketState: function (handle) {
      return BoxedWineNetwork.sockets[handle] || null;
    },

    notify: function (handle) {
      var notify = Module["_boxedwine_browser_socket_notify"];
      if (typeof notify === "function") {
        notify(handle);
      }
    },

    setEvents: function (handle, events, readableBytes) {
      var socket = BoxedWineNetwork.socketState(handle);
      if (!socket) {
        return;
      }
      socket.events = events;
      if (typeof readableBytes === "number") {
        socket.readableBytes = readableBytes;
      }
      BoxedWineNetwork.notify(handle);
    },

    unavailable: function (handle) {
      var socket = BoxedWineNetwork.socketState(handle);
      if (socket) {
        socket.error = 101;
      }
      return -101;
    },

    callTransport: function (name, args, fallback) {
      var transport = BoxedWineNetwork.transport();
      var fn = transport && transport[name];
      if (typeof fn === "function") {
        return fn.apply(transport, args);
      }
      if (typeof fallback === "function") {
        return fallback();
      }
      return fallback;
    },

    makeDataFrame: function (socketId, data) {
      var frame = new Uint8Array(5 + data.length);
      frame[0] = 1;
      frame[1] = (socketId >>> 24) & 0xff;
      frame[2] = (socketId >>> 16) & 0xff;
      frame[3] = (socketId >>> 8) & 0xff;
      frame[4] = socketId & 0xff;
      frame.set(data, 5);
      return frame;
    },

    makeUdpFrame: function (socketId, ipv4, port, data) {
      var frame = new Uint8Array(11 + data.length);
      frame[0] = 2;
      frame[1] = (socketId >>> 24) & 0xff;
      frame[2] = (socketId >>> 16) & 0xff;
      frame[3] = (socketId >>> 8) & 0xff;
      frame[4] = socketId & 0xff;
      frame[5] = ipv4 & 0xff;
      frame[6] = (ipv4 >>> 8) & 0xff;
      frame[7] = (ipv4 >>> 16) & 0xff;
      frame[8] = (ipv4 >>> 24) & 0xff;
      frame[9] = (port >>> 8) & 0xff;
      frame[10] = port & 0xff;
      frame.set(data, 11);
      return frame;
    },

    readFrameSocketId: function (data) {
      return ((data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4]) >>> 0;
    },

    ipv4ToString: function (ipv4) {
      return [
        ipv4 & 0xff,
        (ipv4 >>> 8) & 0xff,
        (ipv4 >>> 16) & 0xff,
        (ipv4 >>> 24) & 0xff,
      ].join(".");
    },

    ipv4FromString: function (host) {
      var parts = String(host || "").split(".");
      if (parts.length !== 4) {
        return 0;
      }
      var value = 0;
      for (var i = 0; i < 4; i++) {
        var part = Number(parts[i]);
        if (!Number.isFinite(part) || part < 0 || part > 255 || Math.floor(part) !== part) {
          return 0;
        }
        value |= part << (i * 8);
      }
      return value >>> 0;
    },

    copyFromHeap: function (buffer, len) {
      return HEAPU8.slice(buffer, buffer + len);
    },

    createWebSocketTcpTransport: function (gatewayUrl) {
      var transport = {
        gatewayUrl: gatewayUrl,
        ws: null,
        wsOpen: false,
        wsError: 0,
        controlQueue: [],
        dataQueue: [],
        sockets: {},
        connecting: false,
        highWaterMark: 1024 * 1024,

        ensureGateway: function () {
          if (this.ws || this.connecting) {
            return;
          }
          this.connecting = true;
          try {
            var ws = new WebSocket(this.gatewayUrl);
            ws.binaryType = "arraybuffer";
            this.ws = ws;
            ws.onopen = function () {
              transport.connecting = false;
              transport.wsOpen = true;
              transport.flushQueues();
              Object.keys(transport.sockets).forEach(function (handle) {
                transport.setEvents(transport.sockets[handle]);
              });
            };
            ws.onerror = function () {
              transport.failAll(BoxedWineNetwork.ENETUNREACH);
            };
            ws.onclose = function () {
              transport.connecting = false;
              transport.wsOpen = false;
              transport.failAll(BoxedWineNetwork.ENETUNREACH);
            };
            ws.onmessage = function (event) {
              transport.onMessage(event.data);
            };
          } catch (error) {
            this.connecting = false;
            this.failAll(BoxedWineNetwork.ENETUNREACH);
          }
        },

        flushQueues: function () {
          if (!this.wsOpen || !this.ws) {
            return;
          }
          while (this.controlQueue.length) {
            this.ws.send(this.controlQueue.shift());
          }
          while (this.dataQueue.length) {
            this.ws.send(this.dataQueue.shift());
          }
        },

        sendControl: function (message) {
          var encoded = JSON.stringify(message);
          this.ensureGateway();
          if (this.wsOpen && this.ws) {
            this.ws.send(encoded);
          } else {
            this.controlQueue.push(encoded);
          }
        },

        sendTrace: function (event, detail) {
          if (!BoxedWineNetwork.debugEnabled()) {
            return;
          }
          this.sendControl({ type: "trace", event: event, detail: detail || "" });
        },

        sendDataFrame: function (socketId, data) {
          this.ensureGateway();
          var frame = BoxedWineNetwork.makeDataFrame(socketId, data);
          if (this.wsOpen && this.ws) {
            this.ws.send(frame);
          } else {
            this.dataQueue.push(frame);
          }
        },

        sendUdpFrame: function (socketId, ipv4, port, data) {
          this.ensureGateway();
          var frame = BoxedWineNetwork.makeUdpFrame(socketId, ipv4, port, data);
          if (this.wsOpen && this.ws) {
            this.ws.send(frame);
          } else {
            this.dataQueue.push(frame);
          }
        },

        getSocket: function (handle) {
          return this.sockets[handle] || null;
        },

        setEvents: function (socket) {
          var events = 0;
          if (socket.error) {
            events |= BoxedWineNetwork.POLLERR;
          }
          if (socket.rxBytes > 0 || socket.remoteClosed || (socket.pendingAccepts && socket.pendingAccepts.length > 0)) {
            events |= BoxedWineNetwork.POLLIN;
          }
          if (socket.state === "open" && !socket.outClosed && this.wsOpen && this.ws && this.ws.bufferedAmount < this.highWaterMark) {
            events |= BoxedWineNetwork.POLLOUT;
          }
          if (socket.remoteClosed || socket.state === "closed") {
            events |= BoxedWineNetwork.POLLHUP;
          }
          socket.events = events;
          BoxedWineNetwork.setEvents(socket.handle, events, socket.rxBytes);
        },

        failSocket: function (socket, error) {
          socket.error = error || BoxedWineNetwork.EIO;
          socket.state = "error";
          this.setEvents(socket);
        },

        failAll: function (error) {
          this.wsError = error || BoxedWineNetwork.ENETUNREACH;
          Object.keys(this.sockets).forEach(function (handle) {
            transport.failSocket(transport.sockets[handle], transport.wsError);
          });
        },

        onMessage: function (data) {
          if (typeof data === "string") {
            BoxedWineNetwork.debugLog("ws control recv len=" + data.length + " text=" + data.substring(0, 160));
            this.onControl(JSON.parse(data));
            return;
          }
          var bytes = new Uint8Array(data);
          BoxedWineNetwork.debugLog("ws binary recv len=" + bytes.length + " first=" + (bytes.length ? bytes[0] : -1) + " bytes=" + BoxedWineNetwork.previewBytes(bytes));
          if (bytes.length < 5) {
            return;
          }
          if (bytes[0] === 2) {
            this.onUdpDatagram(bytes);
            return;
          }
          if (bytes[0] !== 1) {
            return;
          }
          var socketId = BoxedWineNetwork.readFrameSocketId(bytes);
          var socket = this.getSocket(socketId);
          if (!socket) {
            BoxedWineNetwork.debugLog("ws recv dropped socket=" + socketId + " missing");
            this.sendTrace("tcp-drop", "socket=" + socketId + " reason=missing len=" + bytes.length);
            return;
          }
          var payload = bytes.slice(5);
          if (payload.length) {
            BoxedWineNetwork.debugLog("ws recv socket=" + socketId + " len=" + payload.length + " bytes=" + BoxedWineNetwork.previewBytes(payload));
            socket.rx.push(payload);
            socket.rxBytes += payload.length;
          }
          this.setEvents(socket);
        },

        onUdpDatagram: function (bytes) {
          if (bytes.length < 11) {
            return;
          }
          var socketId = BoxedWineNetwork.readFrameSocketId(bytes);
          var socket = this.getSocket(socketId);
          if (!socket || socket.kind !== "udp") {
            BoxedWineNetwork.debugLog("udp recv dropped socket=" + socketId + " reason=" + (!socket ? "missing" : ("kind=" + socket.kind)));
            this.sendTrace("udp-drop", "socket=" + socketId + " reason=" + (!socket ? "missing" : ("kind=" + socket.kind)) + " len=" + bytes.length);
            return;
          }
          var ipv4 = (bytes[5] | (bytes[6] << 8) | (bytes[7] << 16) | (bytes[8] << 24)) >>> 0;
          var port = (bytes[9] << 8) | bytes[10];
          var payload = bytes.slice(11);
          BoxedWineNetwork.debugLog("udp recv socket=" + socketId + " from=" + BoxedWineNetwork.ipv4ToString(ipv4) + ":" + port + " len=" + payload.length + " bytes=" + BoxedWineNetwork.previewBytes(payload));
          socket.rx.push({ data: payload, ipv4: ipv4, port: port });
          socket.rxBytes += payload.length;
          this.setEvents(socket);
          this.sendTrace("udp-recv", "socket=" + socketId + " from=" + BoxedWineNetwork.ipv4ToString(ipv4) + ":" + port + " len=" + payload.length + " rxBytes=" + socket.rxBytes);
        },

        onControl: function (message) {
          if (message.type === "room") {
            this.virtualIp = message.ip || "";
            this.roomPeers = Array.isArray(message.peers) ? message.peers : [];
            BoxedWineNetwork.debugLog("room ip=" + this.virtualIp + " peers=" + this.roomPeers.map(function (peer) { return peer.ip; }).join(","));
            this.sendTrace("room", "ip=" + this.virtualIp + " peers=" + this.roomPeers.map(function (peer) { return peer.ip; }).join(","));
            return;
          }
          var socket = this.getSocket(message.id);
          if (!socket) {
            BoxedWineNetwork.debugLog("control dropped type=" + message.type + " socket=" + message.id + " reason=missing");
            this.sendTrace("control-drop", "type=" + message.type + " socket=" + message.id);
            return;
          }
          if (message.type === "open") {
            if (message.status === 0) {
              socket.state = "open";
              socket.error = 0;
              BoxedWineNetwork.debugLog("open ok socket=" + message.id);
            } else {
              socket.error = -(message.status || -BoxedWineNetwork.EIO);
              socket.state = "error";
              BoxedWineNetwork.debugLog("open failed socket=" + message.id + " error=" + socket.error);
            }
            this.setEvents(socket);
          } else if (message.type === "close") {
            BoxedWineNetwork.debugLog("remote close socket=" + message.id);
            socket.remoteClosed = true;
            socket.state = "closed";
            this.setEvents(socket);
          } else if (message.type === "udp-open") {
            if (message.status === 0) {
              socket.state = "open";
              socket.error = 0;
              BoxedWineNetwork.debugLog("udp open ok socket=" + message.id);
            } else {
              socket.error = -(message.status || -BoxedWineNetwork.EIO);
              socket.state = "error";
              BoxedWineNetwork.debugLog("udp open failed socket=" + message.id + " error=" + socket.error);
            }
            this.setEvents(socket);
          } else if (message.type === "udp-bind") {
            if (message.status === 0) {
              socket.error = 0;
              socket.localIpv4 = BoxedWineNetwork.ipv4FromString(message.host);
              socket.localPort = message.port || socket.localPort || 0;
              BoxedWineNetwork.debugLog("udp bind ok socket=" + message.id + " host=" + message.host + " port=" + message.port);
            } else {
              socket.error = -(message.status || -BoxedWineNetwork.EIO);
              BoxedWineNetwork.debugLog("udp bind failed socket=" + message.id + " error=" + socket.error);
            }
            this.setEvents(socket);
          } else if (message.type === "listen") {
            if (message.status === 0) {
              socket.state = "listening";
              socket.error = 0;
              socket.localIpv4 = BoxedWineNetwork.ipv4FromString(message.host);
              socket.localPort = message.port || socket.bindPort || 0;
              BoxedWineNetwork.debugLog("listen ok socket=" + message.id + " host=" + message.host + " port=" + message.port);
            } else {
              socket.error = -(message.status || -BoxedWineNetwork.EIO);
              socket.state = "error";
              BoxedWineNetwork.debugLog("listen failed socket=" + message.id + " error=" + socket.error);
            }
            this.setEvents(socket);
          } else if (message.type === "pending") {
            if (!socket.pendingAccepts) {
              socket.pendingAccepts = [];
            }
            socket.pendingAccepts.push({
              token: message.token,
              host: message.host || "0.0.0.0",
              port: message.port || 0,
            });
            BoxedWineNetwork.debugLog("accept pending socket=" + message.id + " token=" + message.token + " from=" + (message.host || "0.0.0.0") + ":" + (message.port || 0));
            this.setEvents(socket);
          } else if (message.type === "accept") {
            var acceptedSocket = this.getSocket(message.acceptedId);
            if (acceptedSocket) {
              if (message.status === 0) {
                acceptedSocket.state = "open";
                acceptedSocket.error = 0;
                if (message.host || message.port) {
                  acceptedSocket.peerIpv4 = BoxedWineNetwork.ipv4FromString(message.host);
                  acceptedSocket.peerPort = message.port || 0;
                }
              } else {
                acceptedSocket.state = "error";
                acceptedSocket.error = -(message.status || -BoxedWineNetwork.EIO);
              }
              this.setEvents(acceptedSocket);
            }
          } else if (message.type === "error") {
            socket.error = -(message.status || -BoxedWineNetwork.EIO);
            socket.state = "error";
            BoxedWineNetwork.debugLog("remote error socket=" + message.id + " error=" + socket.error);
            this.setEvents(socket);
          }
        },

        socket: function (domain, type, protocol) {
          if (type !== 1 && type !== 2) {
            BoxedWineNetwork.debugLog("socket rejected domain=" + domain + " type=" + type + " protocol=" + protocol);
            return -BoxedWineNetwork.EAFNOSUPPORT;
          }
          var handle = BoxedWineNetwork.allocateSocket(domain, type, protocol);
          this.sockets[handle] = {
            handle: handle,
            domain: domain,
            type: type,
            protocol: protocol,
            state: "new",
            rx: [],
            rxBytes: 0,
            events: 0,
            error: 0,
            peerIpv4: 0,
            peerPort: 0,
            localIpv4: 0,
            localPort: 0,
            remoteClosed: false,
            outClosed: false,
            kind: type === 2 ? "udp" : "tcp",
            pendingAccepts: [],
          };
          if (type === 2) {
            this.sockets[handle].state = "open";
          }
          BoxedWineNetwork.debugLog("socket handle=" + handle + " domain=" + domain + " type=" + type + " protocol=" + protocol);
          this.ensureGateway();
          if (type === 2) {
            this.sendControl({ type: "udp-open", id: handle });
            this.setEvents(this.sockets[handle]);
          }
          return handle;
        },

        close: function (handle) {
          var socket = this.getSocket(handle);
          if (!socket) {
            return 0;
          }
          this.sendControl({ type: "close", id: handle });
          socket.state = "closed";
          delete this.sockets[handle];
          return 0;
        },

        connect: function (handle, ipv4, port) {
          var socket = this.getSocket(handle);
          if (!socket) {
            return -BoxedWineNetwork.ENOTCONN;
          }
          if (socket.kind === "udp") {
            socket.host = BoxedWineNetwork.ipv4ToString(ipv4);
            socket.ipv4 = ipv4 >>> 0;
            socket.port = port;
            socket.peerIpv4 = ipv4 >>> 0;
            socket.peerPort = port;
            socket.state = "open";
            socket.error = 0;
            this.setEvents(socket);
            BoxedWineNetwork.debugLog("udp connect socket=" + handle + " host=" + socket.host + " port=" + port);
            return 0;
          }
          if (socket.state === "open") {
            BoxedWineNetwork.debugLog("connect socket=" + handle + " already open");
            return 0;
          }
          if (socket.state === "connecting") {
            BoxedWineNetwork.debugLog("connect socket=" + handle + " still pending");
            return -BoxedWineNetwork.EINPROGRESS;
          }
          if (socket.state === "error") {
            BoxedWineNetwork.debugLog("connect socket=" + handle + " failed error=" + socket.error);
            return -(socket.error || BoxedWineNetwork.EIO);
          }
          socket.state = "connecting";
          socket.host = BoxedWineNetwork.ipv4ToString(ipv4);
          socket.port = port;
          socket.peerIpv4 = ipv4 >>> 0;
          socket.peerPort = port;
          BoxedWineNetwork.debugLog("connect socket=" + handle + " host=" + socket.host + " port=" + port);
          this.sendControl({ type: "open", id: handle, host: socket.host, port: port });
          return -BoxedWineNetwork.EINPROGRESS;
        },

        send: function (handle, buffer, len, flags, ipv4, port) {
          var socket = this.getSocket(handle);
          if (!socket) {
            BoxedWineNetwork.debugLog("send rejected socket=" + handle + " missing");
            return -BoxedWineNetwork.ENOTCONN;
          }
          if (socket.kind === "udp") {
            return this.sendUdp(socket, buffer, len, flags, ipv4, port);
          }
          if (socket.error) {
            BoxedWineNetwork.debugLog("tcp send rejected socket=" + handle + " error=" + socket.error);
            return -socket.error;
          }
          if (socket.outClosed) {
            BoxedWineNetwork.debugLog("tcp send rejected socket=" + handle + " shutdown");
            return -BoxedWineNetwork.ESHUTDOWN;
          }
          if (socket.state !== "open") {
            BoxedWineNetwork.debugLog("tcp send pending socket=" + handle + " state=" + socket.state + " len=" + len);
            return -BoxedWineNetwork.EAGAIN;
          }
          if (!this.wsOpen || !this.ws || this.ws.bufferedAmount >= this.highWaterMark) {
            socket.events &= ~BoxedWineNetwork.POLLOUT;
            BoxedWineNetwork.setEvents(handle, socket.events, socket.rxBytes);
            BoxedWineNetwork.debugLog("tcp send backpressure socket=" + handle + " len=" + len);
            return -BoxedWineNetwork.EAGAIN;
          }
          var payload = BoxedWineNetwork.copyFromHeap(buffer, len);
          BoxedWineNetwork.debugLog("tcp send socket=" + handle + " len=" + payload.length + " bytes=" + BoxedWineNetwork.previewBytes(payload));
          this.sendDataFrame(handle, payload);
          this.setEvents(socket);
          return len;
        },

        sendUdp: function (socket, buffer, len, flags, ipv4, port) {
          if (socket.error) {
            BoxedWineNetwork.debugLog("udp send rejected socket=" + socket.handle + " error=" + socket.error);
            return -socket.error;
          }
          if (!ipv4 || !port) {
            ipv4 = socket.ipv4 || 0;
            port = socket.port || 0;
          }
          if (!ipv4 || !port) {
            return -BoxedWineNetwork.EDESTADDRREQ;
          }
          if (this.ws && this.ws.bufferedAmount >= this.highWaterMark) {
            socket.events &= ~BoxedWineNetwork.POLLOUT;
            BoxedWineNetwork.setEvents(socket.handle, socket.events, socket.rxBytes);
            BoxedWineNetwork.debugLog("udp send backpressure socket=" + socket.handle + " len=" + len);
            return -BoxedWineNetwork.EAGAIN;
          }
          var payload = BoxedWineNetwork.copyFromHeap(buffer, len);
          BoxedWineNetwork.debugLog("udp send socket=" + socket.handle + " to=" + BoxedWineNetwork.ipv4ToString(ipv4) + ":" + port + " len=" + payload.length + " bytes=" + BoxedWineNetwork.previewBytes(payload));
          this.sendUdpFrame(socket.handle, ipv4 >>> 0, port, payload);
          this.setEvents(socket);
          return len;
        },

        recv: function (handle, buffer, len, flags, address, addressLen) {
          var socket = this.getSocket(handle);
          if (!socket) {
            return -BoxedWineNetwork.ENOTCONN;
          }
          if (socket.kind === "udp") {
            return this.recvUdp(socket, buffer, len, flags, address, addressLen);
          }
          if (socket.rxBytes === 0) {
            if (socket.error) {
              return -socket.error;
            }
            if (socket.remoteClosed || socket.state === "closed") {
              return 0;
            }
            return -BoxedWineNetwork.EAGAIN;
          }

          var remaining = len;
          var written = 0;
          var peek = (flags & BoxedWineNetwork.MSG_PEEK) !== 0;
          var offset = 0;
          while (remaining > 0 && offset < socket.rx.length) {
            var chunk = socket.rx[offset];
            var toCopy = Math.min(chunk.length, remaining);
            if (written === 0) {
              BoxedWineNetwork.debugLog("native recv socket=" + handle + " len=" + Math.min(socket.rxBytes, len) + " dst=0x" + (buffer >>> 0).toString(16) + " bytes=" + BoxedWineNetwork.previewBytes(chunk.subarray(0, toCopy)));
            }
            HEAPU8.set(chunk.subarray(0, toCopy), buffer + written);
            written += toCopy;
            remaining -= toCopy;
            if (!peek) {
              if (toCopy === chunk.length) {
                socket.rx.shift();
              } else {
                socket.rx[0] = chunk.subarray(toCopy);
              }
              socket.rxBytes -= toCopy;
            } else {
              offset++;
            }
          }
          this.setEvents(socket);
          return written;
        },

        recvUdp: function (socket, buffer, len, flags, address, addressLen) {
          if (socket.rxBytes === 0) {
            if (socket.error) {
              return -socket.error;
            }
            return -BoxedWineNetwork.EAGAIN;
          }

          var peek = (flags & BoxedWineNetwork.MSG_PEEK) !== 0;
          var datagram = socket.rx[0];
          var toCopy = Math.min(datagram.data.length, len);
          socket.lastRecvIpv4 = datagram.ipv4 >>> 0;
          socket.lastRecvPort = datagram.port;
          BoxedWineNetwork.debugLog("udp native recv socket=" + socket.handle + " len=" + toCopy + " from=" + BoxedWineNetwork.ipv4ToString(datagram.ipv4) + ":" + datagram.port + " dst=0x" + (buffer >>> 0).toString(16) + " bytes=" + BoxedWineNetwork.previewBytes(datagram.data.subarray(0, toCopy)));
          if (toCopy) {
            HEAPU8.set(datagram.data.subarray(0, toCopy), buffer);
          }
          if (!peek) {
            socket.rx.shift();
            socket.rxBytes -= datagram.data.length;
          }
          this.setEvents(socket);
          return toCopy;
        },

        shutdown: function (handle, how) {
          var socket = this.getSocket(handle);
          if (!socket) {
            return -BoxedWineNetwork.ENOTCONN;
          }
          if (how === 1 || how === 2) {
            socket.outClosed = true;
          }
          this.sendControl({ type: "shutdown", id: handle, how: how });
          this.setEvents(socket);
          return 0;
        },

        bind: function (handle, ipv4, port) {
          var socket = this.getSocket(handle);
          if (!socket) {
            return -BoxedWineNetwork.EAFNOSUPPORT;
          }
          if (socket.kind === "udp") {
            socket.bindIpv4 = ipv4 >>> 0;
            socket.bindHost = BoxedWineNetwork.ipv4ToString(socket.bindIpv4);
            socket.bindPort = port;
            socket.localIpv4 = socket.bindIpv4;
            socket.localPort = port;
            socket.error = 0;
            BoxedWineNetwork.debugLog("udp bind socket=" + handle + " host=" + socket.bindHost + " port=" + port);
            this.sendControl({ type: "udp-bind", id: handle, host: socket.bindHost, port: port });
            this.setEvents(socket);
            return 0;
          }
          if (socket.kind !== "tcp") {
            return -BoxedWineNetwork.EAFNOSUPPORT;
          }
          socket.bindIpv4 = ipv4 >>> 0;
          socket.bindHost = BoxedWineNetwork.ipv4ToString(socket.bindIpv4);
          socket.bindPort = port;
          socket.localIpv4 = socket.bindIpv4;
          socket.localPort = port;
          socket.state = "bound";
          BoxedWineNetwork.debugLog("bind socket=" + handle + " host=" + socket.bindHost + " port=" + port);
          this.setEvents(socket);
          return 0;
        },

        listen: function (handle, backlog) {
          var socket = this.getSocket(handle);
          if (!socket || socket.kind !== "tcp") {
            return -BoxedWineNetwork.EAFNOSUPPORT;
          }
          if (!socket.bindHost || !socket.bindPort) {
            return -BoxedWineNetwork.EINVAL;
          }
          socket.state = "listening";
          socket.pendingAccepts = socket.pendingAccepts || [];
          BoxedWineNetwork.debugLog("listen socket=" + handle + " host=" + socket.bindHost + " port=" + socket.bindPort + " backlog=" + backlog);
          this.sendControl({ type: "listen", id: handle, host: socket.bindHost, port: socket.bindPort, backlog: backlog });
          this.setEvents(socket);
          return 0;
        },

        accept: function (handle) {
          var listener = this.getSocket(handle);
          if (!listener || listener.kind !== "tcp") {
            return -BoxedWineNetwork.EAFNOSUPPORT;
          }
          if (!listener.pendingAccepts || listener.pendingAccepts.length === 0) {
            return -BoxedWineNetwork.EAGAIN;
          }
          var pending = listener.pendingAccepts.shift();
          var peerIpv4 = BoxedWineNetwork.ipv4FromString(pending.host);
          var acceptedHandle = BoxedWineNetwork.allocateSocket(listener.domain, listener.type, listener.protocol);
          this.sockets[acceptedHandle] = {
            handle: acceptedHandle,
            domain: listener.domain,
            type: listener.type,
            protocol: listener.protocol,
            state: "open",
            rx: [],
            rxBytes: 0,
            events: 0,
            error: 0,
            remoteClosed: false,
            outClosed: false,
            kind: "tcp",
            host: pending.host,
            port: pending.port,
            peerIpv4: peerIpv4,
            peerPort: pending.port,
            localIpv4: listener.localIpv4 || listener.bindIpv4 || 0,
            localPort: listener.localPort || listener.bindPort || 0,
            pendingAccepts: [],
          };
          BoxedWineNetwork.debugLog("accept socket=" + handle + " accepted=" + acceptedHandle + " token=" + pending.token);
          this.sendControl({ type: "accept", id: handle, token: pending.token, acceptedId: acceptedHandle });
          this.setEvents(listener);
          this.setEvents(this.sockets[acceptedHandle]);
          return acceptedHandle;
        },

        getEvents: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? socket.events : BoxedWineNetwork.POLLHUP;
        },

        getError: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? socket.error : BoxedWineNetwork.ENETUNREACH;
        },

        readableBytes: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? socket.rxBytes : 0;
        },

        getPeerIpv4: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? (socket.peerIpv4 || BoxedWineNetwork.ipv4FromString(socket.host)) >>> 0 : 0;
        },

        getPeerPort: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? socket.peerPort || socket.port || 0 : 0;
        },

        getLocalIpv4: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? (socket.localIpv4 || socket.bindIpv4 || 0) >>> 0 : 0;
        },

        getLocalPort: function (handle) {
          var socket = this.getSocket(handle);
          return socket ? socket.localPort || socket.bindPort || 0 : 0;
        },

        setsockopt: function () {
          return 0;
        },
      };
      return transport;
    },
  },

  bw_net_is_enabled__deps: ["$BoxedWineNetwork"],
  bw_net_is_enabled: function () {
    return BoxedWineNetwork.enabled() ? 1 : 0;
  },

  bw_net_is_debug_enabled__deps: ["$BoxedWineNetwork"],
  bw_net_is_debug_enabled: function () {
    return BoxedWineNetwork.debugEnabled() ? 1 : 0;
  },

  bw_net_socket__deps: ["$BoxedWineNetwork"],
  bw_net_socket: function (domain, type, protocol) {
    if (!BoxedWineNetwork.enabled()) {
      return 0;
    }
    return BoxedWineNetwork.callTransport(
      "socket",
      [domain, type, protocol],
      function () {
        return BoxedWineNetwork.allocateSocket(domain, type, protocol);
      }
    );
  },

  bw_net_close__deps: ["$BoxedWineNetwork"],
  bw_net_close: function (handle) {
    BoxedWineNetwork.callTransport("close", [handle], 0);
    delete BoxedWineNetwork.sockets[handle];
  },

  bw_net_get_events__deps: ["$BoxedWineNetwork"],
  bw_net_get_events: function (handle) {
    var transportEvents = BoxedWineNetwork.callTransport("getEvents", [handle], null);
    if (typeof transportEvents === "number") {
      return transportEvents;
    }
    var socket = BoxedWineNetwork.socketState(handle);
    return socket ? socket.events : 0;
  },

  bw_net_get_error__deps: ["$BoxedWineNetwork"],
  bw_net_get_error: function (handle) {
    var transportError = BoxedWineNetwork.callTransport("getError", [handle], null);
    if (typeof transportError === "number") {
      return transportError;
    }
    var socket = BoxedWineNetwork.socketState(handle);
    return socket ? socket.error : 101;
  },

  bw_net_readable_bytes__deps: ["$BoxedWineNetwork"],
  bw_net_readable_bytes: function (handle) {
    var transportBytes = BoxedWineNetwork.callTransport("readableBytes", [handle], null);
    if (typeof transportBytes === "number") {
      return transportBytes;
    }
    var socket = BoxedWineNetwork.socketState(handle);
    return socket ? socket.readableBytes : 0;
  },

  bw_net_get_peer_ipv4__deps: ["$BoxedWineNetwork"],
  bw_net_get_peer_ipv4: function (handle) {
    return BoxedWineNetwork.callTransport("getPeerIpv4", [handle], 0) >>> 0;
  },

  bw_net_get_peer_port__deps: ["$BoxedWineNetwork"],
  bw_net_get_peer_port: function (handle) {
    return BoxedWineNetwork.callTransport("getPeerPort", [handle], 0);
  },

  bw_net_get_local_ipv4__deps: ["$BoxedWineNetwork"],
  bw_net_get_local_ipv4: function (handle) {
    return BoxedWineNetwork.callTransport("getLocalIpv4", [handle], 0) >>> 0;
  },

  bw_net_get_local_port__deps: ["$BoxedWineNetwork"],
  bw_net_get_local_port: function (handle) {
    return BoxedWineNetwork.callTransport("getLocalPort", [handle], 0);
  },

  bw_net_get_room_ipv4__deps: ["$BoxedWineNetwork"],
  bw_net_get_room_ipv4: function () {
    var transport = BoxedWineNetwork.transport();
    if (!transport || !transport.virtualIp) {
      return 0;
    }
    return BoxedWineNetwork.ipv4FromString(transport.virtualIp) >>> 0;
  },

  bw_net_connect__deps: ["$BoxedWineNetwork"],
  bw_net_connect: function (handle, ipv4, port) {
    return BoxedWineNetwork.callTransport(
      "connect",
      [handle, ipv4 >>> 0, port],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_bind__deps: ["$BoxedWineNetwork"],
  bw_net_bind: function (handle, ipv4, port) {
    return BoxedWineNetwork.callTransport(
      "bind",
      [handle, ipv4 >>> 0, port],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_listen__deps: ["$BoxedWineNetwork"],
  bw_net_listen: function (handle, backlog) {
    return BoxedWineNetwork.callTransport(
      "listen",
      [handle, backlog],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_accept__deps: ["$BoxedWineNetwork"],
  bw_net_accept: function (handle) {
    return BoxedWineNetwork.callTransport(
      "accept",
      [handle],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_send__deps: ["$BoxedWineNetwork"],
  bw_net_send: function (handle, buffer, len, flags, ipv4, port) {
    return BoxedWineNetwork.callTransport(
      "send",
      [handle, buffer, len, flags, ipv4 >>> 0, port],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_recv__deps: ["$BoxedWineNetwork"],
  bw_net_recv: function (handle, buffer, len, flags, address, addressLen) {
    return BoxedWineNetwork.callTransport(
      "recv",
      [handle, buffer, len, flags, address, addressLen],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_get_last_recv_ipv4__deps: ["$BoxedWineNetwork"],
  bw_net_get_last_recv_ipv4: function (handle) {
    var transport = BoxedWineNetwork.transport();
    var socket = transport && typeof transport.getSocket === "function" ? transport.getSocket(handle) : null;
    return socket ? (socket.lastRecvIpv4 >>> 0) : 0;
  },

  bw_net_get_last_recv_port__deps: ["$BoxedWineNetwork"],
  bw_net_get_last_recv_port: function (handle) {
    var transport = BoxedWineNetwork.transport();
    var socket = transport && typeof transport.getSocket === "function" ? transport.getSocket(handle) : null;
    return socket ? (socket.lastRecvPort || 0) : 0;
  },

  bw_net_shutdown__deps: ["$BoxedWineNetwork"],
  bw_net_shutdown: function (handle, how) {
    return BoxedWineNetwork.callTransport(
      "shutdown",
      [handle, how],
      function () {
        return BoxedWineNetwork.unavailable(handle);
      }
    );
  },

  bw_net_setsockopt__deps: ["$BoxedWineNetwork"],
  bw_net_setsockopt: function (handle, level, name, value, len) {
    return BoxedWineNetwork.callTransport(
      "setsockopt",
      [handle, level, name, value, len],
      0
    );
  },
});
