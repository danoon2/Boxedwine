// Offline Binaryen output cache. This is separate from the guest JIT cache ABI.
import { createHash, randomUUID } from 'node:crypto';
import { mkdir, readFile, rename, unlink, writeFile } from 'node:fs/promises';
import { join, resolve } from 'node:path';

function sha256(data) {
    return createHash('sha256').update(data).digest('hex');
}

export async function optimizerIdentity(files, configuration) {
    const inputs = [];
    for (const path of [...new Set(files.map(file => resolve(file)))].sort()) {
        inputs.push({ path, sha256: sha256(await readFile(path)) });
    }
    // Include the cache implementation as well as the caller's optimizer code.
    return sha256(JSON.stringify({ version: 1, inputs, configuration,
        implementation: sha256(await readFile(new URL(import.meta.url))) }));
}

export class OptimizerCache {
    constructor(directory, identity) {
        if (!/^[0-9a-f]{64}$/.test(identity)) throw Error('Invalid optimizer identity');
        this.directory = resolve(directory);
        this.identity = identity;
        this.hits = 0;
        this.misses = 0;
        this.invalid = 0;
    }

    async optimize(input, compute) {
        const key = sha256(this.identity + ':' + sha256(input));
        const path = join(this.directory, key + '.json');
        let cached;
        try {
            cached = await readFile(path, 'utf8');
        } catch (error) {
            if (error.code !== 'ENOENT') throw error;
        }
        if (cached !== undefined) {
            try {
                const record = JSON.parse(cached);
                const output = Buffer.from(record.output, 'base64');
                if (record.version !== 1 || record.key !== key || record.sha256 !== sha256(output)
                        || !isWasm(output)) throw Error('Invalid cached optimizer output');
                ++this.hits;
                return output;
            } catch (_) {
                // Interrupted or damaged cache entries must never become compiler output.
                ++this.invalid;
            }
        }
        ++this.misses;
        const output = Buffer.from(await compute());
        if (!isWasm(output)) throw Error('Optimizer did not return a WASM module');
        const record = JSON.stringify({ version: 1, key, sha256: sha256(output), output: output.toString('base64') });
        await mkdir(this.directory, { recursive: true });
        const temporary = join(this.directory, key + '.' + randomUUID() + '.tmp');
        try {
            await writeFile(temporary, record, { flag: 'wx' });
            try {
                await rename(temporary, path);
            } catch (error) {
                // Windows may reject replacement when another writer has just
                // published this entry. Accept only its complete identical data.
                if (!['EEXIST', 'EPERM', 'EACCES'].includes(error.code)
                        || await readFile(path, 'utf8').catch(() => null) !== record) throw error;
            }
        } finally {
            await unlink(temporary).catch(error => { if (error.code !== 'ENOENT') throw error; });
        }
        return output;
    }
}

function isWasm(data) {
    return data.length >= 8 && data.subarray(0, 8).equals(Buffer.from([0, 97, 115, 109, 1, 0, 0, 0]));
}
