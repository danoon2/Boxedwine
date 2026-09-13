// Pure cache tests: fake optimizer callbacks, no emulator, browser, or compiler.
import assert from 'node:assert/strict';
import { mkdtemp, readFile, readdir, rm, writeFile, realpath } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, sep } from 'node:path';
import { optimizerIdentity, OptimizerCache } from './boxedwine-optimizer-cache.mjs';

const root = await mkdtemp(join(tmpdir(), 'boxedwine-optimizer-cache-'));
const wasm = Buffer.from([0, 97, 115, 109, 1, 0, 0, 0]);
let assertions = 0;
function equal(actual, expected) { assert.deepEqual(actual, expected); ++assertions; }
try {
    const tool = join(root, 'tool.js');
    const dependency = join(root, 'tool.wasm');
    await writeFile(tool, 'tool version 1');
    await writeFile(dependency, 'dependency version 1');
    const config = { passes: ['a', 'b'], level: 2 };
    const identity = await optimizerIdentity([tool, dependency], config);
    equal(await optimizerIdentity([dependency, tool, tool], config), identity);
    const folder = join(root, 'cache');
    const cache = new OptimizerCache(folder, identity);
    let calls = 0;
    const compute = () => { ++calls; return wasm; };
    equal(await cache.optimize(wasm, compute), wasm);
    equal([calls, cache.hits, cache.misses], [1, 0, 1]);
    // A new instance represents a later pipeline invocation.
    const warm = new OptimizerCache(folder, identity);
    equal(await warm.optimize(wasm, compute), wasm);
    equal([calls, warm.hits, warm.misses], [1, 1, 0]);
    await warm.optimize(Buffer.concat([wasm, Buffer.from([0])]), compute);
    equal(calls, 2);
    const changedPasses = await optimizerIdentity([tool, dependency], { ...config, passes: ['b', 'a'] });
    assert.notEqual(changedPasses, identity); ++assertions;
    await new OptimizerCache(folder, changedPasses).optimize(wasm, compute);
    equal(calls, 3);
    await writeFile(dependency, 'dependency version 2');
    const changedDependency = await optimizerIdentity([tool, dependency], config);
    assert.notEqual(changedDependency, identity); ++assertions;
    await new OptimizerCache(folder, changedDependency).optimize(wasm, compute);
    equal(calls, 4);
    await writeFile(tool, 'tool version 2');
    const changedTool = await optimizerIdentity([tool, dependency], config);
    assert.notEqual(changedTool, changedDependency); ++assertions;

    const damagedFolder = join(root, 'damaged');
    const damaged = new OptimizerCache(damagedFolder, identity);
    await damaged.optimize(wasm, compute);
    const entry = join(damagedFolder, (await readdir(damagedFolder))[0]);
    for (const mutation of [() => '{broken',
            record => JSON.stringify({ ...record, key: 'wrong' }),
            record => JSON.stringify({ ...record, output: 'broken' }),
            record => JSON.stringify({ ...record, version: 2 })]) {
        await writeFile(entry, mutation(JSON.parse(await readFile(entry, 'utf8'))));
        const previous = calls;
        equal(await damaged.optimize(wasm, compute), wasm);
        equal(calls, previous + 1);
    }
    equal(damaged.invalid, 4);
    equal((await readdir(damagedFolder)).filter(path => path.endsWith('.tmp')), []);

    const failures = new OptimizerCache(join(root, 'failures'), identity);
    await assert.rejects(failures.optimize(wasm, () => { throw Error('compiler failure'); }), /compiler failure/);
    ++assertions;
    await assert.rejects(failures.optimize(wasm, () => Buffer.from('not wasm')), /WASM module/);
    ++assertions;
    equal(await failures.optimize(wasm, compute), wasm);

    // Concurrent writers publish complete identical records, never partial JSON.
    const concurrent = join(root, 'concurrent');
    const a = new OptimizerCache(concurrent, identity), b = new OptimizerCache(concurrent, identity);
    const result = await Promise.all([a.optimize(wasm, async () => wasm), b.optimize(wasm, async () => wasm)]);
    equal(result, [wasm, wasm]);
    equal(await new OptimizerCache(concurrent, identity).optimize(wasm, () => { throw Error('should hit'); }), wasm);
    equal((await readdir(concurrent)).length, 1);
} finally {
    assert((await realpath(root)).startsWith((await realpath(tmpdir())) + sep));
    await rm(root, { recursive: true });
}
console.log(`PASS: ${assertions} optimizer cache checks (fake optimizer only)`);
