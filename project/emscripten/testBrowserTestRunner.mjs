import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';

const source = readFileSync(new URL('./boxedwine-test-runner.js', import.meta.url), 'utf8');
const tick = () => new Promise(resolve => setImmediate(resolve));

function runner({url = 'http://localhost:6923/boxedwine.html', search = '', browser = true} = {}) {
  const requests = [];
  const output = [];
  const errors = [];
  const exits = [];
  const Module = {
    arguments: ['node-argument'],
    print: (...args) => output.push(args),
    printErr: (...args) => errors.push(args),
    onExit: status => exits.push(status),
  };
  const original = {...Module};
  const context = {
    Module, console,
    fetch: (url, {body, method}) => new Promise((resolve, reject) => {
      assert.equal(url, 'stdio.html');
      assert.equal(method, 'POST');
      requests.push({body, resolve, reject});
    }),
  };
  if (browser) {
    context.window = {location: {search}, close: () => assert.fail('runner must not close the browser')};
    context.document = {URL: url};
  }
  vm.runInNewContext(source, context);
  return {Module, original, requests, output, errors, exits};
}

async function acknowledge(request, status = 200) {
  assert.ok(request, 'expected a request');
  request.resolve({ok: status === 200, status, text: async () => 'OK'});
  await tick();
}

test('final flush and all log acknowledgments precede the single exit notification', async () => {
  const r = runner({search: '?-shard&2&16&space%20and%26value'});
  assert.deepEqual(Array.from(r.Module.arguments), ['-shard', '2', '16', 'space and%26value']);
  await tick();
  assert.equal(r.requests[0].body, '^pageload^');
  r.Module.print('Test last ... OK');
  r.Module.printErr('diagnostic + % & é');
  // Model FS.quit flushing a buffered final printf before Module.onExit.
  r.Module.print('0 tests FAILED in 1s');
  r.Module.onExit(0);
  r.Module.onExit(0);
  assert.equal(r.requests.length, 1, 'logs wait for page-load acknowledgment');
  assert.deepEqual(r.exits, [0]);
  await acknowledge(r.requests[0]);
  assert.equal(r.requests[1].body, '^out^1^Test%20last%20...%20OK');
  await acknowledge(r.requests[1]);
  assert.equal(r.requests[2].body, '^err^2^' + encodeURIComponent('diagnostic + % & é'));
  await acknowledge(r.requests[2]);
  assert.equal(r.requests[3].body, '^out^3^0%20tests%20FAILED%20in%201s');

  // Headers alone do not prove the server has finished the acknowledgment.
  let finishBody;
  r.requests[3].resolve({ok: true, text: () => new Promise(resolve => { finishBody = resolve; })});
  await tick();
  assert.equal(r.requests.length, 4);
  finishBody('OK');
  await tick();
  assert.equal(r.requests[4].body, '^exit^0');
  // emrun closes the exit connection without an HTTP response.
  r.requests[4].reject(new Error('connection closed'));
  await tick();
  r.Module.onExit(0);
  await tick();
  assert.equal(r.requests.length, 5);
  assert.deepEqual(r.output, [['Test last ... OK'], ['0 tests FAILED in 1s']]);
  assert.deepEqual(r.errors, [['diagnostic + % & é']]);
});

test('nonzero program exit is preserved after delivery of its failure summary', async () => {
  const r = runner();
  r.Module.print('1 tests FAILED in 1s');
  r.Module.onExit(7);
  await tick();
  await acknowledge(r.requests[0]);
  await acknowledge(r.requests[1]);
  assert.equal(r.requests[2].body, '^exit^7');
});

for (const failure of ['network', 'http', 'response body']) {
  test(`lost output (${failure}) fails even when the program exits zero`, async () => {
    const r = runner();
    r.Module.print('0 tests FAILED in 1s');
    r.Module.onExit(0);
    await tick();
    await acknowledge(r.requests[0]);
    if (failure === 'network') r.requests[1].reject(new Error('connection reset'));
    else if (failure === 'http') await acknowledge(r.requests[1], 500);
    else r.requests[1].resolve({ok: true, text: async () => { throw new Error('truncated response'); }});
    await tick();
    assert.equal(r.requests[2].body, '^exit^1');
    assert.equal(r.errors.length, 1);
    assert.match(r.errors[0][0], /Test output delivery failed/);
  });
}

test('an unfinished runtime does not acquire a synthetic exit or success summary', async () => {
  const r = runner();
  r.Module.print('Test last ... OK');
  await tick();
  await acknowledge(r.requests[0]);
  await acknowledge(r.requests[1]);
  assert.equal(r.requests.length, 2);
  assert.deepEqual(r.exits, []);
});

test('empty browser arguments and emrun default port are supported', async () => {
  const r = runner({url: 'http://127.0.0.1:6931/boxedwine.html'});
  assert.deepEqual(Array.from(r.Module.arguments), []);
  await tick();
  assert.equal(r.requests[0].body, '^pageload^');
});

test('ordinary web pages, Node, and pthread workers retain their normal handlers', async () => {
  for (const options of [{url: 'https://example.com/boxedwine.html'}, {browser: false}]) {
    const r = runner(options);
    for (const key of ['print', 'printErr', 'onExit']) assert.equal(r.Module[key], r.original[key]);
    if (options.browser === false) assert.deepEqual(r.Module.arguments, ['node-argument']);
    await tick();
    assert.equal(r.requests.length, 0);
  }
});
