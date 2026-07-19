#!/usr/bin/env node

import { inflateRawSync } from 'node:zlib';
import { readFile } from 'node:fs/promises';

const WASM_MAGIC = Buffer.from([0x00, 0x61, 0x73, 0x6d]);

function usage() {
  console.log(`Usage:
  node scripts/boxedwine-wasm-jit-cache-verify-merged.mjs grouped.zip merged.zip

Compares a safe grouped-directory BoxedWine wasm JIT cache against an
experimental --merge-groups cache. This verifies structural invariants needed
for merged group execution: all original block modules are represented, group
exports point at the expected functions, ABI payloads match, and function/code
bodies are byte-identical to the optimized source modules.`);
}

function parseArgs(argv) {
  if (argv.includes('-h') || argv.includes('--help')) {
    usage();
    process.exit(0);
  }
  if (argv.length !== 2) throw new Error('Expected grouped.zip and merged.zip');
  return { groupedZip: argv[0], mergedZip: argv[1] };
}

function readU16(buf, off) {
  if (off + 2 > buf.length) throw new Error(`Unexpected EOF at ${off}`);
  return buf.readUInt16LE(off);
}

function readU32(buf, off) {
  if (off + 4 > buf.length) throw new Error(`Unexpected EOF at ${off}`);
  return buf.readUInt32LE(off);
}

function readWasmUleb(buf, off) {
  let value = 0;
  let shift = 0;
  let pos = off;
  while (pos < buf.length) {
    const byte = buf[pos++];
    value |= (byte & 0x7f) << shift;
    if ((byte & 0x80) === 0) return [value >>> 0, pos];
    shift += 7;
  }
  throw new Error(`Unexpected EOF in LEB at ${off}`);
}

function wasmName(buf, off) {
  const [len, p] = readWasmUleb(buf, off);
  const end = p + len;
  if (end > buf.length) throw new Error('Wasm name extends outside buffer');
  return [buf.subarray(p, end).toString('utf8'), end];
}

function findEndOfCentralDirectory(buf) {
  const min = Math.max(0, buf.length - 0xffff - 22);
  for (let off = buf.length - 22; off >= min; off--) {
    if (readU32(buf, off) === 0x06054b50) return off;
  }
  throw new Error('Could not find ZIP end-of-central-directory record');
}

function parseZip(buf) {
  const eocd = findEndOfCentralDirectory(buf);
  const totalEntries = readU16(buf, eocd + 10);
  const centralSize = readU32(buf, eocd + 12);
  const centralOffset = readU32(buf, eocd + 16);
  if (totalEntries === 0xffff || centralSize === 0xffffffff || centralOffset === 0xffffffff) {
    throw new Error('Zip64 archives are not supported');
  }

  const entries = [];
  let off = centralOffset;
  for (let i = 0; i < totalEntries; i++) {
    if (readU32(buf, off) !== 0x02014b50) throw new Error(`Bad central-directory header at ${off}`);
    const flags = readU16(buf, off + 8);
    const compressedSize = readU32(buf, off + 20);
    const uncompressedSize = readU32(buf, off + 24);
    const nameLen = readU16(buf, off + 28);
    const extraLen = readU16(buf, off + 30);
    const commentLen = readU16(buf, off + 32);
    const localOffset = readU32(buf, off + 42);
    const name = buf.subarray(off + 46, off + 46 + nameLen)
      .toString((flags & (1 << 11)) ? 'utf8' : 'latin1');

    if (readU32(buf, localOffset) !== 0x04034b50) throw new Error(`Bad local header for ${name}`);
    const localMethod = readU16(buf, localOffset + 8);
    const localNameLen = readU16(buf, localOffset + 26);
    const localExtraLen = readU16(buf, localOffset + 28);
    const dataStart = localOffset + 30 + localNameLen + localExtraLen;
    const compressed = buf.subarray(dataStart, dataStart + compressedSize);
    let data;
    if (localMethod === 0) data = Buffer.from(compressed);
    else if (localMethod === 8) data = inflateRawSync(compressed);
    else throw new Error(`Unsupported ZIP method ${localMethod} for ${name}`);
    if (data.length !== uncompressedSize) throw new Error(`Bad uncompressed size for ${name}`);
    entries.push({ name, data });
    off += 46 + nameLen + extraLen + commentLen;
  }
  return entries;
}

function parseWasm(wasm) {
  if (wasm.length < 8 || !wasm.subarray(0, 4).equals(WASM_MAGIC)) {
    throw new Error('Entry is not wasm');
  }
  const info = {
    typePayload: null,
    importPayload: null,
    importedFunctions: 0,
    functionTypes: [],
    exports: new Map(),
    codeBodies: [],
    localCalls: 0,
    sections: [],
  };

  let pos = 8;
  while (pos < wasm.length) {
    const id = wasm[pos++];
    const [size, p] = readWasmUleb(wasm, pos);
    pos = p;
    const start = pos;
    const end = pos + size;
    if (end > wasm.length) throw new Error('Wasm section extends outside module');
    info.sections.push(id);
    if (id === 1) {
      info.typePayload = Buffer.from(wasm.subarray(start, end));
    } else if (id === 2) {
      info.importPayload = Buffer.from(wasm.subarray(start, end));
      let q = start;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let unused; [unused, q] = wasmName(wasm, q);
        [unused, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        if (kind === 0) {
          [, q] = readWasmUleb(wasm, q);
          info.importedFunctions++;
        } else if (kind === 1) {
          [, q] = readWasmUleb(wasm, q);
        } else if (kind === 2) {
          const flags = wasm[q++];
          [, q] = readWasmUleb(wasm, q);
          if (flags & 1) [, q] = readWasmUleb(wasm, q);
        } else if (kind === 3) {
          q += 2;
        } else {
          throw new Error(`Unsupported import kind ${kind}`);
        }
      }
    } else if (id === 3) {
      let q = start;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let typeIndex; [typeIndex, q] = readWasmUleb(wasm, q);
        info.functionTypes.push(typeIndex);
      }
    } else if (id === 7) {
      let q = start;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let exportName; [exportName, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        let index; [index, q] = readWasmUleb(wasm, q);
        if (kind === 0) info.exports.set(exportName, index);
      }
    } else if (id === 10) {
      let q = start;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        const bodyStart = q;
        const [bodySize, bodyPayloadStart] = readWasmUleb(wasm, q);
        q = bodyPayloadStart + bodySize;
        const body = Buffer.from(wasm.subarray(bodyStart, q));
        info.codeBodies.push(body);
      }
    }
    pos = end;
  }

  return info;
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const groupedEntries = parseZip(await readFile(opts.groupedZip));
  const mergedEntries = parseZip(await readFile(opts.mergedZip));
  const groupedManifest = JSON.parse(groupedEntries.find((e) => e.name === 'boxedwine-jit-grouped-manifest.json').data.toString('utf8'));
  const mergedManifest = JSON.parse(mergedEntries.find((e) => e.name === 'boxedwine-jit-grouped-manifest.json').data.toString('utf8'));

  const sourceByKey = new Map();
  for (const entry of groupedEntries) {
    if (!entry.name.endsWith('.wasm')) continue;
    const key = entry.name.split('/').pop().replace(/\.wasm$/i, '');
    sourceByKey.set(key, parseWasm(entry.data));
  }

  let checked = 0;
  let bodyMismatches = 0;
  let exportMismatches = 0;
  let abiMismatches = 0;
  let missing = 0;
  for (const group of mergedManifest.groups || []) {
    const mergedEntry = mergedEntries.find((e) => e.name === group.path);
    if (!mergedEntry) {
      missing += (group.entries || []).length;
      continue;
    }
    const merged = parseWasm(mergedEntry.data);
    for (let i = 0; i < (group.entries || []).length; i++) {
      const item = group.entries[i];
      const source = sourceByKey.get(item.key);
      if (!source) {
        missing++;
        continue;
      }
      checked++;
      if (!source.typePayload.equals(merged.typePayload) || !source.importPayload.equals(merged.importPayload)) {
        abiMismatches++;
      }
      const exportIndex = merged.exports.get(item.exportName || item.key);
      const expectedIndex = merged.importedFunctions + i;
      if (exportIndex !== expectedIndex) {
        exportMismatches++;
      }
      const mergedBody = merged.codeBodies[i];
      if (!mergedBody || !source.codeBodies[0].equals(mergedBody)) {
        bodyMismatches++;
      }
    }
  }

  console.log(`Grouped input:      ${opts.groupedZip}`);
  console.log(`Merged input:       ${opts.mergedZip}`);
  console.log(`Grouped modules:    ${sourceByKey.size}`);
  console.log(`Merged groups:      ${(mergedManifest.groups || []).length}`);
  console.log(`Checked entries:    ${checked}`);
  console.log(`Missing entries:    ${missing}`);
  console.log(`ABI mismatches:     ${abiMismatches}`);
  console.log(`Export mismatches:  ${exportMismatches}`);
  console.log(`Body mismatches:    ${bodyMismatches}`);
  console.log(`Grouped manifest:   mergedGroups=${Boolean(groupedManifest.source?.mergedGroups)}`);
  console.log(`Merged manifest:    mergedGroups=${Boolean(mergedManifest.source?.mergedGroups)}`);
  if (missing || abiMismatches || exportMismatches || bodyMismatches) {
    throw new Error('Merged cache verification failed');
  }
  console.log('Status:            OK');
}

main().catch((err) => {
  console.error(`error: ${err.message}`);
  process.exit(1);
});
