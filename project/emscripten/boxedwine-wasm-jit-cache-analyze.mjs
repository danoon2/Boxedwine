#!/usr/bin/env node

import { inflateRawSync } from 'node:zlib';
import { readFile } from 'node:fs/promises';

const WASM_MAGIC = Buffer.from([0x00, 0x61, 0x73, 0x6d]);
const CACHE_NAME_RE = /^(?:v[45]-)?([0-9a-f]{8})-([0-9a-f]{8})\.wasm$/i;
const BRANCH_HINT_SECTION = 'metadata.code.branch_hint';
const PROFILE_NAME = 'boxedwine-jit-profile.txt';
const PROFILE_SWEEP_THRESHOLDS = [200000, 100000, 75000, 50000, 25000, 10000];

function usage() {
  console.log(`Usage:
  node scripts/boxedwine-wasm-jit-cache-analyze.mjs cache.zip

Analyzes the BoxedWine wasm JIT cache module shape. This is intended to guide
offline optimization, merged-module, and direct-call experiments.`);
}

function parseArgs(argv) {
  if (argv.includes('-h') || argv.includes('--help')) {
    usage();
    process.exit(0);
  }
  if (argv.length !== 1) {
    throw new Error('Expected cache.zip');
  }
  return { inputZip: argv[0] };
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
    if (readU32(buf, off) !== 0x02014b50) {
      throw new Error(`Bad central-directory header at ${off}`);
    }
    const flags = readU16(buf, off + 8);
    const method = readU16(buf, off + 10);
    const compressedSize = readU32(buf, off + 20);
    const uncompressedSize = readU32(buf, off + 24);
    const nameLen = readU16(buf, off + 28);
    const extraLen = readU16(buf, off + 30);
    const commentLen = readU16(buf, off + 32);
    const localOffset = readU32(buf, off + 42);
    const name = decodeName(buf.subarray(off + 46, off + 46 + nameLen), flags);

    if (readU32(buf, localOffset) !== 0x04034b50) {
      throw new Error(`Bad local header for ${name}`);
    }
    const localMethod = readU16(buf, localOffset + 8);
    const localNameLen = readU16(buf, localOffset + 26);
    const localExtraLen = readU16(buf, localOffset + 28);
    const dataStart = localOffset + 30 + localNameLen + localExtraLen;
    const compressed = buf.subarray(dataStart, dataStart + compressedSize);
    let data;
    if (localMethod === 0) {
      data = compressed;
    } else if (localMethod === 8) {
      data = inflateRawSync(compressed);
    } else {
      throw new Error(`Unsupported ZIP method ${localMethod} for ${name}`);
    }
    if (data.length !== uncompressedSize) {
      throw new Error(`Bad uncompressed size for ${name}: got ${data.length}, expected ${uncompressedSize}`);
    }
    entries.push({ name, method, localMethod, compressedSize, uncompressedSize, data });

    off += 46 + nameLen + extraLen + commentLen;
  }
  return entries;
}

function parseWasm(wasm) {
  if (wasm.length < 8 || !wasm.subarray(0, 4).equals(WASM_MAGIC)) {
    throw new Error('Entry is not a wasm module');
  }

  const info = {
    types: 0,
    imports: [],
    importedFunctions: 0,
    importedMemories: 0,
    localFunctions: 0,
    exports: [],
    codeBodies: 0,
    codeBytes: 0,
    codeBodyBytes: [],
    branchHintSections: 0,
    branchHints: 0,
    customSections: [],
    sections: [],
  };

  let pos = 8;
  while (pos < wasm.length) {
    const id = wasm[pos++];
    const [size, p] = readWasmUleb(wasm, pos);
    pos = p;
    const sectionEnd = pos + size;
    if (sectionEnd > wasm.length) throw new Error('Wasm section extends outside module');
    info.sections.push(id);

    if (id === 0) {
      const [name, bodyStart] = wasmName(wasm, pos);
      info.customSections.push(name);
      if (name === BRANCH_HINT_SECTION) {
        info.branchHintSections++;
        let q = bodyStart;
        const [functionCount, q2] = readWasmUleb(wasm, q);
        q = q2;
        for (let i = 0; i < functionCount; i++) {
          [, q] = readWasmUleb(wasm, q);
          const [hintCount, q3] = readWasmUleb(wasm, q);
          q = q3;
          info.branchHints += hintCount;
          for (let j = 0; j < hintCount; j++) {
            [, q] = readWasmUleb(wasm, q);
            const [payloadSize, q4] = readWasmUleb(wasm, q);
            q = q4 + payloadSize;
          }
        }
      }
    } else if (id === 1) {
      const [count] = readWasmUleb(wasm, pos);
      info.types = count;
    } else if (id === 2) {
      let q = pos;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let moduleName; [moduleName, q] = wasmName(wasm, q);
        let fieldName; [fieldName, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        const imp = { module: moduleName, field: fieldName, kind };
        if (kind === 0) {
          [imp.typeIndex, q] = readWasmUleb(wasm, q);
          info.importedFunctions++;
        } else if (kind === 1) {
          [, q] = readWasmUleb(wasm, q);
        } else if (kind === 2) {
          const flags = wasm[q++];
          [, q] = readWasmUleb(wasm, q);
          if (flags & 1) [, q] = readWasmUleb(wasm, q);
          info.importedMemories++;
        } else if (kind === 3) {
          q++;
          q++;
        } else {
          throw new Error(`Unsupported import kind ${kind}`);
        }
        info.imports.push(imp);
      }
    } else if (id === 3) {
      const [count] = readWasmUleb(wasm, pos);
      info.localFunctions = count;
    } else if (id === 7) {
      let q = pos;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      for (let i = 0; i < count; i++) {
        let name; [name, q] = wasmName(wasm, q);
        const kind = wasm[q++];
        let index; [index, q] = readWasmUleb(wasm, q);
        info.exports.push({ name, kind, index });
      }
    } else if (id === 10) {
      let q = pos;
      const [count, q2] = readWasmUleb(wasm, q);
      q = q2;
      info.codeBodies = count;
      for (let i = 0; i < count; i++) {
        const [bodySize, bodyStart] = readWasmUleb(wasm, q);
        q = bodyStart + bodySize;
        info.codeBytes += bodySize;
        info.codeBodyBytes.push(bodySize);
      }
    }

    pos = sectionEnd;
  }
  return info;
}

function median(values) {
  if (!values.length) return 0;
  const sorted = [...values].sort((a, b) => a - b);
  return sorted[Math.floor(sorted.length / 2)];
}

function pct(n, d) {
  return d ? (100 * n / d).toFixed(2) : '0.00';
}

function formatBytes(n) {
  return `${n.toLocaleString()} B`;
}

function signature(info) {
  return info.imports.map((i) => `${i.kind}:${i.module}.${i.field}:${i.typeIndex ?? ''}`).join('|') +
    ` exports=${info.exports.map((e) => `${e.kind}:${e.name}`).join('|')}`;
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

function analyzeProfileSplits(profileText, manifest, thresholds = PROFILE_SWEEP_THRESHOLDS) {
  const profiled = parseInteriorProfileText(profileText);
  const manifestEntries = Array.isArray(manifest?.entries) ? manifest.entries : [];
  const entriesByStart = new Map();
  for (const entry of manifestEntries) {
    const start = entry.eip >>> 0;
    if (!entriesByStart.has(start)) entriesByStart.set(start, []);
    entriesByStart.get(start).push(entry);
  }

  function classify(threshold) {
    let belowThreshold = 0;
    let rejected = 0;
    const selectedByBlock = new Map();
    for (const item of profiled) {
      if (item.exactCount < threshold) {
        belowThreshold++;
        continue;
      }
      const candidates = entriesByStart.get(item.blockStart) || [];
      const owner = candidates.find((entry) => {
        const start = entry.eip >>> 0;
        const end = (start + ((entry.emulatedLen || 0) >>> 0)) >>> 0;
        return item.target > start && item.target < end;
      });
      if (!owner) {
        rejected++;
        continue;
      }
      const existing = selectedByBlock.get(item.blockStart);
      if (!existing || item.exactCount > existing.exactCount || item.approxCount > existing.approxCount) {
        selectedByBlock.set(item.blockStart, { ...item, containingKey: owner.key });
      }
    }
    const selected = Array.from(selectedByBlock.values())
      .sort((a, b) => b.exactCount - a.exactCount || b.approxCount - a.approxCount);
    return { threshold, selected, belowThreshold, rejected };
  }

  return {
    parsed: profiled.length,
    thresholds: thresholds.map(classify),
  };
}

function analyzeManifestGraph(manifest, wasmBytes) {
  const entries = Array.isArray(manifest?.entries) ? manifest.entries : [];
  const byEip = new Map();
  for (const entry of entries) {
    byEip.set(entry.eip >>> 0, entry);
  }
  const findInteriorTargets = (targetEip) => entries.filter((entry) => {
    const start = entry.eip >>> 0;
    const len = entry.emulatedLen >>> 0;
    if (!len || targetEip === start) return false;
    return targetEip > start && targetEip < ((start + len) >>> 0);
  });

  const edgeKinds = ['next1', 'next2', 'jump'];
  const edgeStats = {
    total: 0,
    resolved: 0,
    unresolved: 0,
    interior: 0,
    ambiguousInterior: 0,
    self: 0,
    byKind: Object.fromEntries(edgeKinds.map((kind) => [kind, { total: 0, resolved: 0 }])),
  };
  const adjacency = new Map();
  const reverseCount = new Map();

  for (const entry of entries) {
    const from = entry.eip >>> 0;
    const neighbors = new Set();
    for (const kind of edgeKinds) {
      const exit = entry.exits?.[kind];
      if (!exit?.count || !exit.firstTarget) continue;
      const target = exit.firstTarget >>> 0;
      edgeStats.total++;
      edgeStats.byKind[kind].total++;
      if (target === from) edgeStats.self++;
      const targetEntry = byEip.get(target);
      if (targetEntry) {
        edgeStats.resolved++;
        edgeStats.byKind[kind].resolved++;
        neighbors.add(target);
        reverseCount.set(target, (reverseCount.get(target) ?? 0) + 1);
      } else {
        const interiorTargets = findInteriorTargets(target);
        if (interiorTargets.length === 1) edgeStats.interior++;
        else if (interiorTargets.length > 1) edgeStats.ambiguousInterior++;
        edgeStats.unresolved++;
      }
    }
    adjacency.set(from, neighbors);
  }

  const seen = new Set();
  const components = [];
  for (const entry of entries) {
    const start = entry.eip >>> 0;
    if (seen.has(start)) continue;
    const stack = [start];
    const nodes = [];
    let bytes = 0;
    seen.add(start);
    while (stack.length) {
      const eip = stack.pop();
      nodes.push(eip);
      bytes += byEip.get(eip)?.wasmBytes ?? 0;
      for (const next of adjacency.get(eip) ?? []) {
        if (!seen.has(next)) {
          seen.add(next);
          stack.push(next);
        }
      }
      for (const [other, neighbors] of adjacency) {
        if (neighbors.has(eip) && !seen.has(other)) {
          seen.add(other);
          stack.push(other);
        }
      }
    }
    components.push({ nodes: nodes.length, bytes, firstEip: nodes[0] });
  }
  components.sort((a, b) => b.bytes - a.bytes);

  const budgets = [256 * 1024, 512 * 1024, 1024 * 1024];
  const groupsByBudget = budgets.map((budget) => {
    let componentOnlyGroups = 0;
    const items = [];
    for (const component of components) {
      componentOnlyGroups += Math.max(1, Math.ceil(component.bytes / budget));
      if (component.bytes <= budget) {
        items.push(component.bytes);
      } else {
        let remaining = component.bytes;
        while (remaining > 0) {
          items.push(Math.min(remaining, budget));
          remaining -= budget;
        }
      }
    }
    items.sort((a, b) => b - a);
    const bins = [];
    for (const bytes of items) {
      let best = -1;
      let bestRemainder = Infinity;
      for (let i = 0; i < bins.length; i++) {
        const remainder = budget - bins[i] - bytes;
        if (remainder >= 0 && remainder < bestRemainder) {
          best = i;
          bestRemainder = remainder;
        }
      }
      if (best === -1) {
        bins.push(bytes);
      } else {
        bins[best] += bytes;
      }
    }
    return { budget, packedGroups: bins.length, componentOnlyGroups };
  });

  const topFanIn = [...reverseCount.entries()]
    .sort((a, b) => b[1] - a[1])
    .slice(0, 8)
    .map(([eip, count]) => `${hex32(eip)}=${count}`);

  return {
    entries,
    edgeStats,
    components,
    groupsByBudget,
    topFanIn,
    largestComponentPct: components.length ? pct(components[0].bytes, wasmBytes) : '0.00',
  };
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const zip = await readFile(opts.inputZip);
  const zipEntries = parseZip(zip);
  const manifestEntry = zipEntries.find((entry) => entry.name === 'boxedwine-jit-manifest.json');
  const groupedManifestEntry = zipEntries.find((entry) => entry.name === 'boxedwine-jit-grouped-manifest.json');
  const profileEntry = zipEntries.find((entry) => entry.name === PROFILE_NAME);
  let manifest = null;
  let groupedManifest = null;
  if (manifestEntry) {
    manifest = JSON.parse(manifestEntry.data.toString('utf8'));
  }
  if (groupedManifestEntry) {
    groupedManifest = JSON.parse(groupedManifestEntry.data.toString('utf8'));
  }
  const wasmEntries = [];

  for (const entry of zipEntries) {
    if (!entry.data.subarray(0, 4).equals(WASM_MAGIC)) continue;
    const match = CACHE_NAME_RE.exec(entry.name);
    const wasm = parseWasm(entry.data);
    wasmEntries.push({
      ...entry,
      eip: match ? parseInt(match[1], 16) >>> 0 : null,
      blockHash: match ? parseInt(match[2], 16) >>> 0 : null,
      wasm,
    });
  }

  const wasmBytes = wasmEntries.reduce((sum, e) => sum + e.uncompressedSize, 0);
  const compressedBytes = wasmEntries.reduce((sum, e) => sum + e.compressedSize, 0);
  const codeSizes = wasmEntries.flatMap((e) => e.wasm.codeBodyBytes);
  const signatures = new Map();
  const helperFields = new Set();
  let importedFunctions = 0;
  let importedMemories = 0;
  let branchHints = 0;
  let branchHintModules = 0;

  for (const entry of wasmEntries) {
    const sig = signature(entry.wasm);
    signatures.set(sig, (signatures.get(sig) ?? 0) + 1);
    importedFunctions += entry.wasm.importedFunctions;
    importedMemories += entry.wasm.importedMemories;
    branchHints += entry.wasm.branchHints;
    if (entry.wasm.branchHints) branchHintModules++;
    for (const imp of entry.wasm.imports) {
      if (imp.kind === 0 && imp.module === 'helpers') {
        helperFields.add(imp.field);
      }
    }
  }

  console.log(`Input:              ${opts.inputZip}`);
  console.log(`Zip entries:        ${zipEntries.length}`);
  console.log(`Wasm modules:       ${wasmEntries.length}`);
  console.log(`Wasm bytes:         ${formatBytes(wasmBytes)} uncompressed, ${formatBytes(compressedBytes)} compressed`);
  console.log(`Code body bytes:    ${formatBytes(codeSizes.reduce((a, b) => a + b, 0))}`);
  console.log(`Code body size:     min=${Math.min(...codeSizes)} median=${median(codeSizes)} max=${Math.max(...codeSizes)}`);
  console.log(`Import signatures:  ${signatures.size}`);
  console.log(`Imported funcs:     ${importedFunctions} total (${helperFields.size} unique helper fields)`);
  console.log(`Imported memories:  ${importedMemories} total`);
  console.log(`Branch hints:       ${branchHints} hints across ${branchHintModules} modules`);
  if (manifest) {
    const entries = Array.isArray(manifest.entries) ? manifest.entries : [];
    const withNext1 = entries.filter((e) => e.exits?.next1?.count).length;
    const withNext2 = entries.filter((e) => e.exits?.next2?.count).length;
    const withJump = entries.filter((e) => e.exits?.jump?.count).length;
    const manifestBytes = entries.reduce((sum, e) => sum + (e.wasmBytes || 0), 0);
    console.log(`Manifest:           v${manifest.version ?? '?'} with ${entries.length} entries`);
    console.log(`Manifest exits:     next1=${withNext1} next2=${withNext2} jump=${withJump}`);
    console.log(`Manifest wasm bytes:${formatBytes(manifestBytes)} (${pct(manifestBytes, wasmBytes)}% of wasm bytes)`);
    const graph = analyzeManifestGraph(manifest, wasmBytes);
    const edgeStats = graph.edgeStats;
    console.log(`Graph edges:        ${edgeStats.resolved}/${edgeStats.total} resolved (${pct(edgeStats.resolved, edgeStats.total)}%), unresolved=${edgeStats.unresolved}, self=${edgeStats.self}`);
    console.log(`Graph interiors:    ${edgeStats.interior} unresolved edges land inside cached block ranges, ambiguous=${edgeStats.ambiguousInterior}`);
    console.log(`Graph edge kinds:   next1=${edgeStats.byKind.next1.resolved}/${edgeStats.byKind.next1.total} next2=${edgeStats.byKind.next2.resolved}/${edgeStats.byKind.next2.total} jump=${edgeStats.byKind.jump.resolved}/${edgeStats.byKind.jump.total}`);
    console.log(`Graph components:   ${graph.components.length}, largest=${graph.components[0]?.nodes ?? 0} modules/${formatBytes(graph.components[0]?.bytes ?? 0)} (${graph.largestComponentPct}% of wasm)`);
    console.log(`Packed groups:      ${graph.groupsByBudget.map((g) => `${formatBytes(g.budget)}=>${g.packedGroups}`).join(' ')}`);
    console.log(`Component groups:   ${graph.groupsByBudget.map((g) => `${formatBytes(g.budget)}=>${g.componentOnlyGroups}`).join(' ')}`);
    console.log(`Top fan-in targets: ${graph.topFanIn.join(' ') || 'none'}`);
  } else {
    console.log(`Manifest:           none`);
  }
  if (profileEntry) {
    const profileText = profileEntry.data.toString('utf8');
    const profile = analyzeProfileSplits(profileText, manifest);
    console.log(`Profile sidecar:    ${PROFILE_NAME}, ${profile.parsed} interior target pairs`);
    if (manifest) {
      console.log(`Profile threshold sweep:`);
      for (const item of profile.thresholds) {
        const top = item.selected.slice(0, 3)
          .map((split) => `${hex32(split.blockStart)}->${hex32(split.target)}:${split.exactCount}`)
          .join(' ');
        console.log(`  >=${item.threshold}: selected=${item.selected.length} below=${item.belowThreshold} rejected=${item.rejected}${top ? ` top[${top}]` : ''}`);
      }
    } else {
      console.log(`Profile threshold sweep: skipped, no flat manifest to validate containing blocks`);
    }
  }
  if (groupedManifest) {
    const groups = Array.isArray(groupedManifest.groups) ? groupedManifest.groups : [];
    const directCallCandidateEdges = groups.reduce((sum, group) => sum + (group.directCallCandidateEdges || 0), 0);
    const directCallsRewritten = groups.reduce((sum, group) => sum + (group.directCallsRewritten || 0), 0);
    const stats = groupedManifest.stats || {};
    const imports = stats.imports || {};
    console.log(`Grouped manifest:   v${groupedManifest.version ?? '?'} with ${groups.length} groups, budget=${formatBytes(groupedManifest.source?.budgetBytes || 0)}`);
    if (stats.graphEdges) {
      console.log(`Grouped edges:      ${stats.graphEdges.inGroup}/${stats.graphEdges.resolved} resolved intra-group, candidates=${directCallCandidateEdges}`);
      if (stats.graphEdges.interior != null) {
        console.log(`Grouped interiors:  ${stats.graphEdges.interior} unresolved edges land inside cached block ranges, ambiguous=${stats.graphEdges.ambiguousInterior || 0}`);
      }
    }
    if (groupedManifest.source?.directCalls || directCallsRewritten > 0 || stats.directCalls) {
      const directCalls = stats.directCalls || {};
      const details = [];
      if (Array.isArray(groupedManifest.source?.directCallKinds)) {
        details.push(`kinds=${groupedManifest.source.directCallKinds.join(',') || 'none'}`);
      }
      if (directCalls.tailCalls || groupedManifest.source?.tailCalls) details.push('tailCalls=yes');
      if (directCalls.yieldAwareTailCalls || groupedManifest.source?.yieldAwareTailCalls) details.push('yieldAware=yes');
      if (
        directCalls.guardedRewritten != null &&
        (directCalls.yieldAwareTailCalls || groupedManifest.source?.yieldAwareTailCalls || directCalls.depthLimit)
      ) {
        details.push(`guarded=${directCalls.guardedRewritten}`);
      }
      if (directCalls.depthLimit) details.push(`depth=${directCalls.depthLimit}`);
      if (directCalls.cyclicBlocksExcluded != null) details.push(`cyclicBlocks=${directCalls.cyclicBlocksExcluded}`);
      const suffix = details.length ? `, ${details.join(', ')}` : '';
      console.log(`Direct calls:       ${directCalls.rewritten ?? directCallsRewritten} tail sites rewritten, candidateEdges=${directCalls.candidateEdges ?? directCallCandidateEdges}${suffix}`);
    }
    if (imports.functionImportsBefore != null && imports.functionImportsAfterGroupedEstimate != null) {
      console.log(`Grouped imports:    funcs ${imports.functionImportsBefore}->${imports.functionImportsAfterGroupedEstimate}, memories ${imports.memoryImportsBefore}->${imports.memoryImportsAfterGroupedEstimate}`);
    }
    if (groupedManifest.profileGuidedSplits) {
      const profile = groupedManifest.profileGuidedSplits;
      const targets = Array.isArray(profile.targets) ? profile.targets : [];
      console.log(`Profile splits:     ${targets.length} selected, threshold=${profile.threshold || 0}, rejected=${profile.rejected || 0}`);
      for (const split of targets.slice(0, 5)) {
        console.log(`  ${hex32(split.blockStart)} -> ${hex32(split.target)} count=${split.exactCount || split.approxCount || 0}`);
      }
    }
  }

  const topSignatures = [...signatures.entries()].sort((a, b) => b[1] - a[1]).slice(0, 5);
  console.log('');
  console.log('Top import/export signatures:');
  for (const [sig, count] of topSignatures) {
    console.log(`  ${count} modules: ${sig.slice(0, 180)}${sig.length > 180 ? '...' : ''}`);
  }

  const sameAbi = signatures.size === 1;
  const currentHelperImports = importedFunctions;
  const mergedHelperImports = sameAbi ? helperFields.size : null;
  const hasDirectCalls = Boolean(groupedManifest?.source?.directCalls || groupedManifest?.stats?.directCalls?.rewritten);
  console.log('');
  console.log('Merged-module/direct-call notes:');
  if (sameAbi) {
    console.log(`  All wasm cache modules share one ABI shape.`);
    console.log(`  A single merged module could reduce helper function imports from ${currentHelperImports} to about ${mergedHelperImports}.`);
  } else {
    console.log(`  Modules do not all share one ABI shape; grouping by signature would be required first.`);
  }
  if (manifest || groupedManifest) {
    console.log(`  The sidecar manifest provides a successor graph for grouping experiments.`);
    if (hasDirectCalls) {
      console.log(`  This cache includes the first conservative intra-group direct-call rewrite.`);
      console.log(`  Remaining direct-call work: invalidation/version policy, broader exit patterns, and runtime benchmarking.`);
    } else {
      console.log(`  Direct calls still need an invalidation/version policy and a grouped-module loader.`);
    }
  } else {
    console.log(`  Current cache entries contain EIP/hash identity but no successor graph.`);
    console.log(`  Direct calls need extra metadata: branch/fallthrough target EIPs, invalidation/version policy, and a grouped-module loader.`);
    console.log(`  Near-term practical slice: emit a sidecar manifest during export with each block's EIP/hash/size and known next1/next2/jump targets.`);
  }
}

main().catch((err) => {
  console.error(`error: ${err.message}`);
  process.exit(1);
});
