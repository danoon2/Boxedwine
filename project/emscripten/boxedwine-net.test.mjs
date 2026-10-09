import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import test from "node:test";
import vm from "node:vm";

function setup() {
  let library;
  const timers = new Map();
  let nextTimer = 0;
  let now = 1000;
  class WebSocket {
    bufferedAmount = 0;
    sent = [];
    send(frame) { this.sent.push(frame); }
  }
  const context = vm.createContext({
    addToLibrary(value) { library = value; },
    Module: { boxedwineNetworking: { enabled: true } },
    HEAPU8: new Uint8Array(2048), WebSocket, console,
    Date: { now() { return now; } },
    setTimeout(fn) { timers.set(++nextTimer, fn); return nextTimer; },
    clearTimeout(id) { timers.delete(id); },
  });
  vm.runInContext(readFileSync(new URL("./boxedwine-net.js", import.meta.url), "utf8"), context);
  const network = context.BoxedWineNetwork = library.$BoxedWineNetwork;
  const transport = network.createWebSocketTcpTransport("ws://test.invalid");
  const tcp = transport.socket(2, 1, 6);
  transport.ws.onopen();
  transport.onControl({ type: "open", id: tcp, status: 0 });
  return { network, transport, tcp, library, timers, advance(ms) { now += ms; }, tick() {
    const callbacks = [...timers.values()];
    timers.clear();
    callbacks.forEach(fn => fn());
  } };
}

test("TCP and UDP writers wake after WebSocket backpressure drains", () => {
  const { transport: t, tcp, tick, timers } = setup();
  const udp = t.socket(2, 2, 17);
  t.ws.bufferedAmount = t.highWaterMark;
  assert.equal(t.send(tcp, 0, 1, 0, 0, 0), -11);
  assert.equal(t.send(udp, 0, 1, 0, 1, 1234), -11);
  assert.equal(t.getEvents(tcp) & 4, 0);
  assert.equal(timers.size, 1);
  tick(); // Still full; continue watching without another guest send.
  assert.equal(timers.size, 1);
  t.ws.bufferedAmount = 0;
  tick();
  assert.equal(t.getEvents(tcp) & 4, 4);
  assert.equal(t.getEvents(udp) & 4, 4);
  assert.equal(timers.size, 0);
});

test("gateway reconnect backoff clears pending drain notifications and stale frames", () => {
  const { network: n, transport: t, tcp, timers, advance, tick } = setup();
  const oldWs = t.ws;
  oldWs.bufferedAmount = t.highWaterMark;
  assert.equal(t.send(tcp, 0, 1, 0, 0, 0), -11);
  assert.equal(timers.size, 1);
  t.controlQueue.push("old control");
  t.dataQueue.push("old data");
  oldWs.onclose();
  assert.equal(timers.size, 0);
  assert.equal(t.getSocket(tcp).error, n.ENETUNREACH);
  assert.equal(t.controlQueue.length, 0);
  assert.equal(t.dataQueue.length, 0);
  assert.equal(t.reconnectDelay, 500);
  t.ensureGateway();
  assert.equal(t.ws, null);
  assert.equal(timers.size, 1);
  advance(500);
  tick();
  const newWs = t.ws;
  assert.ok(newWs && newWs !== oldWs);
  oldWs.onclose(); // A delayed close cannot fail the replacement transport.
  assert.equal(t.ws, newWs);
  newWs.onopen();
  assert.equal(t.reconnectDelay, 0);
  assert.equal(t.wsOpen, true);
});

test("empty UDP datagrams are readable, peekable, and consumed", () => {
  const { network: n, transport: t } = setup();
  const udp = t.socket(2, 2, 17);
  t.onUdpDatagram(n.makeUdpFrame(udp, n.ipv4FromString("10.0.3.2"), 27910, new Uint8Array()));
  assert.equal(t.getEvents(udp) & 1, 1);
  assert.equal(t.recv(udp, 0, 100, n.MSG_PEEK), 0);
  assert.equal(t.getEvents(udp) & 1, 1);
  assert.equal(t.recv(udp, 0, 100, 0), 0);
  assert.equal(t.getSocket(udp).lastRecvPort, 27910);
  assert.equal(t.recv(udp, 0, 100, 0), -11);
});

test("UDP bind(0) exposes distinct allocated ports immediately", () => {
  const { transport: t } = setup();
  const a = t.socket(2, 2, 17), b = t.socket(2, 2, 17);
  assert.equal(t.bind(a, 0, 0), 0);
  assert.equal(t.bind(b, 0, 0), 0);
  assert.ok(t.getLocalPort(a) >= 49152);
  assert.notEqual(t.getLocalPort(a), t.getLocalPort(b));
});

test("UDP bind rejects occupied ports without changing either socket", () => {
  const { network: n, transport: t } = setup();
  const a = t.socket(2, 2, 17), b = t.socket(2, 2, 17);
  const aPort = t.getLocalPort(a), bPort = t.getLocalPort(b);
  const sent = t.ws.sent.length;
  assert.equal(t.bind(a, 0, bPort), -n.EADDRINUSE);
  assert.equal(t.getLocalPort(a), aPort);
  assert.equal(t.getLocalPort(b), bPort);
  assert.equal(t.getError(a), 0);
  assert.equal(t.ws.sent.length, sent);
  assert.equal(t.bind(a, 0, aPort), 0);
  t.close(b);
  assert.equal(t.bind(a, 0, bPort), 0);
  assert.equal(t.getLocalPort(a), bPort);
  const c = t.socket(2, 2, 17);
  assert.equal(t.getLocalPort(c), aPort);
});

test("delayed UDP acknowledgements cannot overwrite a newer port reservation", () => {
  const { transport: t } = setup();
  const a = t.socket(2, 2, 17), initialPort = t.getLocalPort(a);
  assert.equal(t.bind(a, 0, 27910), 0);
  t.onControl({ type: "udp-open", id: a, status: 0, host: "0.0.0.0", port: initialPort });
  assert.equal(t.getLocalPort(a), 27910);
  assert.equal(t.bind(a, 0, 27911), 0);
  t.onControl({ type: "udp-bind", id: a, status: 0, host: "0.0.0.0", port: 27910 });
  assert.equal(t.getLocalPort(a), 27911);
  const b = t.socket(2, 2, 17);
  assert.equal(t.getLocalPort(b), initialPort);
  assert.equal(t.bind(b, 0, 27911), -98);
});

test("canceling a pending connection updates listener readiness and preserves other accepts", () => {
  const { network: n, transport: t, tcp: listener } = setup();
  t.onControl({ type: "pending", id: listener, token: 1, host: "10.0.3.2", port: 1234 });
  t.onControl({ type: "pending", id: listener, token: 2, host: "10.0.3.3", port: 5678 });
  t.onControl({ type: "pending-cancel", id: listener, token: 1 });
  assert.equal(t.getEvents(listener) & n.POLLIN, n.POLLIN);
  const accepted = t.accept(listener);
  assert.ok(accepted > 0);
  assert.equal(t.getPeerPort(accepted), 5678);
  assert.equal(t.getEvents(listener) & n.POLLIN, 0);
  assert.equal(t.accept(listener), -n.EAGAIN);
  t.onControl({ type: "accept", id: listener, acceptedId: accepted, status: 0 });
  assert.equal(t.send(accepted, 0, 1, 0, 0, 0), 1);
});

test("an accept cancellation race is terminal even after the listener closes", () => {
  for (const status of [-11, -103]) {
    for (const closeListener of [false, true]) {
      const { network: n, transport: t, tcp: listener } = setup();
      t.onControl({ type: "pending", id: listener, token: 1, host: "10.0.3.2", port: 1234 });
      const accepted = t.accept(listener);
      assert.ok(accepted > 0);
      if (closeListener) t.close(listener);
      t.onControl({ type: "pending-cancel", id: listener, token: 1 });
      t.onControl({ type: "accept", id: listener, acceptedId: accepted, status });
      assert.equal(t.getError(accepted), n.ECONNABORTED);
      assert.equal(t.getEvents(accepted) & n.POLLERR, n.POLLERR);
      assert.equal(t.recv(accepted, 0, 1, 0), -n.ECONNABORTED);
      assert.equal(t.send(accepted, 0, 1, 0, 0, 0), -n.ECONNABORTED);
    }
  }
});

test("remote TCP half-close yields EOF while preserving reply writes", () => {
  const { network: n, transport: t, tcp } = setup();
  t.onMessage(n.makeDataFrame(tcp, new Uint8Array([42])).buffer);
  t.onControl({ type: "shutdown", id: tcp, how: 1 });
  assert.equal(t.recv(tcp, 0, 100, 0), 1);
  assert.equal(t.recv(tcp, 0, 100, 0), 0);
  assert.equal(t.getEvents(tcp) & 5, 5);
  assert.equal(t.send(tcp, 0, 1, 0, 0, 0), 1);
  assert.equal(t.shutdown(tcp, 1), 0);
  assert.equal(t.send(tcp, 0, 1, 0, 0, 0), -108);
});

test("refused connections retain POLLERR until observed by guest poll", () => {
  const { network: n, transport: t, library } = setup();
  const fd = t.socket(2, 1, 6);
  t.connect(fd, n.ipv4FromString("10.0.3.2"), 1234);
  t.onControl({ type: "open", id: fd, status: -111 });
  assert.equal(t.getEvents(fd) & 8, 8);
  assert.equal(t.getError(fd), 111);
  assert.equal(library.bw_net_take_notification(fd), 1);
  assert.equal(library.bw_net_take_notification(fd), 0);
});
