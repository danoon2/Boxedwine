import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { once } from "node:events";
import { readFileSync } from "node:fs";
import { createServer } from "node:net";
import { setTimeout as delay } from "node:timers/promises";
import test from "node:test";
import { fileURLToPath } from "node:url";
import vm from "node:vm";

async function gateway(t) {
  const reservation = createServer();
  reservation.listen(0, "127.0.0.1");
  await once(reservation, "listening");
  const port = reservation.address().port;
  await new Promise(resolve => reservation.close(resolve));
  const child = spawn(process.execPath, [fileURLToPath(new URL("./network-gateway.mjs", import.meta.url)), "--port", String(port), "--allow-origin", "none"], { windowsHide: true });
  let output = "";
  child.stdout.on("data", b => output += b);
  child.stderr.on("data", b => output += b);
  const exit = once(child, "exit");
  t.after(async () => { if (child.exitCode === null) child.kill(); await exit; });
  for (let i = 0; i < 150 && !output.includes("listening at") && child.exitCode === null; i++) await delay(20);
  assert.match(output, /listening at/);
  return { child, port };
}

async function client(g, expectRoom = true) {
  const ws = new WebSocket(`ws://127.0.0.1:${g.port}/boxedwine-network`);
  const messages = [];
  ws.binaryType = "arraybuffer";
  ws.addEventListener("message", e => messages.push(typeof e.data === "string" ? JSON.parse(e.data) : Buffer.from(e.data)));
  ws.addEventListener("error", () => {});
  const peer = { ws, messages, send: m => ws.send(JSON.stringify(m)) };
  if (expectRoom) peer.room = await take(peer, m => m.type === "room");
  return peer;
}

async function take(peer, predicate, timeout = 3000) {
  const until = Date.now() + timeout;
  while (Date.now() < until) {
    const index = peer.messages.findIndex(predicate);
    if (index >= 0) return peer.messages.splice(index, 1)[0];
    await delay(5);
  }
  throw new Error("Timed out waiting for gateway message");
}

async function waitFor(predicate) {
  const until = Date.now() + 3000;
  while (Date.now() < until) {
    if (predicate()) return;
    await delay(5);
  }
  throw new Error("Timed out waiting for browser transport");
}

function browserTransport(t, g) {
  let library;
  const context = vm.createContext({
    addToLibrary(value) { library = value; },
    Module: { boxedwineNetworking: { enabled: true } },
    HEAPU8: new Uint8Array(2048), WebSocket, console, setTimeout, clearTimeout,
  });
  vm.runInContext(readFileSync(new URL("./boxedwine-net.js", import.meta.url), "utf8"), context);
  const n = context.BoxedWineNetwork = library.$BoxedWineNetwork;
  const transport = n.createWebSocketTcpTransport(`ws://127.0.0.1:${g.port}/boxedwine-network`);
  t.after(() => { if (transport.ws?.readyState === 1) transport.ws.close(); });
  return transport;
}

function udp(id, ip, port, text) {
  const b = Buffer.alloc(11 + Buffer.byteLength(text));
  b[0] = 2; b.writeUInt32BE(id, 1);
  ip.split(".").forEach((x, i) => b[5 + i] = +x);
  b.writeUInt16BE(port, 9); b.write(text, 11);
  return b;
}

test("UDP ephemeral ports route replies to the correct sender socket", async t => {
  const g = await gateway(t), a = await client(g), b = await client(g);
  for (const id of [1, 2]) {
    a.send({ type: "udp-open", id });
    await take(a, m => m.type === "udp-open" && m.id === id);
    a.send({ type: "udp-bind", id, port: 0 });
    const bound = await take(a, m => m.type === "udp-bind" && m.id === id);
    assert.ok(bound.port > 0);
  }
  b.send({ type: "udp-open", id: 3, port: 27910 });
  await take(b, m => m.type === "udp-open");
  let previousPort;
  for (const id of [1, 2]) {
    a.ws.send(udp(id, b.room.ip, 27910, ""));
    const received = await take(b, Buffer.isBuffer);
    const sourcePort = received.readUInt16BE(9);
    assert.ok(sourcePort > 0);
    assert.notEqual(sourcePort, previousPort);
    previousPort = sourcePort;
    b.ws.send(udp(3, a.room.ip, sourcePort, `reply${id}`));
    const reply = await take(a, Buffer.isBuffer);
    assert.equal(reply.readUInt32BE(1), id);
    assert.equal(reply.subarray(11).toString(), `reply${id}`);
  }
});

test("room TCP half-close reports EOF and still permits a reply", async t => {
  const g = await gateway(t), a = await client(g), b = await client(g);
  b.send({ type: "listen", id: 10, host: "0.0.0.0", port: 19200 });
  await take(b, m => m.type === "listen");
  a.send({ type: "open", id: 11, host: b.room.ip, port: 19200 });
  const pending = await take(b, m => m.type === "pending");
  b.send({ type: "accept", id: 10, token: pending.token, acceptedId: 12 });
  await take(a, m => m.type === "open"); await take(b, m => m.type === "accept");
  a.send({ type: "shutdown", id: 11, how: 1 });
  const eof = await take(b, m => m.type === "shutdown" && m.id === 12);
  assert.equal(eof.how, 1);
  const frame = Buffer.from([1, 0, 0, 0, 12, 42]);
  b.ws.send(frame);
  const reply = await take(a, Buffer.isBuffer);
  assert.equal(reply.readUInt32BE(1), 11);
  assert.equal(reply[5], 42);
});

test("UDP port collisions are rejected and unicast reaches only its owner", async t => {
  const g = await gateway(t), a = await client(g), b = await client(g);
  const ports = [];
  for (const id of [1, 2]) {
    a.send({ type: "udp-open", id });
    ports.push((await take(a, m => m.type === "udp-open" && m.id === id)).port);
  }
  a.send({ type: "udp-bind", id: 1, port: ports[1] });
  assert.equal((await take(a, m => m.type === "udp-bind")).status, -98);
  a.send({ type: "udp-open", id: 4, port: ports[1] });
  assert.equal((await take(a, m => m.type === "error" && m.id === 4)).status, -98);
  b.send({ type: "udp-open", id: 3, port: ports[1] }); // Other room peers have their own ports.
  assert.equal((await take(b, m => m.type === "udp-open")).status, 0);
  for (const id of [1, 2]) {
    b.ws.send(udp(3, a.room.ip, ports[id - 1], `to${id}`));
    assert.equal((await take(a, Buffer.isBuffer)).readUInt32BE(1), id);
  }
  // A same-peer stats response is a barrier after both deliveries.
  a.send({ type: "stats", id: 99 });
  await take(a, m => m.type === "stats");
  assert.equal(a.messages.filter(Buffer.isBuffer).length, 0);
  a.send({ type: "close", id: 2 });
  a.send({ type: "udp-bind", id: 1, port: ports[1] });
  assert.equal((await take(a, m => m.type === "udp-bind")).status, 0);
  a.send({ type: "udp-open", id: 4, port: ports[0] });
  assert.equal((await take(a, m => m.type === "udp-open" && m.id === 4)).status, 0);
});

test("canceled room opens are removed from the browser accept queue", async t => {
  const g = await gateway(t), opener = await client(g), transport = browserTransport(t, g);
  const listener = transport.socket(2, 1, 6);
  transport.bind(listener, 0, 19200);
  transport.listen(listener, 5);
  await waitFor(() => transport.getLocalIpv4(listener) !== 0);
  opener.send({ type: "open", id: 1, host: transport.virtualIp, port: 19200 });
  await waitFor(() => transport.getSocket(listener).pendingAccepts.length === 1);
  opener.send({ type: "close", id: 1 });
  await waitFor(() => transport.getSocket(listener).pendingAccepts.length === 0);
  assert.equal(transport.getEvents(listener) & 1, 0);
  assert.equal(transport.accept(listener), -11);

  opener.send({ type: "open", id: 2, host: transport.virtualIp, port: 19200 });
  await waitFor(() => transport.getSocket(listener).pendingAccepts.length === 1);
  opener.ws.close(); // Losing the whole peer must also cancel queued opens.
  await waitFor(() => transport.getSocket(listener).pendingAccepts.length === 0);
  assert.equal(transport.accept(listener), -11);
});

test("a room accept racing with cancellation fails without a stuck socket", async t => {
  const g = await gateway(t), opener = await client(g), transport = browserTransport(t, g);
  const listener = transport.socket(2, 1, 6);
  transport.bind(listener, 0, 19200);
  transport.listen(listener, 5);
  await waitFor(() => transport.getLocalIpv4(listener) !== 0);
  opener.send({ type: "open", id: 1, host: transport.virtualIp, port: 19200 });
  await waitFor(() => transport.getSocket(listener).pendingAccepts.length === 1);
  // Delay the accept frame so cancellation wins at the gateway after the
  // guest has already received the new descriptor.
  const sendControl = transport.sendControl;
  let acceptMessage;
  transport.sendControl = function (message) {
    if (message.type === "accept") acceptMessage = message;
    else sendControl.call(this, message);
  };
  const accepted = transport.accept(listener);
  assert.ok(accepted > 0);
  opener.send({ type: "close", id: 1 });
  opener.send({ type: "stats", id: 99 });
  await take(opener, m => m.type === "stats");
  sendControl.call(transport, acceptMessage);
  await waitFor(() => transport.getError(accepted) !== 0);
  assert.equal(transport.getEvents(accepted) & 8, 8);
  assert.equal(transport.recv(accepted, 0, 1, 0), -103);
  assert.equal(transport.send(accepted, 0, 1, 0, 0, 0), -103);
});

test("malformed control JSON disconnects only the offending peer", async t => {
  const g = await gateway(t);
  for (const invalid of ["{", "null", "[]"]) {
    const a = await client(g);
    const closed = once(a.ws, "close");
    a.ws.send(invalid);
    await closed;
    assert.equal(g.child.exitCode, null);
  }
  const healthy = await client(g);
  healthy.send({ type: "stats", id: 1 });
  await take(healthy, m => m.type === "stats");
});

test("room addresses are reusable after more than 253 sequential joins", async t => {
  const g = await gateway(t);
  for (let i = 0; i < 260; i++) {
    const a = await client(g);
    const closed = once(a.ws, "close");
    a.ws.close();
    await closed;
  }
  assert.equal(g.child.exitCode, null);
});

test("a full room rejects the extra peer without terminating the gateway", async t => {
  const g = await gateway(t), peers = [];
  for (let i = 0; i < 253; i++) peers.push(await client(g));
  const extra = await client(g, false);
  const [event] = await once(extra.ws, "close");
  assert.equal(event.code, 1013);
  assert.equal(g.child.exitCode, null);
  const closed = once(peers[0].ws, "close");
  peers[0].ws.close();
  await closed;
  const replacement = await client(g);
  assert.equal(replacement.room.ip, peers[0].room.ip);
});
