#!/usr/bin/env node

import { deflateRawSync, inflateRawSync } from 'node:zlib';
import { existsSync } from 'node:fs';
import { readFile, writeFile } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const WASM_MAGIC = Buffer.from([0x00, 0x61, 0x73, 0x6d]);
const CACHE_NAME_RE = /^v3-([0-9a-f]{8})-([0-9a-f]{8})\.wasm$/i;
const GROUPED_MANIFEST = 'boxedwine-jit-grouped-manifest.json';
const PROFILE_NAME = 'boxedwine-jit-profile.txt';

const LOCAL_PASSES = [
  'optimize-instructions',
  'precompute',
  'simplify-locals',
  'local-cse',
  'dce',
  'vacuum',
  'coalesce-locals',
];

const gWarnings = [];

function usage() {
  console.log(`Usage:
  node boxedwine-wasm-jit-cache-pipeline.mjs [options] input.zip output.zip

Options:
  --binaryen-js PATH       Optimize wasm modules with Binaryen JS/WASM.
                           Defaults to ./binaryen_js.js if present.
  --flat                   Binaryen-optimize the per-block modules and re-emit
                           a flat cache zip (same v3-*.wasm layout + manifest),
                           skipping grouping, direct-call rewriting and profile
                           split hints. This is the only mode multi-threaded
                           builds can consume: MT workers instantiate raw
                           per-block bytes from shared memory and cannot share
                           merged group instances. Caches remain per-build —
                           record with the same target (jit or multiThreadedJit)
                           that will replay the output.
  --budget BYTES           Target grouped wasm payload size. Default: 524288.
  --interior-profile PATH  Parse BoxedWine console output and add hot interior
                           target split hints to the grouped manifest. If omitted,
                           boxedwine-jit-profile.txt is read from the input zip.
  --profile-split-threshold N
                           Minimum exact interior-target count. Default: 50000.
  --profile-split-limit N  Maximum split hints to emit. Default: 16.
  --profile-split-rejection-samples N
                           Number of rejected split candidates to print. Default: 4.
  --compression-level N    Output zip deflate level, 0-9. Default: 9.
  -h, --help               Show this help.

Produces the current BoxedWine wasm JIT merged grouped-cache format:
  groups/group-XXXX.wasm
  boxedwine-jit-grouped-manifest.json

The input must contain v3 cache module names and boxedwine-jit-manifest.json.`);
}

function parseArgs(argv) {
  const opts = {
    binaryenJs: null,
    flat: false,
    budget: 512 * 1024,
    interiorProfile: null,
    profileSplitThreshold: 50000,
    profileSplitLimit: 16,
    profileSplitRejectionSamples: 4,
    compressionLevel: 9,
    positional: [],
  };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === '-h' || arg === '--help') {
      usage();
      process.exit(0);
    } else if (arg === '--binaryen-js') {
      opts.binaryenJs = argv[++i];
    } else if (arg.startsWith('--binaryen-js=')) {
      opts.binaryenJs = arg.slice('--binaryen-js='.length);
    } else if (arg === '--flat') {
      opts.flat = true;
    } else if (arg === '--budget') {
      opts.budget = Number(argv[++i]);
    } else if (arg.startsWith('--budget=')) {
      opts.budget = Number(arg.slice('--budget='.length));
    } else if (arg === '--interior-profile') {
      opts.interiorProfile = argv[++i];
    } else if (arg.startsWith('--interior-profile=')) {
      opts.interiorProfile = arg.slice('--interior-profile='.length);
    } else if (arg === '--profile-split-threshold') {
      opts.profileSplitThreshold = Number(argv[++i]);
    } else if (arg.startsWith('--profile-split-threshold=')) {
      opts.profileSplitThreshold = Number(arg.slice('--profile-split-threshold='.length));
    } else if (arg === '--profile-split-limit') {
      opts.profileSplitLimit = Number(argv[++i]);
    } else if (arg.startsWith('--profile-split-limit=')) {
      opts.profileSplitLimit = Number(arg.slice('--profile-split-limit='.length));
    } else if (arg === '--profile-split-rejection-samples') {
      opts.profileSplitRejectionSamples = Number(argv[++i]);
    } else if (arg.startsWith('--profile-split-rejection-samples=')) {
      opts.profileSplitRejectionSamples = Number(arg.slice('--profile-split-rejection-samples='.length));
    } else if (arg === '--compression-level') {
      opts.compressionLevel = Number(argv[++i]);
    } else if (arg.startsWith('--compression-level=')) {
      opts.compressionLevel = Number(arg.slice('--compression-level='.length));
    } else if (arg.startsWith('-')) {
      throw new Error(`Unknown option: ${arg}`);
    } else {
      opts.positional.push(arg);
    }
  }
  if (opts.positional.length !== 2) throw new Error('Expected input.zip and output.zip');
  if (!Number.isSafeInteger(opts.budget) || opts.budget <= 0) throw new Error('--budget must be a positive integer');
  if (!Number.isSafeInteger(opts.profileSplitThreshold) || opts.profileSplitThreshold < 1) throw new Error('--profile-split-threshold must be a positive integer');
  if (!Number.isSafeInteger(opts.profileSplitLimit) || opts.profileSplitLimit < 0) throw new Error('--profile-split-limit must be a non-negative integer');
  if (!Number.isSafeInteger(opts.profileSplitRejectionSamples) || opts.profileSplitRejectionSamples < 0) throw new Error('--profile-split-rejection-samples must be a non-negative integer');
  if (!Number.isInteger(opts.compressionLevel) || opts.compressionLevel < 0 || opts.compressionLevel > 9) {
    throw new Error('--compression-level must be an integer from 0 to 9');
  }
  return { ...opts, inputZip: opts.positional[0], outputZip: opts.positional[1] };
}

function readU16(buf, off) {
  if (off + 2 > buf.length) throw new Error(`Unexpected EOF at ${off}`);
  return buf.readUInt16LE(off);
}

function readU32(buf, off) {
  if (off + 4 > buf.length) throw new Error(`Unexpected EOF at ${off}`);
  return buf.readUInt32LE(off);
}

function writeU16(value) {
  const buf = Buffer.alloc(2);
  buf.writeUInt16LE(value);
  return buf;
}

function writeU32(value) {
  const buf = Buffer.alloc(4);
  buf.writeUInt32LE(value >>> 0);
  return buf;
}

function writeWasmUleb(value) {
  const bytes = [];
  let v = value >>> 0;
  do {
    let byte = v & 0x7f;
    v >>>= 7;
    if (v) byte |= 0x80;
    bytes.push(byte);
  } while (v);
  return Buffer.from(bytes);
}

function writeWasmSleb(value) {
  const bytes = [];
  let v = value | 0;
  let more = true;
  while (more) {
    let byte = v & 0x7f;
    v >>= 7;
    const signBitSet = (byte & 0x40) !== 0;
    if ((v === 0 && !signBitSet) || (v === -1 && signBitSet)) {
      more = false;
    } else {
      byte |= 0x80;
    }
    bytes.push(byte);
  }
  return Buffer.from(bytes);
}

function wasmNameBytes(name) {
  const bytes = Buffer.from(name, 'utf8');
  return Buffer.concat([writeWasmUleb(bytes.length), bytes]);
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

function readWasmSleb(buf, off, bits = 32) {
  let value = 0;
  let shift = 0;
  let pos = off;
  let byte = 0;
  while (pos < buf.length) {
    byte = buf[pos++];
    value |= (byte & 0x7f) << shift;
    shift += 7;
    if ((byte & 0x80) === 0) break;
  }
  if ((shift < bits) && (byte & 0x40)) value |= (~0 << shift);
  return [value | 0, pos];
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

function decodeName(buf, flags) {
  return buf.toString((flags & (1 << 11)) ? 'utf8' : 'latin1');
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
    const method = readU16(buf, off + 10);
    const compressedSize = readU32(buf, off + 20);
    const uncompressedSize = readU32(buf, off + 24);
    const nameLen = readU16(buf, off + 28);
    const extraLen = readU16(buf, off + 30);
    const commentLen = readU16(buf, off + 32);
    const localOffset = readU32(buf, off + 42);
    const name = decodeName(buf.subarray(off + 46, off + 46 + nameLen), flags);
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
    entries.push({ name, method, localMethod, compressedSize, uncompressedSize, data });
    off += 46 + nameLen + extraLen + commentLen;
  }
  return entries;
}

const CRC_TABLE = new Uint32Array(256);
for (let n = 0; n < CRC_TABLE.length; n++) {
  let c = n;
  for (let k = 0; k < 8; k++) c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1);
  CRC_TABLE[n] = c >>> 0;
}

function crc32(buf) {
  let c = 0xffffffff;
  for (const byte of buf) c = CRC_TABLE[(c ^ byte) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
}

function buildZip(entries, level) {
  const locals = [];
  const centrals = [];
  let offset = 0;
  for (const entry of entries) {
    const nameBuf = Buffer.from(entry.name, 'utf8');
    const data = Buffer.from(entry.data);
    const compressed = level === 0 ? data : deflateRawSync(data, { level });
    const method = level === 0 ? 0 : 8;
    const crc = crc32(data);
    const local = Buffer.concat([
      writeU32(0x04034b50), writeU16(20), writeU16(1 << 11), writeU16(method),
      writeU16(0), writeU16(0), writeU32(crc), writeU32(compressed.length),
      writeU32(data.length), writeU16(nameBuf.length), writeU16(0), nameBuf, compressed,
    ]);
    locals.push(local);
    const central = Buffer.concat([
      writeU32(0x02014b50), writeU16(0x031e), writeU16(20), writeU16(1 << 11),
      writeU16(method), writeU16(0), writeU16(0), writeU32(crc), writeU32(compressed.length),
      writeU32(data.length), writeU16(nameBuf.length), writeU16(0), writeU16(0),
      writeU16(0), writeU16(0), writeU32(0), writeU32(offset), nameBuf,
    ]);
    centrals.push(central);
    offset += local.length;
  }
  const centralOffset = offset;
  const centralDir = Buffer.concat(centrals);
  const localData = Buffer.concat(locals);
  const eocd = Buffer.concat([
    writeU32(0x06054b50), writeU16(0), writeU16(0), writeU16(entries.length),
    writeU16(entries.length), writeU32(centralDir.length), writeU32(centralOffset), writeU16(0),
  ]);
  return Buffer.concat([localData, centralDir, eocd]);
}

function isWasm(data) {
  return data.length >= 4 && data.subarray(0, 4).equals(WASM_MAGIC);
}

function parseImportShape(wasm) {
  const imports = [];
  let pos = 8;
  while (pos < wasm.length) {
    const id = wasm[pos++];
    const [size, p] = readWasmUleb(wasm, pos);
    pos = p;
    const sectionEnd = pos + size;
    if (id === 2) {
      let q = pos;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let moduleName; [moduleName, q] = wasmName(wasm, q);
        let fieldName; [fieldName, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        const imp = { module: moduleName, field: fieldName, kind };
        if (kind === 0) [imp.typeIndex, q] = readWasmUleb(wasm, q);
        else if (kind === 1) [, q] = readWasmUleb(wasm, q);
        else if (kind === 2) {
          const flags = wasm[q++];
          [, q] = readWasmUleb(wasm, q);
          if (flags & 1) [, q] = readWasmUleb(wasm, q);
        } else if (kind === 3) {
          q += 2;
        } else {
          throw new Error(`Unsupported import kind ${kind}`);
        }
        imports.push(imp);
      }
      break;
    }
    pos = sectionEnd;
  }
  return {
    helperFields: new Set(imports.filter((i) => i.kind === 0 && i.module === 'helpers').map((i) => i.field)),
    memories: imports.filter((i) => i.kind === 2).length,
    totalFunctions: imports.filter((i) => i.kind === 0).length,
  };
}

function section(id, payload) {
  return Buffer.concat([Buffer.from([id]), writeWasmUleb(payload.length), payload]);
}

function splitCodeBody(body) {
  const [bodySize, payloadStart] = readWasmUleb(body, 0);
  const payloadEnd = payloadStart + bodySize;
  if (payloadEnd !== body.length) throw new Error('Code body size does not match payload');
  let pos = payloadStart;
  const [localDeclCount, localDeclCountEnd] = readWasmUleb(body, pos);
  pos = localDeclCountEnd;
  for (let i = 0; i < localDeclCount; i++) {
    [, pos] = readWasmUleb(body, pos);
    pos++;
    if (pos > payloadEnd) throw new Error('Code body local declarations extend outside payload');
  }
  return {
    localDecls: Buffer.from(body.subarray(payloadStart, pos)),
    instructions: Buffer.from(body.subarray(pos, payloadEnd)),
  };
}

function buildCodeBody(localDecls, instructions) {
  const payload = Buffer.concat([localDecls, instructions]);
  return Buffer.concat([writeWasmUleb(payload.length), payload]);
}

// Globals for a merged group module. Index 0 (when present) replaces the
// stripped "env"."relocBase" import: immutable i32, init 0, so every
// relocatable pointer fast path in the merged bodies fails its guard and
// takes the helper slow path. The mutable direct-call depth counter (when
// directCallDepth > 0) comes after it.
function mergedGlobalsSection(hasRelocGlobal, directCallDepth) {
  const globals = [];
  if (hasRelocGlobal) globals.push(Buffer.from([0x7f, 0x00, 0x41, 0x00, 0x0b])); // i32 const, init 0
  if (directCallDepth > 0) globals.push(Buffer.from([0x7f, 0x01, 0x41, 0x00, 0x0b])); // i32 mut, init 0
  if (!globals.length) return [];
  return [section(6, Buffer.concat([writeWasmUleb(globals.length), ...globals]))];
}

function findStaticEipStoreBefore(instructions, end, maxLookback = 128) {
  const start = Math.max(0, end - maxLookback);
  let best = null;
  for (let pos = start; pos < end; pos++) {
    if (instructions[pos] !== 0x20 || instructions[pos + 1] !== 0x00 || instructions[pos + 2] !== 0x41) continue;
    let value;
    let p;
    try {
      [value, p] = readWasmSleb(instructions, pos + 3);
    } catch {
      continue;
    }
    if (p >= end || instructions[p++] !== 0x36) continue; // i32.store
    let align;
    let offset;
    let q;
    try {
      [align, q] = readWasmUleb(instructions, p);
      [offset, q] = readWasmUleb(instructions, q);
    } catch {
      continue;
    }
    if (align <= 2 && offset === 112 && q <= end) {
      best = { eip: value >>> 0, end: q };
    }
  }
  return best;
}

function directCallTail(targetIndex) {
  return Buffer.concat([
    Buffer.from([0x20, 0x00, 0x10]),
    writeWasmUleb(targetIndex),
    Buffer.from([0x0f]),
  ]);
}

function returnCallTail(targetIndex) {
  return Buffer.concat([
    Buffer.from([0x20, 0x00, 0x12]),
    writeWasmUleb(targetIndex),
  ]);
}

function wasmI32Load(offset, align = 2) {
  return Buffer.concat([Buffer.from([0x28]), writeWasmUleb(align), writeWasmUleb(offset)]);
}

function wasmI32Store(offset, align = 2) {
  return Buffer.concat([Buffer.from([0x36]), writeWasmUleb(align), writeWasmUleb(offset)]);
}

function wasmI32Load8U(offset) {
  return Buffer.concat([Buffer.from([0x2d]), writeWasmUleb(0), writeWasmUleb(offset)]);
}

function wasmI32Store8(offset) {
  return Buffer.concat([Buffer.from([0x3a]), writeWasmUleb(0), writeWasmUleb(offset)]);
}

function yieldAwareReturnCallTail(targetIndex, opCount, runtime) {
  const blockInstructionCountOffset = runtime.cpu.blockInstructionCountOffset >>> 0;
  const yieldOffset = runtime.cpu.yieldOffset >>> 0;
  const contextTimeRemainingPtr = runtime.scheduler.contextTimeRemainingPtr >>> 0;
  const count = Math.max(1, opCount >>> 0);
  return Buffer.concat([
    // cpu->blockInstructionCount += opCount
    Buffer.from([0x20, 0x00, 0x20, 0x00]),
    wasmI32Load(blockInstructionCountOffset),
    Buffer.from([0x41]), writeWasmSleb(count),
    Buffer.from([0x6a]),
    wasmI32Store(blockInstructionCountOffset),

    // if ((int)cpu->blockInstructionCount >= contextTimeRemaining || cpu->yield)
    Buffer.from([0x20, 0x00]),
    wasmI32Load(blockInstructionCountOffset),
    Buffer.from([0x41]), writeWasmSleb(contextTimeRemainingPtr | 0),
    wasmI32Load(0),
    Buffer.from([0x4e]), // i32.ge_s
    Buffer.from([0x20, 0x00]),
    wasmI32Load8U(yieldOffset),
    Buffer.from([0x72]), // i32.or
    Buffer.from([0x04, 0x40]), // if
    Buffer.from([0x20, 0x00, 0x41, 0x01]),
    wasmI32Store8(yieldOffset),
    Buffer.from([0x20, 0x00, 0x10, 0x06, 0x0f]), // call fetchNextOp; return
    Buffer.from([0x0b]), // end

    Buffer.from([0x20, 0x00, 0x12]), writeWasmUleb(targetIndex),
  ]);
}

function guardedDirectCallTail(targetIndex, depthLimit, depthGlobalIndex = 0) {
  const g = writeWasmUleb(depthGlobalIndex);
  return Buffer.concat([
    Buffer.from([0x23]), g,                  // global.get depth
    Buffer.from([0x41]), writeWasmSleb(depthLimit),
    Buffer.from([0x49]),                     // i32.lt_u
    Buffer.from([0x04, 0x40]),               // if
    Buffer.from([0x23]), g, Buffer.from([0x41]), writeWasmSleb(1), Buffer.from([0x6a, 0x24]), g, // depth++
    Buffer.from([0x20, 0x00, 0x10]), writeWasmUleb(targetIndex),
    Buffer.from([0x23]), g, Buffer.from([0x41]), writeWasmSleb(1), Buffer.from([0x6b, 0x24]), g, // depth--
    Buffer.from([0x0f, 0x0b]),               // return; end
    Buffer.from([0x20, 0x00, 0x10, 0x06, 0x0f]), // original helper fallback
  ]);
}

function rewriteDirectCallsInBody(body, targetIndexByEip, options = {}) {
  if (!targetIndexByEip.size) return { body, rewritten: 0 };
  const { localDecls, instructions } = splitCodeBody(body);
  const out = [];
  let pos = 0;
  let rewritten = 0;
  let guarded = 0;
  while (pos < instructions.length) {
    if (instructions[pos] === 0x20 && instructions[pos + 1] === 0x00 &&
        instructions[pos + 2] === 0x10 && instructions[pos + 3] === 0x06 &&
        instructions[pos + 4] === 0x0f) {
      const eipStore = findStaticEipStoreBefore(instructions, pos);
      const target = eipStore ? targetIndexByEip.get(eipStore.eip) : undefined;
      if (target !== undefined) {
        if (target.yieldAwareTailCall) {
          out.push(yieldAwareReturnCallTail(target.index, options.opCount, options.runtime));
          guarded++;
        } else if (target.tailCall) {
          out.push(returnCallTail(target.index));
        } else if (target.guarded) {
          out.push(guardedDirectCallTail(target.index, options.directCallDepth, options.depthGlobalIndex || 0));
          guarded++;
        } else {
          out.push(directCallTail(target.index));
        }
        pos += 5;
        rewritten++;
        continue;
      }
    }
    out.push(Buffer.from([instructions[pos++]]));
  }
  return {
    body: buildCodeBody(localDecls, Buffer.concat(out)),
    rewritten,
    guarded,
  };
}

function parseWasmForMerge(wasm) {
  if (!isWasm(wasm)) throw new Error('Entry is not a wasm module');
  const parsed = {
    typePayload: null,
    importPayload: null,
    importedFunctions: 0,
    functionTypes: [],
    codeBodies: [],
    hadRelocImport: false,
  };
  let pos = 8;
  while (pos < wasm.length) {
    const id = wasm[pos++];
    const [size, p] = readWasmUleb(wasm, pos);
    pos = p;
    const sectionStart = pos;
    const sectionEnd = pos + size;
    if (sectionEnd > wasm.length) throw new Error('Wasm section extends outside module');
    if (id === 1) {
      parsed.typePayload = Buffer.from(wasm.subarray(sectionStart, sectionEnd));
    } else if (id === 2) {
      // Relocatable (persistence-mode) modules import an immutable i32
      // global "env"."relocBase" whose value is per-block and only known to
      // the runtime. Merged group modules can't supply per-function values,
      // so strip the import here and let buildMergedGroupWasm define a
      // local global 0 initialized to 0 instead — the generated fast paths
      // guard relocBase != 0 and fall back to their helper slow paths, so
      // a merged module behaves exactly like pre-reloc relocatable code.
      // Stripping also normalizes the payload so modules with and without
      // the import pass the ABI-equality check below.
      let q = sectionStart;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      const keptEntries = [];
      for (let i = 0; i < count; i++) {
        const entryStart = q;
        let moduleName; [moduleName, q] = wasmName(wasm, q);
        let fieldName; [fieldName, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        if (kind === 0) {
          [, q] = readWasmUleb(wasm, q);
          parsed.importedFunctions++;
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
        if (kind === 3 && moduleName === 'env' && fieldName === 'relocBase') {
          parsed.hadRelocImport = true;
          continue;
        }
        keptEntries.push(Buffer.from(wasm.subarray(entryStart, q)));
      }
      parsed.importPayload = parsed.hadRelocImport
        ? Buffer.concat([writeWasmUleb(count - 1), ...keptEntries])
        : Buffer.from(wasm.subarray(sectionStart, sectionEnd));
    } else if (id === 3) {
      let q = sectionStart;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let typeIndex; [typeIndex, q] = readWasmUleb(wasm, q);
        parsed.functionTypes.push(typeIndex);
      }
    } else if (id === 10) {
      let q = sectionStart;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        const bodyStart = q;
        const [bodySize, bodyPayloadStart] = readWasmUleb(wasm, q);
        q = bodyPayloadStart + bodySize;
        parsed.codeBodies.push(Buffer.from(wasm.subarray(bodyStart, q)));
      }
    }
    pos = sectionEnd;
  }
  if (!parsed.typePayload || !parsed.importPayload || parsed.functionTypes.length !== 1 || parsed.codeBodies.length !== 1) {
    throw new Error('Expected each JIT wasm module to contain one local function and one code body');
  }
  return parsed;
}

function buildMergedGroupWasm(sortedEntries, options = {}) {
  if (!sortedEntries.length) throw new Error('Cannot merge an empty group');
  const parsedEntries = sortedEntries.map((entry) => parseWasmForMerge(entry.data));
  const first = parsedEntries[0];
  // Any body that referenced the stripped relocBase import needs the merged
  // module to define global 0 (= 0) in its place; the depth counter (when
  // enabled) then lives at index 1 instead of 0.
  const hasRelocGlobal = parsedEntries.some((parsed) => parsed.hadRelocImport);
  const depthGlobalIndex = hasRelocGlobal ? 1 : 0;
  const sortedKeys = new Set(sortedEntries.map((entry) => entry.key));
  const functionIndexByKey = new Map(sortedEntries.map((entry, i) => [entry.key, first.importedFunctions + i]));
  const directTargetsByKey = new Map();
  if (options.directCalls) {
    for (const edge of options.edges || []) {
      if (!edge.resolved || !sortedKeys.has(edge.from) || !sortedKeys.has(edge.to)) continue;
      let targets = directTargetsByKey.get(edge.from);
      if (!targets) {
        targets = new Map();
        directTargetsByKey.set(edge.from, targets);
      }
      targets.set(edge.targetEip >>> 0, {
        index: functionIndexByKey.get(edge.to),
        tailCall: Boolean(options.tailCalls),
        yieldAwareTailCall: Boolean(options.yieldAwareTailCalls && edge.guarded),
        guarded: Boolean(edge.guarded),
      });
    }
  }
  const functionTypes = [];
  const codeBodies = [];
  let directCallsRewritten = 0;
  let guardedDirectCallsRewritten = 0;
  for (let entryIndex = 0; entryIndex < sortedEntries.length; entryIndex++) {
    const entry = sortedEntries[entryIndex];
    const parsed = parsedEntries[entryIndex];
    if (!parsed.typePayload.equals(first.typePayload) || !parsed.importPayload.equals(first.importPayload)) {
      throw new Error(`Module ABI mismatch while merging ${entry.key}`);
    }
    functionTypes.push(...parsed.functionTypes);
    let body = parsed.codeBodies[0];
    const directTargets = directTargetsByKey.get(entry.key);
    if (directTargets) {
      const rewritten = rewriteDirectCallsInBody(body, directTargets, {
        ...options,
        opCount: entry.opCount,
        depthGlobalIndex,
      });
      body = rewritten.body;
      directCallsRewritten += rewritten.rewritten;
      guardedDirectCallsRewritten += rewritten.guarded;
    }
    codeBodies.push(body);
  }
  const functionPayload = Buffer.concat([
    writeWasmUleb(functionTypes.length),
    ...functionTypes.map((typeIndex) => writeWasmUleb(typeIndex)),
  ]);
  const exportItems = [];
  for (let i = 0; i < sortedEntries.length; i++) {
    exportItems.push(Buffer.concat([
      wasmNameBytes(sortedEntries[i].key),
      Buffer.from([0]),
      writeWasmUleb(first.importedFunctions + i),
    ]));
  }
  const exportPayload = Buffer.concat([writeWasmUleb(exportItems.length), ...exportItems]);
  const codePayload = Buffer.concat([writeWasmUleb(codeBodies.length), ...codeBodies]);
  const wasm = Buffer.concat([
    WASM_MAGIC,
    Buffer.from([0x01, 0x00, 0x00, 0x00]),
    section(1, first.typePayload),
    section(2, first.importPayload),
    section(3, functionPayload),
    ...mergedGlobalsSection(hasRelocGlobal, options.directCallDepth),
    section(7, exportPayload),
    section(10, codePayload),
  ]);
  return { wasm, directCallsRewritten, guardedDirectCallsRewritten };
}

function hex32(value) {
  return `0x${(value >>> 0).toString(16).padStart(8, '0')}`;
}

function parseHex32(text) {
  return Number.parseInt(text.replace(/^0x/i, ''), 16) >>> 0;
}

function parseInteriorProfileText(text) {
  const pairs = new Map();
  const topRe = /interiorTop\[([^\]]*)\]/g;
  let topMatch;
  while ((topMatch = topRe.exec(text)) !== null) {
    const body = topMatch[1].trim();
    if (!body) continue;
    for (const rawPart of body.split(',')) {
      const part = rawPart.trim();
      const match = /^([0-9a-f]{8})@([0-9a-f]{8})=(\d+)(?:\/(\d+))?$/i.exec(part);
      if (!match) continue;
      const target = parseHex32(match[1]);
      const blockStart = parseHex32(match[2]);
      const approxCount = Number(match[3]);
      const exactCount = Number(match[4] ?? match[3]);
      if (!Number.isSafeInteger(approxCount) || !Number.isSafeInteger(exactCount)) continue;
      const key = `${match[2].toLowerCase()}:${match[1].toLowerCase()}`;
      const existing = pairs.get(key);
      if (!existing || exactCount > existing.exactCount || approxCount > existing.approxCount) {
        pairs.set(key, { blockStart, target, approxCount, exactCount });
      }
    }
  }
  return Array.from(pairs.values());
}

async function loadProfileGuidedSplits(opts, entries, embeddedProfileText = null) {
  if (!opts.interiorProfile || opts.profileSplitLimit === 0) {
    if (!embeddedProfileText || opts.profileSplitLimit === 0) {
      return { source: opts.interiorProfile || null, threshold: opts.profileSplitThreshold, limit: opts.profileSplitLimit, parsed: 0, belowThreshold: 0, rejected: 0, rejectedByReason: {}, rejectedSamples: [], targets: [] };
    }
  }

  const text = opts.interiorProfile ? await readFile(opts.interiorProfile, 'utf8') : embeddedProfileText;
  const profiled = parseInteriorProfileText(text);
  const entriesByStart = new Map();
  for (const entry of entries) {
    const start = entry.eip >>> 0;
    const len = entry.emulatedLen >>> 0;
    if (!len || !entry.key) continue;
    if (!entriesByStart.has(start)) entriesByStart.set(start, []);
    entriesByStart.get(start).push(entry);
  }

  let rejected = 0;
  let belowThreshold = 0;
  const rejectedByReason = {
    noBlockStart: 0,
    targetAtBlockStart: 0,
    targetAtBlockEnd: 0,
    targetNotInterior: 0,
  };
  const rejectedSamples = [];
  const selectedByBlock = new Map();
  for (const item of profiled) {
    if (item.exactCount < opts.profileSplitThreshold) {
      belowThreshold++;
      continue;
    }
    const candidates = entriesByStart.get(item.blockStart) || [];
    const owner = candidates.find((entry) => {
      const start = entry.eip >>> 0;
      const end = (start + (entry.emulatedLen >>> 0)) >>> 0;
      return item.target > start && item.target < end;
    });
    if (!owner) {
      rejected++;
      let reason = 'targetNotInterior';
      if (candidates.length === 0) {
        reason = 'noBlockStart';
      } else if (candidates.some((entry) => item.target === (entry.eip >>> 0))) {
        reason = 'targetAtBlockStart';
      } else if (candidates.some((entry) => item.target === (((entry.eip >>> 0) + ((entry.emulatedLen || 0) >>> 0)) >>> 0))) {
        reason = 'targetAtBlockEnd';
      }
      rejectedByReason[reason]++;
      if (rejectedSamples.length < opts.profileSplitRejectionSamples) {
        rejectedSamples.push({
          reason,
          blockStart: item.blockStart,
          target: item.target,
          approxCount: item.approxCount,
          exactCount: item.exactCount,
          candidates: candidates.slice(0, 3).map((entry) => ({
            key: entry.key,
            start: entry.eip >>> 0,
            end: ((entry.eip >>> 0) + ((entry.emulatedLen || 0) >>> 0)) >>> 0,
            emulatedLen: (entry.emulatedLen || 0) >>> 0,
          })),
        });
      }
      continue;
    }
    const existing = selectedByBlock.get(item.blockStart);
    if (!existing || item.exactCount > existing.exactCount || item.approxCount > existing.approxCount) {
      selectedByBlock.set(item.blockStart, {
        blockStart: item.blockStart,
        target: item.target,
        byteDistance: (item.target - item.blockStart) >>> 0,
        approxCount: item.approxCount,
        exactCount: item.exactCount,
        containingKey: owner.key,
      });
    }
  }

  const targets = Array.from(selectedByBlock.values())
    .sort((a, b) => b.exactCount - a.exactCount || b.approxCount - a.approxCount)
    .slice(0, opts.profileSplitLimit);
  return {
    source: opts.interiorProfile || PROFILE_NAME,
    threshold: opts.profileSplitThreshold,
    limit: opts.profileSplitLimit,
    parsed: profiled.length,
    belowThreshold,
    rejected,
    rejectedByReason,
    rejectedSamples,
    targets,
  };
}

function formatBytes(n) {
  return `${n.toLocaleString()} B`;
}

function pct(n, d) {
  return d ? (100 * n / d).toFixed(2) : '0.00';
}

function buildGraph(entries) {
  const byKey = new Map(entries.map((entry) => [entry.key, entry]));
  const byEip = new Map();
  for (const entry of entries) {
    const eip = entry.eip >>> 0;
    if (!byEip.has(eip)) byEip.set(eip, []);
    byEip.get(eip).push(entry);
  }
  const findInteriorTargets = (targetEip) => entries.filter((entry) => {
    const start = entry.eip >>> 0;
    const len = entry.emulatedLen >>> 0;
    if (!len || targetEip === start) return false;
    return targetEip > start && targetEip < ((start + len) >>> 0);
  });
  const adjacency = new Map();
  const reverse = new Map();
  const edges = [];
  for (const entry of entries) {
    const from = entry.key;
    const neighbors = new Set();
    for (const kind of ['next1', 'next2', 'jump']) {
      const exit = entry.exits?.[kind];
      if (!exit?.count || !exit.firstTarget) continue;
      const to = exit.firstTarget >>> 0;
      const candidates = byEip.get(to) || [];
      const resolved = candidates.length === 1;
      const interiorCandidates = resolved ? [] : findInteriorTargets(to);
      edges.push({
        from,
        to: resolved ? candidates[0].key : null,
        interiorTo: interiorCandidates.length === 1 ? interiorCandidates[0].key : null,
        targetEip: to,
        kind,
        count: exit.count,
        resolved,
        ambiguous: candidates.length > 1,
        interior: !resolved && interiorCandidates.length === 1,
        ambiguousInterior: !resolved && interiorCandidates.length > 1,
      });
      if (resolved) {
        const targetKey = candidates[0].key;
        neighbors.add(targetKey);
        if (!reverse.has(targetKey)) reverse.set(targetKey, new Set());
        reverse.get(targetKey).add(from);
      }
    }
    adjacency.set(from, neighbors);
  }
  return { byKey, byEip, adjacency, reverse, edges };
}

function findCyclicKeys(entries, edges) {
  const adjacency = new Map(entries.map((entry) => [entry.key, []]));
  for (const edge of edges) {
    if (edge.resolved && edge.to) adjacency.get(edge.from)?.push(edge.to);
  }

  let nextIndex = 0;
  const indexByKey = new Map();
  const lowByKey = new Map();
  const stack = [];
  const onStack = new Set();
  const cyclic = new Set();

  function visit(key) {
    indexByKey.set(key, nextIndex);
    lowByKey.set(key, nextIndex);
    nextIndex++;
    stack.push(key);
    onStack.add(key);

    for (const next of adjacency.get(key) || []) {
      if (!indexByKey.has(next)) {
        visit(next);
        lowByKey.set(key, Math.min(lowByKey.get(key), lowByKey.get(next)));
      } else if (onStack.has(next)) {
        lowByKey.set(key, Math.min(lowByKey.get(key), indexByKey.get(next)));
      }
    }

    if (lowByKey.get(key) === indexByKey.get(key)) {
      const component = [];
      let item;
      do {
        item = stack.pop();
        onStack.delete(item);
        component.push(item);
      } while (item !== key);

      if (component.length > 1) {
        for (const member of component) cyclic.add(member);
      } else {
        const only = component[0];
        if ((adjacency.get(only) || []).includes(only)) cyclic.add(only);
      }
    }
  }

  for (const entry of entries) {
    if (!indexByKey.has(entry.key)) visit(entry.key);
  }
  return cyclic;
}

function findComponents(entries, graph) {
  const seen = new Set();
  const components = [];
  for (const entry of entries) {
    const start = entry.key;
    if (seen.has(start)) continue;
    const stack = [start];
    const keys = [];
    seen.add(start);
    while (stack.length) {
      const key = stack.pop();
      keys.push(key);
      for (const next of graph.adjacency.get(key) ?? []) {
        if (!seen.has(next)) {
          seen.add(next);
          stack.push(next);
        }
      }
      for (const prev of graph.reverse.get(key) ?? []) {
        if (!seen.has(prev)) {
          seen.add(prev);
          stack.push(prev);
        }
      }
    }
    const componentEntries = keys.map((key) => graph.byKey.get(key));
    const wasmBytes = componentEntries.reduce((sum, item) => sum + (item.wasmBytes || 0), 0);
    components.push({ entries: componentEntries, wasmBytes });
  }
  return components.sort((a, b) => b.wasmBytes - a.wasmBytes);
}

function splitOversizedComponent(component, budget) {
  if (component.wasmBytes <= budget) return [component];
  const chunks = [];
  let current = [];
  let currentBytes = 0;
  for (const entry of [...component.entries].sort((a, b) => (b.wasmBytes || 0) - (a.wasmBytes || 0))) {
    const bytes = entry.wasmBytes || 0;
    if (current.length && currentBytes + bytes > budget) {
      chunks.push({ entries: current, wasmBytes: currentBytes });
      current = [];
      currentBytes = 0;
    }
    current.push(entry);
    currentBytes += bytes;
  }
  if (current.length) chunks.push({ entries: current, wasmBytes: currentBytes });
  return chunks;
}

function packComponents(components, budget) {
  const items = components.flatMap((component) => splitOversizedComponent(component, budget));
  items.sort((a, b) => b.wasmBytes - a.wasmBytes);
  const groups = [];
  for (const item of items) {
    let best = null;
    let bestRemainder = Infinity;
    for (const group of groups) {
      const remainder = budget - group.wasmBytes - item.wasmBytes;
      if (remainder >= 0 && remainder < bestRemainder) {
        best = group;
        bestRemainder = remainder;
      }
    }
    if (!best) {
      best = { entries: [], wasmBytes: 0 };
      groups.push(best);
    }
    best.entries.push(...item.entries);
    best.wasmBytes += item.wasmBytes;
  }
  return groups.sort((a, b) => b.wasmBytes - a.wasmBytes);
}

function findDefaultBinaryenJs() {
  for (const candidate of ['./binaryen_js.js', './binaryen.js', './bin/binaryen_js.js', './bin/binaryen.js']) {
    const resolved = resolve(candidate);
    if (existsSync(resolved)) return resolved;
  }
  return null;
}

async function loadBinaryen(path) {
  const binaryenPath = resolve(path);
  const imported = await import(pathToFileURL(binaryenPath).href);
  const mod = imported.default ?? imported.Binaryen ?? imported;
  if (typeof mod === 'function') {
    return await mod({ locateFile(file) { return resolve(dirname(binaryenPath), file); } });
  }
  return mod;
}

function configureLocalPreset(binaryen) {
  binaryen.setOptimizeLevel?.(2);
  binaryen.setShrinkLevel?.(0);
  return LOCAL_PASSES;
}

function optimizeWasm(binaryen, data) {
  // The generated block modules use post-MVP features (sign-extension ops;
  // shared memory in MT builds). Older binaryen.js exposed
  // readBinaryWithFeatures; newer versions dropped it, so fall back to
  // readBinary + Module.setFeatures before validating/optimizing.
  const features = binaryen.Features?.All ??
    ((binaryen.Features?.SignExt ?? 0) | (binaryen.Features?.Atomics ?? 0));
  const mod = binaryen.readBinaryWithFeatures
    ? binaryen.readBinaryWithFeatures(new Uint8Array(data), features)
    : binaryen.readBinary(new Uint8Array(data));
  try {
    mod.setFeatures?.(features);
    mod.runPasses(configureLocalPreset(binaryen));
    if (!mod.validate()) throw new Error('Binaryen validation failed');
    return Buffer.from(mod.emitBinary());
  } finally {
    mod.dispose();
  }
}

async function loadCacheZip(path) {
  const zipBuf = await readFile(path);
  const zipEntries = parseZip(zipBuf);
  const rawManifest = zipEntries.find((entry) => entry.name === 'boxedwine-jit-manifest.json');
  if (!rawManifest) throw new Error(`${path} must contain boxedwine-jit-manifest.json`);
  const manifest = JSON.parse(rawManifest.data.toString('utf8'));
  if (manifest.version !== 1 || !Array.isArray(manifest.entries)) {
    throw new Error(`Unsupported or missing boxedwine-jit-manifest.json entries in ${path}`);
  }
  const rawProfile = zipEntries.find((entry) => entry.name === PROFILE_NAME);
  const wasmByKey = new Map();
  for (const entry of zipEntries) {
    if (!isWasm(entry.data)) continue;
    const match = CACHE_NAME_RE.exec(entry.name);
    if (!match) continue;
    const key = entry.name.replace(/\.wasm$/i, '');
    wasmByKey.set(key, {
      key,
      eip: parseInt(match[1], 16) >>> 0,
      blockHash: parseInt(match[2], 16) >>> 0,
      data: Buffer.from(entry.data),
    });
  }

  const missingManifestKeys = [];
  const entries = manifest.entries.flatMap((entry) => {
    const wasm = wasmByKey.get(entry.key);
    if (!wasm) {
      missingManifestKeys.push(entry.key);
      return [];
    }
    return [{
      key: entry.key,
      eip: entry.eip >>> 0,
      blockHash: entry.blockHash >>> 0,
      wasmBytes: entry.wasmBytes || wasm.data.length,
      opCount: entry.opCount || 0,
      emulatedLen: entry.emulatedLen || 0,
      exits: entry.exits || {},
      data: wasm.data,
    }];
  });
  if (missingManifestKeys.length) {
    gWarnings.push(`${path}: ignored ${missingManifestKeys.length} manifest entries with no wasm entry: ${missingManifestKeys.slice(0, 8).join(', ')}${missingManifestKeys.length > 8 ? ', ...' : ''}`);
  }

  return {
    path,
    zipBuf,
    manifest,
    rawProfile,
    profileText: rawProfile ? rawProfile.data.toString('utf8') : null,
    entries,
    missingManifestKeys,
  };
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const inputCache = await loadCacheZip(opts.inputZip);
  const manifest = inputCache.manifest;
  const entries = inputCache.entries;
  const embeddedProfileText = inputCache.profileText;
  const runtime = manifest.runtime;
  if (!runtime?.cpu || !runtime?.scheduler ||
      !Number.isInteger(runtime.cpu.blockInstructionCountOffset) ||
      !Number.isInteger(runtime.cpu.yieldOffset) ||
      !Number.isInteger(runtime.scheduler.contextTimeRemainingPtr)) {
    throw new Error('Input manifest is missing runtime CPU/scheduler constants; regenerate the JIT cache zip with the updated BoxedWine build');
  }
  // Multi-threaded builds have no scheduler slice-budget global, so their
  // manifests carry contextTimeRemainingPtr=0. Grouped mode would bake that
  // address into yield-aware direct tail calls — refuse rather than emit
  // modules that poke memory at 0. (Grouped output is single-threaded-only
  // anyway; MT consumes --flat output, which never bakes runtime addresses.)
  if (!opts.flat && runtime.scheduler.contextTimeRemainingPtr === 0) {
    throw new Error('Input was recorded by a multi-threaded build; grouped mode is single-threaded-only — run with --flat');
  }
  // Flat mode carries no grouped manifest, so there is nowhere to put split
  // hints; skip the profile parse entirely.
  const profileGuidedSplits = opts.flat
    ? { targets: [] }
    : await loadProfileGuidedSplits(opts, entries, embeddedProfileText);

  const binaryenJs = opts.binaryenJs ? resolve(opts.binaryenJs) : findDefaultBinaryenJs();
  if (!binaryenJs) throw new Error('Could not find binaryen_js.js; pass --binaryen-js PATH');
  const binaryen = await loadBinaryen(binaryenJs);

  let optimized = 0;
  let wasmBefore = 0;
  let wasmAfter = 0;
  for (const entry of entries) {
    wasmBefore += entry.data.length;
    entry.data = optimizeWasm(binaryen, entry.data);
    optimized++;
    entry.wasmBytes = entry.data.length;
    wasmAfter += entry.data.length;
  }

  if (opts.flat) {
    // Re-emit the optimized per-block modules in the original flat layout.
    // The runtime consumes this exactly like a recorded zip (flat lookup in
    // ST, raw-bytes preference in MT); the refreshed manifest keeps the zip
    // analyzable and re-pipeable.
    const outEntries = entries.map((entry) => ({
      name: `${entry.key}.wasm`,
      data: entry.data,
    }));
    const flatManifest = {
      version: 1,
      generatedAt: new Date().toISOString(),
      entryCount: entries.length,
      runtime: manifest.runtime,
      source: {
        inputZip: opts.inputZip,
        inputManifestVersion: manifest.version,
        optimized: true,
        preset: 'local',
        flat: true,
      },
      entries: entries.map((entry) => ({
        key: entry.key,
        eip: entry.eip,
        blockHash: entry.blockHash,
        wasmBytes: entry.data.length,
        opCount: entry.opCount,
        emulatedLen: entry.emulatedLen,
        exits: entry.exits,
      })),
    };
    outEntries.push({
      name: 'boxedwine-jit-manifest.json',
      data: Buffer.from(`${JSON.stringify(flatManifest, null, 2)}\n`, 'utf8'),
    });
    if (embeddedProfileText) {
      outEntries.push({ name: PROFILE_NAME, data: Buffer.from(embeddedProfileText, 'utf8') });
    }
    const out = buildZip(outEntries, opts.compressionLevel);
    await writeFile(opts.outputZip, out);

    console.log(`Input:              ${opts.inputZip}`);
    console.log(`Output:             ${opts.outputZip}`);
    console.log(`Mode:               flat (Binaryen optimize only; no grouping/direct calls/splits)`);
    console.log(`Optimized:          ${optimized}/${entries.length}`);
    console.log(`Wasm bytes:         ${formatBytes(wasmBefore)} -> ${formatBytes(wasmAfter)} (${pct(wasmAfter - wasmBefore, wasmBefore)}%)`);
    console.log(`Zip bytes:          ${formatBytes(inputCache.zipBuf.length)} -> ${formatBytes(out.length)}`);
    for (const warning of gWarnings) {
      console.log(`Warning:            ${warning}`);
    }
    return;
  }

  const shape = parseImportShape(entries[0].data);
  const graph = buildGraph(entries);
  const cyclicKeys = findCyclicKeys(entries, graph.edges);
  const components = findComponents(entries, graph);
  const groups = packComponents(components, opts.budget);
  const groupByKey = new Map();
  for (let i = 0; i < groups.length; i++) {
    for (const entry of groups[i].entries) groupByKey.set(entry.key, i);
  }
  let resolvedEdges = 0;
  let unresolvedEdges = 0;
  let ambiguousEdges = 0;
  let interiorEdges = 0;
  let ambiguousInteriorEdges = 0;
  let inGroupEdges = 0;
  const directEdgesByGroup = new Array(groups.length).fill(0);
  const directCallsRewrittenByGroup = new Array(groups.length).fill(0);
  const guardedDirectCallsRewrittenByGroup = new Array(groups.length).fill(0);
  for (const edge of graph.edges) {
    if (!edge.resolved) {
      if (edge.ambiguous) ambiguousEdges++;
      if (edge.interior) interiorEdges++;
      if (edge.ambiguousInterior) ambiguousInteriorEdges++;
      unresolvedEdges++;
      continue;
    }
    resolvedEdges++;
    const groupId = groupByKey.get(edge.from);
    if (groupId === groupByKey.get(edge.to)) {
      inGroupEdges++;
      directEdgesByGroup[groupId]++;
    }
  }

  const outEntries = [];
  const manifestGroups = groups.map((group, index) => {
    const groupName = `group-${String(index).padStart(4, '0')}`;
    const sorted = [...group.entries].sort((a, b) => a.eip - b.eip);
    const groupPath = `groups/${groupName}.wasm`;
    const directEdges = graph.edges
      .filter((edge) => edge.resolved)
      .filter((edge) => groupByKey.get(edge.from) === groupByKey.get(edge.to))
      .map((edge) => ({
        ...edge,
        guarded: cyclicKeys.has(edge.from) || cyclicKeys.has(edge.to),
      }));
    const merged = buildMergedGroupWasm(sorted, {
      directCalls: true,
      tailCalls: true,
      yieldAwareTailCalls: true,
      runtime: manifest.runtime,
      directCallDepth: 0,
      edges: directEdges,
    });
    const mergedWasm = merged.wasm;
    directCallsRewrittenByGroup[index] = merged.directCallsRewritten;
    guardedDirectCallsRewrittenByGroup[index] = merged.guardedDirectCallsRewritten;
    const groupWasmBytes = mergedWasm.length;
    outEntries.push({
      name: groupPath,
      data: mergedWasm,
    });
    return {
      id: index,
      name: groupName,
      path: groupPath,
      wasmBytes: groupWasmBytes,
      directCallCandidateEdges: directEdgesByGroup[index],
      directCallsRewritten: directCallsRewrittenByGroup[index],
      guardedDirectCallsRewritten: guardedDirectCallsRewrittenByGroup[index],
      entries: sorted.map((entry) => ({
        key: entry.key,
        path: groupPath,
        exportName: entry.key,
        eip: hex32(entry.eip),
        blockHash: hex32(entry.blockHash),
        wasmBytes: entry.wasmBytes,
        opCount: entry.opCount,
        emulatedLen: entry.emulatedLen,
      })),
    };
  });

  const groupedManifest = {
    version: 2,
    format: 'boxedwine-wasm-jit-grouped-cache',
    source: {
      inputZip: opts.inputZip,
      inputManifestVersion: manifest.version,
      optimized: true,
      preset: 'local',
      budgetBytes: opts.budget,
      mergedGroups: true,
      directCalls: true,
      tailCalls: true,
      yieldAwareTailCalls: true,
      directCallDepth: 0,
      interiorProfile: opts.interiorProfile,
      profileSplitThreshold: opts.profileSplitThreshold,
      profileSplitLimit: opts.profileSplitLimit,
    },
    stats: {
      modules: entries.length,
      groups: groups.length,
      components: components.length,
      wasmBytesBefore: wasmBefore,
      wasmBytesAfter: wasmAfter,
      mergedWasmBytes: manifestGroups.reduce((sum, group) => sum + group.wasmBytes, 0),
      graphEdges: {
        total: graph.edges.length,
        resolved: resolvedEdges,
        unresolved: unresolvedEdges,
        ambiguous: ambiguousEdges,
        interior: interiorEdges,
        ambiguousInterior: ambiguousInteriorEdges,
        inGroup: inGroupEdges,
      },
      directCalls: {
        candidateEdges: inGroupEdges,
        rewritten: directCallsRewrittenByGroup.reduce((sum, count) => sum + count, 0),
        guardedRewritten: guardedDirectCallsRewrittenByGroup.reduce((sum, count) => sum + count, 0),
        tailCalls: true,
        yieldAwareTailCalls: true,
        cyclicBlocksExcluded: cyclicKeys.size,
        depthLimit: 0,
      },
      imports: {
        helperFieldsPerModule: shape.helperFields.size,
        functionImportsBefore: shape.totalFunctions * entries.length,
        functionImportsAfterGroupedEstimate: shape.totalFunctions * groups.length,
        memoryImportsBefore: shape.memories * entries.length,
        memoryImportsAfterGroupedEstimate: shape.memories * groups.length,
      },
    },
    profileGuidedSplits,
    groups: manifestGroups,
  };
  outEntries.push({
    name: GROUPED_MANIFEST,
    data: Buffer.from(`${JSON.stringify(groupedManifest, null, 2)}\n`, 'utf8'),
  });

  const out = buildZip(outEntries, opts.compressionLevel);
  await writeFile(opts.outputZip, out);

  console.log(`Input:              ${opts.inputZip}`);
  console.log(`Output:             ${opts.outputZip}`);
  console.log(`Optimized:          ${optimized}/${entries.length}`);
  console.log(`Budget:             ${formatBytes(opts.budget)}`);
  console.log(`Modules:            ${entries.length}`);
  console.log(`Groups:             ${groups.length}`);
  console.log(`Merged groups:      yes`);
  const directStats = groupedManifest.stats.directCalls;
  console.log(`Direct calls:       ${directStats.rewritten} tail sites rewritten, candidateEdges=${directStats.candidateEdges}, tailCalls=yes, yieldAware=yes, guarded=${directStats.guardedRewritten}, cyclicBlocks=${directStats.cyclicBlocksExcluded}`);
  console.log(`Components:         ${components.length}`);
  console.log(`Wasm bytes:         ${formatBytes(wasmBefore)} -> ${formatBytes(wasmAfter)} (${pct(wasmAfter - wasmBefore, wasmBefore)}%)`);
  console.log(`Graph edges:        ${inGroupEdges}/${resolvedEdges} resolved intra-group; unresolved=${unresolvedEdges} interior=${interiorEdges} ambiguousInterior=${ambiguousInteriorEdges}`);
  console.log(`Profile splits:     ${profileGuidedSplits.targets.length}` +
              (profileGuidedSplits.source ? ` selected from ${profileGuidedSplits.parsed || 0} profile pairs, belowThreshold=${profileGuidedSplits.belowThreshold || 0}, rejected=${profileGuidedSplits.rejected || 0}` : ' (no profile)'));
  if (profileGuidedSplits.rejected) {
    const rejectedByReason = profileGuidedSplits.rejectedByReason || {};
    const reasonText = Object.entries(rejectedByReason)
      .filter(([, count]) => count)
      .map(([reason, count]) => `${reason}=${count}`)
      .join(' ');
    if (reasonText) console.log(`Profile split rejects: ${reasonText}`);
    for (const sample of (profileGuidedSplits.rejectedSamples || [])) {
      const candidates = (sample.candidates || [])
        .map((entry) => `${hex32(entry.start)}-${hex32(entry.end)} ${entry.key}`)
        .join('; ');
      console.log(`  rejected ${hex32(sample.blockStart)} -> ${hex32(sample.target)} count=${sample.exactCount} reason=${sample.reason}${candidates ? ` candidates[${candidates}]` : ''}`);
    }
  }
  for (const split of profileGuidedSplits.targets.slice(0, 8)) {
    console.log(`  ${hex32(split.blockStart)} -> ${hex32(split.target)} count=${split.exactCount} key=${split.containingKey}`);
  }
  console.log(`Function imports:   ${groupedManifest.stats.imports.functionImportsBefore} -> ${groupedManifest.stats.imports.functionImportsAfterGroupedEstimate} estimated`);
  console.log(`Memory imports:     ${groupedManifest.stats.imports.memoryImportsBefore} -> ${groupedManifest.stats.imports.memoryImportsAfterGroupedEstimate} estimated`);
  console.log(`Zip bytes:          ${formatBytes(inputCache.zipBuf.length)} -> ${formatBytes(out.length)}`);
  for (const warning of gWarnings) {
    console.log(`Warning:            ${warning}`);
  }
}

main().catch((err) => {
  console.error(`error: ${err.message}`);
  process.exit(1);
});
