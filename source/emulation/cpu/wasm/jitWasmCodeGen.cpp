/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"

#ifdef BOXEDWINE_WASM_JIT

#include "jitWasmCodeGen.h"
#include "../jit/jitCodeGen.h"
#ifdef __TEST
#include "../jit/jitFPU.h"
#endif
#include "../common/cpu.h"
#include "../common/common_fpu.h"
#include "../common/common_sse2.h"
#include "../normal/normal_strings.h"
#include "../normal/normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../../softmmu/kmemory_soft.h"

#include <atomic>
#include <bit>      // std::bit_cast (C++20) — used in boxedwine_wasm_call_block
#include <emscripten.h>
#ifdef __TEST
#include <type_traits>
#endif
#include <emscripten/em_js.h>
#include <map>      // g_wasmMtGroupProcState (node-based: stable references)
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

static constexpr U32 WASM_FMASK_TEST = CF | PF | AF | ZF | SF | OF;
static_assert(WASM_LOCAL_COUNT <= 0xff,
              "JitReg/MMXRegInternal store WASM local ids in U8 fields");

static std::atomic<bool> g_wasmJitSimdEnabledLogged{ false };

static void logWasmJitSimdEnabledOnce() {
    bool expected = false;
    if (g_wasmJitSimdEnabledLogged.compare_exchange_strong(expected, true)) {
        klog("WASM JIT simd enabled");
    }
}

#ifdef __TEST
static_assert(std::is_base_of<JitSSE, JitWasmCodeGen>::value,
              "WASM JIT should reuse the shared JitSSE base");
#endif

#if !defined(BOXEDWINE_MULTI_THREADED) && !defined(__TEST)
// minizip/unzip: used for JIT module import (ST builds only).
// Export (saving) is handled entirely in JavaScript; only import (reading the
// zip that JS writes to the virtual FS) needs C code.
// Must follow fszip.h's pattern: extern "C" + undef OF before/after.
#undef OF
#define STRICTUNZIP
extern "C" {
#include "../../../../lib/zlib/contrib/minizip/unzip.h"
}
#undef OF
#endif

#ifndef BOXEDWINE_MULTI_THREADED
// Defined in kscheduler.cpp (single-threaded scheduler only — MT threads run
// on real pthreads and check cpu->yield instead of a slice budget). The
// address of this scheduler budget is exported in the saved-cache manifest
// (runtime constants) so the offline pipeline can rewrite intra-group block
// exits into yield-aware direct tail calls that test the same budget the
// dispatcher loop uses. MT manifests carry 0 here, and the pipeline's grouped
// mode rejects such input (MT consumes --flat output, which never bakes it).
extern S32 contextTimeRemaining;
#endif

#ifdef BOXEDWINE_MULTI_THREADED
// Monotonically increasing counter for atomic table slot allocation.
// Stored in WASM linear memory (SharedArrayBuffer in pthreads builds) so
// Atomics.add in boxedwine_wasm_instantiate_mt can claim unique slots
// across all worker threads without a mutex.
// Value 0 is the "not yet initialized" sentinel; boxedwine_wasm_instantiate_mt
// uses Atomics.compareExchange to set it to wasmTable.length on the first call.
static int32_t g_wasmTableNextSlot = 0;
// Saved WASM bytes for slots published in shared DecodedOps. Each browser
// worker has local wasmTable visibility, so another worker may need to lazily
// instantiate the same bytes into the same slot before call_indirect can run.
// Entries intentionally follow the monotonic multi-threaded slot lifetime: MT
// slots are not reused or cleared because another worker may have read the slot
// from pfnJitCode immediately before a code invalidation race.
static std::mutex g_wasmBlockBinariesMutex;
static std::unordered_map<int, std::vector<U8>> g_wasmBlockBinaries;
#endif

// Per-block relocation slot arrays for persistence-mode modules, keyed by
// wasmTable index. Each array's address is passed to the block function as
// its second parameter on every call (see WASM_RELOC_LOCAL); the values are
// host DecodedOp pointers resolved against the session that compiled the
// block (see addRelocSlot()). Every pointer stored in a block's slots
// shares that block's lifetime (slot 0 = own block start, next2 = own
// fall-through chain, jump targets = in-block ops, next1 = the dispatcher-
// managed nextJump cache slot), so the array can be freed exactly when the
// block is evicted.
// ST: a flat vector indexed by table index — O(1) and lock-free on the hot
// chain-loop path; freed in clearJitBlock. MT: a map protected by
// g_wasmBlockBinariesMutex (the MT call path already pays a JS slot_check
// per call, so the lock is noise); entries intentionally follow the
// monotonic never-freed slot lifetime of g_wasmBlockBinaries.
#ifdef BOXEDWINE_MULTI_THREADED
static std::unordered_map<int, U32*> g_wasmRelocArrays;
#else
static std::vector<U32*> g_wasmRelocBaseByTable;
#endif
static inline U32 wasmJitRelocBaseForTable(int tableIndex);

#ifdef BOXEDWINE_MULTI_THREADED
// Persistent cache key -> WASM bytes. Populated on the main thread before
// main() runs (Module.onRuntimeInitialized in boxedwine-shell.js calls
// wasm_jit_mt_register for every entry loaded from the server zip) and
// extended by worker threads as they compile fresh blocks. The key combines
// the CS-relative EIP with a decoded-block hash so reused code addresses do
// not consume stale wasm. Protected by g_wasmBlockBinariesMutex (shared with
// g_wasmBlockBinaries).
static std::unordered_map<uint64_t, std::vector<U8>> g_wasmCacheByKey;

// Per-key exit metadata for freshly compiled blocks (the MT equivalent of the
// ST Module.wasmJitBlockMeta map, which workers cannot reach). Mirrored to JS
// by wasm_jit_mt_prepare_export so MT-recorded zips carry the
// boxedwine-jit-manifest.json the offline pipeline requires. Protected by
// g_wasmBlockBinariesMutex.
struct WasmJitMtBlockMeta {
    U32 opCount;
    U32 emulatedLen;
    U32 next1Target;
    U32 next2Target;
    U32 jumpTarget;
    U32 next1Count;
    U32 next2Count;
    U32 jumpCount;
    U32 relocCount;
};
static std::unordered_map<uint64_t, WasmJitMtBlockMeta> g_wasmCacheMetaByKey;

// Interior-transition profile snapshot lines, appended by whichever worker
// crosses the 5M-exit threshold and mirrored to Module at export time (the
// MT replacement for the ST boxedwine_wasm_record_transition_profile EM_JS).
// Protected by g_wasmBlockBinariesMutex.
static std::vector<std::string> g_wasmInteriorProfileLines;

// ---------------------------------------------------------------------------
// Piped (grouped) module registry — the MT counterpart of the ST shell's
// Module.wasmJitGroupUnpatched staging. Worker EM_JS cannot reach main-thread
// JS Maps, so the unpatched group bytes, entry table and direct-call patch
// table live in shared C++ memory, registered from the main thread before
// main() via the wasm_jit_mt_register_group* exports.
//
// Per-process state mirrors the ST loader's design: every entry gets a
// zero-filled U32[relocCount] array on the C++ heap (shared memory, visible
// to every worker), the direct-call sites get the *target* entry's array
// address patched into their 5-byte padded-SLEB i32.const placeholder, and
// the patched bytes are compiled per worker on first touch. Zeroed arrays
// are safe: un-promoted entries stay on the generated code's guarded slow
// paths, exactly as in ST. Unlike ST there is no refresh-copy: the promotion
// path registers the group array itself in g_wasmRelocArrays, so table calls
// and intra-group direct calls read the same slots.
// All protected by g_wasmBlockBinariesMutex.
struct WasmJitMtGroupEntry {
    uint64_t key;
    std::string exportName;
    U32 relocCount;
};
struct WasmJitMtGroupPatch {
    U32 offset;          // byte offset of the i32.const opcode (0x41)
    uint64_t targetKey;
};
struct WasmJitMtGroup {
    std::vector<U8> bytes; // unpatched merged module
    std::vector<WasmJitMtGroupEntry> entries;
    std::vector<WasmJitMtGroupPatch> patches;
};
struct WasmJitMtGroupProcState {
    std::vector<U8> patchedBytes;
    std::vector<U32*> entryArrays;   // index parallel to group entries; never freed (MT slot lifetime)
};
static std::vector<WasmJitMtGroup> g_wasmMtGroups;
static std::unordered_map<uint64_t, std::pair<U32, U32>> g_wasmMtGroupByKey; // key -> (groupIdx, entryIdx)
// (groupIdx, memId) -> per-process patched bytes + arrays
static std::map<std::pair<U32, U32>, WasmJitMtGroupProcState> g_wasmMtGroupProcState;
// table slot -> (groupIdx, memId) so cross-worker lazy installs route to the
// group instead of g_wasmBlockBinaries. exportName resolved via entryIdx.
struct WasmJitMtGroupSlotRef {
    U32 groupIdx;
    U32 entryIdx;
    U32 memId;
};
static std::unordered_map<int, WasmJitMtGroupSlotRef> g_wasmMtGroupSlotRefs;
#endif

// Runtime persistence mode. When active, generated modules must not embed
// host pointers (DecodedOp*, nextJump cache slots, ...) because those are only
// valid for the run that compiled them. The relocatable codegen paths below
// trade those pointer fast paths (~12% throughput in single-threaded
// benchmarks) for modules that can be exported, served and re-imported on a
// later run — so the mode is off by default and a plain session pays nothing.
//
// boxedwine-shell.js activates it (via the exported
// wasm_jit_set_persistence_active, or implicitly through wasm_jit_mt_register)
// when a server cache zip was imported or the jit-record URL parameter is set.
// Activation happens in Module.onRuntimeInitialized — after the wasm-jit-cache
// run dependency resolved, before main() — so the flag is latched before the
// first block is compiled. It is deliberately one-way: only blocks compiled
// while active are saved/exported, so a session that mixed modes would export
// an incomplete cache and pay the relocatable cost without the benefit.
// BOXEDWINE_WASM_JIT_FORCE_PERSISTENCE is a test/diagnostic define: it latches
// the mode from startup so the CPU test suite (which has no shell to activate
// it) exercises the relocatable codegen paths, e.g.
//   make -B testJit GCC_EXTRA_FLAGS=-DBOXEDWINE_WASM_JIT_FORCE_PERSISTENCE
#ifdef BOXEDWINE_WASM_JIT_FORCE_PERSISTENCE
static std::atomic<bool> g_wasmJitPersistenceActive{true};
#else
static std::atomic<bool> g_wasmJitPersistenceActive{false};
#endif

static inline bool wasmJitPersistenceActive() {
    return g_wasmJitPersistenceActive.load(std::memory_order_relaxed);
}

// Record sessions (?jit-record=true) additionally collect the fetchNext
// transition profile that the Save-JIT-Cache export embeds for the offline
// pipeline's split hints. Replay sessions never export, so they must not pay
// for the recording (per-landing atomic increments contend across MT
// workers). FORCE_PERSISTENCE keeps it on so the test suite covers the path.
#ifdef BOXEDWINE_WASM_JIT_FORCE_PERSISTENCE
static std::atomic<bool> g_wasmJitRecordActive{true};
#else
static std::atomic<bool> g_wasmJitRecordActive{false};
#endif

static inline bool wasmJitRecordActive() {
    return g_wasmJitRecordActive.load(std::memory_order_relaxed);
}

// Exported to JS (see EXPORTED_FUNCTIONS in project/emscripten/makefile).
extern "C" void wasm_jit_set_persistence_active() {
    g_wasmJitPersistenceActive.store(true, std::memory_order_relaxed);
}

extern "C" void wasm_jit_set_record_active() {
    g_wasmJitRecordActive.store(true, std::memory_order_relaxed);
}

static inline uint64_t wasmJitCacheKey(U32 eip, U32 blockHash) {
    return ((uint64_t)eip << 32) | blockHash;
}

// FNV-1a over the fields of each decoded op in the block. The hash guards the
// persistent cache against the same EIP holding different code across runs
// (relocated modules, self-modifying code, different app versions).
static inline U32 wasmJitHashMix(U32 hash, U32 value) {
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static inline U32 wasmJitHashDecodedOp(U32 hash, DecodedOp* op) {
    hash = wasmJitHashMix(hash, (U32)op->inst);
    hash = wasmJitHashMix(hash, op->imm);
    // For direct branches, DecodedData stores data.nextJump, a host pointer
    // cache location. That pointer is not part of the x86 instruction shape
    // and is unstable across runs, so do not hash it as data.disp.
    if (!op->isDirectBranch()) {
        hash = wasmJitHashMix(hash, op->data.disp);
    }
    hash = wasmJitHashMix(hash, op->reg | (op->rm << 8) | (op->base << 16));
    hash = wasmJitHashMix(hash, op->sibIndex | (op->sibScale << 8) | (op->len << 16));
    hash = wasmJitHashMix(hash, op->lock | (op->repZero << 1) |
        (op->repNotZero << 2) | (op->ea16 << 3));
    return hash;
}

static inline U32 wasmJitFinalizeBlockHash(U32 decodedOpsHash, U32 blockOpCount, U32 emulatedLen) {
    U32 hash = wasmJitHashMix(decodedOpsHash, blockOpCount);
    hash = wasmJitHashMix(hash, emulatedLen);
    return hash ? hash : 1;
}

// ---------------------------------------------------------------------------
// Helper-call diagnostics (opt-in, BOXEDWINE_WASM_JIT_HELPER_CALL_STATS).
//
// Counts every generated-wasm -> C++ helper call by family and, for the
// bounded x87/SSE helpers, by specific helper. Used to decide which helper to
// inline or specialize next in the cache-pipeline improvement work (see
// docs/Wasm-JIT-Cache-Direct-Call-Improvements.md). Intentionally not part of
// normal benchmark builds: the counters sit inside hot helper paths and can
// perturb timing.
// ---------------------------------------------------------------------------
#if defined(BOXEDWINE_WASM_JIT_HELPER_CALL_STATS) && !defined(BOXEDWINE_MULTI_THREADED)
enum class WasmJitHelperStat : U8 {
    ReadMem,
    WriteMem,
    WriteMemCheck,
    FetchNext,
    Emulate,
    Flags,
    Cond,
    X87,
    Sse,
    Movsd32r,
    BlockEnter,
    Count,
};

enum class WasmJitHelperDetail : U8 {
    CacheFloat,
    MovsdXmmE64,
    MovsdE64Xmm,
    Movsd32r,
    Count,
};

static U64 g_wasmJitHelperStatCounts[(U32)WasmJitHelperStat::Count] = {};
static U64 g_wasmJitHelperDetailCounts[(U32)WasmJitHelperDetail::Count] = {};
static U64 g_wasmJitHelperStatTotal = 0;

static const char* wasmJitHelperStatName(WasmJitHelperStat stat) {
    switch (stat) {
    case WasmJitHelperStat::ReadMem:       return "mem_r";
    case WasmJitHelperStat::WriteMem:      return "mem_w";
    case WasmJitHelperStat::WriteMemCheck: return "mem_wc";
    case WasmJitHelperStat::FetchNext:     return "fetch_next";
    case WasmJitHelperStat::Emulate:       return "emulate";
    case WasmJitHelperStat::Flags:         return "flags";
    case WasmJitHelperStat::Cond:          return "cond";
    case WasmJitHelperStat::X87:           return "x87";
    case WasmJitHelperStat::Sse:           return "sse";
    case WasmJitHelperStat::Movsd32r:      return "movsd32r";
    case WasmJitHelperStat::BlockEnter:    return "enter";
    default:                               return "unknown";
    }
}

static const char* wasmJitHelperDetailName(WasmJitHelperDetail detail) {
    switch (detail) {
    case WasmJitHelperDetail::CacheFloat:      return "cache_float";
    case WasmJitHelperDetail::MovsdXmmE64:     return "movsd_xmm_e64";
    case WasmJitHelperDetail::MovsdE64Xmm:     return "movsd_e64_xmm";
    case WasmJitHelperDetail::Movsd32r:        return "movsd32r";
    default:                                   return "unknown";
    }
}

static void wasmJitRecordHelperCall(WasmJitHelperStat stat) {
    g_wasmJitHelperStatCounts[(U32)stat]++;
    g_wasmJitHelperStatTotal++;
    if (g_wasmJitHelperStatTotal % 5000000 != 0) {
        return;
    }
    char counts[512] = {};
    size_t offset = 0;
    for (U32 i = 0; i < (U32)WasmJitHelperStat::Count; i++) {
        int written = snprintf(counts + offset, sizeof(counts) - offset,
            "%s%s=%llu",
            i ? "," : "",
            wasmJitHelperStatName((WasmJitHelperStat)i),
            g_wasmJitHelperStatCounts[i]);
        if (written <= 0) {
            break;
        }
        offset += (size_t)written;
        if (offset >= sizeof(counts)) {
            counts[sizeof(counts) - 1] = 0;
            break;
        }
    }
    char details[512] = {};
    offset = 0;
    for (U32 i = 0; i < (U32)WasmJitHelperDetail::Count; i++) {
        int written = snprintf(details + offset, sizeof(details) - offset,
            "%s%s=%llu",
            i ? "," : "",
            wasmJitHelperDetailName((WasmJitHelperDetail)i),
            g_wasmJitHelperDetailCounts[i]);
        if (written <= 0) {
            break;
        }
        offset += (size_t)written;
        if (offset >= sizeof(details)) {
            details[sizeof(details) - 1] = 0;
            break;
        }
    }
    klog_fmt("[WASM JIT helper calls] total=%llu counts[%s] detail[%s]",
        g_wasmJitHelperStatTotal, counts, details);
}

static void wasmJitRecordHelperDetail(WasmJitHelperDetail detail) {
    g_wasmJitHelperDetailCounts[(U32)detail]++;
}

#define WASM_JIT_HELPER_STAT(STAT) wasmJitRecordHelperCall(WasmJitHelperStat::STAT)
#define WASM_JIT_HELPER_DETAIL(DETAIL) wasmJitRecordHelperDetail(WasmJitHelperDetail::DETAIL)
#else
#define WASM_JIT_HELPER_STAT(STAT) do {} while (0)
#define WASM_JIT_HELPER_DETAIL(DETAIL) do {} while (0)
#endif

#ifdef BOXEDWINE_MULTI_THREADED
static constexpr U32 WASM_JIT_CHAIN_BLOCK_LIMIT = 1;
#else
static constexpr U32 WASM_JIT_CHAIN_BLOCK_LIMIT = 2048;
#endif
static constexpr U32 WASM_DIRECT_LOOP_ITERATIONS = 64;

static inline bool wasmJitCanChainTo(CPU* cpu, DecodedOp* nextOp) {
    return nextOp &&
        (nextOp->flags & OP_FLAG_JIT) &&
        nextOp->pfnJitCode &&
        nextOp->pfn == cpu->thread->process->startJITOp;
}

// ---------------------------------------------------------------------------
// fetchNextOp transition profiling.
//
// Classifies every helper-dispatched block exit (the path direct tail calls in
// pipeline-optimized modules bypass) and, on single-threaded builds, keeps an
// approximate top list of interior-target EIPs. Each periodic snapshot is also
// pushed to Module.wasmJitInteriorProfileLines, which saveJitModules() embeds
// in the cache zip as boxedwine-jit-profile.txt — the offline pipeline turns
// it into profile-guided split hints.
//
// Runtime-gated on wasmJitPersistenceActive(): a plain session pays one
// predictable branch in fetchNextOp and nothing else, while record/replay
// sessions (which already run relocatable codegen) carry the counters.
// ---------------------------------------------------------------------------
static std::atomic<U64> g_wasmJitTransitionFetchNext{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextYield{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetJit{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetInterior{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetNull{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetNoJitFlag{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetNoTable{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetPfn{0};
static std::atomic<U64> g_wasmJitTransitionFetchNextTargetOther{0};

#ifndef BOXEDWINE_MULTI_THREADED
// Push one transition-profile snapshot line to the main-thread JS side so the
// Save-JIT-Cache export can embed the profile sidecar. ST-only and fully
// self-contained (no shell-page function dependencies). MT builds collect the
// same lines in g_wasmInteriorProfileLines instead (workers cannot reach the
// main-thread Module) and mirror them in wasm_jit_mt_prepare_export.
EM_JS(void, boxedwine_wasm_record_transition_profile, (U64 total, U64 yieldCount,
      U64 jit, U64 interior, U64 nullTarget, U64 noFlag, U64 noTable, U64 pfn, U64 other,
      const char* interiorTopPtr), {
    if (!Module.wasmJitInteriorProfileLines) Module.wasmJitInteriorProfileLines = [];
    var interiorTop = UTF8ToString(interiorTopPtr);
    var line = '[WASM JIT transitions] fetchNext=' + Number(total) +
        ' yield=' + Number(yieldCount) +
        ' target[jit=' + Number(jit) +
        ',interior=' + Number(interior) +
        ',null=' + Number(nullTarget) +
        ',noFlag=' + Number(noFlag) +
        ',noTable=' + Number(noTable) +
        ',pfn=' + Number(pfn) +
        ',other=' + Number(other) +
        '] interiorTop[' + interiorTop + ']';
    Module.wasmJitInteriorProfileLines.push(line);
    if (Module.wasmJitInteriorProfileLines.length > 256) {
        Module.wasmJitInteriorProfileLines.shift();
    }
});
#endif // !BOXEDWINE_MULTI_THREADED

// Approximate space-saving top list of interior transition targets. The
// approximate count ranks entries (it can over-count after an eviction); the
// exact count restarts at 1 whenever a slot is recycled.
// In MT builds the slots are updated by multiple workers without
// synchronization — entries can tear or under-count. That is acceptable: this
// is an approximate ranking that feeds offline split *hints*, every hint is
// re-validated against manifest block ranges by the pipeline, and putting
// atomics or a lock in the fetchNextOp path would perturb the very behavior
// being profiled.
static constexpr U32 WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT = 16;
static U32 g_wasmJitInteriorTargetSampleEip[WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT] = {};
static U32 g_wasmJitInteriorTargetSampleBlockStart[WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT] = {};
static U64 g_wasmJitInteriorTargetSampleCount[WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT] = {};
static U64 g_wasmJitInteriorTargetSampleExactCount[WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT] = {};

static inline U64 wasmJitRecordInteriorTargetSample(U32 targetEip, U32 blockStartEip) {
    for (U32 i = 0; i < WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT; i++) {
        if (g_wasmJitInteriorTargetSampleEip[i] == targetEip &&
            g_wasmJitInteriorTargetSampleBlockStart[i] == blockStartEip) {
            g_wasmJitInteriorTargetSampleCount[i]++;
            g_wasmJitInteriorTargetSampleExactCount[i]++;
            return g_wasmJitInteriorTargetSampleExactCount[i];
        }
    }
    for (U32 i = 0; i < WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT; i++) {
        if (!g_wasmJitInteriorTargetSampleEip[i]) {
            g_wasmJitInteriorTargetSampleEip[i] = targetEip;
            g_wasmJitInteriorTargetSampleBlockStart[i] = blockStartEip;
            g_wasmJitInteriorTargetSampleCount[i] = 1;
            g_wasmJitInteriorTargetSampleExactCount[i] = 1;
            return 1;
        }
    }
    U32 minIndex = 0;
    U64 minCount = g_wasmJitInteriorTargetSampleCount[0];
    for (U32 i = 1; i < WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT; i++) {
        if (g_wasmJitInteriorTargetSampleCount[i] < minCount) {
            minCount = g_wasmJitInteriorTargetSampleCount[i];
            minIndex = i;
        }
    }
    g_wasmJitInteriorTargetSampleEip[minIndex] = targetEip;
    g_wasmJitInteriorTargetSampleBlockStart[minIndex] = blockStartEip;
    g_wasmJitInteriorTargetSampleCount[minIndex] = minCount + 1;
    g_wasmJitInteriorTargetSampleExactCount[minIndex] = 1;
    return 1;
}

// Byte offset of nextOp from its containing block's start, or 0 if the chain
// can't be walked. Used to report the containing block-start EIP alongside
// the interior target so the offline pipeline can match manifest ranges.
static inline U32 wasmJitInteriorByteDistance(DecodedOp* nextOp) {
    if (!nextOp || !nextOp->blockStart || nextOp->blockStart == nextOp) {
        return 0;
    }
    U32 byteOffset = 0;
    U32 index = 0;
    DecodedOp* cur = nextOp->blockStart;
    U32 blockOpCount = cur ? cur->blockOpCount : 0;
    while (cur && cur != nextOp && index < blockOpCount) {
        byteOffset += cur->len;
        cur = cur->next;
        index++;
    }
    return cur == nextOp ? byteOffset : 0;
}

static inline void wasmJitFormatInteriorTargetSamples(char* out, U32 outSize) {
    if (!outSize) {
        return;
    }
    out[0] = 0;
    U32 used = 0;
    bool selected[WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT] = {};
    for (U32 rank = 0; rank < WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT; rank++) {
        U32 best = WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT;
        U64 bestCount = 0;
        for (U32 i = 0; i < WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT; i++) {
            if (selected[i] || !g_wasmJitInteriorTargetSampleEip[i] || g_wasmJitInteriorTargetSampleCount[i] <= bestCount) {
                continue;
            }
            best = i;
            bestCount = g_wasmJitInteriorTargetSampleCount[i];
        }
        if (best == WASM_JIT_INTERIOR_TARGET_SAMPLE_COUNT) {
            break;
        }
        selected[best] = true;
        S32 written = snprintf(out + used, outSize - used, "%s%08x@%08x=%llu/%llu",
            used ? "," : "",
            g_wasmJitInteriorTargetSampleEip[best],
            g_wasmJitInteriorTargetSampleBlockStart[best],
            (unsigned long long)g_wasmJitInteriorTargetSampleCount[best],
            (unsigned long long)g_wasmJitInteriorTargetSampleExactCount[best]);
        if (written <= 0) {
            break;
        }
        used += (U32)written;
        if (used >= outSize) {
            out[outSize - 1] = 0;
            break;
        }
    }
}

static inline void wasmJitRecordFetchNextTransition(CPU* cpu, DecodedOp* nextOp) {
    U64 total = g_wasmJitTransitionFetchNext.fetch_add(1, std::memory_order_relaxed) + 1;
    if (cpu->yield) {
        g_wasmJitTransitionFetchNextYield.fetch_add(1, std::memory_order_relaxed);
    }
    if (!nextOp) {
        g_wasmJitTransitionFetchNextTargetNull.fetch_add(1, std::memory_order_relaxed);
    } else if (wasmJitCanChainTo(cpu, nextOp)) {
        g_wasmJitTransitionFetchNextTargetJit.fetch_add(1, std::memory_order_relaxed);
    } else if (!(nextOp->flags & OP_FLAG_JIT)) {
        g_wasmJitTransitionFetchNextTargetNoJitFlag.fetch_add(1, std::memory_order_relaxed);
    } else if (!nextOp->pfnJitCode) {
        g_wasmJitTransitionFetchNextTargetNoTable.fetch_add(1, std::memory_order_relaxed);
    } else if (nextOp->blockStart != nextOp) {
        g_wasmJitTransitionFetchNextTargetInterior.fetch_add(1, std::memory_order_relaxed);
        U32 targetEip = cpu->eip.u32 - cpu->seg[CS].address;
        U32 byteDistance = wasmJitInteriorByteDistance(nextOp);
        wasmJitRecordInteriorTargetSample(targetEip, targetEip - byteDistance);
    } else if (nextOp->pfn != cpu->thread->process->startJITOp) {
        g_wasmJitTransitionFetchNextTargetPfn.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_wasmJitTransitionFetchNextTargetOther.fetch_add(1, std::memory_order_relaxed);
    }
    if (total % 5000000 == 0) {
        char interiorTop[512];
        wasmJitFormatInteriorTargetSamples(interiorTop, sizeof(interiorTop));
        U64 yieldCount = g_wasmJitTransitionFetchNextYield.load(std::memory_order_relaxed);
        U64 jitCount = g_wasmJitTransitionFetchNextTargetJit.load(std::memory_order_relaxed);
        U64 interiorCount = g_wasmJitTransitionFetchNextTargetInterior.load(std::memory_order_relaxed);
        U64 nullCount = g_wasmJitTransitionFetchNextTargetNull.load(std::memory_order_relaxed);
        U64 noFlagCount = g_wasmJitTransitionFetchNextTargetNoJitFlag.load(std::memory_order_relaxed);
        U64 noTableCount = g_wasmJitTransitionFetchNextTargetNoTable.load(std::memory_order_relaxed);
        U64 pfnCount = g_wasmJitTransitionFetchNextTargetPfn.load(std::memory_order_relaxed);
        U64 otherCount = g_wasmJitTransitionFetchNextTargetOther.load(std::memory_order_relaxed);
        klog_fmt("[WASM JIT transitions] fetchNext=%llu yield=%llu target[jit=%llu,interior=%llu,null=%llu,noFlag=%llu,noTable=%llu,pfn=%llu,other=%llu] interiorTop[%s]",
            (unsigned long long)total,
            (unsigned long long)yieldCount,
            (unsigned long long)jitCount,
            (unsigned long long)interiorCount,
            (unsigned long long)nullCount,
            (unsigned long long)noFlagCount,
            (unsigned long long)noTableCount,
            (unsigned long long)pfnCount,
            (unsigned long long)otherCount,
            interiorTop);
#ifndef BOXEDWINE_MULTI_THREADED
        boxedwine_wasm_record_transition_profile(
            total,
            yieldCount,
            jitCount,
            interiorCount,
            nullCount,
            noFlagCount,
            noTableCount,
            pfnCount,
            otherCount,
            interiorTop);
#else
        // Workers cannot reach the main-thread Module, so collect the same
        // snapshot line in shared memory; wasm_jit_mt_prepare_export mirrors
        // it to Module.wasmJitInteriorProfileLines for the zip export.
        {
            char line[768];
            snprintf(line, sizeof(line),
                "[WASM JIT transitions] fetchNext=%llu yield=%llu target[jit=%llu,interior=%llu,null=%llu,noFlag=%llu,noTable=%llu,pfn=%llu,other=%llu] interiorTop[%s]",
                (unsigned long long)total,
                (unsigned long long)yieldCount,
                (unsigned long long)jitCount,
                (unsigned long long)interiorCount,
                (unsigned long long)nullCount,
                (unsigned long long)noFlagCount,
                (unsigned long long)noTableCount,
                (unsigned long long)pfnCount,
                (unsigned long long)otherCount,
                interiorTop);
            std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
            if (g_wasmInteriorProfileLines.size() >= 256) {
                g_wasmInteriorProfileLines.erase(g_wasmInteriorProfileLines.begin());
            }
            g_wasmInteriorProfileLines.push_back(line);
        }
#endif
    }
}

#ifdef BOXEDWINE_WASM_JIT_PROFILE
static std::atomic<U64> g_wasmJitProfileStartEntries{0};
static std::atomic<U64> g_wasmJitProfileCallBlockEntries{0};
static std::atomic<U64> g_wasmJitProfileBlockExits{0};
static std::atomic<U64> g_wasmJitProfileSlotMisses{0};
static std::atomic<U64> g_wasmJitProfileCompiledBlocks{0};
static std::atomic<U64> g_wasmJitProfileCompiledOps{0};
static std::atomic<U64> g_wasmJitProfileBlockOps1{0};
static std::atomic<U64> g_wasmJitProfileBlockOps2{0};
static std::atomic<U64> g_wasmJitProfileBlockOps3To4{0};
static std::atomic<U64> g_wasmJitProfileBlockOps5To8{0};
static std::atomic<U64> g_wasmJitProfileBlockOps9To16{0};
static std::atomic<U64> g_wasmJitProfileBlockOps17Plus{0};
static std::atomic<U64> g_wasmJitProfileExitNext1{0};
static std::atomic<U64> g_wasmJitProfileExitNext2{0};
static std::atomic<U64> g_wasmJitProfileExitJump{0};
static std::atomic<U64> g_wasmJitProfileExitGeneric{0};
static std::atomic<U64> g_wasmJitProfileChainNextJit{0};
static std::atomic<U64> g_wasmJitProfileChainNextJitPlain{0};
static std::atomic<U64> g_wasmJitProfileChainNextJitMemArrays{0};
static std::atomic<U64> g_wasmJitProfileChainNextNotJit{0};
static std::atomic<U64> g_wasmJitProfileChainNextNull{0};
static std::atomic<U64> g_wasmJitProfileChainStopNull{0};
static std::atomic<U64> g_wasmJitProfileChainStopNoJitFlag{0};
static std::atomic<U64> g_wasmJitProfileChainStopNoTable{0};
static std::atomic<U64> g_wasmJitProfileChainStopInterior{0};
static std::atomic<U64> g_wasmJitProfileChainStopPfn{0};
static std::atomic<U64> g_wasmJitProfileChainStopOther{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx1{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx2{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx3To4{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx5To8{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx9To16{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorIdx17Plus{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorByte1To15{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorByte16To31{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorByte32To63{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorByte64To127{0};
static std::atomic<U64> g_wasmJitProfileChainStopInteriorByte128Plus{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopsCreated{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopOps{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopIterations{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopOps1To4{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopOps5To8{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopOps9To16{0};
static std::atomic<U64> g_wasmJitProfileDirectLoopOps17Plus{0};
static std::atomic<U64> g_wasmJitProfileInteriorByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileInteriorBlockStartByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileChainStopPfnByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileChainStopOtherByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileLoopEntries{0};
static std::atomic<U64> g_wasmJitProfileLoopExtraBlocks{0};
static std::atomic<U64> g_wasmJitProfileLoopLimitStops{0};
static std::atomic<U64> g_wasmJitProfileLoopSampleEntries{0};
static std::atomic<U64> g_wasmJitProfileLoopLen1{0};
static std::atomic<U64> g_wasmJitProfileLoopLen2{0};
static std::atomic<U64> g_wasmJitProfileLoopLen3To4{0};
static std::atomic<U64> g_wasmJitProfileLoopLen5To8{0};
static std::atomic<U64> g_wasmJitProfileLoopLen9To16{0};
static std::atomic<U64> g_wasmJitProfileLoopLen17To32{0};
static std::atomic<U64> g_wasmJitProfileLoopLen33To64{0};
static std::atomic<U64> g_wasmJitProfileLoopLen65To128{0};
static std::atomic<U64> g_wasmJitProfileLoopLen129To511{0};
static std::atomic<U64> g_wasmJitProfileLoopLen512To1023{0};
static std::atomic<U64> g_wasmJitProfileLoopLen1024To2047{0};
static std::atomic<U64> g_wasmJitProfileLoopLenCap{0};
static std::atomic<U64> g_wasmJitProfileMemArrayChecks{0};
static std::atomic<U64> g_wasmJitProfileMemArrayRefreshes{0};
static std::atomic<U64> g_wasmJitProfileFetchNextCalls{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetJit{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetInterior{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetNull{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetNoJitFlag{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetNoTable{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetPfn{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetOther{0};
static std::atomic<U64> g_wasmJitProfileFetchNextTargetByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileGenericExitByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileHelperMemRead{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWrite{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWriteCheck{0};
static std::atomic<U64> g_wasmJitProfileHelperBlockEnter{0};
static std::atomic<U64> g_wasmJitProfileHelperEmulate{0};
static std::atomic<U64> g_wasmJitProfileHelperFlags{0};
static std::atomic<U64> g_wasmJitProfileHelperFlagCF{0};
static std::atomic<U64> g_wasmJitProfileHelperFlagZF{0};
static std::atomic<U64> g_wasmJitProfileHelperFlagFill{0};
static std::atomic<U64> g_wasmJitProfileHelperCond{0};
static std::atomic<U64> g_wasmJitProfileInlineCond{0};
static std::atomic<U64> g_wasmJitProfileCondByType[16];
static std::atomic<U64> g_wasmJitProfileCondByLazyFlag[52];
static std::atomic<U64> g_wasmJitProfileEmulateByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileRmwByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileMovsdTotal{0};
static std::atomic<U64> g_wasmJitProfileMovsdRep{0};
static std::atomic<U64> g_wasmJitProfileMovsdSingle{0};
static std::atomic<U64> g_wasmJitProfileMovsdEa16{0};
static std::atomic<U64> g_wasmJitProfileMovsdEa32{0};
static std::atomic<U64> g_wasmJitProfileMovsdSegmented{0};
static std::atomic<U64> g_wasmJitProfileMovsdFlat{0};
static std::atomic<U64> g_wasmJitProfileMovsdDf1{0};
static std::atomic<U64> g_wasmJitProfileMovsdDf0{0};
static std::atomic<U64> g_wasmJitProfileMovsdRepFlat{0};
static std::atomic<U64> g_wasmJitProfileMovsdRepSegmented{0};
static std::atomic<U64> g_wasmJitProfileMovsdRepEa16{0};
static std::atomic<U64> g_wasmJitProfileJitUs{0};
static std::atomic<U64> g_wasmJitProfileInstantiateUs{0};
static std::atomic<U64> g_wasmJitProfileStartUs{0};
static std::atomic<U64> g_wasmJitProfileStartPreCallUs{0};
static std::atomic<U64> g_wasmJitProfileStartPostCallUs{0};
static std::atomic<U64> g_wasmJitProfileCallBlockUs{0};
static std::atomic<U64> g_wasmJitProfileFetchNextUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemReadUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWriteUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWriteCheckUs{0};
static std::atomic<U64> g_wasmJitProfileHelperBlockEnterUs{0};
static std::atomic<U64> g_wasmJitProfileHelperEmulateUs{0};
static std::atomic<U64> g_wasmJitProfileHelperFlagsUs{0};
static std::atomic<U64> g_wasmJitProfileHelperCondUs{0};
static std::atomic<U32> g_wasmJitProfileLastLogMs{0};

static constexpr U64 WASM_JIT_PROFILE_TIMING_SAMPLE = 1024;
static constexpr U32 WASM_JIT_PROFILE_SAMPLE_MASK = (U32)WASM_JIT_PROFILE_TIMING_SAMPLE - 1;

static constexpr U32 WASM_JIT_PROFILE_MOVSD_REP = 1;
static constexpr U32 WASM_JIT_PROFILE_MOVSD_EA16 = 2;
static constexpr U32 WASM_JIT_PROFILE_MOVSD_SEGMENTED = 4;

static inline U64 wasmJitProfileNowNs() {
#ifdef __EMSCRIPTEN__
    return (U64)(emscripten_get_now() * 1000000.0);
#else
    return KSystem::getSystemTimeAsMicroSeconds() * 1000;
#endif
}

static inline void wasmJitProfileAddElapsed(std::atomic<U64>& counter, U64 startNs) {
    counter.fetch_add(wasmJitProfileNowNs() - startNs, std::memory_order_relaxed);
}

static inline void wasmJitProfileAddElapsedScaled(std::atomic<U64>& counter, U64 startNs) {
    counter.fetch_add((wasmJitProfileNowNs() - startNs) * WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
}

class WasmJitProfileTimer {
public:
    explicit WasmJitProfileTimer(std::atomic<U64>& counter, bool active = true, U64 scale = 1)
        : counter(counter), active(active), scale(scale), startNs(active ? wasmJitProfileNowNs() : 0) {}
    ~WasmJitProfileTimer() {
        if (active) {
            counter.fetch_add((wasmJitProfileNowNs() - startNs) * scale, std::memory_order_relaxed);
        }
    }
private:
    std::atomic<U64>& counter;
    bool active;
    U64 scale;
    U64 startNs;
};

static U64 wasmJitProfileUs(U64 totalNs) {
    return totalNs / 1000;
}

static U64 wasmJitProfileAvgNs(U64 totalNs, U64 count) {
    return count ? (totalNs / count) : 0;
}

static const char* wasmJitProfileCondName(U32 cond) {
    static const char* names[] = {
        "O", "NO", "B", "NB", "Z", "NZ", "BE", "NBE",
        "S", "NS", "P", "NP", "L", "NL", "LE", "NLE"
    };
    return cond < 16 ? names[cond] : "?";
}

static const char* wasmJitProfileLazyFlagName(U32 lazyFlagType) {
    static const char* names[] = {
        "NONE", "ADD8", "ADD16", "ADD32", "OR8", "OR16", "OR32",
        "ADC8", "ADC16", "ADC32", "SBB8", "SBB16", "SBB32",
        "AND8", "AND16", "AND32", "SUB8", "SUB16", "SUB32",
        "XOR8", "XOR16", "XOR32", "INC8", "INC16", "INC32",
        "DEC8", "DEC16", "DEC32", "SHL8", "SHL16", "SHL32",
        "SHR8", "SHR16", "SHR32", "SAR8", "SAR16", "SAR32",
        "CMP8", "CMP16", "CMP32", "TEST8", "TEST16", "TEST32",
        "DSHL16", "DSHL32", "DSHR16", "DSHR32", "NEG8", "NEG16",
        "NEG32", "CFOF", "NULL"
    };
    return lazyFlagType < 52 ? names[lazyFlagType] : "?";
}

static void wasmJitProfileFormatTopCounters(char* dst, size_t dstSize, const std::atomic<U64>* counters, U32 count, const char* (*nameFn)(U32)) {
    U32 topIndex[4] = {0, 0, 0, 0};
    U64 topValue[4] = {0, 0, 0, 0};
    for (U32 i = 0; i < count; i++) {
        U64 value = counters[i].load(std::memory_order_relaxed);
        for (U32 j = 0; j < 4; j++) {
            if (value > topValue[j]) {
                for (U32 k = 3; k > j; k--) {
                    topValue[k] = topValue[k - 1];
                    topIndex[k] = topIndex[k - 1];
                }
                topValue[j] = value;
                topIndex[j] = i;
                break;
            }
        }
    }
    snprintf(dst, dstSize, "%s=%llu %s=%llu %s=%llu %s=%llu",
             nameFn(topIndex[0]), (unsigned long long)topValue[0],
             nameFn(topIndex[1]), (unsigned long long)topValue[1],
             nameFn(topIndex[2]), (unsigned long long)topValue[2],
             nameFn(topIndex[3]), (unsigned long long)topValue[3]);
}

static const char* wasmJitProfileInstName(U32 inst) {
    switch ((Instruction)inst) {
    case Pause: return "Pause";
    case FDIV_ST0_STj: return "FDIV_ST0_STj";
    case Int80: return "Int80";
    case Movsd: return "Movsd";
    case MovsdXmmE64: return "MovsdXmmE64";
    case MovsdE64Xmm: return "MovsdE64Xmm";
    case AddR32E32: return "AddR32E32";
    case AddE32R32: return "AddE32R32";
    case AddR32R32: return "AddR32R32";
    case AddR32I32: return "AddR32I32";
    case AddE32I32: return "AddE32I32";
    case AdcE32R32: return "AdcE32R32";
    case AdcE32I32: return "AdcE32I32";
    case SbbE32I32: return "SbbE32I32";
    case SubE32I32: return "SubE32I32";
    case SbbR32R32: return "SbbR32R32";
    case SbbR32I32: return "SbbR32I32";
    case XorR32R32: return "XorR32R32";
    case CmpR32E32: return "CmpR32E32";
    case CmpE32R32: return "CmpE32R32";
    case CmpR32R32: return "CmpR32R32";
    case CmpR32I32: return "CmpR32I32";
    case CmpE32I32: return "CmpE32I32";
    case JumpO: return "JumpO";
    case JumpNO: return "JumpNO";
    case JumpB: return "JumpB";
    case JumpNB: return "JumpNB";
    case JumpZ: return "JumpZ";
    case JumpNZ: return "JumpNZ";
    case JumpBE: return "JumpBE";
    case JumpNBE: return "JumpNBE";
    case JumpS: return "JumpS";
    case JumpNS: return "JumpNS";
    case JumpP: return "JumpP";
    case JumpNP: return "JumpNP";
    case JumpL: return "JumpL";
    case JumpNL: return "JumpNL";
    case JumpLE: return "JumpLE";
    case JumpNLE: return "JumpNLE";
    case CallR32: return "CallR32";
    case CallE32: return "CallE32";
    case Retn32Iw: return "Retn32Iw";
    case Retn32: return "Retn32";
    case JmpR32: return "JmpR32";
    case JmpE32: return "JmpE32";
    case MovE8R8: return "MovE8R8";
    case MovR32R32: return "MovR32R32";
    case MovE32R32: return "MovE32R32";
    case MovR32E32: return "MovR32E32";
    case MovR32I32: return "MovR32I32";
    case MovE32I32: return "MovE32I32";
    case MovOdEax: return "MovOdEax";
    case MovGdXzE8: return "MovGdXzE8";
    case MovGdXzE16: return "MovGdXzE16";
    case MovGdSxE8: return "MovGdSxE8";
    case LeaR32: return "LeaR32";
    case TestR32R32: return "TestR32R32";
    case OrE32R32: return "OrE32R32";
    case IncE32: return "IncE32";
    case DecE32: return "DecE32";
    case XaddR32E32: return "XaddR32E32";
    case JmpJb: return "JmpJb";
    case IncR32: return "IncR32";
    case PushR32: return "PushR32";
    case PopR32: return "PopR32";
    case FLD1: return "FLD1";
    case FLD_STi: return "FLD_STi";
    case FLD_SINGLE_REAL: return "FLD_SINGLE_REAL";
    case FST_SINGLE_REAL_Pop: return "FST_SINGLE_REAL_Pop";
    case FIST_DWORD_INTEGER_Pop: return "FIST_DWORD_INTEGER_Pop";
    case RcrR32I8: return "RcrR32I8";
    case FADD_ST0_STj: return "FADD_ST0_STj";
    case FCOM_SINGLE_REAL_Pop: return "FCOM_SINGLE_REAL_Pop";
    case FLD_DOUBLE_REAL: return "FLD_DOUBLE_REAL";
    case FNSTSW_AX: return "FNSTSW_AX";
    case Bswap32: return "Bswap32";
    case SubsdXmmXmm: return "SubsdXmmXmm";
    case AndpdXmmXmm: return "AndpdXmmXmm";
    case OrpdXmmXmm: return "OrpdXmmXmm";
    case MovapdXmmXmm: return "MovapdXmmXmm";
    case Enter16: return "Enter16";
    case Lodsw: return "Lodsw";
    case RorR16I8: return "RorR16I8";
    case PsllwXmmXmm: return "PsllwXmmXmm";
    default: return nullptr;
    }
}

static void wasmJitProfileFormatTopInstCounters(char* dst, size_t dstSize, const std::atomic<U64>* counters) {
    static constexpr U32 TopInstCount = 12;
    U32 topIndex[TopInstCount] = {};
    U64 topValue[TopInstCount] = {};
    for (U32 i = 0; i < InstructionCount; i++) {
        U64 value = counters[i].load(std::memory_order_relaxed);
        for (U32 slot = 0; slot < TopInstCount; slot++) {
            if (value > topValue[slot]) {
                for (U32 move = TopInstCount - 1; move > slot; move--) {
                    topValue[move] = topValue[move - 1];
                    topIndex[move] = topIndex[move - 1];
                }
                topValue[slot] = value;
                topIndex[slot] = i;
                break;
            }
        }
    }
    char names[TopInstCount][48];
    for (U32 i = 0; i < TopInstCount; i++) {
        const char* name = wasmJitProfileInstName(topIndex[i]);
        if (name) {
            snprintf(names[i], sizeof(names[i]), "%s(%u)", name, topIndex[i]);
        } else {
            snprintf(names[i], sizeof(names[i]), "%u", topIndex[i]);
        }
    }
    size_t used = 0;
    for (U32 i = 0; i < TopInstCount && used < dstSize; i++) {
        int written = snprintf(dst + used, dstSize - used, "%s%s=%llu",
                               i ? " " : "",
                               names[i],
                               (unsigned long long)topValue[i]);
        if (written < 0) {
            break;
        }
        used += (size_t)written;
    }
}

static void wasmJitProfileMaybeLog() {
    U32 now = KSystem::getMilliesSinceStart();
    U32 last = g_wasmJitProfileLastLogMs.load(std::memory_order_relaxed);
    if (last && now - last < 5000) {
        return;
    }
    if (!g_wasmJitProfileLastLogMs.compare_exchange_strong(last, now, std::memory_order_relaxed)) {
        return;
    }
    U64 compiledBlocks = g_wasmJitProfileCompiledBlocks.load(std::memory_order_relaxed);
    U64 compiledOps = g_wasmJitProfileCompiledOps.load(std::memory_order_relaxed);
    U64 avgOpsX10 = compiledBlocks ? (compiledOps * 10 / compiledBlocks) : 0;
    U64 startCount = g_wasmJitProfileStartEntries.load(std::memory_order_relaxed);
    U64 callBlockCount = g_wasmJitProfileCallBlockEntries.load(std::memory_order_relaxed);
    U64 blockExitCount = g_wasmJitProfileBlockExits.load(std::memory_order_relaxed);
    U64 fetchNextCount = g_wasmJitProfileFetchNextCalls.load(std::memory_order_relaxed);
    U64 memReadCount = g_wasmJitProfileHelperMemRead.load(std::memory_order_relaxed);
    U64 memWriteCount = g_wasmJitProfileHelperMemWrite.load(std::memory_order_relaxed);
    U64 memWriteCheckCount = g_wasmJitProfileHelperMemWriteCheck.load(std::memory_order_relaxed);
    U64 blockEnterCount = g_wasmJitProfileHelperBlockEnter.load(std::memory_order_relaxed);
    U64 emulateCount = g_wasmJitProfileHelperEmulate.load(std::memory_order_relaxed);
    U64 flagsCount = g_wasmJitProfileHelperFlags.load(std::memory_order_relaxed);
    U64 condCount = g_wasmJitProfileHelperCond.load(std::memory_order_relaxed);
    U64 movsdTotal = g_wasmJitProfileMovsdTotal.load(std::memory_order_relaxed);
    U64 chainNextJit = g_wasmJitProfileChainNextJit.load(std::memory_order_relaxed);
    U64 chainNextNotJit = g_wasmJitProfileChainNextNotJit.load(std::memory_order_relaxed);
    U64 chainNextNull = g_wasmJitProfileChainNextNull.load(std::memory_order_relaxed);
    U64 loopEntries = g_wasmJitProfileLoopEntries.load(std::memory_order_relaxed);
    U64 loopExtraBlocks = g_wasmJitProfileLoopExtraBlocks.load(std::memory_order_relaxed);
    U64 loopAvgBlocksX10 = loopEntries ? ((loopEntries + loopExtraBlocks) * 10 / loopEntries) : 0;
    U64 directLoopsCreated = g_wasmJitProfileDirectLoopsCreated.load(std::memory_order_relaxed);
    U64 directLoopAvgOpsX10 = directLoopsCreated ?
        (g_wasmJitProfileDirectLoopOps.load(std::memory_order_relaxed) * 10 / directLoopsCreated) : 0;
    U64 directLoopAvgIterations = directLoopsCreated ?
        (g_wasmJitProfileDirectLoopIterations.load(std::memory_order_relaxed) / directLoopsCreated) : 0;
    U64 memArrayChecks = g_wasmJitProfileMemArrayChecks.load(std::memory_order_relaxed);
    U64 jitUs = g_wasmJitProfileJitUs.load(std::memory_order_relaxed);
    U64 instantiateUs = g_wasmJitProfileInstantiateUs.load(std::memory_order_relaxed);
    U64 startUs = g_wasmJitProfileStartUs.load(std::memory_order_relaxed);
    U64 startPreCallUs = g_wasmJitProfileStartPreCallUs.load(std::memory_order_relaxed);
    U64 startPostCallUs = g_wasmJitProfileStartPostCallUs.load(std::memory_order_relaxed);
    U64 callBlockUs = g_wasmJitProfileCallBlockUs.load(std::memory_order_relaxed);
    U64 fetchNextUs = g_wasmJitProfileFetchNextUs.load(std::memory_order_relaxed);
    U64 memReadUs = g_wasmJitProfileHelperMemReadUs.load(std::memory_order_relaxed);
    U64 memWriteUs = g_wasmJitProfileHelperMemWriteUs.load(std::memory_order_relaxed);
    U64 memWriteCheckUs = g_wasmJitProfileHelperMemWriteCheckUs.load(std::memory_order_relaxed);
    U64 blockEnterUs = g_wasmJitProfileHelperBlockEnterUs.load(std::memory_order_relaxed);
    U64 emulateUs = g_wasmJitProfileHelperEmulateUs.load(std::memory_order_relaxed);
    U64 flagsUs = g_wasmJitProfileHelperFlagsUs.load(std::memory_order_relaxed);
    U64 condUs = g_wasmJitProfileHelperCondUs.load(std::memory_order_relaxed);
    char topCond[96];
    char topLazy[128];
    char topEmulate[512];
    char topInterior[512];
    char topInteriorBlockStart[512];
    char topChainStopPfn[512];
    char topChainStopOther[512];
    char topFetchNextTarget[512];
    char topGenericExit[512];
    char topRmw[512];
    wasmJitProfileFormatTopCounters(topCond, sizeof(topCond), g_wasmJitProfileCondByType, 16, wasmJitProfileCondName);
    wasmJitProfileFormatTopCounters(topLazy, sizeof(topLazy), g_wasmJitProfileCondByLazyFlag, 52, wasmJitProfileLazyFlagName);
    wasmJitProfileFormatTopInstCounters(topEmulate, sizeof(topEmulate), g_wasmJitProfileEmulateByInst);
    wasmJitProfileFormatTopInstCounters(topInterior, sizeof(topInterior), g_wasmJitProfileInteriorByInst);
    wasmJitProfileFormatTopInstCounters(topInteriorBlockStart, sizeof(topInteriorBlockStart), g_wasmJitProfileInteriorBlockStartByInst);
    wasmJitProfileFormatTopInstCounters(topChainStopPfn, sizeof(topChainStopPfn), g_wasmJitProfileChainStopPfnByInst);
    wasmJitProfileFormatTopInstCounters(topChainStopOther, sizeof(topChainStopOther), g_wasmJitProfileChainStopOtherByInst);
    wasmJitProfileFormatTopInstCounters(topFetchNextTarget, sizeof(topFetchNextTarget), g_wasmJitProfileFetchNextTargetByInst);
    wasmJitProfileFormatTopInstCounters(topGenericExit, sizeof(topGenericExit), g_wasmJitProfileGenericExitByInst);
    wasmJitProfileFormatTopInstCounters(topRmw, sizeof(topRmw), g_wasmJitProfileRmwByInst);
    klog_fmt("[WASM JIT profile] start=%llu call_block=%llu block_exit=%llu slot_miss=%llu "
             "compiled=%llu avg_ops=%llu.%llu ops[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17+=%llu] "
             "exits[next1=%llu next2=%llu jump=%llu generic=%llu] "
             "helpers[fetch_next=%llu mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu inline_cond=%llu] "
             "flag_helpers[cf=%llu zf=%llu fill=%llu] "
             "fetch_next_target[jit=%llu interior=%llu null=%llu no_flag=%llu no_table=%llu pfn=%llu other=%llu] "
             "chain[next_jit=%llu plain=%llu mem_arrays=%llu not_jit=%llu null=%llu pct=%llu] "
             "chain_stop[null=%llu no_flag=%llu no_table=%llu interior=%llu pfn=%llu other=%llu] "
             "interior_idx[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17+=%llu] "
             "interior_byte[1-15=%llu 16-31=%llu 32-63=%llu 64-127=%llu 128+=%llu] "
             "direct_loop[created=%llu avg_ops=%llu.%llu avg_iter=%llu ops1-4=%llu ops5-8=%llu ops9-16=%llu ops17+=%llu] "
             "loop[entries=%llu extra=%llu avg_blocks=%llu.%llu limit=%llu] "
             "loop_len[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17-32=%llu 33-64=%llu 65-128=%llu 129-511=%llu 512-1023=%llu 1024-2047=%llu cap=%llu] "
             "mem_arrays[checks=%llu refresh=%llu] "
             "time_us[jit=%llu instantiate=%llu codegen=%llu start=%llu start_pre=%llu start_post=%llu call_block=%llu fetch_next=%llu "
             "mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu] "
             "avg_ns[start=%llu start_pre=%llu start_post=%llu call_block=%llu fetch_next=%llu mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu] "
             "movsd[total=%llu rep=%llu single=%llu ea16=%llu ea32=%llu seg=%llu flat=%llu df1=%llu df0=%llu rep_flat=%llu rep_seg=%llu rep_ea16=%llu] "
             "cond_top[%s] lazy_top[%s] emulate_top[%s] interior_top[%s] interior_block_top[%s] "
             "chain_pfn_top[%s] chain_other_top[%s] fetch_next_top[%s] generic_exit_top[%s] rmw_top[%s]",
             (unsigned long long)startCount,
             (unsigned long long)callBlockCount,
             (unsigned long long)blockExitCount,
             (unsigned long long)g_wasmJitProfileSlotMisses.load(std::memory_order_relaxed),
             (unsigned long long)compiledBlocks,
             (unsigned long long)(avgOpsX10 / 10),
             (unsigned long long)(avgOpsX10 % 10),
             (unsigned long long)g_wasmJitProfileBlockOps1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBlockOps2.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBlockOps3To4.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBlockOps5To8.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBlockOps9To16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBlockOps17Plus.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileExitNext1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileExitNext2.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileExitJump.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileExitGeneric.load(std::memory_order_relaxed),
             (unsigned long long)fetchNextCount,
             (unsigned long long)memReadCount,
             (unsigned long long)memWriteCount,
             (unsigned long long)memWriteCheckCount,
             (unsigned long long)blockEnterCount,
             (unsigned long long)emulateCount,
             (unsigned long long)flagsCount,
             (unsigned long long)condCount,
             (unsigned long long)g_wasmJitProfileInlineCond.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileHelperFlagCF.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileHelperFlagZF.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileHelperFlagFill.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetJit.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetInterior.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetNull.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetNoJitFlag.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetNoTable.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetPfn.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileFetchNextTargetOther.load(std::memory_order_relaxed),
             (unsigned long long)chainNextJit,
             (unsigned long long)g_wasmJitProfileChainNextJitPlain.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainNextJitMemArrays.load(std::memory_order_relaxed),
             (unsigned long long)chainNextNotJit,
             (unsigned long long)chainNextNull,
             (unsigned long long)(callBlockCount ? (chainNextJit * 100 / callBlockCount) : 0),
             (unsigned long long)g_wasmJitProfileChainStopNull.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopNoJitFlag.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopNoTable.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInterior.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopPfn.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopOther.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx2.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx3To4.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx5To8.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx9To16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorIdx17Plus.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorByte1To15.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorByte16To31.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorByte32To63.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorByte64To127.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileChainStopInteriorByte128Plus.load(std::memory_order_relaxed),
             (unsigned long long)directLoopsCreated,
             (unsigned long long)(directLoopAvgOpsX10 / 10),
             (unsigned long long)(directLoopAvgOpsX10 % 10),
             (unsigned long long)directLoopAvgIterations,
             (unsigned long long)g_wasmJitProfileDirectLoopOps1To4.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileDirectLoopOps5To8.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileDirectLoopOps9To16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileDirectLoopOps17Plus.load(std::memory_order_relaxed),
             (unsigned long long)loopEntries,
             (unsigned long long)loopExtraBlocks,
             (unsigned long long)(loopAvgBlocksX10 / 10),
             (unsigned long long)(loopAvgBlocksX10 % 10),
             (unsigned long long)g_wasmJitProfileLoopLimitStops.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen2.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen3To4.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen5To8.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen9To16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen17To32.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen33To64.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen65To128.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen129To511.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen512To1023.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLen1024To2047.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileLoopLenCap.load(std::memory_order_relaxed),
             (unsigned long long)memArrayChecks,
             (unsigned long long)g_wasmJitProfileMemArrayRefreshes.load(std::memory_order_relaxed),
             (unsigned long long)wasmJitProfileUs(jitUs),
             (unsigned long long)wasmJitProfileUs(instantiateUs),
             (unsigned long long)wasmJitProfileUs(jitUs > instantiateUs ? jitUs - instantiateUs : 0),
             (unsigned long long)wasmJitProfileUs(startUs),
             (unsigned long long)wasmJitProfileUs(startPreCallUs),
             (unsigned long long)wasmJitProfileUs(startPostCallUs),
             (unsigned long long)wasmJitProfileUs(callBlockUs),
             (unsigned long long)wasmJitProfileUs(fetchNextUs),
             (unsigned long long)wasmJitProfileUs(memReadUs),
             (unsigned long long)wasmJitProfileUs(memWriteUs),
             (unsigned long long)wasmJitProfileUs(memWriteCheckUs),
             (unsigned long long)wasmJitProfileUs(blockEnterUs),
             (unsigned long long)wasmJitProfileUs(emulateUs),
             (unsigned long long)wasmJitProfileUs(flagsUs),
             (unsigned long long)wasmJitProfileUs(condUs),
             (unsigned long long)wasmJitProfileAvgNs(startUs, startCount),
             (unsigned long long)wasmJitProfileAvgNs(startPreCallUs, callBlockCount),
             (unsigned long long)wasmJitProfileAvgNs(startPostCallUs, callBlockCount),
             (unsigned long long)wasmJitProfileAvgNs(callBlockUs, callBlockCount),
             (unsigned long long)wasmJitProfileAvgNs(fetchNextUs, fetchNextCount),
             (unsigned long long)wasmJitProfileAvgNs(memReadUs, memReadCount),
             (unsigned long long)wasmJitProfileAvgNs(memWriteUs, memWriteCount),
             (unsigned long long)wasmJitProfileAvgNs(memWriteCheckUs, memWriteCheckCount),
             (unsigned long long)wasmJitProfileAvgNs(blockEnterUs, blockEnterCount),
             (unsigned long long)wasmJitProfileAvgNs(emulateUs, emulateCount),
             (unsigned long long)wasmJitProfileAvgNs(flagsUs, flagsCount),
             (unsigned long long)wasmJitProfileAvgNs(condUs, condCount),
             (unsigned long long)movsdTotal,
             (unsigned long long)g_wasmJitProfileMovsdRep.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdSingle.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdEa16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdEa32.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdSegmented.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdFlat.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdDf1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdDf0.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdRepFlat.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdRepSegmented.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileMovsdRepEa16.load(std::memory_order_relaxed),
             topCond,
             topLazy,
             topEmulate,
             topInterior,
             topInteriorBlockStart,
             topChainStopPfn,
             topChainStopOther,
             topFetchNextTarget,
             topGenericExit,
             topRmw);
}

static inline bool wasmJitProfileStartEntry() {
    static thread_local U32 sampleCounter = 0;
    bool sample = (++sampleCounter & WASM_JIT_PROFILE_SAMPLE_MASK) == 0;
    if (sample) {
        g_wasmJitProfileStartEntries.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
        wasmJitProfileMaybeLog();
    }
    return sample;
}

static inline bool wasmJitProfileCallBlock() {
    static thread_local U32 sampleCounter = 0;
    bool sample = (++sampleCounter & WASM_JIT_PROFILE_SAMPLE_MASK) == 0;
    if (sample) {
        g_wasmJitProfileCallBlockEntries.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return sample;
}

static inline void wasmJitProfileBlockExit(U64 scale = 1) {
    g_wasmJitProfileBlockExits.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileSlotMiss() {
    g_wasmJitProfileSlotMisses.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileCompiledBlock(U32 opCount) {
    g_wasmJitProfileCompiledBlocks.fetch_add(1, std::memory_order_relaxed);
    g_wasmJitProfileCompiledOps.fetch_add(opCount, std::memory_order_relaxed);
    if (opCount <= 1) {
        g_wasmJitProfileBlockOps1.fetch_add(1, std::memory_order_relaxed);
    } else if (opCount == 2) {
        g_wasmJitProfileBlockOps2.fetch_add(1, std::memory_order_relaxed);
    } else if (opCount <= 4) {
        g_wasmJitProfileBlockOps3To4.fetch_add(1, std::memory_order_relaxed);
    } else if (opCount <= 8) {
        g_wasmJitProfileBlockOps5To8.fetch_add(1, std::memory_order_relaxed);
    } else if (opCount <= 16) {
        g_wasmJitProfileBlockOps9To16.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileBlockOps17Plus.fetch_add(1, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileExitNext1(U64 scale = 1) {
    g_wasmJitProfileExitNext1.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitNext2(U64 scale = 1) {
    g_wasmJitProfileExitNext2.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitJump(U64 scale = 1) {
    g_wasmJitProfileExitJump.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitGeneric(U32 inst, U64 scale = 1) {
    g_wasmJitProfileExitGeneric.fetch_add(scale, std::memory_order_relaxed);
    if (inst < InstructionCount) {
        g_wasmJitProfileGenericExitByInst[inst].fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline bool wasmJitProfileSample(U32& counter) {
    return (++counter & WASM_JIT_PROFILE_SAMPLE_MASK) == 0;
}

static inline bool wasmJitProfileHelperMemRead() {
    static thread_local U32 sampleCounter = 0;
    bool sample = wasmJitProfileSample(sampleCounter);
    if (sample) {
        g_wasmJitProfileHelperMemRead.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return sample;
}

static inline bool wasmJitProfileHelperMemWrite() {
    static thread_local U32 sampleCounter = 0;
    bool sample = wasmJitProfileSample(sampleCounter);
    if (sample) {
        g_wasmJitProfileHelperMemWrite.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return sample;
}

static inline bool wasmJitProfileHelperMemWriteCheck() {
    static thread_local U32 sampleCounter = 0;
    bool sample = wasmJitProfileSample(sampleCounter);
    if (sample) {
        g_wasmJitProfileHelperMemWriteCheck.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return sample;
}

static inline void wasmJitProfileHelperBlockEnter() {
    g_wasmJitProfileHelperBlockEnter.fetch_add(1, std::memory_order_relaxed);
}

static inline bool wasmJitProfileHelperEmulate() {
    static thread_local U32 sampleCounter = 0;
    bool sample = wasmJitProfileSample(sampleCounter);
    if (sample) {
        g_wasmJitProfileHelperEmulate.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return sample;
}

static inline bool wasmJitProfileHelperFlags(U32 detail) {
    static thread_local U32 sampleCounter[3] = {};
    bool sample = detail < 3 && wasmJitProfileSample(sampleCounter[detail]);
    if (!sample) {
        return false;
    }
    g_wasmJitProfileHelperFlags.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    std::atomic<U64>* detailCounters[] = {
        &g_wasmJitProfileHelperFlagCF,
        &g_wasmJitProfileHelperFlagZF,
        &g_wasmJitProfileHelperFlagFill,
    };
    detailCounters[detail]->fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    return true;
}

static inline bool wasmJitProfileHelperCond(U32 cond, U32 lazyFlagType) {
    static thread_local U32 sampleCounter = 0;
    bool sample = wasmJitProfileSample(sampleCounter);
    if (!sample) {
        return false;
    }
    g_wasmJitProfileHelperCond.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    if (cond < 16) {
        g_wasmJitProfileCondByType[cond].fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    if (lazyFlagType < 52) {
        g_wasmJitProfileCondByLazyFlag[lazyFlagType].fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    return true;
}

static inline void wasmJitProfileInlineCond(U64 scale = 1) {
    g_wasmJitProfileInlineCond.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileChainTarget(CPU* cpu, DecodedOp* nextOp, U64 scale) {
    if (!nextOp) {
        g_wasmJitProfileChainNextNull.fetch_add(scale, std::memory_order_relaxed);
        return;
    }
    if (wasmJitCanChainTo(cpu, nextOp)) {
        g_wasmJitProfileChainNextJit.fetch_add(scale, std::memory_order_relaxed);
        if (nextOp->flags2 & OP_FLAG2_WASM_JIT_MEM_ARRAYS) {
            g_wasmJitProfileChainNextJitMemArrays.fetch_add(scale, std::memory_order_relaxed);
        } else {
            g_wasmJitProfileChainNextJitPlain.fetch_add(scale, std::memory_order_relaxed);
        }
        return;
    }
    g_wasmJitProfileChainNextNotJit.fetch_add(scale, std::memory_order_relaxed);
}

static inline void wasmJitProfileChainStop(CPU* cpu, DecodedOp* nextOp, U64 scale) {
    if (!nextOp) {
        g_wasmJitProfileChainStopNull.fetch_add(scale, std::memory_order_relaxed);
    } else if (!(nextOp->flags & OP_FLAG_JIT)) {
        g_wasmJitProfileChainStopNoJitFlag.fetch_add(scale, std::memory_order_relaxed);
    } else if (!nextOp->pfnJitCode) {
        g_wasmJitProfileChainStopNoTable.fetch_add(scale, std::memory_order_relaxed);
    } else if (nextOp->blockStart != nextOp) {
        g_wasmJitProfileChainStopInterior.fetch_add(scale, std::memory_order_relaxed);
        U32 index = 0;
        U32 byteOffset = 0;
        DecodedOp* cur = nextOp->blockStart;
        U32 blockOpCount = cur ? cur->blockOpCount : 0;
        while (cur && cur != nextOp && index < blockOpCount) {
            byteOffset += cur->len;
            cur = cur->next;
            index++;
        }
        if (nextOp->inst < InstructionCount) {
            g_wasmJitProfileInteriorByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
        }
        if (nextOp->blockStart && nextOp->blockStart->inst < InstructionCount) {
            g_wasmJitProfileInteriorBlockStartByInst[nextOp->blockStart->inst].fetch_add(scale, std::memory_order_relaxed);
        }
        if (index <= 1) {
            g_wasmJitProfileChainStopInteriorIdx1.fetch_add(scale, std::memory_order_relaxed);
        } else if (index == 2) {
            g_wasmJitProfileChainStopInteriorIdx2.fetch_add(scale, std::memory_order_relaxed);
        } else if (index <= 4) {
            g_wasmJitProfileChainStopInteriorIdx3To4.fetch_add(scale, std::memory_order_relaxed);
        } else if (index <= 8) {
            g_wasmJitProfileChainStopInteriorIdx5To8.fetch_add(scale, std::memory_order_relaxed);
        } else if (index <= 16) {
            g_wasmJitProfileChainStopInteriorIdx9To16.fetch_add(scale, std::memory_order_relaxed);
        } else {
            g_wasmJitProfileChainStopInteriorIdx17Plus.fetch_add(scale, std::memory_order_relaxed);
        }
        if (byteOffset <= 15) {
            g_wasmJitProfileChainStopInteriorByte1To15.fetch_add(scale, std::memory_order_relaxed);
        } else if (byteOffset <= 31) {
            g_wasmJitProfileChainStopInteriorByte16To31.fetch_add(scale, std::memory_order_relaxed);
        } else if (byteOffset <= 63) {
            g_wasmJitProfileChainStopInteriorByte32To63.fetch_add(scale, std::memory_order_relaxed);
        } else if (byteOffset <= 127) {
            g_wasmJitProfileChainStopInteriorByte64To127.fetch_add(scale, std::memory_order_relaxed);
        } else {
            g_wasmJitProfileChainStopInteriorByte128Plus.fetch_add(scale, std::memory_order_relaxed);
        }
    } else if (nextOp->pfn != cpu->thread->process->startJITOp) {
        g_wasmJitProfileChainStopPfn.fetch_add(scale, std::memory_order_relaxed);
        if (nextOp->inst < InstructionCount) {
            g_wasmJitProfileChainStopPfnByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
        }
    } else {
        g_wasmJitProfileChainStopOther.fetch_add(scale, std::memory_order_relaxed);
        if (nextOp->inst < InstructionCount) {
            g_wasmJitProfileChainStopOtherByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
        }
    }
}

static inline void wasmJitProfileFetchNextTarget(CPU* cpu, DecodedOp* nextOp, U64 scale) {
    if (!nextOp) {
        g_wasmJitProfileFetchNextTargetNull.fetch_add(scale, std::memory_order_relaxed);
    } else if (wasmJitCanChainTo(cpu, nextOp)) {
        g_wasmJitProfileFetchNextTargetJit.fetch_add(scale, std::memory_order_relaxed);
    } else if (!(nextOp->flags & OP_FLAG_JIT)) {
        g_wasmJitProfileFetchNextTargetNoJitFlag.fetch_add(scale, std::memory_order_relaxed);
    } else if (!nextOp->pfnJitCode) {
        g_wasmJitProfileFetchNextTargetNoTable.fetch_add(scale, std::memory_order_relaxed);
    } else if (nextOp->blockStart != nextOp) {
        g_wasmJitProfileFetchNextTargetInterior.fetch_add(scale, std::memory_order_relaxed);
    } else if (nextOp->pfn != cpu->thread->process->startJITOp) {
        g_wasmJitProfileFetchNextTargetPfn.fetch_add(scale, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileFetchNextTargetOther.fetch_add(scale, std::memory_order_relaxed);
    }
    if (nextOp && nextOp->inst < InstructionCount) {
        g_wasmJitProfileFetchNextTargetByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileLoop(U32 blocks, bool stoppedAtLimit, U64 scale) {
    g_wasmJitProfileLoopEntries.fetch_add(scale, std::memory_order_relaxed);
    if (blocks > 1) {
        g_wasmJitProfileLoopExtraBlocks.fetch_add((blocks - 1) * scale, std::memory_order_relaxed);
    }
    if (blocks <= 1) {
        g_wasmJitProfileLoopLen1.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks == 2) {
        g_wasmJitProfileLoopLen2.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 4) {
        g_wasmJitProfileLoopLen3To4.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 8) {
        g_wasmJitProfileLoopLen5To8.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 16) {
        g_wasmJitProfileLoopLen9To16.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 32) {
        g_wasmJitProfileLoopLen17To32.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 64) {
        g_wasmJitProfileLoopLen33To64.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 128) {
        g_wasmJitProfileLoopLen65To128.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 511) {
        g_wasmJitProfileLoopLen129To511.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks <= 1023) {
        g_wasmJitProfileLoopLen512To1023.fetch_add(scale, std::memory_order_relaxed);
    } else if (blocks < WASM_JIT_CHAIN_BLOCK_LIMIT) {
        g_wasmJitProfileLoopLen1024To2047.fetch_add(scale, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileLoopLenCap.fetch_add(scale, std::memory_order_relaxed);
    }
    if (stoppedAtLimit) {
        g_wasmJitProfileLoopLimitStops.fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline bool wasmJitProfileLoopSample() {
    static thread_local U32 sampleCounter = 0;
    return wasmJitProfileSample(sampleCounter);
}

static inline void wasmJitProfileMemArrayCheck(bool refreshed, U64 scale) {
    g_wasmJitProfileMemArrayChecks.fetch_add(scale, std::memory_order_relaxed);
    if (refreshed) {
        g_wasmJitProfileMemArrayRefreshes.fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileMovsd(U32 shape, bool df, U64 scale) {
    g_wasmJitProfileMovsdTotal.fetch_add(scale, std::memory_order_relaxed);
    bool rep = (shape & WASM_JIT_PROFILE_MOVSD_REP) != 0;
    bool ea16 = (shape & WASM_JIT_PROFILE_MOVSD_EA16) != 0;
    bool segmented = (shape & WASM_JIT_PROFILE_MOVSD_SEGMENTED) != 0;
    (rep ? g_wasmJitProfileMovsdRep : g_wasmJitProfileMovsdSingle).fetch_add(scale, std::memory_order_relaxed);
    (ea16 ? g_wasmJitProfileMovsdEa16 : g_wasmJitProfileMovsdEa32).fetch_add(scale, std::memory_order_relaxed);
    (segmented ? g_wasmJitProfileMovsdSegmented : g_wasmJitProfileMovsdFlat).fetch_add(scale, std::memory_order_relaxed);
    (df ? g_wasmJitProfileMovsdDf1 : g_wasmJitProfileMovsdDf0).fetch_add(scale, std::memory_order_relaxed);
    if (rep) {
        (segmented ? g_wasmJitProfileMovsdRepSegmented : g_wasmJitProfileMovsdRepFlat).fetch_add(scale, std::memory_order_relaxed);
        if (ea16) {
            g_wasmJitProfileMovsdRepEa16.fetch_add(scale, std::memory_order_relaxed);
        }
    }
}
#define WASM_JIT_PROFILE_ONLY(stmt) stmt
#else
#define WASM_JIT_PROFILE_ONLY(stmt)
#endif

static bool wasmMemoryPageArraysNeedRefresh(CPU* cpu, KMemoryData** memoryDataOut) {
    KMemoryData* d = cpu->memory->getData();
    *memoryDataOut = d;
    U32 memoryData = (U32)(uintptr_t)d;
    return cpu->wasmJitMemoryData != memoryData;
}

static void wasmPrepareBlockEnter(CPU* cpu, KMemoryData* memoryData) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperBlockEnter();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperBlockEnterUs);
#endif
    cpu->wasmJitMemoryData = (U32)(uintptr_t)memoryData;
    cpu->wasmReadPageBaseArray  = (U32)(uintptr_t)memoryData->wasmReadPageBase;
    cpu->wasmWritePageBaseArray = (U32)(uintptr_t)memoryData->wasmWritePageBase;
}

// ---------------------------------------------------------------------------
// JS interop: compile and instantiate a WASM module synchronously.
//
// The generated module imports:
//   - "env"."memory"          : Emscripten linear memory
//   - "helpers"."fn_0" etc.   : C++ helper functions (by table index)
//
// Returns the wasmTable index of the compiled "execute(cpu_ptr: i32)" function,
// or -1 on failure.
// ---------------------------------------------------------------------------
// Single-threaded build: use Emscripten's addFunction helper.
EM_JS(int, boxedwine_wasm_instantiate,
      (const void* bytes, int size, const void** importFns, int importCount),
{
    try {
        var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
        var mod = new WebAssembly.Module(wasmBytes);

        // Build the helper imports object.  Each entry in importFns is a C++
        // function pointer that already lives in wasmTable (since all Emscripten
        // functions are in the table).
        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }

        var inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });

        // Add the "execute" export to wasmTable and return its index.
        // 'vii': every block takes (cpu_ptr, relocBase).
        var fn = inst.exports['execute'];
        if (!fn) return -1;
        var idx = addFunction(fn, 'vii');

#ifndef __TEST
        if (!Module.wasmJitStats) {
            Module.wasmJitStats = { hits: 0, misses: 0, eipMisses: 0, hashMisses: 0, stale: 0, cachedInstalls: 0, freshCompiled: 0, saved: 0, eipMissSamples: [], hashMissSamples: [] };
        }
        Module.wasmJitStats.freshCompiled++;
        if (Module.wasmJitStats.freshCompiled % 1000 === 0) {
            console.log('[WASM JIT] freshCompiled=' + Module.wasmJitStats.freshCompiled +
                ' latestSlot=' + idx +
                ' tableLen=' + wasmTable.length);
        }
#endif

        return idx;
    } catch(e) {
        console.error('boxedwine_wasm_instantiate failed:', e);
        return -1;
    }
});

#if !defined(BOXEDWINE_MULTI_THREADED) && !defined(__TEST)

// ---------------------------------------------------------------------------
// JIT module persistence (single-threaded JS-cache path).
// Only reached when the runtime persistence mode is active (see
// wasmJitPersistenceActive); a plain session never calls into here.
//
// Main-thread JS caches, created/preloaded by initWasmJitCache() in
// boxedwine-shell.js before the emulator starts:
//   Module.wasmJitCache            key -> wasm bytes (flat modules)
//   Module.wasmJitCompiledCache    key -> precompiled WebAssembly.Module
//   Module.wasmJitGroupModules     group path -> compiled merged group module
//   Module.wasmJitGroupInstances   group path -> instantiated merged group
//   Module.wasmJitGroupEntryMap    key -> { groupPath, exportName }
//   Module.wasmJitInstalledCache   key -> wasmTable index of installed export
//   Module.wasmJitInstalledByTableIndex   reverse map for free-block cleanup
//   Module.wasmJitProfileSplitTargets     blockStart hex -> target hex hints
//
// The cache key convention is 'v5-<eip hex8>-<blockHash hex8>'; it is computed
// inline here (rather than calling a shell-defined helper) so these EM_JS
// bodies stay self-contained and never depend on which shell page loaded us.
// ---------------------------------------------------------------------------

// Consulted by shouldStopBlockBefore() while a block is being built: returns 1
// when a profile-guided split hint says the block must end before currentEip
// so the hot interior target can become its own block-start cache entry.
EM_JS(int, boxedwine_wasm_profile_split_before, (U32 blockStartEip, U32 currentEip), {
    if (!Module.wasmJitProfileSplitTargets) return 0;
    function hex32(v) { return ('00000000' + ((v >>> 0).toString(16))).slice(-8); }
    var blockStartKey = hex32(blockStartEip);
    var target = Module.wasmJitProfileSplitTargets.get(blockStartKey);
    if (!target) return 0;
    if (!Module.wasmJitProfileSplitStats) {
        Module.wasmJitProfileSplitStats = {
            hints: Module.wasmJitProfileSplitTargets.size,
            hintedLookups: 0, exactHits: 0, installedHits: 0, groupHits: 0, flatHits: 0,
            bypasses: 0, hintedCompileChecks: 0, compileStops: 0, splitTargetSaves: 0
        };
    }
    var stats = Module.wasmJitProfileSplitStats;
    stats.hintedCompileChecks++;
    if (target === hex32(currentEip)) {
        stats.compileStops++;
        if (stats.compileStops <= 8 || stats.compileStops % 100 === 0) {
            console.log('[WASM JIT profile split] compile stop block=' + blockStartKey +
                ' target=' + target + ' ' + JSON.stringify(stats));
        }
        return 1;
    }
    return 0;
});

// Look up a previously compiled block by CS-relative EIP + block hash.
// Resolution order: already-installed merged export -> merged group entry
// (instantiating the group module on first touch) -> flat cached bytes /
// precompiled module. Returns the wasmTable index (directly usable as
// pfnJitCode), or -1 on miss / stale entry / instantiation failure.
EM_JS(int, boxedwine_wasm_lookup_cached,
      (U32 eip, U32 blockHash, const void** importFns, int importCount, U32 relocBase, U32 relocCount, U32 memId),
{
    if (!Module.wasmJitStats) {
        Module.wasmJitStats = { hits: 0, misses: 0, eipMisses: 0, hashMisses: 0, stale: 0, cachedInstalls: 0, freshCompiled: 0, saved: 0, eipMissSamples: [], hashMissSamples: [] };
    }
    function hex32(v) { return ('00000000' + ((v >>> 0).toString(16))).slice(-8); }
    function splitStats() {
        if (!Module.wasmJitProfileSplitStats) {
            Module.wasmJitProfileSplitStats = {
                hints: Module.wasmJitProfileSplitTargets ? Module.wasmJitProfileSplitTargets.size : 0,
                hintedLookups: 0, exactHits: 0, installedHits: 0, groupHits: 0, flatHits: 0,
                bypasses: 0, hintedCompileChecks: 0, compileStops: 0, splitTargetSaves: 0
            };
        }
        return Module.wasmJitProfileSplitStats;
    }
    function recordSplitExactHit(kind) {
        var p = splitStats();
        p.exactHits++;
        if (kind === 'installed') p.installedHits++;
        else if (kind === 'group') p.groupHits++;
        else if (kind === 'flat') p.flatHits++;
    }
    function getMergedProfile() {
        if (!Module.wasmJitMergedProfile) {
            var groupedStats = (Module.wasmJitGroupedManifest && Module.wasmJitGroupedManifest.stats) || {};
            var directStats = groupedStats.directCalls || {};
            var groups = (Module.wasmJitGroupedManifest && Module.wasmJitGroupedManifest.groups) || [];
            var manifestEntries = 0;
            for (var gi = 0; gi < groups.length; gi++) {
                manifestEntries += (groups[gi].entries || []).length;
            }
            Module.wasmJitMergedProfile = {
                lookups: 0,
                groupAvailable: 0,
                groupHits: 0,
                flatHits: 0,
                installedReuse: 0,
                relocCopies: 0,
                groupInstantiates: 0,
                groupInstanceReuse: 0,
                groupInstallAttempts: 0,
                groupInstallSuccess: 0,
                groupInstallFail: 0,
                manifestEntries: manifestEntries,
                manifestGroups: groups.length,
                directSites: directStats.rewritten || 0,
                candidateEdges: directStats.candidateEdges || 0,
                guarded: directStats.guardedRewritten || 0,
                yieldAware: !!directStats.yieldAwareTailCalls
            };
        }
        return Module.wasmJitMergedProfile;
    }
    function logMergedProfile() {
        var p = getMergedProfile();
        if (p.lookups > 0 && p.lookups % 1000 === 0) {
            console.log('[WASM JIT merged profile] lookups=' + p.lookups +
                ' groupAvailable=' + p.groupAvailable +
                ' groupHits=' + p.groupHits +
                ' flatHits=' + p.flatHits +
                ' installedReuse=' + p.installedReuse +
                ' relocCopies=' + p.relocCopies +
                ' groupInstantiates=' + p.groupInstantiates +
                ' groupInstanceReuse=' + p.groupInstanceReuse +
                ' groupInstallAttempts=' + p.groupInstallAttempts +
                ' groupInstallSuccess=' + p.groupInstallSuccess +
                ' groupInstallFail=' + p.groupInstallFail +
                ' manifestGroups=' + p.manifestGroups +
                ' manifestEntries=' + p.manifestEntries +
                ' directSites=' + p.directSites +
                ' candidateEdges=' + p.candidateEdges +
                ' guarded=' + p.guarded +
                ' yieldAware=' + (p.yieldAware ? 1 : 0));
        }
    }
    getMergedProfile().lookups++;
    function logStats() {
        var s = Module.wasmJitStats;
        var lookups = s.hits + s.misses + s.stale;
        if (lookups > 0 && lookups % 1000 === 0) {
            var samples = "";
            if (s.eipMissSamples.length > 0) {
                samples += ' eipMissSamples=' + s.eipMissSamples.join(',');
            }
            if (s.hashMissSamples.length > 0) {
                samples += ' hashMissSamples=' + s.hashMissSamples.join(',');
            }
            console.log('[WASM JIT cache] hits=' + s.hits +
                ' misses=' + s.misses +
                ' eipMisses=' + s.eipMisses +
                ' hashMisses=' + s.hashMisses +
                ' stale=' + s.stale +
                ' cachedInstalls=' + s.cachedInstalls +
                ' freshCompiled=' + s.freshCompiled +
                ' saved=' + s.saved +
                ' tableLen=' + wasmTable.length +
                samples);
            logMergedProfile();
            if (Module.wasmJitProfileSplitTargets && Module.wasmJitProfileSplitTargets.size > 0) {
                console.log('[WASM JIT profile split] ' + JSON.stringify(splitStats()));
            }
        }
    }
    var eipKey = hex32(eip);
    var key = 'v5-' + eipKey + '-' + hex32(blockHash);
    // Per-process identity. DecodedOp pointers are per guest process, so
    // grouped reloc arrays, installed exports and group instances must all
    // be keyed by (key, memId): wineserver and wine map the same libraries
    // at the same addresses and would otherwise consume each other's
    // DecodedOps through a shared slot array (page fault in wineserver was
    // the symptom). memId is the KMemory pointer — if an exited process's
    // id is recycled, every array it left behind was already zeroed by
    // free_block during its opCache teardown, so reuse is safe.
    var procSuffix = '|' + (memId >>> 0).toString(16);
    var procKey = key + procSuffix;
    // Grouped reloc parity: merged group entries read their DecodedOp
    // pointers from a per-(entry, process) array (intra-group direct-call
    // sites pass the target's array as a patched constant argument). The
    // values are only known now — the runtime just compiled this block — so
    // copy them from the C++ array into the entry's array on every
    // grouped/installed hit. free_block zeroes the array on eviction, so a
    // refill is always needed.
    function refreshGroupRelocArray() {
        if (!relocBase || !relocCount || !Module.wasmJitGroupRelocArrays) return;
        var arr = Module.wasmJitGroupRelocArrays.get(procKey);
        if (!arr) return;
        if (arr.count !== relocCount) {
            // Manifest disagrees with the current build's codegen — leave the
            // array zeroed so the guards keep the entry on its slow paths.
            if (!refreshGroupRelocArray._warned) {
                refreshGroupRelocArray._warned = true;
                console.warn('[WASM JIT grouped reloc] relocCount mismatch key=' + key +
                    ' manifest=' + arr.count + ' runtime=' + relocCount + ' (leaving zeroed)');
            }
            return;
        }
        HEAPU32.set(HEAPU32.subarray(relocBase >>> 2, (relocBase >>> 2) + relocCount), arr.ptr >>> 2);
        getMergedProfile().relocCopies++;
    }
    var splitTarget = Module.wasmJitProfileSplitTargets && Module.wasmJitProfileSplitTargets.get(eipKey);
    var hasExactSplitKey = false;
    if (splitTarget) splitStats().hintedLookups++;
    function recordSample(arr, sample) {
        if (arr.length < 12 && arr.indexOf(sample) < 0) arr.push(sample);
    }
    function recordMiss() {
        Module.wasmJitStats.misses++;
        var knownHashes = Module.wasmJitCacheEipHashes && Module.wasmJitCacheEipHashes.get(eipKey);
        if (knownHashes) {
            Module.wasmJitStats.hashMisses++;
            recordSample(Module.wasmJitStats.hashMissSamples,
                eipKey + ':' + hex32(blockHash) + '!=' + knownHashes.join('|'));
        } else {
            Module.wasmJitStats.eipMisses++;
            recordSample(Module.wasmJitStats.eipMissSamples, eipKey);
        }
        logStats();
    }
    function isInstalledSlotLive(tableIndex) {
        try {
            return tableIndex !== undefined && wasmTable.get(tableIndex) !== null;
        } catch(e) {
            return false;
        }
    }
    function instantiateMergedGroup(groupPath) {
        if (!Module.wasmJitGroupInstances) Module.wasmJitGroupInstances = new Map();
        // Instances are per process: reloc groups carry patched-in array
        // addresses, which are per-process state.
        var instKey = groupPath + procSuffix;
        var inst = Module.wasmJitGroupInstances.get(instKey);
        if (inst) {
            getMergedProfile().groupInstanceReuse++;
            return inst;
        }
        // Shared precompiled module (groups without reloc work) ...
        var mod = Module.wasmJitGroupModules && Module.wasmJitGroupModules.get(groupPath);
        if (!mod) {
            // ... otherwise patch a per-process copy of the unpatched bytes:
            // allocate this process's zero-filled array for every entry, then
            // point each intra-group direct-call site at its target's array.
            var unpatched = Module.wasmJitGroupUnpatched && Module.wasmJitGroupUnpatched.get(groupPath);
            if (!unpatched) return false;
            if (!Module.wasmJitGroupRelocArrays) Module.wasmJitGroupRelocArrays = new Map();
            var data = new Uint8Array(unpatched.bytes);
            unpatched.entries.forEach(function(en) {
                var count = en.relocCount >>> 0;
                if (!count) return;
                var akey = en.key + procSuffix;
                if (Module.wasmJitGroupRelocArrays.has(akey)) return;
                var ptr = _malloc(count * 4);
                HEAPU32.fill(0, ptr >>> 2, (ptr >>> 2) + count);
                Module.wasmJitGroupRelocArrays.set(akey, { ptr: ptr, count: count });
            });
            unpatched.patches.forEach(function(patch) {
                var off = patch.offset >>> 0;
                if (data[off] !== 0x41) { // i32.const opcode self-check
                    throw new Error('bad direct-call patch offset for target=' + patch.targetKey);
                }
                var target = Module.wasmJitGroupRelocArrays.get(patch.targetKey + procSuffix);
                var v = target ? (target.ptr >>> 0) : 0;
                data[off + 1] = (v & 0x7f) | 0x80;
                data[off + 2] = ((v >>> 7) & 0x7f) | 0x80;
                data[off + 3] = ((v >>> 14) & 0x7f) | 0x80;
                data[off + 4] = ((v >>> 21) & 0x7f) | 0x80;
                data[off + 5] = ((v >>> 28) & 0x0f) | ((v & 0x80000000) ? 0x70 : 0);
            });
            mod = new WebAssembly.Module(data); // per-(group, process) compile
        }

        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }
        inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });
        Module.wasmJitGroupInstances.set(instKey, inst);
        getMergedProfile().groupInstantiates++;
        return inst;
    }
    function installMergedGroupEntry(key, groupEntry) {
        if (!Module.wasmJitInstalledCache) Module.wasmJitInstalledCache = new Map();
        var tableIndex = Module.wasmJitInstalledCache.get(procKey);
        if (isInstalledSlotLive(tableIndex)) {
            getMergedProfile().installedReuse++;
            return tableIndex;
        }
        if (tableIndex !== undefined) {
            Module.wasmJitInstalledCache.delete(procKey);
            if (Module.wasmJitInstalledByTableIndex) Module.wasmJitInstalledByTableIndex.delete(tableIndex);
        }

        getMergedProfile().groupInstallAttempts++;
        var inst = instantiateMergedGroup(groupEntry.groupPath);
        if (!inst) {
            getMergedProfile().groupInstallFail++;
            return undefined;
        }
        var exportName = groupEntry.exportName || key;
        var fn = inst.exports[exportName];
        if (!fn) {
            getMergedProfile().groupInstallFail++;
            return undefined;
        }
        tableIndex = addFunction(fn, 'vii');
        Module.wasmJitInstalledCache.set(procKey, tableIndex);
        if (!Module.wasmJitInstalledByTableIndex) Module.wasmJitInstalledByTableIndex = new Map();
        // The reverse map stores the composite key so free_block zeroes the
        // right process's array.
        Module.wasmJitInstalledByTableIndex.set(tableIndex, procKey);
        getMergedProfile().groupInstallSuccess++;
        return tableIndex;
    }
    var installed = Module.wasmJitInstalledCache && Module.wasmJitInstalledCache.get(procKey);
    if (installed !== undefined) {
        hasExactSplitKey = true;
        if (!isInstalledSlotLive(installed)) {
            Module.wasmJitInstalledCache.delete(procKey);
            if (Module.wasmJitInstalledByTableIndex) Module.wasmJitInstalledByTableIndex.delete(installed);
        } else {
            Module.wasmJitStats.hits++;
            Module.wasmJitStats.cachedInstalls++;
            if (splitTarget) recordSplitExactHit('installed');
            getMergedProfile().groupAvailable++;
            getMergedProfile().groupHits++;
            refreshGroupRelocArray();
            logStats();
            return installed;
        }
    }
    var groupEntry = Module.wasmJitGroupEntryMap && Module.wasmJitGroupEntryMap.get(key);
    if (groupEntry) {
        hasExactSplitKey = true;
        getMergedProfile().groupAvailable++;
        try {
            installed = installMergedGroupEntry(key, groupEntry);
            if (installed !== undefined) {
                Module.wasmJitStats.hits++;
                Module.wasmJitStats.cachedInstalls++;
                if (splitTarget) recordSplitExactHit('group');
                getMergedProfile().groupHits++;
                refreshGroupRelocArray();
                logStats();
                return installed;
            }
        } catch(e) {
            Module.wasmJitStats.stale++;
            console.warn('[WASM JIT cache] merged group install failed for key=' +
                key + ', recompiling');
            logStats();
            return -1;
        }
    }
    var cached = Module.wasmJitCache && Module.wasmJitCache.get(key);
    var mod = Module.wasmJitCompiledCache && Module.wasmJitCompiledCache.get(key);
    if (splitTarget && (cached || mod)) {
        hasExactSplitKey = true;
    }
    if (splitTarget && !hasExactSplitKey) {
        // A split hint names this block start but no split-shaped cache entry
        // exists yet: force a miss so the runtime compiles (and saves) the
        // split-shaped block instead of installing the old unsplit one.
        var sStats = splitStats();
        sStats.bypasses++;
        if (sStats.bypasses <= 8 || sStats.bypasses % 100 === 0) {
            console.log('[WASM JIT profile split] bypass cached block=' + eipKey +
                ' target=' + splitTarget + ' ' + JSON.stringify(sStats));
        }
        recordMiss();
        return -1;
    }
    if (!cached && !mod) {
        recordMiss();
        return -1;
    }
    try {
        if (!mod) {
            mod = new WebAssembly.Module(cached);
            if (!Module.wasmJitCompiledCache) Module.wasmJitCompiledCache = new Map();
            Module.wasmJitCompiledCache.set(key, mod);
        }
        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }
        var inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });
        var fn = inst.exports['execute'];
        if (!fn) {
            Module.wasmJitStats.stale++;
            if (Module.wasmJitCache) Module.wasmJitCache.delete(key);
            if (Module.wasmJitBlockMeta) Module.wasmJitBlockMeta.delete(key);
            if (Module.wasmJitCompiledCache) Module.wasmJitCompiledCache.delete(key);
            logStats();
            return -1;
        }
        var idx = addFunction(fn, 'vii');
        Module.wasmJitStats.hits++;
        Module.wasmJitStats.cachedInstalls++;
        if (splitTarget) recordSplitExactHit('flat');
        getMergedProfile().flatHits++;
        logStats();
        return idx;
    } catch(e) {
        Module.wasmJitStats.stale++;
        console.warn('[WASM JIT cache] stale entry for key=' + key + ', recompiling');
        if (Module.wasmJitCache) Module.wasmJitCache.delete(key);
        if (Module.wasmJitBlockMeta) Module.wasmJitBlockMeta.delete(key);
        if (Module.wasmJitCompiledCache) Module.wasmJitCompiledCache.delete(key);
        logStats();
        return -1;
    }
});

// Save a freshly compiled block to Module.wasmJitCache (the source of truth
// for the Save-JIT-Cache export) and async-write it to IndexedDB. The extra
// arguments capture the per-block exit metadata and runtime constants that
// saveJitModules() serializes as boxedwine-jit-manifest.json for the offline
// cache pipeline.
EM_JS(void, boxedwine_wasm_save_block,
      (U32 eip, U32 blockHash, const void* bytes, int size, U32 opCount, U32 emulatedLen,
       U32 next1Target, U32 next2Target, U32 jumpTarget, U32 next1Count, U32 next2Count, U32 jumpCount,
       U32 cpuBlockInstructionCountOffset, U32 cpuYieldOffset, U32 contextTimeRemainingPtr,
       U32 relocCount),
{
    function hex32(v) { return ('00000000' + ((v >>> 0).toString(16))).slice(-8); }
    var eipKey = hex32(eip);
    var hashKey = hex32(blockHash);
    var key = 'v5-' + eipKey + '-' + hashKey;
    var binary = new Uint8Array(HEAPU8.buffer, bytes, size).slice();
    if (!Module.wasmJitStats) {
        Module.wasmJitStats = { hits: 0, misses: 0, eipMisses: 0, hashMisses: 0, stale: 0, cachedInstalls: 0, freshCompiled: 0, saved: 0, eipMissSamples: [], hashMissSamples: [] };
    }
    Module.wasmJitStats.saved++;
    if (!Module.wasmJitCache) Module.wasmJitCache = new Map();
    if (!Module.wasmJitCacheEips) Module.wasmJitCacheEips = new Set();
    if (!Module.wasmJitCacheEipHashes) Module.wasmJitCacheEipHashes = new Map();
    if (Module.wasmJitProfileSplitTargetSources && Module.wasmJitProfileSplitTargetSources.has(eipKey) &&
            Module.wasmJitProfileSplitStats) {
        var splitStats = Module.wasmJitProfileSplitStats;
        splitStats.splitTargetSaves++;
        if (splitStats.splitTargetSaves <= 8 || splitStats.splitTargetSaves % 100 === 0) {
            console.log('[WASM JIT profile split] saved target=' + eipKey +
                ' sources=' + Module.wasmJitProfileSplitTargetSources.get(eipKey).join('|') +
                ' ' + JSON.stringify(splitStats));
        }
    }
    var hashes = Module.wasmJitCacheEipHashes.get(eipKey);
    if (!hashes) {
        hashes = [];
        Module.wasmJitCacheEipHashes.set(eipKey, hashes);
    }
    if (hashes.length < 4 && hashes.indexOf(hashKey) < 0) hashes.push(hashKey);
    Module.wasmJitCache.set(key, binary);
    Module.wasmJitCacheEips.add(eipKey);
    Module.wasmJitRuntimeConstants = {
        version: 1,
        cpu: {
            blockInstructionCountOffset: cpuBlockInstructionCountOffset >>> 0,
            yieldOffset: cpuYieldOffset >>> 0
        },
        scheduler: {
            contextTimeRemainingPtr: contextTimeRemainingPtr >>> 0
        }
    };
    if (!Module.wasmJitBlockMeta) Module.wasmJitBlockMeta = new Map();
    Module.wasmJitBlockMeta.set(key, {
        key: key,
        eip: eip >>> 0,
        blockHash: blockHash >>> 0,
        wasmBytes: size >>> 0,
        opCount: opCount >>> 0,
        emulatedLen: emulatedLen >>> 0,
        relocCount: relocCount >>> 0,
        exits: {
            next1: { count: next1Count >>> 0, firstTarget: next1Target >>> 0 },
            next2: { count: next2Count >>> 0, firstTarget: next2Target >>> 0 },
            jump:  { count: jumpCount >>> 0, firstTarget: jumpTarget >>> 0 }
        }
    });
    if (Module.wasmJitDb) {
        try {
            var tx = Module.wasmJitDb.transaction('blocks', 'readwrite');
            tx.objectStore('blocks').put(binary, key);
        } catch(e) { /* fire-and-forget; ignore */ }
    }
});

// Called from C++ (wasm_jit_import_from_file) for each imported zip entry.
// No block metadata is available for these, so they participate in lookups
// and IDB but not in the exported manifest.
EM_JS(void, wasm_jit_js_store_entry, (U32 eip, U32 blockHash, const void* bytes, int size),
{
    function hex32(v) { return ('00000000' + ((v >>> 0).toString(16))).slice(-8); }
    var eipKey = hex32(eip);
    var hashKey = hex32(blockHash);
    var key = 'v5-' + eipKey + '-' + hashKey;
    var binary = new Uint8Array(HEAPU8.buffer, bytes, size).slice();
    if (!Module.wasmJitCache) Module.wasmJitCache = new Map();
    if (!Module.wasmJitCacheEips) Module.wasmJitCacheEips = new Set();
    if (!Module.wasmJitCacheEipHashes) Module.wasmJitCacheEipHashes = new Map();
    var hashes = Module.wasmJitCacheEipHashes.get(eipKey);
    if (!hashes) {
        hashes = [];
        Module.wasmJitCacheEipHashes.set(eipKey, hashes);
    }
    if (hashes.length < 4 && hashes.indexOf(hashKey) < 0) hashes.push(hashKey);
    Module.wasmJitCache.set(key, binary);
    Module.wasmJitCacheEips.add(eipKey);
    if (Module.wasmJitDb) {
        try {
            var tx = Module.wasmJitDb.transaction('blocks', 'readwrite');
            tx.objectStore('blocks').put(binary, key);
        } catch(e) {}
    }
});

#endif // !BOXEDWINE_MULTI_THREADED && !__TEST

// Multi-threaded build: bypass addFunction() with Atomics-based slot allocation.
//
// The root cause of "table index is out of bounds" in pthreads builds:
// Emscripten's addFunction() uses per-worker JS variables (functionsInTableMap,
// freeTableIndexes) that are NOT shared across workers.  Even when the C++ side
// holds a std::recursive_mutex, multiple workers still run their own JS context
// concurrently and can all see the same wasmTable.length, all call table.grow(1),
// and all claim the same slot index.
//
// Fix: g_wasmTableNextSlot lives in WASM linear memory (SharedArrayBuffer in
// pthreads builds), so Atomics.add() gives each worker a unique, sequentially
// assigned slot index.  No mutex is needed; wasmTable.grow() is atomic at the
// engine level, so concurrent grows are safe (each grow is an indivisible
// operation and the while-loop retries until the table is large enough).
EM_JS(int, boxedwine_wasm_instantiate_mt,
      (const void* bytes, int size, const void** importFns, int importCount, int* nextSlotPtr),
{
    try {
        var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
        var mod = new WebAssembly.Module(wasmBytes);

        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }

        var inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });

        var fn = inst.exports['execute'];
        if (!fn) return -1;

        // Create a TypedArray view over the shared-memory counter.
        // nextSlotPtr points to g_wasmTableNextSlot (int32_t) in the main
        // WASM module's linear memory, which is SharedArrayBuffer in pthreads
        // builds, making Atomics operations valid and cross-worker safe.
        var nextSlotTA = new Int32Array(HEAPU8.buffer, nextSlotPtr, 1);

        // Lazy-initialize the counter to the current table size.
        // Atomics.compareExchange(ta, index, expected, replacement): if the
        // value at ta[index] == expected, replace it; returns the OLD value.
        // Only the first worker (seeing 0) succeeds; others see the already-set
        // value and do nothing — the add below always gives a unique slot.
        Atomics.compareExchange(nextSlotTA, 0, 0, wasmTable.length);

        // Atomically claim the next free slot index.
        // Atomics.add returns the value BEFORE the increment, so 'slot' is
        // the exclusive index this worker owns.
        var slot = Atomics.add(nextSlotTA, 0, 1);

        // Grow the table until it contains our claimed slot.
        // wasmTable.grow() is an atomic engine operation; concurrent grows
        // from different workers are safe — each call either succeeds or
        // throws if the engine limit is hit.  The while-loop re-checks
        // wasmTable.length after each grow in case another worker already
        // grew the table far enough.
        while (slot >= wasmTable.length) {
            try {
                wasmTable.grow(slot - wasmTable.length + 1);
            } catch(e) {
                if (slot < wasmTable.length) break; // another worker grew it
                console.error('[WASM JIT] wasmTable.grow failed:', e);
                return -1;
            }
        }

        wasmTable.set(slot, fn);

        var hasFn = false;
        try {
            hasFn = slot >= 0 && slot < wasmTable.length && !!wasmTable.get(slot);
        } catch(e) {
            hasFn = false;
        }

        if (!hasFn) {
            var pthread = (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD);
            var pthreadSelf = (typeof _pthread_self === 'function') ? _pthread_self() : 0;
            console.warn('[WASM JIT install anomaly] slot=' + slot +
                ' tableLen=' + wasmTable.length +
                ' pthread=' + pthread +
                ' pthreadSelf=0x' + (pthreadSelf >>> 0).toString(16));
        }

#ifndef __TEST
        if (!boxedwine_wasm_instantiate_mt._count) boxedwine_wasm_instantiate_mt._count = 0;
        boxedwine_wasm_instantiate_mt._count++;
        if (boxedwine_wasm_instantiate_mt._count % 1000 === 0) {
            var pthread = (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD);
            var pthreadSelf = (typeof _pthread_self === 'function') ? _pthread_self() : 0;
            console.log('[WASM JIT] compiled=' + boxedwine_wasm_instantiate_mt._count +
                ' latestSlot=' + slot +
                ' tableLen=' + wasmTable.length +
                ' pthreadSelf=0x' + (pthreadSelf >>> 0).toString(16));
        }
#endif

        return slot;
    } catch(e) {
        console.error('boxedwine_wasm_instantiate_mt failed:', e);
        return -1;
    }
});

// Instantiate (or reuse) a per-process patched merged-group module in THIS
// worker and publish one of its exports at a freshly claimed shared slot.
// Self-contained: the per-worker instance cache lives on globalThis (workers
// do not load boxedwine-shell.js), keyed by (groupIdx, memId) — the bytes are
// per-process patched, so the cache key must carry the process identity.
// Slot claiming is the same Atomics protocol as boxedwine_wasm_instantiate_mt.
EM_JS(int, boxedwine_wasm_instantiate_group_mt,
      (const void* bytes, int size, const void** importFns, int importCount,
       int* nextSlotPtr, const char* exportNamePtr, int groupIdx, U32 memId),
{
    try {
        if (!globalThis.bwJitMtGroupInstances) globalThis.bwJitMtGroupInstances = new Map();
        var instKey = groupIdx + '|' + (memId >>> 0).toString(16);
        var inst = globalThis.bwJitMtGroupInstances.get(instKey);
        if (!inst) {
            var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
            var mod = new WebAssembly.Module(wasmBytes);
            var helpers = {};
            var view = new Int32Array(HEAP32.buffer, importFns, importCount);
            for (var i = 0; i < importCount; i++) {
                helpers['fn_' + i] = wasmTable.get(view[i]);
            }
            inst = new WebAssembly.Instance(mod, {
                'env':     { 'memory': wasmMemory },
                'helpers': helpers
            });
            globalThis.bwJitMtGroupInstances.set(instKey, inst);
        }
        var exportName = UTF8ToString(exportNamePtr);
        var fn = inst.exports[exportName];
        if (!fn) {
            console.warn('[WASM JIT MT group] missing export ' + exportName + ' in group ' + groupIdx);
            return -1;
        }

        var nextSlotTA = new Int32Array(HEAPU8.buffer, nextSlotPtr, 1);
        Atomics.compareExchange(nextSlotTA, 0, 0, wasmTable.length);
        var slot = Atomics.add(nextSlotTA, 0, 1);
        while (slot >= wasmTable.length) {
            try {
                wasmTable.grow(slot - wasmTable.length + 1);
            } catch(e) {
                if (slot < wasmTable.length) break;
                console.error('[WASM JIT MT group] wasmTable.grow failed:', e);
                return -1;
            }
        }
        wasmTable.set(slot, fn);
        return slot;
    } catch(e) {
        console.error('boxedwine_wasm_instantiate_group_mt failed:', e);
        return -1;
    }
});

// Group counterpart of boxedwine_wasm_install_existing_mt: make a shared slot
// that some other worker claimed for a merged-group export callable in THIS
// worker, reusing (or creating) this worker's instance of the group.
EM_JS(int, boxedwine_wasm_install_existing_group_mt,
      (const void* bytes, int size, const void** importFns, int importCount,
       int tableIndex, const char* exportNamePtr, int groupIdx, U32 memId),
{
    try {
        if (tableIndex < 0) return 0;
        if (tableIndex < wasmTable.length && wasmTable.get(tableIndex)) return 1;
        if (!globalThis.bwJitMtGroupInstances) globalThis.bwJitMtGroupInstances = new Map();
        var instKey = groupIdx + '|' + (memId >>> 0).toString(16);
        var inst = globalThis.bwJitMtGroupInstances.get(instKey);
        if (!inst) {
            var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
            var mod = new WebAssembly.Module(wasmBytes);
            var helpers = {};
            var view = new Int32Array(HEAP32.buffer, importFns, importCount);
            for (var i = 0; i < importCount; i++) {
                helpers['fn_' + i] = wasmTable.get(view[i]);
            }
            inst = new WebAssembly.Instance(mod, {
                'env':     { 'memory': wasmMemory },
                'helpers': helpers
            });
            globalThis.bwJitMtGroupInstances.set(instKey, inst);
        }
        var fn = inst.exports[UTF8ToString(exportNamePtr)];
        if (!fn) return 0;
        while (tableIndex >= wasmTable.length) {
            wasmTable.grow(tableIndex - wasmTable.length + 1);
        }
        wasmTable.set(tableIndex, fn);
        return 1;
    } catch(e) {
        console.warn('[WASM JIT MT group lazy install failed] slot=' + tableIndex + ' error=' + e);
        return 0;
    }
});

// Multi-threaded lazy install: instantiate an already-compiled block into this
// worker's local wasmTable at the shared slot before call_indirect uses it.
EM_JS(int, boxedwine_wasm_install_existing_mt,
      (const void* bytes, int size, const void** importFns, int importCount, int tableIndex),
{
    try {
        if (tableIndex < 0) return 0;
        if (tableIndex < wasmTable.length && wasmTable.get(tableIndex)) return 1;

        var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
        var mod = new WebAssembly.Module(wasmBytes);

        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }

        var inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });

        var fn = inst.exports['execute'];
        if (!fn) return 0;

        while (tableIndex >= wasmTable.length) {
            wasmTable.grow(tableIndex - wasmTable.length + 1);
        }
        wasmTable.set(tableIndex, fn);

        return 1;
    } catch(e) {
        console.warn('[WASM JIT lazy install failed] slot=' + tableIndex +
            ' tableLen=' + wasmTable.length +
            ' error=' + e);
        return 0;
    }
});

#ifdef BOXEDWINE_MULTI_THREADED

// ---------------------------------------------------------------------------
// JIT module persistence (multi-threaded path).
//
// Worker threads compile blocks, so the JS-side Module.wasmJitCache (a main-
// thread Map) cannot be the working cache: an EM_JS body executes in the JS
// context of the *calling* thread, and pthread workers do not load
// boxedwine-shell.js. (That is exactly the bug the first MT attempt hit —
// worker EM_JS referenced a shell-defined boxedwineWasmJitCacheKey() and threw
// ReferenceError.) Instead the C++ map g_wasmCacheByKey, which lives in shared
// linear memory, is the working cache:
//   - load:   Module.onRuntimeInitialized (main thread, boxedwine-shell.js)
//             calls the exported wasm_jit_mt_register() for each server-zip
//             entry before main() runs.
//   - save:   commitJIT() inserts freshly compiled blocks under the mutex.
//   - export: the Save-JIT-Cache button calls the exported
//             wasm_jit_mt_prepare_export(), which mirrors the C++ map into
//             Module.wasmJitCache on the main thread; saveJitModules() then
//             zips that Map exactly as in the single-threaded build.
// ---------------------------------------------------------------------------

// Mirror one block from the C++ heap into Module.wasmJitCache. Only ever
// called on the main thread (from wasm_jit_mt_prepare_export), but the cache
// key is still computed inline so the body never depends on shell-page JS.
EM_JS(void, wasm_jit_mt_populate_js_cache_entry,
      (U32 eip, U32 blockHash, const void* bytes, int size),
{
    var key = 'v5-' + ('00000000' + ((eip >>> 0).toString(16))).slice(-8) +
              '-'   + ('00000000' + ((blockHash >>> 0).toString(16))).slice(-8);
    var binary = new Uint8Array(HEAPU8.buffer, bytes, size).slice();
    if (!Module.wasmJitCache) Module.wasmJitCache = new Map();
    Module.wasmJitCache.set(key, binary);
});

// Exported to JS (see EXPORTED_FUNCTIONS in project/emscripten/makefile).
extern "C" void wasm_jit_mt_register(uint32_t eip, uint32_t blockHash, const void* bytes, int size) {
    if (size <= 0 || !bytes) return;
    // Registering imported blocks implies a replay session — switch the JIT
    // into relocatable codegen before any worker compiles a block.
    wasm_jit_set_persistence_active();
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    auto& v = g_wasmCacheByKey[wasmJitCacheKey(eip, blockHash)];
    v.assign(static_cast<const U8*>(bytes),
             static_cast<const U8*>(bytes) + size);
}

// Piped (grouped) registration — called from the main thread in
// onRuntimeInitialized, one _register_group per merged module followed by
// one _register_group_entry / _register_group_patch call per manifest row.
// Exported to JS (see EXPORTED_FUNCTIONS in project/emscripten/makefile).
extern "C" int wasm_jit_mt_register_group(const void* bytes, int size) {
    if (size <= 0 || !bytes) return -1;
    wasm_jit_set_persistence_active();
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    g_wasmMtGroups.emplace_back();
    g_wasmMtGroups.back().bytes.assign(static_cast<const U8*>(bytes),
                                       static_cast<const U8*>(bytes) + size);
    return (int)g_wasmMtGroups.size() - 1;
}

extern "C" void wasm_jit_mt_register_group_entry(int groupIdx, uint32_t eip, uint32_t blockHash,
                                                 const char* exportName, uint32_t relocCount) {
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    if (groupIdx < 0 || (size_t)groupIdx >= g_wasmMtGroups.size() || !exportName) return;
    WasmJitMtGroup& group = g_wasmMtGroups[groupIdx];
    uint64_t key = wasmJitCacheKey(eip, blockHash);
    // First registration wins, matching the flat map's behavior for
    // duplicate keys across groups.
    if (!g_wasmMtGroupByKey.emplace(key, std::make_pair((U32)groupIdx, (U32)group.entries.size())).second) {
        return;
    }
    group.entries.push_back({ key, std::string(exportName), relocCount });
}

extern "C" void wasm_jit_mt_register_group_patch(int groupIdx, uint32_t offset,
                                                 uint32_t targetEip, uint32_t targetHash) {
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    if (groupIdx < 0 || (size_t)groupIdx >= g_wasmMtGroups.size()) return;
    WasmJitMtGroup& group = g_wasmMtGroups[groupIdx];
    if (offset + 6 > group.bytes.size() || group.bytes[offset] != 0x41) {
        klog_fmt("[WASM JIT MT group] bad direct-call patch offset %u for group %d (dropped)", offset, groupIdx);
        return;
    }
    group.patches.push_back({ offset, wasmJitCacheKey(targetEip, targetHash) });
}

// Mirror one block's exit metadata into Module.wasmJitBlockMeta and (re)set
// Module.wasmJitRuntimeConstants — the same shape boxedwine_wasm_save_block
// produces on ST builds, so saveJitModules() emits an identical
// boxedwine-jit-manifest.json for MT-recorded zips.
EM_JS(void, wasm_jit_mt_populate_js_meta_entry,
      (U32 eip, U32 blockHash, U32 wasmSize, U32 opCount, U32 emulatedLen,
       U32 next1Target, U32 next2Target, U32 jumpTarget,
       U32 next1Count, U32 next2Count, U32 jumpCount,
       U32 cpuBlockInstructionCountOffset, U32 cpuYieldOffset, U32 contextTimeRemainingPtr,
       U32 relocCount),
{
    var key = 'v5-' + ('00000000' + ((eip >>> 0).toString(16))).slice(-8) +
              '-'   + ('00000000' + ((blockHash >>> 0).toString(16))).slice(-8);
    Module.wasmJitRuntimeConstants = {
        version: 1,
        cpu: {
            blockInstructionCountOffset: cpuBlockInstructionCountOffset >>> 0,
            yieldOffset: cpuYieldOffset >>> 0
        },
        scheduler: {
            contextTimeRemainingPtr: contextTimeRemainingPtr >>> 0
        }
    };
    if (!Module.wasmJitBlockMeta) Module.wasmJitBlockMeta = new Map();
    Module.wasmJitBlockMeta.set(key, {
        key: key,
        eip: eip >>> 0,
        blockHash: blockHash >>> 0,
        wasmBytes: wasmSize >>> 0,
        opCount: opCount >>> 0,
        emulatedLen: emulatedLen >>> 0,
        relocCount: relocCount >>> 0,
        exits: {
            next1: { count: next1Count >>> 0, firstTarget: next1Target >>> 0 },
            next2: { count: next2Count >>> 0, firstTarget: next2Target >>> 0 },
            jump:  { count: jumpCount >>> 0, firstTarget: jumpTarget >>> 0 }
        }
    });
});

// Mirror one interior-transition profile snapshot line into
// Module.wasmJitInteriorProfileLines so the exported zip carries the
// boxedwine-jit-profile.txt sidecar.
EM_JS(void, wasm_jit_mt_populate_js_profile_line, (const char* line),
{
    if (!Module.wasmJitInteriorProfileLines) Module.wasmJitInteriorProfileLines = [];
    Module.wasmJitInteriorProfileLines.push(UTF8ToString(line));
    if (Module.wasmJitInteriorProfileLines.length > 256) {
        Module.wasmJitInteriorProfileLines.shift();
    }
});

// Exported to JS; called by saveJitModules() right before building the zip.
extern "C" void wasm_jit_mt_prepare_export() {
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    for (auto& kv : g_wasmCacheByKey) {
        wasm_jit_mt_populate_js_cache_entry(
            (U32)(kv.first >> 32), (U32)kv.first,
            kv.second.data(), (int)kv.second.size());
        auto metaIt = g_wasmCacheMetaByKey.find(kv.first);
        if (metaIt != g_wasmCacheMetaByKey.end()) {
            const WasmJitMtBlockMeta& meta = metaIt->second;
            wasm_jit_mt_populate_js_meta_entry(
                (U32)(kv.first >> 32), (U32)kv.first,
                (U32)kv.second.size(),
                meta.opCount,
                meta.emulatedLen,
                meta.next1Target,
                meta.next2Target,
                meta.jumpTarget,
                meta.next1Count,
                meta.next2Count,
                meta.jumpCount,
                (U32)offsetof(CPU, blockInstructionCount),
                (U32)offsetof(CPU, yield),
                // No slice-budget global in MT (threads check cpu->yield);
                // 0 marks the manifest as flat-pipeline-only.
                0,
                meta.relocCount);
        }
    }
    for (const std::string& line : g_wasmInteriorProfileLines) {
        wasm_jit_mt_populate_js_profile_line(line.c_str());
    }
}

#endif // BOXEDWINE_MULTI_THREADED

// Call a JIT-compiled WASM block via a direct WASM indirect call (no JS frame).
//
// The previous implementation used EM_JS (a JavaScript wrapper around
// wasmTable.get(tableIndex)(cpuPtr)).  That created a JavaScript stack frame
// between the C++ caller and the JIT WASM module.  When C++ code called from
// inside the JIT block throws an exception (e.g. seg_mapper throws 2 after
// calling runSignal), the exception propagates out of the JIT WASM module as a
// WebAssembly.Exception object in JavaScript.  In Emscripten's wasm-exceptions
// model the exception does not reliably re-enter the main WASM module as a
// catchable C++ exception after crossing the JS frame — causing "Uncaught
// [object WebAssembly.Exception] Error: int" to reach the browser even though
// our catch(...) and runThreadSlice's catch(...) should have intercepted it.
//
// Casting the table index to a C++ function pointer generates a WASM
// call_indirect instruction instead.  The call remains entirely within the WASM
// world: no JS frame is created, and C++ exceptions propagate to the scheduler's
// catch(...) exactly as they do for the x32/arm JIT backends.
typedef void (*WasmBlockFn)(int, int);
static inline void boxedwine_wasm_call_block(int tableIndex, int cpuPtr, int relocBase) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileCallBlock();
    U64 profileStartNs = profileSample ? wasmJitProfileNowNs() : 0;
#endif
    static bool fired = false;
    if (!fired) {
        fired = true;
        klog_fmt("[WASM JIT] first JIT block executed, tableIdx: %d", tableIndex);
    }
#ifdef BOXEDWINE_MULTI_THREADED
    // Guard against stale / garbage pfnJitCode values that could produce an
    // engine-level "table index is out of bounds" trap.
    //
    // g_wasmTableNextSlot is initialized to wasmTable.length on the first JIT
    // compilation and then incremented for each subsequent block; every valid
    // allocated slot S satisfies S < g_wasmTableNextSlot.  A tableIndex that
    // falls outside (0, g_wasmTableNextSlot) was never issued by our allocator
    // (could be a torn read, a stale DecodedOp value from before JIT, etc.).
    // Log it and bail out so the normal CPU interpreter picks up execution on
    // the next dispatch rather than crashing the worker thread.
    if (tableIndex <= 0 || tableIndex >= g_wasmTableNextSlot) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        wasmJitProfileSlotMiss();
#endif
        klog_fmt("[WASM JIT] WARNING: call_block skipped bad tableIndex=%d "
                 "(nextSlot=%d)", tableIndex, (int)g_wasmTableNextSlot);
        return;
    }
#endif
    // Emscripten WASM32: the value returned by addFunction() (and stored in
    // pfnJitCode as void*) IS the indirect-call table index — i.e. the "function
    // pointer" in Emscripten's ABI.  Clang C++ mode rejects a direct
    // int → function-pointer reinterpret_cast, so we use std::bit_cast<> which
    // is the sanctioned C++20 bit-level reinterpretation primitive.
    // sizeof(WasmBlockFn) == sizeof(unsigned) == 4 in WASM32; the static_assert
    // below enforces this so the bit-cast is always safe.
    static_assert(sizeof(WasmBlockFn) == sizeof(unsigned int),
                  "WASM32: function-pointer and unsigned must be the same width");
    WasmBlockFn fn = std::bit_cast<WasmBlockFn>((unsigned int)tableIndex);
    fn(cpuPtr, relocBase);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    if (profileSample) {
        wasmJitProfileAddElapsedScaled(g_wasmJitProfileCallBlockUs, profileStartNs);
    }
#endif
}

// Single-threaded: return the slot to Emscripten's free-list. If the slot
// held an installed merged-group export, drop the installed-cache entries so
// a later lookup re-installs instead of returning a freed table index.
EM_JS(void, boxedwine_wasm_free_block, (int tableIndex),
{
    if (Module.wasmJitInstalledByTableIndex) {
        var key = Module.wasmJitInstalledByTableIndex.get(tableIndex);
        if (key !== undefined) {
            Module.wasmJitInstalledByTableIndex.delete(tableIndex);
            if (Module.wasmJitInstalledCache) Module.wasmJitInstalledCache.delete(key);
            // Grouped reloc parity: the entry's DecodedOp pointers die with
            // the block. A direct tail call can still reach the function
            // after eviction (group instances outlive entries), so zero the
            // array — the guards then fall back to the slow path until the
            // next lookup refills it.
            if (Module.wasmJitGroupRelocArrays) {
                var arr = Module.wasmJitGroupRelocArrays.get(key);
                if (arr && arr.count) {
                    HEAPU32.fill(0, arr.ptr >>> 2, (arr.ptr >>> 2) + arr.count);
                }
            }
        }
    }
    removeFunction(tableIndex);
});

// Multi-threaded: the slot counter is monotonically increasing, so slots are
// never reused.  We intentionally leave the table entry non-null: if another
// thread read pfnJitCode just before removeCodeBlock cleared it, that thread
// is about to call call_indirect(slot).  Null-setting the slot here would
// cause a "RuntimeError: null function" in that thread.  Since the slot is
// never reassigned (monotonic counter), leaving the old function reference
// in place is safe — at worst, a racing thread runs one extra (stale) JIT
// block call, which is no worse than the native JIT backends' behaviour under
// eviction races.
//
// Browser WASM/module limits still matter here. The single-threaded path can
// call removeFunction(), but the pthread path cannot safely reclaim/reuse a
// published slot without a stronger cross-worker quiescence protocol. The saved
// WASM bytes in g_wasmBlockBinaries follow this same lifetime so workers can
// lazily install any still-observable slot.
EM_JS(void, boxedwine_wasm_free_block_mt, (int tableIndex),
{
    // Intentionally empty: see comment above.
    void(tableIndex);
});

// JS-side check local to the executing worker: returns 1 if wasmTable[tableIndex]
// is non-null, 0 otherwise. In pthread builds the shared DecodedOp stores one
// table index, but the worker that reaches wasmStartJITOp may not have that
// slot populated in its own JS/WASM table yet.
EM_JS(int, boxedwine_wasm_slot_check, (int tableIndex, int cpuPtr, int opPtr),
{
    try {
        var hasFn = tableIndex >= 0 && tableIndex < wasmTable.length && !!wasmTable.get(tableIndex);
        if (hasFn) return 1;
        return 0;
    } catch(e) {
        return 0;
    }
});

#ifdef BOXEDWINE_MULTI_THREADED
static bool lazyInstallWasmJitBlockForWorker(int tableIndex);
static bool wasmJitSlotReadyForWorker(int tableIndex, CPU* cpu, DecodedOp* op);

static void disableWasmJitBlockAfterSlotMiss(DecodedOp* op) {
    DecodedOp* blockOp = op->blockStart ? op->blockStart : op;
    U32 blockOpCount = blockOp->blockOpCount ? blockOp->blockOpCount : 1;
    DecodedOp* cur = blockOp;
    for (U32 i = 0; i < blockOpCount && cur; i++) {
        cur->pfnJitCode = nullptr;
        cur->flags &= ~OP_FLAG_JIT;
        cur->flags |= OP_FLAG_NO_JIT;
        cur->flags2 &= ~OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        cur->runCount = JIT_RUN_COUNT + 1;
        cur->blockStart = nullptr;
        cur->blockOpCount = 0;
        cur->blockLen = 0;
        cur->pfn = NormalCPU::getFunctionForOp(cur);
        cur = cur->next;
    }
}
#endif

// ---------------------------------------------------------------------------
// The OpCallback installed as process->startJITOp for WASM-compiled blocks.
// DecodedOp::pfnJitCode stores the wasmTable index (cast to void*).
// ---------------------------------------------------------------------------
void OPCALL wasmStartJITOp(CPU* cpu, DecodedOp* op) {
#ifdef BOXEDWINE_MULTI_THREADED
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileStartEntry();
    U64 profileStartNs = profileSample ? wasmJitProfileNowNs() : 0;
    U64 startPreCallNs = profileStartNs;
#endif
    if (op->pfnJitCode) {
        // Guard against cross-worker table visibility. Readiness cannot be
        // cached on DecodedOp because DecodedOp is shared, while table slot
        // visibility is local to the worker that performs call_indirect; the
        // per-worker cache makes the JS slot_check a one-time cost per slot.
        int tableIndex = (int)(uintptr_t)op->pfnJitCode;
        if (!wasmJitSlotReadyForWorker(tableIndex, cpu, op)) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
            wasmJitProfileSlotMiss();
#endif
            disableWasmJitBlockAfterSlotMiss(op);
            NormalCPU::getFunctionForOp(op)(cpu, op);
            return;
        }
        U8 wasmJitSetupFlags = op->flags2 & OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        if (wasmJitSetupFlags) {
            KMemoryData* memoryData = nullptr;
            if (wasmMemoryPageArraysNeedRefresh(cpu, &memoryData)) {
                wasmPrepareBlockEnter(cpu, memoryData);
            }
        }
        WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPreCallUs, startPreCallNs); })
        boxedwine_wasm_call_block((int)(uintptr_t)op->pfnJitCode, (int)(uintptr_t)cpu, (int)wasmJitRelocBaseForTable((int)(uintptr_t)op->pfnJitCode));
        WASM_JIT_PROFILE_ONLY(if (profileSample) { U64 startPostCallNs = wasmJitProfileNowNs(); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPostCallUs, startPostCallNs); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartUs, profileStartNs); })
        // nextOp is updated by the WASM block itself (via helper call).
    }
#else
    U32 chainedBlocks = 0;
    U32 chainedInstructionCount = 0;
    bool memoryArraysChecked = false;
    WASM_JIT_PROFILE_ONLY(bool loopProfileSample = wasmJitProfileLoopSample();)
    while (op && op->pfnJitCode && chainedBlocks < WASM_JIT_CHAIN_BLOCK_LIMIT && !cpu->yield) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        bool profileSample = wasmJitProfileStartEntry();
        U64 profileStartNs = profileSample ? wasmJitProfileNowNs() : 0;
        U64 startPreCallNs = profileStartNs;
#endif
#ifdef BOXEDWINE_MULTI_THREADED
        // Guard against cross-worker table visibility. Readiness cannot be
        // cached on DecodedOp because DecodedOp is shared, while table slot
        // visibility is local to the worker that performs call_indirect.
        int tableIndex = (int)(uintptr_t)op->pfnJitCode;
        if (!boxedwine_wasm_slot_check(tableIndex, (int)(uintptr_t)cpu, (int)(uintptr_t)op)) {
            if (!lazyInstallWasmJitBlockForWorker(tableIndex) ||
                !boxedwine_wasm_slot_check(tableIndex, (int)(uintptr_t)cpu, (int)(uintptr_t)op)) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
                wasmJitProfileSlotMiss();
#endif
                disableWasmJitBlockAfterSlotMiss(op);
                NormalCPU::getFunctionForOp(op)(cpu, op);
                return;
            }
        }
#endif
        DecodedOp* executedOp = op;
        U8 wasmJitSetupFlags = op->flags2 & OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        if (wasmJitSetupFlags && !memoryArraysChecked) {
            KMemoryData* memoryData = nullptr;
            bool refreshed = wasmMemoryPageArraysNeedRefresh(cpu, &memoryData);
            if (refreshed) {
                wasmPrepareBlockEnter(cpu, memoryData);
            }
            WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileMemArrayCheck(refreshed, WASM_JIT_PROFILE_TIMING_SAMPLE); })
            memoryArraysChecked = true;
        }
        WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPreCallUs, startPreCallNs); })
        boxedwine_wasm_call_block((int)(uintptr_t)op->pfnJitCode, (int)(uintptr_t)cpu, (int)wasmJitRelocBaseForTable((int)(uintptr_t)op->pfnJitCode));
        WASM_JIT_PROFILE_ONLY(if (profileSample) { U64 startPostCallNs = wasmJitProfileNowNs(); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPostCallUs, startPostCallNs); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartUs, profileStartNs); })
        WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileChainTarget(cpu, cpu->nextOp, WASM_JIT_PROFILE_TIMING_SAMPLE); })
        // nextOp is updated by the WASM block itself (via helper call).
        if (chainedBlocks) {
            chainedInstructionCount += executedOp->blockOpCount;
        }
        chainedBlocks++;
        op = cpu->nextOp;
        if (!wasmJitCanChainTo(cpu, op)) {
            break;
        }
    }
#if !defined(BOXEDWINE_MULTI_THREADED)
    cpu->blockInstructionCount += chainedInstructionCount;
    WASM_JIT_PROFILE_ONLY(if (loopProfileSample) { bool stoppedAtLimit = chainedBlocks == WASM_JIT_CHAIN_BLOCK_LIMIT && wasmJitCanChainTo(cpu, op); wasmJitProfileLoop(chainedBlocks, stoppedAtLimit, WASM_JIT_PROFILE_TIMING_SAMPLE); if (!stoppedAtLimit) { wasmJitProfileChainStop(cpu, op, WASM_JIT_PROFILE_TIMING_SAMPLE); } })
#endif
#endif
}

// ---------------------------------------------------------------------------
// C++ helpers imported by generated WASM modules
// ---------------------------------------------------------------------------

// Memory helpers read their address from cpu->memHelperAddr and their
// value to/from cpu->memHelperValue. Using dedicated scratch fields
// (rather than cpu->src.u32 / cpu->dst.u32) keeps lazy-flag state intact
// across an RMW op's callback.
static void wasmHelper_readMem32(CPU* cpu) {
    WASM_JIT_HELPER_STAT(ReadMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memHelperValue = cpu->memory->readd(cpu->memHelperAddr);
}
static void wasmHelper_writeMem32(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
}
static void wasmHelper_readMem8(CPU* cpu) {
    WASM_JIT_HELPER_STAT(ReadMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memHelperValue = cpu->memory->readb(cpu->memHelperAddr);
}
static void wasmHelper_writeMem8(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
}
static void wasmHelper_readMem16(CPU* cpu) {
    WASM_JIT_HELPER_STAT(ReadMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memHelperValue = cpu->memory->readw(cpu->memHelperAddr);
}
static void wasmHelper_writeMem16(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMem);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writew(cpu->memHelperAddr, (U16)cpu->memHelperValue);
}

// Same as the regular write helpers but additionally check whether the
// active JIT block's pfnJitCode got cleared by removeCodeBlock — meaning
// the write landed on the bytes we're currently executing. Used by
// `write(address, src)` in the inline JIT path; the JIT then emits a
// post-call check that exits the block on bailout. (We don't put this
// check on every write because push/pop go through `write` in
// `push16`/`push32` which is wrapped in IfSmallStack — adding the if/end
// check inside that nested control flow broke Xchg tests.)
static inline void checkActiveBlockAfterWrite(CPU* cpu) {
    DecodedOp* active = cpu->wasmJitActiveBlock;
    if (active && active->pfnJitCode == nullptr) {
        cpu->wasmJitBailout = 1;
    }
}
static void wasmHelper_writeMem32_check(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMemCheck);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem16_check(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMemCheck);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writew(cpu->memHelperAddr, (U16)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem8_check(CPU* cpu) {
    WASM_JIT_HELPER_STAT(WriteMemCheck);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}

// Update cpu->nextOp by decoding the block at cpu->eip.u32
static void wasmHelper_fetchNextOp(CPU* cpu) {
    WASM_JIT_HELPER_STAT(FetchNext);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    static thread_local U32 profileSampleCounter = 0;
    bool profileSample = wasmJitProfileSample(profileSampleCounter);
    if (profileSample) {
        g_wasmJitProfileFetchNextCalls.fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    WasmJitProfileTimer profileTimer(g_wasmJitProfileFetchNextUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    if (!cpu->thread->terminating) {
        cpu->nextOp = cpu->getNextOp();
        if (wasmJitRecordActive()) {
            wasmJitRecordFetchNextTransition(cpu, cpu->nextOp);
        }
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        if (profileSample) {
            wasmJitProfileFetchNextTarget(cpu, cpu->nextOp, WASM_JIT_PROFILE_TIMING_SAMPLE);
        }
#endif
    }
}

// Sync CPU lazy flags — flags are maintained in cpu->flags directly by WASM code
static void wasmHelper_syncFlags(CPU* cpu) {}

// Kept in the helper table for module import-index stability. Block-entry
// setup is now done in wasmStartJITOp before calling the generated block,
// which avoids one generated WASM -> imported C++ helper call per block.
static void wasmHelper_blockEnter(CPU* cpu) {
    WASM_JIT_HELPER_STAT(BlockEnter);
    (void)cpu;
}

// Fall back to the normal CPU interpreter for one instruction (used for
// operations the WASM JIT doesn't inline, e.g. FPU/SSE/complex shifts).
void jitRunSingleOp(CPU* cpu);   // defined in jitCodeGen.cpp
static void wasmHelper_emulateSingleOp(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Emulate);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperEmulate();
    if (profileSample && cpu->tmpReg < InstructionCount) {
        g_wasmJitProfileEmulateByInst[cpu->tmpReg].fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
    if (profileSample && cpu->tmpReg == Movsd) {
        wasmJitProfileMovsd(cpu->memHelperValue, (cpu->flags & DF) != 0, WASM_JIT_PROFILE_TIMING_SAMPLE);
    }
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperEmulateUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    jitRunSingleOp(cpu);
    // Interpreter-backed instructions can write code too (notably REP
    // MOVS). Mark an invalidated active block so a structured JIT backedge
    // returns to the dispatcher instead of re-entering stale WASM.
    checkActiveBlockAfterWrite(cpu);
}

// Compute CF using the C++ lazy-flag machinery and stash the result in
// cpu->tmpReg. Used by the WASM backend's getCF() override since it can't
// do a nakedCall to the per-flag-type computation functions.
static void wasmHelper_computeCF(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Flags);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperFlags(0);
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->tmpReg = cpu->getCF() ? 1 : 0;
}
static void wasmHelper_computeZF(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Flags);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperFlags(1);
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    cpu->tmpReg = cpu->getZF() ? 1 : 0;
}

// Materialize all lazy flags into cpu->flags; reset lazyFlagType to FLAGS_NONE.
void common_fillFlags(CPU* cpu);
static void wasmHelper_fillFlags(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Flags);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    bool profileSample = wasmJitProfileHelperFlags(2);
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);
#endif
    common_fillFlags(cpu);
}

// One helper per JitConditional: evaluates the condition against cpu state
// (honoring lazy flags via cpu->getXF()) and stashes 0/1 in cpu->tmpReg.
// Separate helpers avoid any need for an input parameter — using
// cpu->src.u32 would clobber live lazy-flag state.
#define WASM_COND_HELPER(NAME, EXPR)                                           \
    static void wasmHelper_cond_##NAME(CPU* cpu) {                             \
        WASM_JIT_HELPER_STAT(Cond);                                            \
        WASM_JIT_PROFILE_ONLY(bool profileSample = wasmJitProfileHelperCond((U32)JitConditional::NAME, (U32)cpu->lazyFlagType);) \
        WASM_JIT_PROFILE_ONLY(WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperCondUs, profileSample, WASM_JIT_PROFILE_TIMING_SAMPLE);) \
        cpu->tmpReg = (EXPR) ? 1 : 0;                                          \
    }
WASM_COND_HELPER(O,   cpu->getOF())
WASM_COND_HELPER(NO,  !cpu->getOF())
WASM_COND_HELPER(B,   cpu->getCF())
WASM_COND_HELPER(NB,  !cpu->getCF())
WASM_COND_HELPER(Z,   cpu->getZF())
WASM_COND_HELPER(NZ,  !cpu->getZF())
WASM_COND_HELPER(BE,  cpu->getCF() || cpu->getZF())
WASM_COND_HELPER(NBE, !cpu->getCF() && !cpu->getZF())
WASM_COND_HELPER(S,   cpu->getSF())
WASM_COND_HELPER(NS,  !cpu->getSF())
WASM_COND_HELPER(P,   cpu->getPF())
WASM_COND_HELPER(NP,  !cpu->getPF())
WASM_COND_HELPER(L,   (cpu->getSF() ? 1 : 0) != (cpu->getOF() ? 1 : 0))
WASM_COND_HELPER(NL,  (cpu->getSF() ? 1 : 0) == (cpu->getOF() ? 1 : 0))
WASM_COND_HELPER(LE,  cpu->getZF() || ((cpu->getSF() ? 1 : 0) != (cpu->getOF() ? 1 : 0)))
WASM_COND_HELPER(NLE, !cpu->getZF() && ((cpu->getSF() ? 1 : 0) == (cpu->getOF() ? 1 : 0)))
#undef WASM_COND_HELPER

#ifdef BOXEDWINE_WASM_JIT_PROFILE
static void wasmHelper_profileBlockExit(CPU* cpu) {
    (void)cpu;
    wasmJitProfileBlockExit(WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileExitNext1(CPU* cpu) {
    (void)cpu;
    wasmJitProfileBlockExit(WASM_JIT_PROFILE_TIMING_SAMPLE);
    wasmJitProfileExitNext1(WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileExitNext2(CPU* cpu) {
    (void)cpu;
    wasmJitProfileBlockExit(WASM_JIT_PROFILE_TIMING_SAMPLE);
    wasmJitProfileExitNext2(WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileExitJump(CPU* cpu) {
    (void)cpu;
    wasmJitProfileBlockExit(WASM_JIT_PROFILE_TIMING_SAMPLE);
    wasmJitProfileExitJump(WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileExitGeneric(CPU* cpu) {
    wasmJitProfileBlockExit(WASM_JIT_PROFILE_TIMING_SAMPLE);
    wasmJitProfileExitGeneric(cpu->tmpReg, WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileInlineCond(CPU* cpu) {
    (void)cpu;
    wasmJitProfileInlineCond(WASM_JIT_PROFILE_TIMING_SAMPLE);
}
static void wasmHelper_profileRmw(CPU* cpu) {
    if (cpu->tmpReg < InstructionCount) {
        g_wasmJitProfileRmwByInst[cpu->tmpReg].fetch_add(WASM_JIT_PROFILE_TIMING_SAMPLE, std::memory_order_relaxed);
    }
}
#endif

static void wasmHelper_cacheFloat(CPU* cpu) {
    WASM_JIT_HELPER_STAT(X87);
    WASM_JIT_HELPER_DETAIL(CacheFloat);
    U32 regIndex = cpu->memHelperValue;
    cpu->fpu.getF64(cpu->fpu.STV(regIndex));
    cpu->fpu.getF64(cpu->fpu.STV(0));
}

static void wasmHelper_movsdXmmE64(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Sse);
    WASM_JIT_HELPER_DETAIL(MovsdXmmE64);
    common_movsdXmmE64(cpu, cpu->memHelperValue, cpu->memHelperAddr);
}

static void wasmHelper_movsdE64Xmm(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Sse);
    WASM_JIT_HELPER_DETAIL(MovsdE64Xmm);
    common_movsdE64Xmm(cpu, cpu->memHelperValue, cpu->memHelperAddr);
}

static void wasmHelper_movsd32r(CPU* cpu) {
    WASM_JIT_HELPER_STAT(Movsd32r);
    WASM_JIT_HELPER_DETAIL(Movsd32r);
    movsd32r(cpu, cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}

// ---------------------------------------------------------------------------
// Helper table: array of C++ function pointers exported to WASM modules.
// Order must match WasmHelperIdx below.
// ---------------------------------------------------------------------------
static const void* g_wasmHelperTable[] = {
    (void*)wasmHelper_readMem8,
    (void*)wasmHelper_writeMem8,
    (void*)wasmHelper_readMem16,
    (void*)wasmHelper_writeMem16,
    (void*)wasmHelper_readMem32,
    (void*)wasmHelper_writeMem32,
    (void*)wasmHelper_fetchNextOp,
    (void*)wasmHelper_syncFlags,
    (void*)wasmHelper_emulateSingleOp,
    (void*)wasmHelper_computeCF,
    (void*)wasmHelper_computeZF,
    (void*)wasmHelper_fillFlags,
    // 16 condition helpers — index order MUST match JitConditional enum.
    (void*)wasmHelper_cond_O,
    (void*)wasmHelper_cond_NO,
    (void*)wasmHelper_cond_B,
    (void*)wasmHelper_cond_NB,
    (void*)wasmHelper_cond_Z,
    (void*)wasmHelper_cond_NZ,
    (void*)wasmHelper_cond_BE,
    (void*)wasmHelper_cond_NBE,
    (void*)wasmHelper_cond_S,
    (void*)wasmHelper_cond_NS,
    (void*)wasmHelper_cond_P,
    (void*)wasmHelper_cond_NP,
    (void*)wasmHelper_cond_L,
    (void*)wasmHelper_cond_NL,
    (void*)wasmHelper_cond_LE,
    (void*)wasmHelper_cond_NLE,
    (void*)wasmHelper_blockEnter,
    (void*)wasmHelper_writeMem8_check,
    (void*)wasmHelper_writeMem16_check,
    (void*)wasmHelper_writeMem32_check,
    (void*)wasmHelper_cacheFloat,
    (void*)wasmHelper_movsdXmmE64,
    (void*)wasmHelper_movsdE64Xmm,
    (void*)wasmHelper_movsd32r,
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    (void*)wasmHelper_profileBlockExit,
    (void*)wasmHelper_profileExitNext1,
    (void*)wasmHelper_profileExitNext2,
    (void*)wasmHelper_profileExitJump,
    (void*)wasmHelper_profileExitGeneric,
    (void*)wasmHelper_profileInlineCond,
    (void*)wasmHelper_profileRmw,
#endif
};
static constexpr int WASM_HELPER_COUNT = (int)(sizeof(g_wasmHelperTable) / sizeof(g_wasmHelperTable[0]));

#ifdef BOXEDWINE_MULTI_THREADED
// Get or build the per-(group, process) state: zero-filled reloc arrays for
// every entry plus a patched byte copy whose direct-call sites carry this
// process's target array addresses. Caller holds g_wasmBlockBinariesMutex.
// References stay stable after creation: g_wasmMtGroupProcState is a
// node-based map, and g_wasmMtGroups is append-only and only written before
// main() (registration is main-thread, pre-worker).
static WasmJitMtGroupProcState& wasmJitMtGroupProcStateLocked(U32 groupIdx, U32 memId) {
    auto key = std::make_pair(groupIdx, memId);
    auto it = g_wasmMtGroupProcState.find(key);
    if (it != g_wasmMtGroupProcState.end()) {
        return it->second;
    }
    const WasmJitMtGroup& group = g_wasmMtGroups[groupIdx];
    WasmJitMtGroupProcState st;
    st.entryArrays.resize(group.entries.size(), nullptr);
    for (size_t i = 0; i < group.entries.size(); i++) {
        U32 count = group.entries[i].relocCount;
        if (count) {
            st.entryArrays[i] = new U32[count](); // zero-filled: un-promoted entries stay on guarded slow paths
        }
    }
    st.patchedBytes = group.bytes;
    for (const WasmJitMtGroupPatch& patch : group.patches) {
        U32 v = 0;
        auto kt = g_wasmMtGroupByKey.find(patch.targetKey);
        if (kt != g_wasmMtGroupByKey.end() && kt->second.first == groupIdx) {
            v = (U32)(uintptr_t)st.entryArrays[kt->second.second];
        }
        // 5-byte padded SLEB128 i32.const operand, same encoding as the ST
        // shell loader (offset validity checked at registration).
        U8* data = st.patchedBytes.data() + patch.offset;
        data[1] = (U8)((v & 0x7f) | 0x80);
        data[2] = (U8)(((v >> 7) & 0x7f) | 0x80);
        data[3] = (U8)(((v >> 14) & 0x7f) | 0x80);
        data[4] = (U8)(((v >> 21) & 0x7f) | 0x80);
        data[5] = (U8)(((v >> 28) & 0x0f) | ((v & 0x80000000u) ? 0x70 : 0));
    }
    return g_wasmMtGroupProcState.emplace(key, std::move(st)).first->second;
}

static bool lazyInstallWasmJitBlockForWorker(int tableIndex) {
    // Group slots route to the per-worker group instance; the patched bytes
    // and export name are read under the mutex but the (potentially large)
    // compile runs outside it. Pointers stay valid by the stability rules on
    // wasmJitMtGroupProcStateLocked.
    {
        const U8* groupBytes = nullptr;
        int groupSize = 0;
        const char* exportName = nullptr;
        U32 groupIdx = 0, memId = 0;
        {
            std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
            auto refIt = g_wasmMtGroupSlotRefs.find(tableIndex);
            if (refIt != g_wasmMtGroupSlotRefs.end()) {
                const WasmJitMtGroupSlotRef& ref = refIt->second;
                WasmJitMtGroupProcState& st = wasmJitMtGroupProcStateLocked(ref.groupIdx, ref.memId);
                groupBytes = st.patchedBytes.data();
                groupSize = (int)st.patchedBytes.size();
                exportName = g_wasmMtGroups[ref.groupIdx].entries[ref.entryIdx].exportName.c_str();
                groupIdx = ref.groupIdx;
                memId = ref.memId;
            }
        }
        if (groupBytes) {
            return boxedwine_wasm_install_existing_group_mt(
                groupBytes, groupSize,
                g_wasmHelperTable, WASM_HELPER_COUNT,
                tableIndex, exportName, (int)groupIdx, memId) != 0;
        }
    }
    std::vector<U8> bytes;
    {
        std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
        auto it = g_wasmBlockBinaries.find(tableIndex);
        if (it == g_wasmBlockBinaries.end()) {
            return false;
        }
        bytes = it->second;
    }
    if (bytes.empty()) {
        return false;
    }
    return boxedwine_wasm_install_existing_mt(
        bytes.data(),
        (int)bytes.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT,
        tableIndex
    ) != 0;
}
#endif

#ifdef BOXEDWINE_MULTI_THREADED
// Set once when the first reloc array registers (persistence sessions only).
// Plain MT sessions never register any, so the per-call lookup below stays a
// single relaxed atomic load — taking g_wasmBlockBinariesMutex on every block
// call would serialize all worker threads on the hottest path.
static std::atomic<bool> g_wasmHasRelocArrays{false};
#endif

// relocBase for a block call: the per-call second parameter of every
// compiled block (0 = no slots; the generated guards take the slow paths).
static inline U32 wasmJitRelocBaseForTable(int tableIndex) {
#ifdef BOXEDWINE_MULTI_THREADED
    if (!g_wasmHasRelocArrays.load(std::memory_order_relaxed)) {
        return 0;
    }
    // MT table slots and their relocation arrays have monotonic lifetimes:
    // slots are never reused and arrays remain live until process shutdown.
    // Cache both hits and misses per worker so persistence replay does not
    // serialize every generated block call on g_wasmBlockBinariesMutex.
    static thread_local std::vector<U32> cachedBases;
    static thread_local std::vector<bool> cached;
    if (tableIndex >= 0 && (size_t)tableIndex < cached.size() && cached[(size_t)tableIndex]) {
        return cachedBases[(size_t)tableIndex];
    }
    std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
    auto it = g_wasmRelocArrays.find(tableIndex);
    U32 base = it != g_wasmRelocArrays.end() ? (U32)(uintptr_t)it->second : 0;
    if (tableIndex >= 0) {
        size_t required = (size_t)tableIndex + 64;
        if (cached.size() < required) {
            cached.resize(required, false);
            cachedBases.resize(required, 0);
        }
        cachedBases[(size_t)tableIndex] = base;
        cached[(size_t)tableIndex] = true;
    }
    return base;
#else
    return (tableIndex >= 0 && (size_t)tableIndex < g_wasmRelocBaseByTable.size())
        ? (U32)(uintptr_t)g_wasmRelocBaseByTable[tableIndex] : 0;
#endif
}

#ifdef BOXEDWINE_MULTI_THREADED
// Per-worker slot readiness. Worker wasmTable visibility is per host thread
// (the shared DecodedOp can't carry it), but MT slots are monotonic and never
// freed or reused (free_block_mt is deliberately empty), so once a slot is
// verified/installed in this worker it stays callable forever. The first call
// per (slot, worker) pays the JS slot_check (+ lazy install); every later
// call is a thread_local vector read, removing an EM_JS round trip from
// every block dispatch.
static bool wasmJitSlotReadyForWorker(int tableIndex, CPU* cpu, DecodedOp* op) {
    static thread_local std::vector<bool> ready;
    if (tableIndex > 0 && (size_t)tableIndex < ready.size() && ready[(size_t)tableIndex]) {
        return true;
    }
    if (!boxedwine_wasm_slot_check(tableIndex, (int)(uintptr_t)cpu, (int)(uintptr_t)op)) {
        if (!lazyInstallWasmJitBlockForWorker(tableIndex) ||
            !boxedwine_wasm_slot_check(tableIndex, (int)(uintptr_t)cpu, (int)(uintptr_t)op)) {
            return false;
        }
    }
    if ((size_t)tableIndex >= ready.size()) {
        ready.resize((size_t)tableIndex + 64, false);
    }
    ready[(size_t)tableIndex] = true;
    return true;
}
#endif

enum WasmHelperIdx {
    HELPER_READ_MEM8         = 0,
    HELPER_WRITE_MEM8        = 1,
    HELPER_READ_MEM16        = 2,
    HELPER_WRITE_MEM16       = 3,
    HELPER_READ_MEM32        = 4,
    HELPER_WRITE_MEM32       = 5,
    HELPER_FETCH_NEXT        = 6,
    HELPER_SYNC_FLAGS        = 7,
    HELPER_EMULATE_SINGLE_OP = 8,
    HELPER_COMPUTE_CF        = 9,
    HELPER_COMPUTE_ZF        = 10,
    HELPER_FILL_FLAGS        = 11,
    HELPER_COND_BASE         = 12, // add JitConditional index to this
    HELPER_BLOCK_ENTER       = 28, // 12 + 16 condition helpers
    HELPER_WRITE_MEM8_CHECK  = 29,
    HELPER_WRITE_MEM16_CHECK = 30,
    HELPER_WRITE_MEM32_CHECK = 31,
    HELPER_CACHE_FLOAT      = 32,
    HELPER_MOVSD_XMM_E64    = 33,
    HELPER_MOVSD_E64_XMM    = 34,
    HELPER_MOVSD32R         = 35,
    HELPER_PROFILE_BLOCK_EXIT = 36,
    HELPER_PROFILE_EXIT_NEXT1 = 37,
    HELPER_PROFILE_EXIT_NEXT2 = 38,
    HELPER_PROFILE_EXIT_JUMP = 39,
    HELPER_PROFILE_EXIT_GENERIC = 40,
    HELPER_PROFILE_INLINE_COND = 41,
    HELPER_PROFILE_RMW = 42,
};

// ---------------------------------------------------------------------------
// JitWasmCodeGen constructor
// ---------------------------------------------------------------------------
JitWasmCodeGen::JitWasmCodeGen(CPU* cpu) : JitSSE(cpu) {
    // Register common function types for the module being built.
    m_typeVoidVoid   = m_emitter.addFuncType({}, {});
    m_typeVoidI32    = m_emitter.addFuncType({ WasmType::I32 }, {});
    m_typeI32Void    = m_emitter.addFuncType({}, { WasmType::I32 });
    m_typeI32I32     = m_emitter.addFuncType({ WasmType::I32 }, { WasmType::I32 });
    m_typeVoidI32I32 = m_emitter.addFuncType({ WasmType::I32, WasmType::I32 }, {});

    // Import linear memory.
    m_emitter.addMemoryImport("env", "memory");

    // Import the C++ helpers (all have signature (i32) -> () i.e. cpu_ptr).
    for (int i = 0; i < WASM_HELPER_COUNT; i++) {
        char name[16];
        snprintf(name, sizeof(name), "fn_%d", i);
        m_emitter.addFunctionImport("helpers", name, m_typeVoidI32);
    }
    // Map well-known helpers to their import indices.
    m_helperReadMemIdx          = HELPER_READ_MEM32;
    m_helperWriteMemIdx         = HELPER_WRITE_MEM32;
    m_helperGetNextOpIdx        = HELPER_FETCH_NEXT;
    m_helperSyncToHostIdx       = HELPER_SYNC_FLAGS;
    m_helperEmulateSingleOpIdx  = HELPER_EMULATE_SINGLE_OP;

    // Declare the single local function and export it. Two parameters:
    // (cpu_ptr, relocBase) — relocBase is the block's relocation slot array
    // (0 when the caller has none; persistence-mode fast paths guard it).
    U32 execFuncIdx = m_emitter.addFunction(m_typeVoidI32I32);
    m_emitter.addExport("execute", execFuncIdx);

    // Begin the function body. Locals layout:
    //   local 0 (param): cpu_ptr i32
    //   local 1 (param): relocBase i32
    //   locals 2-9 (i32 x8): GP registers eax-edi
    //   locals 10-13 (i32 x4): segment addresses cs, ds, ss, es
    //   locals 14-45 (i32 x32): scratch temporaries
    //   local 46 (i64 x1): i64 scratch for 64-bit multiply
    //   locals 47-54 (f64 x8): FPU temporaries for shared JitFPU code
    //   locals 55-66 (v128 x12): MMX/SIMD scratch locals
    //   local 67 (i32 x1): bounded direct-loop iteration budget
    m_emitter.beginFunction({
        { 8,  WasmType::I32 },  // GP registers (locals 2-9)
        { 4,  WasmType::I32 },  // segment addresses (locals 10-13)
        { 32, WasmType::I32 },  // scratch temporaries (locals 14-45)
        { 1,  WasmType::I64 },  // i64 scratch for 64-bit multiply (local 46)
        { 8,  WasmType::F64 },  // FPU temporaries (locals 47-54)
        { 12, WasmType::V128 }, // MMX/SIMD scratch locals (locals 55-66)
        { 1,  WasmType::I32 },  // direct-loop budget (local 67)
    });

    m_gpLoaded.fill(false);
    m_gpDirty.fill(false);
    m_segLoaded.fill(false);
    m_scratchInUse.fill(false);
    m_f64ScratchInUse.fill(false);
    m_v128ScratchInUse.fill(false);
}

JitWasmCodeGen::~JitWasmCodeGen() {}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
void JitWasmCodeGen::pushCpuPtr() {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
}

static U32 cpuRegOffset32(U8 emulatedReg) {
    return CPU::offsetofReg32(emulatedReg);
}

void JitWasmCodeGen::loadGPReg(U8 emulatedReg) {
    if (m_gpLoaded[emulatedReg]) return;
    U32 local = wasmLocalForGPReg(emulatedReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(cpuRegOffset32(emulatedReg));
    m_emitter.emitLocalSet(local);
    m_gpLoaded[emulatedReg] = true;
}

void JitWasmCodeGen::storeGPReg(U8 emulatedReg) {
    U32 local = wasmLocalForGPReg(emulatedReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(local);
    m_emitter.emitI32Store(cpuRegOffset32(emulatedReg));
    m_gpDirty[emulatedReg] = false;
}

void JitWasmCodeGen::syncDirtyRegsToHost() {
    for (U8 i = 0; i < WASM_GP_LOCAL_COUNT; i++) {
        if (m_gpDirty[i]) storeGPReg(i);
    }
}

void JitWasmCodeGen::syncStateBeforeFaultingMemoryHelper() {
    writeEip(this->currentEip - cpu->seg[CS].address);
    syncDirtyRegsToHost();
}

void JitWasmCodeGen::pushRegValue(RegPtr reg) {
    if (!reg) return;
    U8 emReg = reg->emulatedReg;
    if (emReg < WASM_GP_LOCAL_COUNT) {
        loadGPReg(emReg);
        m_emitter.emitLocalGet(wasmLocalForGPReg(emReg));
    } else {
        // temp register — just use local directly
        m_emitter.emitLocalGet(reg->hardwareReg());
    }
    // High-byte register reference (AH/CH/DH/BH): shift low byte of high pair
    // into position so callers masking with 0xff get the right byte.
    if (reg->isHigh) {
        m_emitter.emitI32Const(8);
        m_emitter.emitOp(WASM_I32_SHR_U);
    }
}

void JitWasmCodeGen::popToReg(JitWidth w, RegPtr reg) {
    if (!reg) { m_emitter.emitOp(WASM_DROP); return; }
    U8 emReg = reg->emulatedReg;
    bool isGP = emReg < WASM_GP_LOCAL_COUNT;
    U32 local = isGP ? wasmLocalForGPReg(emReg) : reg->hardwareReg();

    // Scratch temporaries are always full-width; any masking needed was done
    // by the caller. GP register writes at sub-word widths must preserve the
    // upper bits of the emulated register (x86 semantics for mov al, imm8
    // etc.), so we merge the new value with the existing local.
    if (!isGP || w == JitWidth::b32 || w == JitWidth::b64) {
        m_emitter.emitLocalSet(local);
    } else if (w == JitWidth::b16) {
        // stack: new16
        m_emitter.emitI32Const(0xFFFF);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalGet(local);
        m_emitter.emitI32Const((S32)0xFFFF0000);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitLocalSet(local);
    } else { // b8
        if (reg->isHigh) {
            // (existing & 0xFFFF00FF) | ((new8 & 0xFF) << 8)
            m_emitter.emitI32Const(0xFF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const(8);
            m_emitter.emitOp(WASM_I32_SHL);
            m_emitter.emitLocalGet(local);
            m_emitter.emitI32Const((S32)0xFFFF00FF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalSet(local);
        } else {
            // (existing & 0xFFFFFF00) | (new8 & 0xFF)
            m_emitter.emitI32Const(0xFF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(local);
            m_emitter.emitI32Const((S32)0xFFFFFF00);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalSet(local);
        }
    }
    if (isGP) {
        m_gpLoaded[emReg] = true;
        m_gpDirty[emReg]  = true;
    }
}

U32 JitWasmCodeGen::allocScratch() {
    for (U32 i = 0; i < WASM_TMP_LOCAL_COUNT; i++) {
        if (!m_scratchInUse[i]) {
            m_scratchInUse[i] = true;
            return WASM_TMP_LOCAL_BASE + i;
        }
    }
    kpanic("JitWasmCodeGen: scratch local pool exhausted");
    return WASM_TMP_LOCAL_BASE;
}

void JitWasmCodeGen::freeScratch(U32 local) {
    if (local >= WASM_TMP_LOCAL_BASE && local < WASM_TMP_LOCAL_BASE + WASM_TMP_LOCAL_COUNT) {
        m_scratchInUse[local - WASM_TMP_LOCAL_BASE] = false;
    }
}

U32 JitWasmCodeGen::allocF64Scratch() {
    for (U32 i = 0; i < WASM_F64_LOCAL_COUNT; i++) {
        if (!m_f64ScratchInUse[i]) {
            m_f64ScratchInUse[i] = true;
            return WASM_F64_LOCAL_BASE + i;
        }
    }
    kpanic("JitWasmCodeGen: f64 scratch local pool exhausted");
    return WASM_F64_LOCAL_BASE;
}

void JitWasmCodeGen::freeF64Scratch(U32 local) {
    if (local >= WASM_F64_LOCAL_BASE && local < WASM_F64_LOCAL_BASE + WASM_F64_LOCAL_COUNT) {
        m_f64ScratchInUse[local - WASM_F64_LOCAL_BASE] = false;
    }
}

U32 JitWasmCodeGen::allocV128Scratch() {
    for (U32 i = 0; i < WASM_V128_LOCAL_COUNT; i++) {
        if (!m_v128ScratchInUse[i]) {
            m_v128ScratchInUse[i] = true;
            return WASM_V128_LOCAL_BASE + i;
        }
    }
    kpanic("JitWasmCodeGen: v128 scratch local pool exhausted");
    return WASM_V128_LOCAL_BASE;
}

void JitWasmCodeGen::freeV128Scratch(U32 local) {
    if (local >= WASM_V128_LOCAL_BASE && local < WASM_V128_LOCAL_BASE + WASM_V128_LOCAL_COUNT) {
        m_v128ScratchInUse[local - WASM_V128_LOCAL_BASE] = false;
    }
}

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg);
}

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg, bool isHigh) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg, isHigh);
}

static bool canReleaseScratchReg(const RegPtr& reg);
static void emitWasmMemBase(WasmEmitter& emitter, const std::function<void(RegPtr)>& pushReg, MemPtr address, U32& memOffset);

MMXRegPtr JitWasmCodeGen::makeWasmMMXReg(U8 hwLocal, U8 emulatedReg) {
    return std::make_shared<MMXRegInternal>(hwLocal, emulatedReg);
}

// Stack unchanged; replaces the high i64 lane of the v128 local with zero.
void JitWasmCodeGen::emitZeroHigh64(MMXRegPtr reg) {
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitI64Const(0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(reg->hardwareReg());
}

// Consumes an i64 stack value, writes it to the low lane of a v128 local,
// zeroes the high lane, and clobbers WASM_I64_SCRATCH.
void JitWasmCodeGen::emitI64ToMmxLocal(U32 local) {
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(local);
}

// Pushes the low i64 lane of a v128 local onto the stack.
void JitWasmCodeGen::emitMmxLocalToI64(U32 local) {
    m_emitter.emitLocalGet(local);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
}

void JitWasmCodeGen::pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_DOT_I16X8_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 dwordLanes0And2[16] = {0, 1, 2, 3, 8, 9, 10, 11, 0, 1, 2, 3, 8, 9, 10, 11};

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitI8x16Shuffle(dwordLanes0And2);

    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI8x16Shuffle(dwordLanes0And2);

    m_emitter.emitSimdOp(WASM_SIMD_I64X2_EXTMUL_LOW_I32X4_U);
    m_emitter.emitLocalSet(dst->hardwareReg());
}
static void emitWasmXmmZero(WasmEmitter& emitter, SSERegPtr dst) {
    const U8 zero[16] = {};
    emitter.emitV128Const(zero);
    emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pslldqXmm(SSERegPtr dst, U32 imm) {
    if (imm > 15) {
        emitWasmXmmZero(m_emitter, dst);
        return;
    }

    U8 lanes[16];
    for (U32 i = 0; i < 16; i++) {
        lanes[i] = i >= imm ? (U8)(i - imm) : (U8)(16 + i);
    }

    const U8 zero[16] = {};
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitV128Const(zero);
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::psrldqXmm(SSERegPtr dst, U32 imm) {
    if (imm > 15) {
        emitWasmXmmZero(m_emitter, dst);
        return;
    }

    U8 lanes[16];
    for (U32 i = 0; i < 16; i++) {
        lanes[i] = i + imm < 16 ? (U8)(i + imm) : (U8)(16 + i);
    }

    const U8 zero[16] = {};
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitV128Const(zero);
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

#define WASM_XMM_LOGICAL_SHIFT_IMM(name, op, maxCount) \
    void JitWasmCodeGen::name(SSERegPtr dst, U32 imm) { \
        if (imm > maxCount) { \
            emitWasmXmmZero(m_emitter, dst); \
            return; \
        } \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitI32Const((S32)imm); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

#define WASM_XMM_ARITH_SHIFT_IMM(name, op, maxCount) \
    void JitWasmCodeGen::name(SSERegPtr dst, U32 imm) { \
        if (imm > maxCount) \
            imm = maxCount; \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitI32Const((S32)imm); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_XMM_LOGICAL_SHIFT_IMM(psllqXmm, WASM_SIMD_I64X2_SHL, 63)
WASM_XMM_LOGICAL_SHIFT_IMM(pslldXmm, WASM_SIMD_I32X4_SHL, 31)
WASM_XMM_LOGICAL_SHIFT_IMM(psllwXmm, WASM_SIMD_I16X8_SHL, 15)
WASM_XMM_ARITH_SHIFT_IMM(psradXmm, WASM_SIMD_I32X4_SHR_S, 31)
WASM_XMM_ARITH_SHIFT_IMM(psrawXmm, WASM_SIMD_I16X8_SHR_S, 15)
WASM_XMM_LOGICAL_SHIFT_IMM(psrlqXmm, WASM_SIMD_I64X2_SHR_U, 63)
WASM_XMM_LOGICAL_SHIFT_IMM(psrldXmm, WASM_SIMD_I32X4_SHR_U, 31)
WASM_XMM_LOGICAL_SHIFT_IMM(psrlwXmm, WASM_SIMD_I16X8_SHR_U, 15)

#undef WASM_XMM_ARITH_SHIFT_IMM
#undef WASM_XMM_LOGICAL_SHIFT_IMM

static void emitWasmXmmShiftCount(WasmEmitter& emitter, SSERegPtr src) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    emitter.emitLocalSet(WASM_I64_SCRATCH);
}

#define WASM_XMM_LOGICAL_SHIFT_VAR(name, op, overshiftBits) \
    void JitWasmCodeGen::name(SSERegPtr dst, SSERegPtr src) { \
        emitWasmXmmShiftCount(m_emitter, src); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitI64Const(overshiftBits); \
        m_emitter.emitOp(WASM_I64_SHR_U); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitOp(WASM_I64_NE); \
        m_emitter.emitIf(); \
        emitWasmXmmZero(m_emitter, dst); \
        m_emitter.emitElse(); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitOp(WASM_I32_WRAP_I64); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
        m_emitter.emitEnd(); \
    }

#define WASM_XMM_ARITH_SHIFT_VAR(name, op, maxCount, overshiftBits) \
    void JitWasmCodeGen::name(SSERegPtr dst, SSERegPtr src) { \
        emitWasmXmmShiftCount(m_emitter, src); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitI64Const(overshiftBits); \
        m_emitter.emitOp(WASM_I64_SHR_U); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitOp(WASM_I64_NE); \
        m_emitter.emitIf(WasmType::I32); \
        m_emitter.emitI32Const(maxCount); \
        m_emitter.emitElse(); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitOp(WASM_I32_WRAP_I64); \
        m_emitter.emitEnd(); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_XMM_LOGICAL_SHIFT_VAR(psllqXmmXmm, WASM_SIMD_I64X2_SHL, 6)
WASM_XMM_LOGICAL_SHIFT_VAR(pslldXmmXmm, WASM_SIMD_I32X4_SHL, 5)
WASM_XMM_LOGICAL_SHIFT_VAR(psllwXmmXmm, WASM_SIMD_I16X8_SHL, 4)
WASM_XMM_ARITH_SHIFT_VAR(psradXmmXmm, WASM_SIMD_I32X4_SHR_S, 31, 5)
WASM_XMM_ARITH_SHIFT_VAR(psrawXmmXmm, WASM_SIMD_I16X8_SHR_S, 15, 4)
WASM_XMM_LOGICAL_SHIFT_VAR(psrlqXmmXmm, WASM_SIMD_I64X2_SHR_U, 6)
WASM_XMM_LOGICAL_SHIFT_VAR(psrldXmmXmm, WASM_SIMD_I32X4_SHR_U, 5)
WASM_XMM_LOGICAL_SHIFT_VAR(psrlwXmmXmm, WASM_SIMD_I16X8_SHR_U, 4)

#undef WASM_XMM_ARITH_SHIFT_VAR
#undef WASM_XMM_LOGICAL_SHIFT_VAR
void JitWasmCodeGen::comisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (currentLazyFlags != FLAGS_NONE && currentLazyFlags != FLAGS_NULL) {
        fillFlags(WASM_FMASK_TEST);
    }

    U32 lhs = allocF64Scratch();
    U32 rhs = allocF64Scratch();
    U32 result = allocScratch();
    RegPtr flags = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    m_emitter.emitLocalSet(lhs);
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    m_emitter.emitLocalSet(rhs);

    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(CF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_LT);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(ZF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_EQ);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(CF | PF | ZF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    pushRegValue(flags);
    m_emitter.emitI32Const((S32)~WASM_FMASK_TEST);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalGet(result);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(flags->hardwareReg());

    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), flags);
    storeLazyFlagType(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
    freeScratch(flags->hardwareReg());
    freeScratch(result);
    freeF64Scratch(rhs);
    freeF64Scratch(lhs);
}

void JitWasmCodeGen::ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comisdXmmXmm(dst, src);
}

void JitWasmCodeGen::cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F64X2_CONVERT_LOW_I32X4_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F32X4_CONVERT_I32X4_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F64X2_CONVERT_LOW_I32X4_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F32X4_DEMOTE_F64X2_ZERO);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F64X2_PROMOTE_LOW_F32X4);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    m_emitter.emitOp(WASM_F32_DEMOTE_F64);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    pushRegValue(src);
    m_emitter.emitOp(WASM_F64_CONVERT_I32_S);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

#ifdef BOXEDWINE_64
void JitWasmCodeGen::cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) {
    (void)dst;
    (void)src;
    kpanic("WASM cvtsi2sdXmmR64 is not supported for 32-bit guest RegPtr sources");
}
#endif

void JitWasmCodeGen::cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    m_emitter.emitOp(WASM_F64_PROMOTE_F32);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::IfSseLessThan(SSERegPtr src1, SSERegPtr src2) {
    branchBoundary();
    m_emitter.emitLocalGet(src1->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    m_emitter.emitLocalGet(src2->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    m_emitter.emitOp(WASM_F32_LT);
    finishIf();
}

void JitWasmCodeGen::movmskpd(RegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I64X2_BITMASK);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
}
void JitWasmCodeGen::maskmovdqu(SSERegPtr dst, SSERegPtr src, MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE maskmovdqu expects a host MemPtr");
    }

    U32 oldValue = allocV128Scratch();
    U32 byteMask = allocV128Scratch();

    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitV128Load(memOffset, 0);
    m_emitter.emitLocalSet(oldValue);

    // Convert each mask byte's high bit to a full 0x00/0xff byte mask.
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI32Const(7);
    m_emitter.emitSimdOp(WASM_SIMD_I8X16_SHR_S);
    m_emitter.emitLocalSet(byteMask);

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(oldValue);
    m_emitter.emitLocalGet(byteMask);
    m_emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    m_emitter.emitLocalSet(byteMask);

    memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(byteMask);
    m_emitter.emitV128Store(memOffset, 0);

    freeV128Scratch(byteMask);
    freeV128Scratch(oldValue);
}
void JitWasmCodeGen::psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr originalDst = getTmpSSE();
    RegPtr sumLow = getTmpReg();
    RegPtr sumHigh = getTmpReg();
    RegPtr left = getTmpReg();
    RegPtr right = getTmpReg();

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalSet(originalDst->hardwareReg());

    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(sumLow->hardwareReg());
    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(sumHigh->hardwareReg());

    for (U8 lane = 0; lane < 16; ++lane) {
        m_emitter.emitLocalGet(originalDst->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I8X16_EXTRACT_LANE_U, lane);
        m_emitter.emitLocalSet(left->hardwareReg());
        m_emitter.emitLocalGet(src->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I8X16_EXTRACT_LANE_U, lane);
        m_emitter.emitLocalSet(right->hardwareReg());

        RegPtr sum = lane < 8 ? sumLow : sumHigh;
        m_emitter.emitLocalGet(sum->hardwareReg());
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_SELECT);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitLocalSet(sum->hardwareReg());
    }

    const U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalGet(sumLow->hardwareReg());
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalGet(sumHigh->hardwareReg());
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeScratch(right->hardwareReg());
    freeScratch(left->hardwareReg());
    freeScratch(sumHigh->hardwareReg());
    freeScratch(sumLow->hardwareReg());
}
void JitWasmCodeGen::pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_EXTMUL_LOW_I16X8_S);
    m_emitter.emitI32Const(16);
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_SHR_S);

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_EXTMUL_HIGH_I16X8_S);
    m_emitter.emitI32Const(16);
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_SHR_S);

    m_emitter.emitSimdOp(WASM_SIMD_I16X8_NARROW_I32X4_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_EXTMUL_LOW_I16X8_U);
    m_emitter.emitI32Const(16);
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_SHR_U);

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_EXTMUL_HIGH_I16X8_U);
    m_emitter.emitI32Const(16);
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_SHR_U);

    m_emitter.emitSimdOp(WASM_SIMD_I16X8_NARROW_I32X4_U);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::sfence() {
}

void JitWasmCodeGen::stmxcsr(MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE stmxcsr expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, mxcsr));
    m_emitter.emitI32Store(memOffset, 0);
}

void JitWasmCodeGen::ldmxcsr(MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE ldmxcsr expects a host MemPtr");
    }
    U32 memOffset = 0;
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI32Load(memOffset, 0);
    m_emitter.emitI32Store((U32)offsetof(CPU, mxcsr));
}

void JitWasmCodeGen::lfence() {
}

void JitWasmCodeGen::mfence() {
}

void JitWasmCodeGen::clflush(MemPtr address) {
    (void)address;
}

void JitWasmCodeGen::pause() {
}

static void emitXmmBinarySimdOp(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_GT_S);
}

void JitWasmCodeGen::pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_GT_S);
}

void JitWasmCodeGen::pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I32X4_GT_S);
}

void JitWasmCodeGen::pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_EQ);
}

void JitWasmCodeGen::pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_EQ);
}

void JitWasmCodeGen::pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I32X4_EQ);
}

void JitWasmCodeGen::paddbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_ADD);
}

void JitWasmCodeGen::paddwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_ADD);
}

void JitWasmCodeGen::padddXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I32X4_ADD);
}

void JitWasmCodeGen::paddqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I64X2_ADD);
}

void JitWasmCodeGen::paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_ADD_SAT_S);
}

void JitWasmCodeGen::paddswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_ADD_SAT_S);
}

void JitWasmCodeGen::paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_ADD_SAT_U);
}

void JitWasmCodeGen::padduswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_ADD_SAT_U);
}

void JitWasmCodeGen::pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_MUL);
}

void JitWasmCodeGen::psubbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_SUB);
}

void JitWasmCodeGen::psubwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_SUB);
}

void JitWasmCodeGen::psubdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I32X4_SUB);
}

void JitWasmCodeGen::psubqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I64X2_SUB);
}

void JitWasmCodeGen::psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_SUB_SAT_S);
}

void JitWasmCodeGen::psubswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_SUB_SAT_S);
}

void JitWasmCodeGen::psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_SUB_SAT_U);
}

void JitWasmCodeGen::psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_SUB_SAT_U);
}

void JitWasmCodeGen::packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_NARROW_I32X4_S);
}

void JitWasmCodeGen::packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_NARROW_I16X8_S);
}

void JitWasmCodeGen::packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_NARROW_I16X8_U);
}

void JitWasmCodeGen::pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_AVGR_U);
}

void JitWasmCodeGen::pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_AVGR_U);
}

void JitWasmCodeGen::pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_MAX_S);
}

void JitWasmCodeGen::pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_MAX_U);
}

void JitWasmCodeGen::pminswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I16X8_MIN_S);
}

void JitWasmCodeGen::pminubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_I8X16_MIN_U);
}

void JitWasmCodeGen::pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_U, imm & 7);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
}

void JitWasmCodeGen::pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    pushRegValue(src);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_REPLACE_LANE, imm & 7);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I8X16_BITMASK);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
}

static void emitXmmUnarySimdOp(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmF32CompareSelect(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 compareOp) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(compareOp);
    emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarF32CompareSelect(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 compareOp, U32 tmpLocal) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(compareOp);
    emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmF64CompareSelect(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 compareOp) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(compareOp);
    emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarF64CompareSelect(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 compareOp, U32 tmpLocal) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(compareOp);
    emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarBinaryF32Op(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op, U32 tmpLocal) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarUnaryF32Op(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op, U32 tmpLocal) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarBinaryF64Op(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op, U32 tmpLocal) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmScalarUnaryF64Op(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 op, U32 tmpLocal) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(op);
    emitter.emitLocalSet(tmpLocal);
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, 0);
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_REPLACE_LANE, 0);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmF32CompareMask(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 pred) {
    switch (pred & 7) {
    case 0:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        break;
    case 1:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_LT);
        break;
    case 2:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_LE);
        break;
    case 3:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_AND);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 4:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 5:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_LT);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 6:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_LE);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 7:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F32X4_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_AND);
        break;
    }
}

static void emitXmmF64CompareMask(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, U32 pred) {
    switch (pred & 7) {
    case 0:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        break;
    case 1:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_LT);
        break;
    case 2:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_LE);
        break;
    case 3:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_AND);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 4:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 5:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_LT);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 6:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_LE);
        emitter.emitSimdOp(WASM_SIMD_V128_NOT);
        break;
    case 7:
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitLocalGet(dst->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitLocalGet(src->hardwareReg());
        emitter.emitSimdOp(WASM_SIMD_F64X2_EQ);
        emitter.emitSimdOp(WASM_SIMD_V128_AND);
        break;
    }
}

static void emitXmmAndNot(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitSimdOp(WASM_SIMD_V128_ANDNOT);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitXmmShuffle(WasmEmitter& emitter, SSERegPtr dst, SSERegPtr src, const U8 lanes[16]) {
    emitter.emitLocalGet(dst->hardwareReg());
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitI8x16Shuffle(lanes);
    emitter.emitLocalSet(dst->hardwareReg());
}

static void emitWasmF64ConstBits(WasmEmitter& emitter, U64 bits) {
    emitter.emitI64Const((S64)bits);
    emitter.emitOp(WASM_F64_REINTERPRET_I64);
}

static void emitWasmF32x4SplatBits(WasmEmitter& emitter, U32 bits) {
    emitter.emitI32Const((S32)bits);
    emitter.emitOp(WASM_F32_REINTERPRET_I32);
    emitter.emitSimdOp(WASM_SIMD_F32X4_SPLAT);
}

static void emitWasmI32x4Splat(WasmEmitter& emitter, U32 value) {
    emitter.emitI32Const((S32)value);
    emitter.emitSimdOp(WASM_SIMD_I32X4_SPLAT);
}

static void emitWasmRcpApproxF32x4(WasmEmitter& emitter, SSERegPtr src, U32 tmpLocal) {
    emitWasmI32x4Splat(emitter, 0x7ef311c3u);
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(WASM_SIMD_I32X4_SUB);
    emitter.emitLocalSet(tmpLocal);

    emitter.emitLocalGet(tmpLocal);
    emitWasmF32x4SplatBits(emitter, 0x40000000u);
    emitter.emitLocalGet(tmpLocal);
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
    emitter.emitSimdOp(WASM_SIMD_F32X4_SUB);
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
}

static void emitWasmRsqrtApproxF32x4(WasmEmitter& emitter, SSERegPtr src, U32 tmpLocal) {
    emitWasmI32x4Splat(emitter, 0x5f375a82u);
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitI32Const(1);
    emitter.emitSimdOp(WASM_SIMD_I32X4_SHR_U);
    emitter.emitSimdOp(WASM_SIMD_I32X4_SUB);
    emitter.emitLocalSet(tmpLocal);

    emitter.emitLocalGet(tmpLocal);
    emitWasmF32x4SplatBits(emitter, 0x3fc01d31u);
    emitter.emitLocalGet(src->hardwareReg());
    emitWasmF32x4SplatBits(emitter, 0x3f000000u);
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
    emitter.emitLocalGet(tmpLocal);
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
    emitter.emitSimdOp(WASM_SIMD_F32X4_SUB);
    emitter.emitSimdOp(WASM_SIMD_F32X4_MUL);
}

static void emitWasmF32LaneAsF64(WasmEmitter& emitter, SSERegPtr src, U8 lane) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, lane);
    emitter.emitOp(WASM_F64_PROMOTE_F32);
}

static void emitWasmRoundF64ForSseI32(WasmEmitter& emitter, U32 f64Local, U32 mxcsrLocal, bool truncate) {
    if (truncate) {
        emitter.emitOp(WASM_F64_TRUNC);
        emitter.emitLocalSet(f64Local);
        return;
    }

    emitter.emitLocalSet(f64Local);

    emitter.emitLocalGet(mxcsrLocal);
    emitter.emitI32Const(13);
    emitter.emitOp(WASM_I32_SHR_U);
    emitter.emitI32Const(3);
    emitter.emitOp(WASM_I32_AND);
    emitter.emitOp(WASM_I32_EQZ);
    emitter.emitIf();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_NEAREST);
    emitter.emitLocalSet(f64Local);
    emitter.emitElse();
    emitter.emitLocalGet(mxcsrLocal);
    emitter.emitI32Const(13);
    emitter.emitOp(WASM_I32_SHR_U);
    emitter.emitI32Const(3);
    emitter.emitOp(WASM_I32_AND);
    emitter.emitI32Const(ROUND_Down);
    emitter.emitOp(WASM_I32_EQ);
    emitter.emitIf();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_FLOOR);
    emitter.emitLocalSet(f64Local);
    emitter.emitElse();
    emitter.emitLocalGet(mxcsrLocal);
    emitter.emitI32Const(13);
    emitter.emitOp(WASM_I32_SHR_U);
    emitter.emitI32Const(3);
    emitter.emitOp(WASM_I32_AND);
    emitter.emitI32Const(ROUND_Up);
    emitter.emitOp(WASM_I32_EQ);
    emitter.emitIf();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_CEIL);
    emitter.emitLocalSet(f64Local);
    emitter.emitElse();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_TRUNC);
    emitter.emitLocalSet(f64Local);
    emitter.emitEnd();
    emitter.emitEnd();
    emitter.emitEnd();
}

// WebAssembly's scalar truncation instructions trap for NaN and out-of-range
// values.  Masked x86 x87/SSE conversions instead produce the integer
// indefinite value, so validate the rounded input before emitting truncation.
static void emitWasmF64ToI32OrIndefinite(WasmEmitter& emitter, U32 f64Local) {
    emitter.emitLocalGet(f64Local);
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_NE);

    emitter.emitLocalGet(f64Local);
    emitWasmF64ConstBits(emitter, 0xc1e0000000000000ULL); // -2147483648.0
    emitter.emitOp(WASM_F64_LT);
    emitter.emitOp(WASM_I32_OR);

    emitter.emitLocalGet(f64Local);
    emitWasmF64ConstBits(emitter, 0x41e0000000000000ULL); // 2147483648.0
    emitter.emitOp(WASM_F64_GE);
    emitter.emitOp(WASM_I32_OR);

    emitter.emitIf(WasmType::I32);
    emitter.emitI32Const((S32)0x80000000u);
    emitter.emitElse();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_I32_TRUNC_F64_S);
    emitter.emitEnd();
}

static void emitWasmF64ToI64OrIndefinite(WasmEmitter& emitter, U32 f64Local) {
    emitter.emitLocalGet(f64Local);
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_F64_NE);

    emitter.emitLocalGet(f64Local);
    emitWasmF64ConstBits(emitter, 0xc3e0000000000000ULL); // -9223372036854775808.0
    emitter.emitOp(WASM_F64_LT);
    emitter.emitOp(WASM_I32_OR);

    emitter.emitLocalGet(f64Local);
    emitWasmF64ConstBits(emitter, 0x43e0000000000000ULL); // 9223372036854775808.0
    emitter.emitOp(WASM_F64_GE);
    emitter.emitOp(WASM_I32_OR);

    emitter.emitIf(WasmType::I64);
    emitter.emitI64Const((S64)(-9223372036854775807LL - 1));
    emitter.emitElse();
    emitter.emitLocalGet(f64Local);
    emitter.emitOp(WASM_I64_TRUNC_F64_S);
    emitter.emitEnd();
}

static void emitWasmSseF32LaneToI32(WasmEmitter& emitter, SSERegPtr src, U8 lane, U32 f64Local, U32 mxcsrLocal, bool truncate) {
    emitWasmF32LaneAsF64(emitter, src, lane);
    emitWasmRoundF64ForSseI32(emitter, f64Local, mxcsrLocal, truncate);
    emitWasmF64ToI32OrIndefinite(emitter, f64Local);
}

static void emitWasmSseF64LaneToI32(WasmEmitter& emitter, SSERegPtr src, U8 lane, U32 f64Local, U32 mxcsrLocal, bool truncate) {
    emitter.emitLocalGet(src->hardwareReg());
    emitter.emitSimdLaneOp(WASM_SIMD_F64X2_EXTRACT_LANE, lane);
    emitWasmRoundF64ForSseI32(emitter, f64Local, mxcsrLocal, truncate);
    emitWasmF64ToI32OrIndefinite(emitter, f64Local);
}

static void emitWasmZeroV128Local(WasmEmitter& emitter, U32 local) {
    U8 zero[16] = {};
    emitter.emitV128Const(zero);
    emitter.emitLocalSet(local);
}

void JitWasmCodeGen::cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    SSERegPtr converted = getTmpSSE();
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F32X4_CONVERT_I32X4_S);
    m_emitter.emitLocalSet(converted->hardwareReg());

    const U8 lanes[16] = { 16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15 };
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(converted->hardwareReg());
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));

    emitWasmZeroV128Local(m_emitter, dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 1, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));
    SSERegPtr result = getTmpSSE();

    emitWasmZeroV128Local(m_emitter, result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 1, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();

    emitWasmZeroV128Local(m_emitter, dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 1, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));
    SSERegPtr result = getTmpSSE();

    emitWasmZeroV128Local(m_emitter, result->hardwareReg());

    for (U8 lane = 0; lane < 4; ++lane) {
        m_emitter.emitLocalGet(result->hardwareReg());
        emitWasmSseF32LaneToI32(m_emitter, src, lane, f64Local, mxcsr->hardwareReg(), false);
        m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, lane);
        m_emitter.emitLocalSet(result->hardwareReg());
    }

    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));

    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF32LaneToI32(m_emitter, src, 0, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF32LaneToI32(m_emitter, src, 1, f64Local, mxcsr->hardwareReg(), false);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    pushRegValue(src);
    m_emitter.emitOp(WASM_F64_CONVERT_I32_S);
    m_emitter.emitOp(WASM_F32_DEMOTE_F64);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));
    emitWasmSseF32LaneToI32(m_emitter, src, 0, f64Local, mxcsr->hardwareReg(), false);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    RegPtr mxcsr = readCPU(JitWidth::b32, (U32)offsetof(CPU, mxcsr));
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, mxcsr->hardwareReg(), false);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
    freeF64Scratch(f64Local);
    freeScratch(mxcsr->hardwareReg());
}

void JitWasmCodeGen::cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();

    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF32LaneToI32(m_emitter, src, 0, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmSseF32LaneToI32(m_emitter, src, 1, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    SSERegPtr result = getTmpSSE();

    emitWasmZeroV128Local(m_emitter, result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    emitWasmSseF64LaneToI32(m_emitter, src, 1, f64Local, 0, true);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    SSERegPtr result = getTmpSSE();

    emitWasmZeroV128Local(m_emitter, result->hardwareReg());

    for (U8 lane = 0; lane < 4; ++lane) {
        m_emitter.emitLocalGet(result->hardwareReg());
        emitWasmSseF32LaneToI32(m_emitter, src, lane, f64Local, 0, true);
        m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, lane);
        m_emitter.emitLocalSet(result->hardwareReg());
    }

    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());

    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    emitWasmSseF32LaneToI32(m_emitter, src, 0, f64Local, 0, true);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    U32 f64Local = allocF64Scratch();
    emitWasmSseF64LaneToI32(m_emitter, src, 0, f64Local, 0, true);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
    freeF64Scratch(f64Local);
}

void JitWasmCodeGen::movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 24, 25, 26, 27, 28, 29, 30, 31, 8, 9, 10, 11, 12, 13, 14, 15 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::movmskpsR32Xmm(RegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_BITMASK);
    popToReg(JitWidth::b32, dst);
    forceSyncBackIfNotCached(dst);
}

void JitWasmCodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    U8 lanes[16];
    const U32 lane0 = (imm >> 0) & 3;
    const U32 lane1 = (imm >> 2) & 3;
    const U32 lane2 = (imm >> 4) & 3;
    const U32 lane3 = (imm >> 6) & 3;

    for (U32 i = 0; i < 4; i++) {
        lanes[i] = (U8)(lane0 * 4 + i);
        lanes[4 + i] = (U8)(lane1 * 4 + i);
        lanes[8 + i] = (U8)(16 + lane2 * 4 + i);
        lanes[12 + i] = (U8)(16 + lane3 * 4 + i);
    }
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    U8 lanes[16];

    for (U32 out = 0; out < 4; out++) {
        const U32 in = (imm >> (out * 2)) & 3;
        for (U32 i = 0; i < 4; i++) {
            lanes[out * 4 + i] = (U8)(in * 4 + i);
        }
    }

    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    U8 lanes[16] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    for (U32 out = 4; out < 8; out++) {
        const U32 in = 4 + ((imm >> ((out - 4) * 2)) & 3);
        lanes[out * 2] = (U8)(in * 2);
        lanes[out * 2 + 1] = (U8)(in * 2 + 1);
    }

    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    U8 lanes[16];

    for (U32 out = 0; out < 4; out++) {
        const U32 in = (imm >> (out * 2)) & 3;
        lanes[out * 2] = (U8)(in * 2);
        lanes[out * 2 + 1] = (U8)(in * 2 + 1);
    }
    for (U32 i = 8; i < 16; i++) {
        lanes[i] = (U8)i;
    }

    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    U8 lanes[16];
    const U32 dstQword = imm & 1;
    const U32 srcQword = (imm >> 1) & 1;

    for (U32 i = 0; i < 8; i++) {
        lanes[i] = (U8)(dstQword * 8 + i);
        lanes[8 + i] = (U8)(16 + srcQword * 8 + i);
    }
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 2, 3, 16, 17, 18, 19, 4, 5, 6, 7, 20, 21, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 16, 17, 2, 3, 18, 19, 4, 5, 20, 21, 6, 7, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 2, 3, 16, 17, 18, 19, 4, 5, 6, 7, 20, 21, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 9, 24, 25, 10, 11, 26, 27, 12, 13, 28, 29, 14, 15, 30, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    const U8 lanes[16] = { 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31 };
    emitXmmShuffle(m_emitter, dst, src, lanes);
}

void JitWasmCodeGen::addpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F32X4_ADD);
}

void JitWasmCodeGen::addssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF32Op(m_emitter, dst, src, WASM_SIMD_F32X4_ADD, tmp->hardwareReg());
}

void JitWasmCodeGen::addpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F64X2_ADD);
}

void JitWasmCodeGen::addsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF64Op(m_emitter, dst, src, WASM_SIMD_F64X2_ADD, tmp->hardwareReg());
}

void JitWasmCodeGen::subpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F32X4_SUB);
}

void JitWasmCodeGen::subssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF32Op(m_emitter, dst, src, WASM_SIMD_F32X4_SUB, tmp->hardwareReg());
}

void JitWasmCodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F64X2_SUB);
}

void JitWasmCodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF64Op(m_emitter, dst, src, WASM_SIMD_F64X2_SUB, tmp->hardwareReg());
}

void JitWasmCodeGen::mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F32X4_MUL);
}

void JitWasmCodeGen::mulssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF32Op(m_emitter, dst, src, WASM_SIMD_F32X4_MUL, tmp->hardwareReg());
}

void JitWasmCodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F64X2_MUL);
}

void JitWasmCodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF64Op(m_emitter, dst, src, WASM_SIMD_F64X2_MUL, tmp->hardwareReg());
}

void JitWasmCodeGen::divpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F32X4_DIV);
}

void JitWasmCodeGen::divssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF32Op(m_emitter, dst, src, WASM_SIMD_F32X4_DIV, tmp->hardwareReg());
}

void JitWasmCodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_F64X2_DIV);
}

void JitWasmCodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarBinaryF64Op(m_emitter, dst, src, WASM_SIMD_F64X2_DIV, tmp->hardwareReg());
}

void JitWasmCodeGen::minpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmF32CompareSelect(m_emitter, dst, src, WASM_SIMD_F32X4_LT);
}

void JitWasmCodeGen::minssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarF32CompareSelect(m_emitter, dst, src, WASM_SIMD_F32X4_LT, tmp->hardwareReg());
}

void JitWasmCodeGen::minpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmF64CompareSelect(m_emitter, dst, src, WASM_SIMD_F64X2_LT);
}

void JitWasmCodeGen::minsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarF64CompareSelect(m_emitter, dst, src, WASM_SIMD_F64X2_LT, tmp->hardwareReg());
}

void JitWasmCodeGen::maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmF32CompareSelect(m_emitter, dst, src, WASM_SIMD_F32X4_GT);
}

void JitWasmCodeGen::maxssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarF32CompareSelect(m_emitter, dst, src, WASM_SIMD_F32X4_GT, tmp->hardwareReg());
}

void JitWasmCodeGen::maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmF64CompareSelect(m_emitter, dst, src, WASM_SIMD_F64X2_GT);
}

void JitWasmCodeGen::maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarF64CompareSelect(m_emitter, dst, src, WASM_SIMD_F64X2_GT, tmp->hardwareReg());
}

void JitWasmCodeGen::sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmUnarySimdOp(m_emitter, dst, src, WASM_SIMD_F32X4_SQRT);
}

void JitWasmCodeGen::sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarUnaryF32Op(m_emitter, dst, src, WASM_SIMD_F32X4_SQRT, tmp->hardwareReg());
}

void JitWasmCodeGen::sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmUnarySimdOp(m_emitter, dst, src, WASM_SIMD_F64X2_SQRT);
}

void JitWasmCodeGen::sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmScalarUnaryF64Op(m_emitter, dst, src, WASM_SIMD_F64X2_SQRT, tmp->hardwareReg());
}

void JitWasmCodeGen::rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitWasmRcpApproxF32x4(m_emitter, src, tmp->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    emitWasmF32x4SplatBits(m_emitter, 0x3f800000u);
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_F32X4_DIV);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitWasmRsqrtApproxF32x4(m_emitter, src, tmp->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    emitWasmRsqrtApproxF32x4(m_emitter, src, tmp->hardwareReg());
    m_emitter.emitLocalSet(tmp->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(tmp->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_F32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    emitXmmF32CompareMask(m_emitter, dst, src, imm);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmF32CompareMask(m_emitter, dst, src, imm);
    m_emitter.emitLocalSet(tmp->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(tmp->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    emitXmmF64CompareMask(m_emitter, dst, src, imm);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    SSERegPtr tmp = getTmpSSE();
    emitXmmF64CompareMask(m_emitter, dst, src, imm);
    m_emitter.emitLocalSet(tmp->hardwareReg());

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(tmp->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (currentLazyFlags != FLAGS_NONE && currentLazyFlags != FLAGS_NULL) {
        fillFlags(WASM_FMASK_TEST);
    }

    U32 lhs = allocF64Scratch();
    U32 rhs = allocF64Scratch();
    U32 result = allocScratch();
    RegPtr flags = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));

    emitWasmF32LaneAsF64(m_emitter, dst, 0);
    m_emitter.emitLocalSet(lhs);
    emitWasmF32LaneAsF64(m_emitter, src, 0);
    m_emitter.emitLocalSet(rhs);

    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(CF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_LT);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(ZF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_EQ);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    m_emitter.emitI32Const(CF | PF | ZF);
    m_emitter.emitLocalGet(result);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitLocalGet(lhs);
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitLocalGet(rhs);
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result);

    pushRegValue(flags);
    m_emitter.emitI32Const((S32)~WASM_FMASK_TEST);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalGet(result);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(flags->hardwareReg());

    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), flags);
    storeLazyFlagType(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
    freeScratch(flags->hardwareReg());
    freeScratch(result);
    freeF64Scratch(rhs);
    freeF64Scratch(lhs);
}

void JitWasmCodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comissXmmXmm(dst, src);
}

void JitWasmCodeGen::andpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_AND);
}

void JitWasmCodeGen::orpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_OR);
}

void JitWasmCodeGen::xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_XOR);
}

void JitWasmCodeGen::andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmAndNot(m_emitter, dst, src);
}

void JitWasmCodeGen::andpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_AND);
}

void JitWasmCodeGen::orpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_OR);
}

void JitWasmCodeGen::xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_XOR);
}

void JitWasmCodeGen::andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmAndNot(m_emitter, dst, src);
}

void JitWasmCodeGen::pandXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_AND);
}

void JitWasmCodeGen::porXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_OR);
}

void JitWasmCodeGen::pxorXmmXmm(SSERegPtr dst, SSERegPtr src) {
    logWasmJitSimdEnabledOnce();
    emitXmmBinarySimdOp(m_emitter, dst, src, WASM_SIMD_V128_XOR);
}

void JitWasmCodeGen::pandnXmmXmm(SSERegPtr dst, SSERegPtr src) {
    emitXmmAndNot(m_emitter, dst, src);
}

void JitWasmCodeGen::movssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::movupdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::movd(RegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_EXTRACT_LANE, 0);
    popToReg(JitWidth::b32, dst);
}

void JitWasmCodeGen::movd(SSERegPtr dst, RegPtr src) {
    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    pushRegValue(src);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::movdq2q(MMXRegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    emitI64ToMmxLocal(dst->hardwareReg());
}

void JitWasmCodeGen::movq2dq(SSERegPtr dst, MMXRegPtr src) {
    emitMmxLocalToI64(src->hardwareReg());
    emitI64ToMmxLocal(dst->hardwareReg());
}

void JitWasmCodeGen::movq(SSERegPtr dst, SSERegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    emitI64ToMmxLocal(dst->hardwareReg());
}

SSERegPtr JitWasmCodeGen::getTmpSSE() {
    return std::shared_ptr<SSERegInternal>(new SSERegInternal((U8)allocV128Scratch(), SSE_TMP_INDEX), [this](SSERegInternal* p) {
        freeV128Scratch(p->hardwareReg());
        delete p;
    });
}

bool JitWasmCodeGen::isSseRegCached(U8 reg) {
    (void)reg;
    return true;
}

void JitWasmCodeGen::storeCpuXMMReg(SSERegPtr reg, U32 index) {
    if (index >= 8) {
        kpanic("WASM storeCpuXMMReg invalid XMM index");
        return;
    }
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitV128Store((U32)(offsetof(CPU, xmm) + index * sizeof(cpu->xmm[0])));
}

SSERegPtr JitWasmCodeGen::loadCpuXMMReg(U8 index) {
    if (index >= 8) {
        kpanic("WASM loadCpuXMMReg invalid XMM index");
        return getTmpSSE();
    }
    SSERegPtr tmp = getTmpSSE();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitV128Load((U32)(offsetof(CPU, xmm) + index * sizeof(cpu->xmm[0])));
    m_emitter.emitLocalSet(tmp->hardwareReg());
    return tmp;
}

SSERegPtr JitWasmCodeGen::loadXMMFromMem128(U8 index, MemPtr address, SSERegPtr result) {
    (void)index;
    if (!result) {
        result = getTmpSSE();
    }
    if (address->emulatedAddress) {
        kpanic("WASM SSE loadXMMFromMem128 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitV128Load(memOffset, 0);
    m_emitter.emitLocalSet(result->hardwareReg());
    return result;
}

SSERegPtr JitWasmCodeGen::loadXMMFromMem32(U8 index, MemPtr address) {
    (void)index;
    SSERegPtr tmp = getTmpSSE();
    if (address->emulatedAddress) {
        kpanic("WASM SSE loadXMMFromMem32 expects a host MemPtr");
    }
    U32 memOffset = 0;
    U32 valueLocal = allocScratch();
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI32Load(memOffset, 0);
    m_emitter.emitLocalSet(valueLocal);
    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalGet(valueLocal);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    freeScratch(valueLocal);
    return tmp;
}

SSERegPtr JitWasmCodeGen::loadXMMFromMem64(U8 index, MemPtr address) {
    (void)index;
    SSERegPtr tmp = getTmpSSE();
    if (address->emulatedAddress) {
        kpanic("WASM SSE loadXMMFromMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI64Load(memOffset, 0);
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    return tmp;
}

SSERegPtr JitWasmCodeGen::loadLowXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = loadCpuXMMReg(index);
    if (address->emulatedAddress) {
        kpanic("WASM SSE loadLowXMMFromMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI64Load(memOffset, 0);
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 0);
    m_emitter.emitLocalSet(reg->hardwareReg());
    return reg;
}

SSERegPtr JitWasmCodeGen::loadHighXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = loadCpuXMMReg(index);
    if (address->emulatedAddress) {
        kpanic("WASM SSE loadHighXMMFromMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI64Load(memOffset, 0);
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_REPLACE_LANE, 1);
    m_emitter.emitLocalSet(reg->hardwareReg());
    return reg;
}

void JitWasmCodeGen::storeXMMToMem128(SSERegPtr reg, MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE storeXMMToMem128 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitV128Store(memOffset, 0);
}

void JitWasmCodeGen::storeXMMToMem64(SSERegPtr reg, MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE storeXMMToMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 0);
    m_emitter.emitI64Store(memOffset, 0);
}

void JitWasmCodeGen::storeXMMToMem32(SSERegPtr reg, MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE storeXMMToMem32 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I32X4_EXTRACT_LANE, 0);
    m_emitter.emitI32Store(memOffset, 0);
}

void JitWasmCodeGen::storeHighXMMToMem64(SSERegPtr reg, MemPtr address) {
    if (address->emulatedAddress) {
        kpanic("WASM SSE storeHighXMMToMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I64X2_EXTRACT_LANE, 1);
    m_emitter.emitI64Store(memOffset, 0);
}

MMXRegPtr JitWasmCodeGen::getTmpMMX() {
    return std::shared_ptr<MMXRegInternal>(new MMXRegInternal((U8)allocV128Scratch(), MMX_TMP_INDEX), [this](MMXRegInternal* p) {
        freeV128Scratch(p->hardwareReg());
        delete p;
    });
}

MMXRegPtr JitWasmCodeGen::loadMMXFromReg(RegPtr reg) {
    MMXRegPtr tmp = getTmpMMX();
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitI64ToMmxLocal(tmp->hardwareReg());
    return tmp;
}

MMXRegPtr JitWasmCodeGen::loadCpuMMXReg(U8 index) {
    MMXRegPtr tmp = getTmpMMX();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI64Load((U32)(index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
    emitI64ToMmxLocal(tmp->hardwareReg());
    return tmp;
}

MMXRegPtr JitWasmCodeGen::loadMMXFromMem32(U8 index, MemPtr address) {
    (void)index;
    MMXRegPtr tmp = getTmpMMX();
    RegPtr value = read(JitWidth::b32, address);
    pushRegValue(value);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitI64ToMmxLocal(tmp->hardwareReg());
    if (canReleaseScratchReg(value))
        freeScratch(value->hardwareReg());
    return tmp;
}

MMXRegPtr JitWasmCodeGen::loadMMXFromMem64(U8 index, MemPtr address) {
    (void)index;
    MMXRegPtr tmp = getTmpMMX();
    // Shared JitMMX reaches this from read(..., customOp), which has already
    // resolved the guest address through the inline TLB to a host MemPtr.
    if (address->emulatedAddress) {
        kpanic("WASM MMX loadMMXFromMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitI64Load(memOffset, 0);
    emitI64ToMmxLocal(tmp->hardwareReg());
    return tmp;
}

void JitWasmCodeGen::storeCpuMMXReg(MMXRegPtr reg, U32 index) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    emitMmxLocalToI64(reg->hardwareReg());
    m_emitter.emitI64Store((U32)(index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
}

void JitWasmCodeGen::storeMMXToReg(MMXRegPtr mmx, RegPtr reg) {
    emitMmxLocalToI64(mmx->hardwareReg());
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    popToReg(JitWidth::b32, reg);
}

void JitWasmCodeGen::storeMMXToMem32(MMXRegPtr reg, MemPtr address) {
    RegPtr tmp = getTmpReg();
    emitMmxLocalToI64(reg->hardwareReg());
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    write(JitWidth::b32, address, tmp);
    freeScratch(tmp->hardwareReg());
}

void JitWasmCodeGen::storeMMXToMem64(MMXRegPtr reg, MemPtr address) {
    // Shared JitMMX reaches this from write(..., customOp), so code-page
    // fallback and cross-page behavior stay owned by the generic write path.
    if (address->emulatedAddress) {
        kpanic("WASM MMX storeMMXToMem64 expects a host MemPtr");
    }
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    emitMmxLocalToI64(reg->hardwareReg());
    m_emitter.emitI64Store(memOffset, 0);
}

void JitWasmCodeGen::xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_V128_XOR);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::orMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_V128_OR);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::andMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_V128_AND);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_V128_ANDNOT);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

#define WASM_MMX_BINARY_SIMD(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(src->hardwareReg()); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_MMX_BINARY_SIMD(pcmpeqbMmxMmx, WASM_SIMD_I8X16_EQ)
WASM_MMX_BINARY_SIMD(pcmpeqwMmxMmx, WASM_SIMD_I16X8_EQ)
WASM_MMX_BINARY_SIMD(pcmpeqdMmxMmx, WASM_SIMD_I32X4_EQ)
WASM_MMX_BINARY_SIMD(pcmpgtbMmxMmx, WASM_SIMD_I8X16_GT_S)
WASM_MMX_BINARY_SIMD(pcmpgtwMmxMmx, WASM_SIMD_I16X8_GT_S)
WASM_MMX_BINARY_SIMD(pcmpgtdMmxMmx, WASM_SIMD_I32X4_GT_S)

#undef WASM_MMX_BINARY_SIMD

#define WASM_MMX_UNPACK(name, ...) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        const U8 lanes[16] = { __VA_ARGS__ }; \
        /* MMX uses only the low 64 bits; these masks place the architectural result there. */ \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(src->hardwareReg()); \
        m_emitter.emitI8x16Shuffle(lanes); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_MMX_UNPACK(punpcklbwMmxMmx, 0, 16, 1, 17, 2, 18, 3, 19, 8, 24, 9, 25, 10, 26, 11, 27)
WASM_MMX_UNPACK(punpcklwdMmxMmx, 0, 1, 16, 17, 2, 3, 18, 19, 8, 9, 24, 25, 10, 11, 26, 27)
WASM_MMX_UNPACK(punpckldqMmxMmx, 0, 1, 2, 3, 16, 17, 18, 19, 8, 9, 10, 11, 24, 25, 26, 27)
WASM_MMX_UNPACK(punpckhbwMmxMmx, 4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31)
WASM_MMX_UNPACK(punpckhwdMmxMmx, 4, 5, 20, 21, 6, 7, 22, 23, 12, 13, 28, 29, 14, 15, 30, 31)
WASM_MMX_UNPACK(punpckhdqMmxMmx, 4, 5, 6, 7, 20, 21, 22, 23, 12, 13, 14, 15, 28, 29, 30, 31)

#undef WASM_MMX_UNPACK

#define WASM_MMX_PACK(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        const U8 lanes[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23 }; \
        /* Narrow after arranging dst.low64 + src.low64, matching MMX pack source order. */ \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(src->hardwareReg()); \
        m_emitter.emitI8x16Shuffle(lanes); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_MMX_PACK(packsswbMmxMmx, WASM_SIMD_I8X16_NARROW_I16X8_S)
WASM_MMX_PACK(packssdwMmxMmx, WASM_SIMD_I16X8_NARROW_I32X4_S)
WASM_MMX_PACK(packuswbMmxMmx, WASM_SIMD_I8X16_NARROW_I16X8_U)

#undef WASM_MMX_PACK

// Variable shifts compare the full 64-bit MMX source; in-range SIMD shifts
// then use the low count. The scalar qword path keeps that count as i64 here.
#define WASM_MMX_SET_ZERO(dst) \
    do { \
        const U8 zero[16] = {}; \
        m_emitter.emitV128Const(zero); \
        m_emitter.emitLocalSet((dst)->hardwareReg()); \
    } while (0)

#define WASM_MMX_LOGICAL_SHIFT_VAR(name, op, overshiftBits) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        emitMmxLocalToI64(src->hardwareReg()); \
        m_emitter.emitLocalSet(WASM_I64_SCRATCH); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitI64Const(overshiftBits); \
        m_emitter.emitOp(WASM_I64_SHR_U); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitOp(WASM_I64_NE); \
        m_emitter.emitIf(); \
        WASM_MMX_SET_ZERO(dst); \
        m_emitter.emitElse(); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitOp(WASM_I32_WRAP_I64); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
        m_emitter.emitEnd(); \
    }

#define WASM_MMX_ARITH_SHIFT_VAR(name, op, maxCount, overshiftBits) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        emitMmxLocalToI64(src->hardwareReg()); \
        m_emitter.emitLocalSet(WASM_I64_SCRATCH); \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitI64Const(overshiftBits); \
        m_emitter.emitOp(WASM_I64_SHR_U); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitOp(WASM_I64_NE); \
        m_emitter.emitIf(WasmType::I32); \
        m_emitter.emitI32Const(maxCount); \
        m_emitter.emitElse(); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitOp(WASM_I32_WRAP_I64); \
        m_emitter.emitEnd(); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

#define WASM_MMX_LOGICAL_SHIFT_IMM(name, op, maxCount) \
    void JitWasmCodeGen::name(MMXRegPtr dst, U32 imm) { \
        if (imm > maxCount) { \
            WASM_MMX_SET_ZERO(dst); \
            return; \
        } \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitI32Const((S32)imm); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

#define WASM_MMX_ARITH_SHIFT_IMM(name, op, maxCount) \
    void JitWasmCodeGen::name(MMXRegPtr dst, U32 imm) { \
        if (imm > maxCount) \
            imm = maxCount; \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitI32Const((S32)imm); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

#define WASM_MMX_Q_LOGICAL_SHIFT_VAR(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        emitMmxLocalToI64(src->hardwareReg()); \
        m_emitter.emitLocalSet(WASM_I64_SCRATCH); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitI64Const(6); \
        m_emitter.emitOp(WASM_I64_SHR_U); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitOp(WASM_I64_NE); \
        m_emitter.emitIf(WasmType::I64); \
        m_emitter.emitI64Const(0); \
        m_emitter.emitElse(); \
        emitMmxLocalToI64(dst->hardwareReg()); \
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); \
        m_emitter.emitOp(op); \
        m_emitter.emitEnd(); \
        emitI64ToMmxLocal(dst->hardwareReg()); \
    }

#define WASM_MMX_Q_LOGICAL_SHIFT_IMM(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, U32 imm) { \
        if (imm > 63) { \
            m_emitter.emitI64Const(0); \
        } else { \
            emitMmxLocalToI64(dst->hardwareReg()); \
            m_emitter.emitI64Const((S64)imm); \
            m_emitter.emitOp(op); \
        } \
        emitI64ToMmxLocal(dst->hardwareReg()); \
    }

WASM_MMX_LOGICAL_SHIFT_VAR(psllwMmxMmx, WASM_SIMD_I16X8_SHL, 4)
WASM_MMX_LOGICAL_SHIFT_VAR(psrlwMmxMmx, WASM_SIMD_I16X8_SHR_U, 4)
WASM_MMX_ARITH_SHIFT_VAR(psrawMmxMmx, WASM_SIMD_I16X8_SHR_S, 15, 4)
WASM_MMX_LOGICAL_SHIFT_IMM(psllwMmx, WASM_SIMD_I16X8_SHL, 15)
WASM_MMX_LOGICAL_SHIFT_IMM(psrlwMmx, WASM_SIMD_I16X8_SHR_U, 15)
WASM_MMX_ARITH_SHIFT_IMM(psrawMmx, WASM_SIMD_I16X8_SHR_S, 15)
WASM_MMX_LOGICAL_SHIFT_VAR(pslldMmxMmx, WASM_SIMD_I32X4_SHL, 5)
WASM_MMX_LOGICAL_SHIFT_VAR(psrldMmxMmx, WASM_SIMD_I32X4_SHR_U, 5)
WASM_MMX_ARITH_SHIFT_VAR(psradMmxMmx, WASM_SIMD_I32X4_SHR_S, 31, 5)
WASM_MMX_LOGICAL_SHIFT_IMM(pslldMmx, WASM_SIMD_I32X4_SHL, 31)
WASM_MMX_LOGICAL_SHIFT_IMM(psrldMmx, WASM_SIMD_I32X4_SHR_U, 31)
WASM_MMX_ARITH_SHIFT_IMM(psradMmx, WASM_SIMD_I32X4_SHR_S, 31)
WASM_MMX_Q_LOGICAL_SHIFT_VAR(psllqMmxMmx, WASM_I64_SHL)
WASM_MMX_Q_LOGICAL_SHIFT_VAR(psrlqMmxMmx, WASM_I64_SHR_U)
WASM_MMX_Q_LOGICAL_SHIFT_IMM(psllqMmx, WASM_I64_SHL)
WASM_MMX_Q_LOGICAL_SHIFT_IMM(psrlqMmx, WASM_I64_SHR_U)

#undef WASM_MMX_Q_LOGICAL_SHIFT_IMM
#undef WASM_MMX_Q_LOGICAL_SHIFT_VAR
#undef WASM_MMX_ARITH_SHIFT_IMM
#undef WASM_MMX_LOGICAL_SHIFT_IMM
#undef WASM_MMX_ARITH_SHIFT_VAR
#undef WASM_MMX_LOGICAL_SHIFT_VAR
#undef WASM_MMX_SET_ZERO

// MMX stores only the low 64-bit architectural result back through JitMMX.
#define WASM_MMX_BINARY_LANE_OP(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(src->hardwareReg()); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_MMX_BINARY_LANE_OP(paddbMmxMmx, WASM_SIMD_I8X16_ADD)
WASM_MMX_BINARY_LANE_OP(paddwMmxMmx, WASM_SIMD_I16X8_ADD)
WASM_MMX_BINARY_LANE_OP(padddMmxMmx, WASM_SIMD_I32X4_ADD)
WASM_MMX_BINARY_LANE_OP(paddsbMmxMmx, WASM_SIMD_I8X16_ADD_SAT_S)
WASM_MMX_BINARY_LANE_OP(paddswMmxMmx, WASM_SIMD_I16X8_ADD_SAT_S)
WASM_MMX_BINARY_LANE_OP(paddusbMmxMmx, WASM_SIMD_I8X16_ADD_SAT_U)
WASM_MMX_BINARY_LANE_OP(padduswMmxMmx, WASM_SIMD_I16X8_ADD_SAT_U)
WASM_MMX_BINARY_LANE_OP(psubbMmxMmx, WASM_SIMD_I8X16_SUB)
WASM_MMX_BINARY_LANE_OP(psubwMmxMmx, WASM_SIMD_I16X8_SUB)
WASM_MMX_BINARY_LANE_OP(psubdMmxMmx, WASM_SIMD_I32X4_SUB)
WASM_MMX_BINARY_LANE_OP(psubsbMmxMmx, WASM_SIMD_I8X16_SUB_SAT_S)
WASM_MMX_BINARY_LANE_OP(psubswMmxMmx, WASM_SIMD_I16X8_SUB_SAT_S)
WASM_MMX_BINARY_LANE_OP(psubusbMmxMmx, WASM_SIMD_I8X16_SUB_SAT_U)
WASM_MMX_BINARY_LANE_OP(psubuswMmxMmx, WASM_SIMD_I16X8_SUB_SAT_U)
WASM_MMX_BINARY_LANE_OP(paddqMmxMmx, WASM_SIMD_I64X2_ADD)
WASM_MMX_BINARY_LANE_OP(psubqMmxMmx, WASM_SIMD_I64X2_SUB)

#undef WASM_MMX_BINARY_LANE_OP

void JitWasmCodeGen::pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I16X8_MUL);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr originalDst = getTmpMMX();

    // JitMMX passes distinct scratch locals for dst/src, even for same-register ops.
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalSet(originalDst->hardwareReg());

    const U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalSet(dst->hardwareReg());

    for (U8 lane = 0; lane < 4; ++lane) {
        m_emitter.emitLocalGet(dst->hardwareReg());
        m_emitter.emitLocalGet(originalDst->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_S, lane);
        m_emitter.emitLocalGet(src->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_S, lane);
        m_emitter.emitOp(WASM_I32_MUL);
        m_emitter.emitI32Const(16);
        m_emitter.emitOp(WASM_I32_SHR_S);
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_REPLACE_LANE, lane);
        m_emitter.emitLocalSet(dst->hardwareReg());
    }
}

void JitWasmCodeGen::pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I32X4_DOT_I16X8_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    emitMmxLocalToI64(dst->hardwareReg());
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitMmxLocalToI64(src->hardwareReg());
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    m_emitter.emitOp(WASM_I64_MUL);
    emitI64ToMmxLocal(dst->hardwareReg());
}

void JitWasmCodeGen::pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_U, srcIndex & 3);
    popToReg(JitWidth::b32, dst);
}

void JitWasmCodeGen::pinsrwMmxReg(MMXRegPtr dst, RegPtr src, U8 dstIndex) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    pushRegValue(src);
    m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_REPLACE_LANE, dstIndex & 3);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_I8X16_BITMASK);
    m_emitter.emitI32Const(0xff);
    m_emitter.emitOp(WASM_I32_AND);
    popToReg(JitWidth::b32, dst);
}

void JitWasmCodeGen::pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 mask) {
    U8 lanes[16] = {};
    for (U8 outWord = 0; outWord < 4; ++outWord) {
        U8 inWord = (mask >> (outWord * 2)) & 3;
        lanes[outWord * 2] = inWord * 2;
        lanes[outWord * 2 + 1] = inWord * 2 + 1;
    }
    for (U8 i = 8; i < 16; ++i) {
        lanes[i] = i;
    }
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitI8x16Shuffle(lanes);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

#define WASM_MMX_SPECIAL_LANE_OP(name, op) \
    void JitWasmCodeGen::name(MMXRegPtr dst, MMXRegPtr src) { \
        m_emitter.emitLocalGet(dst->hardwareReg()); \
        m_emitter.emitLocalGet(src->hardwareReg()); \
        m_emitter.emitSimdOp(op); \
        m_emitter.emitLocalSet(dst->hardwareReg()); \
    }

WASM_MMX_SPECIAL_LANE_OP(pavgbMmxMmx, WASM_SIMD_I8X16_AVGR_U)
WASM_MMX_SPECIAL_LANE_OP(pavgwMmxMmx, WASM_SIMD_I16X8_AVGR_U)
WASM_MMX_SPECIAL_LANE_OP(pmaxswMmxMmx, WASM_SIMD_I16X8_MAX_S)
WASM_MMX_SPECIAL_LANE_OP(pmaxubMmxMmx, WASM_SIMD_I8X16_MAX_U)
WASM_MMX_SPECIAL_LANE_OP(pminswMmxMmx, WASM_SIMD_I16X8_MIN_S)
WASM_MMX_SPECIAL_LANE_OP(pminubMmxMmx, WASM_SIMD_I8X16_MIN_U)

#undef WASM_MMX_SPECIAL_LANE_OP

void JitWasmCodeGen::pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr originalDst = getTmpMMX();

    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalSet(originalDst->hardwareReg());

    const U8 zero[16] = {};
    m_emitter.emitV128Const(zero);
    m_emitter.emitLocalSet(dst->hardwareReg());

    for (U8 lane = 0; lane < 4; ++lane) {
        m_emitter.emitLocalGet(dst->hardwareReg());
        m_emitter.emitLocalGet(originalDst->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_U, lane);
        m_emitter.emitLocalGet(src->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_EXTRACT_LANE_U, lane);
        m_emitter.emitOp(WASM_I32_MUL);
        m_emitter.emitI32Const(16);
        m_emitter.emitOp(WASM_I32_SHR_U);
        m_emitter.emitSimdLaneOp(WASM_SIMD_I16X8_REPLACE_LANE, lane);
        m_emitter.emitLocalSet(dst->hardwareReg());
    }
}

void JitWasmCodeGen::psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    RegPtr sum = getTmpReg();
    RegPtr left = getTmpReg();
    RegPtr right = getTmpReg();

    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(sum->hardwareReg());
    for (U8 lane = 0; lane < 8; ++lane) {
        m_emitter.emitLocalGet(dst->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I8X16_EXTRACT_LANE_U, lane);
        m_emitter.emitLocalSet(left->hardwareReg());
        m_emitter.emitLocalGet(src->hardwareReg());
        m_emitter.emitSimdLaneOp(WASM_SIMD_I8X16_EXTRACT_LANE_U, lane);
        m_emitter.emitLocalSet(right->hardwareReg());

        m_emitter.emitLocalGet(sum->hardwareReg());
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitLocalGet(left->hardwareReg());
        m_emitter.emitLocalGet(right->hardwareReg());
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_SELECT);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitLocalSet(sum->hardwareReg());
    }
    m_emitter.emitLocalGet(sum->hardwareReg());
    m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitI64ToMmxLocal(dst->hardwareReg());

    freeScratch(sum->hardwareReg());
    freeScratch(left->hardwareReg());
    freeScratch(right->hardwareReg());
}

void JitWasmCodeGen::maskmovq(MMXRegPtr src, MMXRegPtr mask, MemPtr destAddress) {
    if (destAddress->emulatedAddress) {
        kpanic("WASM MMX maskmovq expects a host MemPtr");
    }

    MMXRegPtr oldValue = getTmpMMX();
    MMXRegPtr byteMask = getTmpMMX();

    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, destAddress, memOffset);
    m_emitter.emitI64Load(memOffset, 0);
    emitI64ToMmxLocal(oldValue->hardwareReg());

    // Convert each mask byte's high bit to a full 0x00/0xff byte mask.
    m_emitter.emitLocalGet(mask->hardwareReg());
    m_emitter.emitI32Const(7);
    m_emitter.emitSimdOp(WASM_SIMD_I8X16_SHR_S);
    m_emitter.emitLocalSet(byteMask->hardwareReg());

    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalGet(oldValue->hardwareReg());
    m_emitter.emitLocalGet(byteMask->hardwareReg());
    m_emitter.emitSimdOp(WASM_SIMD_V128_BITSELECT);
    m_emitter.emitLocalSet(byteMask->hardwareReg());

    memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, destAddress, memOffset);
    emitMmxLocalToI64(byteMask->hardwareReg());
    m_emitter.emitI64Store(memOffset, 0);
}

static bool canReleaseScratchReg(const RegPtr& reg) {
    return reg && reg->emulatedReg == 0xff && reg.use_count() == 1;
}

FPURegPtr JitWasmCodeGen::getFPUTmp() {
    return std::shared_ptr<FPURegInternal>(new FPURegInternal((U8)allocF64Scratch()), [this](FPURegInternal* p) {
        freeF64Scratch(p->hardwareReg());
        delete p;
    });
}

void JitWasmCodeGen::storeCpuFpuReg(FPURegPtr reg, RegPtr index) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(index);
    m_emitter.emitI32Const(3);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitLocalGet(reg->hardwareReg());
    m_emitter.emitF64Store((U32)offsetof(CPU, fpu.regCache[0].d));
}

void JitWasmCodeGen::loadCpuFpuReg(FPURegPtr reg, RegPtr index) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(index);
    m_emitter.emitI32Const(3);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitF64Load((U32)offsetof(CPU, fpu.regCache[0].d));
    m_emitter.emitLocalSet(reg->hardwareReg());
}

void JitWasmCodeGen::loadCpuFpuRegConst(FPURegPtr reg, U32 offset) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitF64Load(offset);
    m_emitter.emitLocalSet(reg->hardwareReg());
}

void JitWasmCodeGen::cacheFpuReg(U32 regIndex) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)regIndex);
    m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_CACHE_FLOAT);
}

static void emitWasmMemBase(WasmEmitter& emitter, const std::function<void(RegPtr)>& pushReg, MemPtr address, U32& memOffset) {
    memOffset = 0;
    if (address->rm) {
        pushReg(address->rm);
        if (address->sib) {
            pushReg(address->sib);
            if (address->lsl) {
                emitter.emitI32Const((S32)address->lsl);
                emitter.emitOp(WASM_I32_SHL);
            }
            emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        emitter.emitI32Const((S32)address->offset);
    }
}

void JitWasmCodeGen::storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    m_emitter.emitLocalGet(reg->hardwareReg());
    if (width == DYN_FPU_64_BIT) {
        m_emitter.emitF64Store(memOffset);
    } else {
        m_emitter.emitOp(WASM_F32_DEMOTE_F64);
        m_emitter.emitF32Store(memOffset);
    }
}

void JitWasmCodeGen::loadFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    U32 memOffset = 0;
    emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
    if (width == DYN_FPU_64_BIT) {
        m_emitter.emitF64Load(memOffset);
    } else {
        m_emitter.emitF32Load(memOffset);
        m_emitter.emitOp(WASM_F64_PROMOTE_F32);
    }
    m_emitter.emitLocalSet(reg->hardwareReg());
}

void JitWasmCodeGen::loadFpuRegFromInt(FPURegPtr reg, MemPtr address) {
    auto tmp = getTmpReg();
    readHost(JitWidth::b32, address, tmp);
    pushRegValue(tmp);
    m_emitter.emitOp(WASM_F64_CONVERT_I32_S);
    m_emitter.emitLocalSet(reg->hardwareReg());
    freeScratch(tmp->hardwareReg());
}

void JitWasmCodeGen::dynamic_FILD_QWORD_INTEGER(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
        RegPtr topReg = getTopReg();
        dynamic_FPU_PREP_PUSH(topReg, true); // will change topReg

        U32 memOffset = 0;
        emitWasmMemBase(m_emitter, [this](RegPtr reg) { pushRegValue(reg); }, address, memOffset);
        m_emitter.emitI64Load(memOffset);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);

        U32 signExp = allocScratch();
        U32 dist = allocScratch();
        m_emitter.emitI32Const(0);
        m_emitter.emitLocalSet(signExp);

        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Const(0);
        m_emitter.emitOp(WASM_I64_NE);
        m_emitter.emitIf();
        {
            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitI64Const(63);
            m_emitter.emitOp(WASM_I64_SHR_U);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitI32Const(15);
            m_emitter.emitOp(WASM_I32_SHL);
            m_emitter.emitLocalSet(signExp);

            m_emitter.emitLocalGet(signExp);
            m_emitter.emitIf();
            {
                m_emitter.emitI64Const(0);
                m_emitter.emitLocalGet(WASM_I64_SCRATCH);
                m_emitter.emitOp(WASM_I64_SUB);
                m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            }
            m_emitter.emitEnd();

            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I64_CLZ);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitLocalSet(dist);

            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitLocalGet(dist);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_SHL);
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);

            m_emitter.emitLocalGet(signExp);
            m_emitter.emitI32Const(0x403e);
            m_emitter.emitLocalGet(dist);
            m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalSet(signExp);
        }
        m_emitter.emitEnd();

        setRegIsCached(topReg, false);
        shlValue(JitWidth::b32, topReg, 4);

        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        pushRegValue(topReg);
        m_emitter.emitI32Const((S32)offsetof(CPU, fpu.regs[0].signExp));
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitLocalGet(signExp);
        m_emitter.emitI32Store16(0);

        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        pushRegValue(topReg);
        m_emitter.emitI32Const((S32)offsetof(CPU, fpu.regs[0].signif));
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Store(0);

        freeScratch(dist);
        freeScratch(signExp);
    });
}

void JitWasmCodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    if (dst->hardwareReg() == src->hardwareReg()) {
        return;
    }
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    if (dst->hardwareReg() == src->hardwareReg()) {
        return;
    }
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F32_DEMOTE_F64);
    m_emitter.emitOp(WASM_F64_PROMOTE_F32);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

RegPtr JitWasmCodeGen::fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) {
    auto result = getTmpReg();
    if (truncate) {
        m_emitter.emitLocalGet(fpuRegSrc->hardwareReg());
    } else {
        U32 rounded = allocF64Scratch();
        RegPtr round = readCPU(JitWidth::b32, (U32)offsetof(CPU, fpu.round));
        IfNot(JitWidth::b32, round); {
            m_emitter.emitLocalGet(fpuRegSrc->hardwareReg());
            m_emitter.emitOp(WASM_F64_NEAREST);
            m_emitter.emitLocalSet(rounded);
        } StartElse(); {
            IfEqual(JitWidth::b32, round, ROUND_Down); {
                m_emitter.emitLocalGet(fpuRegSrc->hardwareReg());
                m_emitter.emitOp(WASM_F64_FLOOR);
                m_emitter.emitLocalSet(rounded);
            } StartElse(); {
                IfEqual(JitWidth::b32, round, ROUND_Up); {
                    m_emitter.emitLocalGet(fpuRegSrc->hardwareReg());
                    m_emitter.emitOp(WASM_F64_CEIL);
                    m_emitter.emitLocalSet(rounded);
                } StartElse(); {
                    m_emitter.emitLocalGet(fpuRegSrc->hardwareReg());
                    m_emitter.emitOp(WASM_F64_TRUNC);
                    m_emitter.emitLocalSet(rounded);
                } EndIf();
            } EndIf();
        } EndIf();
        m_emitter.emitLocalGet(rounded);
        freeF64Scratch(rounded);
    }
    U32 rounded = allocF64Scratch();
    m_emitter.emitLocalSet(rounded);
    emitWasmF64ToI32OrIndefinite(m_emitter, rounded);
    freeF64Scratch(rounded);
    m_emitter.emitLocalSet(result->hardwareReg());
    return result;
}

void JitWasmCodeGen::regToFpuReg(FPURegPtr dst, RegPtr src) {
    pushRegValue(src);
    m_emitter.emitOp(WASM_F64_CONVERT_I32_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

#ifdef BOXEDWINE_64
void JitWasmCodeGen::regToFpuReg64(FPURegPtr dst, RegPtr src) {
    pushRegValue(src);
    m_emitter.emitOp(WASM_F64_CONVERT_I64_S);
    m_emitter.emitLocalSet(dst->hardwareReg());
}
#endif

void JitWasmCodeGen::updateFPURounding() {
}

void JitWasmCodeGen::restoreFPURounding() {
}

void JitWasmCodeGen::roundFPUToInt64(FPURegPtr src) {
    RegPtr round = readCPU(JitWidth::b32, (U32)offsetof(CPU, fpu.round));
    IfNot(JitWidth::b32, round); {
        m_emitter.emitLocalGet(src->hardwareReg());
        m_emitter.emitOp(WASM_F64_NEAREST);
        m_emitter.emitLocalSet(src->hardwareReg());
    } StartElse(); {
        IfEqual(JitWidth::b32, round, ROUND_Down); {
            m_emitter.emitLocalGet(src->hardwareReg());
            m_emitter.emitOp(WASM_F64_FLOOR);
            m_emitter.emitLocalSet(src->hardwareReg());
        } StartElse(); {
            IfEqual(JitWidth::b32, round, ROUND_Up); {
                m_emitter.emitLocalGet(src->hardwareReg());
                m_emitter.emitOp(WASM_F64_CEIL);
                m_emitter.emitLocalSet(src->hardwareReg());
            } StartElse(); {
                m_emitter.emitLocalGet(src->hardwareReg());
                m_emitter.emitOp(WASM_F64_TRUNC);
                m_emitter.emitLocalSet(src->hardwareReg());
            } EndIf();
        } EndIf();
    } EndIf();
}

void JitWasmCodeGen::storeFPUToInt64(FPURegPtr src, MemPtr address, bool truncate) {
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    if (!truncate) {
        roundFPUToInt64(src);
    }
    m_emitter.emitLocalGet(src->hardwareReg());
    U32 rounded = allocF64Scratch();
    m_emitter.emitLocalSet(rounded);
    emitWasmF64ToI64OrIndefinite(m_emitter, rounded);
    freeF64Scratch(rounded);
    m_emitter.emitI64Store(memOffset);
}

void JitWasmCodeGen::fpuAdd(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F64_ADD);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuMul(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F64_MUL);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuSub(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F64_SUB);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuDiv(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F64_DIV);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuXor(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitOp(WASM_I64_XOR);
    m_emitter.emitOp(WASM_F64_REINTERPRET_I64);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuAnd(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(dst->hardwareReg());
    m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitOp(WASM_I64_AND);
    m_emitter.emitOp(WASM_F64_REINTERPRET_I64);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuSqrt(FPURegPtr dst, FPURegPtr src) {
    m_emitter.emitLocalGet(src->hardwareReg());
    m_emitter.emitOp(WASM_F64_SQRT);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

RegPtr JitWasmCodeGen::fcompareResult(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags) {
    auto result = getTmpReg();

    subValue(JitWidth::b8, ordTags, TAG_Empty);

    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitI32Const(1);
    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalGet(fpuReg2->hardwareReg());
    m_emitter.emitLocalGet(fpuReg1->hardwareReg());
    m_emitter.emitOp(WASM_F64_LT);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitI32Const(2);
    m_emitter.emitLocalGet(result->hardwareReg());
    m_emitter.emitLocalGet(fpuReg2->hardwareReg());
    m_emitter.emitLocalGet(fpuReg1->hardwareReg());
    m_emitter.emitOp(WASM_F64_EQ);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result->hardwareReg());

    m_emitter.emitI32Const(3);
    m_emitter.emitLocalGet(result->hardwareReg());
    pushRegValue(ordTags);
    maskToWidth(JitWidth::b8);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalGet(fpuReg1->hardwareReg());
    m_emitter.emitLocalGet(fpuReg1->hardwareReg());
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalGet(fpuReg2->hardwareReg());
    m_emitter.emitLocalGet(fpuReg2->hardwareReg());
    m_emitter.emitOp(WASM_F64_NE);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(result->hardwareReg());

    return result;
}

void JitWasmCodeGen::emitSelectI32ByFCompareResult(RegPtr compare, U32 greater, U32 less, U32 equal, U32 invalid) {
    U32 selected = allocScratch();

    m_emitter.emitI32Const((S32)greater);
    m_emitter.emitLocalSet(selected);

    m_emitter.emitI32Const((S32)less);
    m_emitter.emitLocalGet(selected);
    pushRegValue(compare);
    m_emitter.emitI32Const(1);
    m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(selected);

    m_emitter.emitI32Const((S32)equal);
    m_emitter.emitLocalGet(selected);
    pushRegValue(compare);
    m_emitter.emitI32Const(2);
    m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitLocalSet(selected);

    m_emitter.emitI32Const((S32)invalid);
    m_emitter.emitLocalGet(selected);
    pushRegValue(compare);
    m_emitter.emitI32Const(3);
    m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitOp(WASM_SELECT);

    freeScratch(selected);
}

void JitWasmCodeGen::doFCOM(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags) {
    RegPtr compare = fcompareResult(fpuReg1, fpuReg2, ordTags);
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));

    pushRegValue(sw);
    emitSelectI32ByFCompareResult(compare, ~0x4700, ~0x4600, ~0x0700, ~0x0200);
    m_emitter.emitOp(WASM_I32_AND);
    emitSelectI32ByFCompareResult(compare, 0, 0x0100, 0x4000, 0x4500);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(sw->hardwareReg());

    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
    freeScratch(compare->hardwareReg());
}

void JitWasmCodeGen::doFCOMI(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags) {
    RegPtr compare = fcompareResult(fpuReg1, fpuReg2, ordTags);
    RegPtr flags = readCPU(JitWidth::b32, offsetof(CPU, flags));

    pushRegValue(flags);
    m_emitter.emitI32Const((S32)~WASM_FMASK_TEST);
    m_emitter.emitOp(WASM_I32_AND);
    emitSelectI32ByFCompareResult(compare, 0, CF, ZF, CF | PF | ZF);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(flags->hardwareReg());

    writeCPU(JitWidth::b32, offsetof(CPU, flags), flags);
    storeLazyFlagType(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
    freeScratch(compare->hardwareReg());
}

void JitWasmCodeGen::fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags,
                              const std::function<void()>& pfnEqual,
                              const std::function<void()>& pfnLessThan,
                              const std::function<void()>& pfnGreaterThan,
                              const std::function<void()>& pfnInvalid) {
    subValue(JitWidth::b8, ordTags, TAG_Empty);
    IfNot(JitWidth::b8, ordTags); {
        pfnInvalid();
    } StartElse(); {
        branchBoundary();
        m_emitter.emitLocalGet(fpuReg1->hardwareReg());
        m_emitter.emitLocalGet(fpuReg1->hardwareReg());
        m_emitter.emitOp(WASM_F64_NE);
        m_emitter.emitLocalGet(fpuReg2->hardwareReg());
        m_emitter.emitLocalGet(fpuReg2->hardwareReg());
        m_emitter.emitOp(WASM_F64_NE);
        m_emitter.emitOp(WASM_I32_OR);
        finishIf(); {
            pfnInvalid();
        } StartElse(); {
            branchBoundary();
            m_emitter.emitLocalGet(fpuReg2->hardwareReg());
            m_emitter.emitLocalGet(fpuReg1->hardwareReg());
            m_emitter.emitOp(WASM_F64_EQ);
            finishIf(); {
                pfnEqual();
            } StartElse(); {
                branchBoundary();
                m_emitter.emitLocalGet(fpuReg2->hardwareReg());
                m_emitter.emitLocalGet(fpuReg1->hardwareReg());
                m_emitter.emitOp(WASM_F64_LT);
                finishIf(); {
                    pfnLessThan();
                } StartElse(); {
                    pfnGreaterThan();
                } EndIf();
            } EndIf();
        } EndIf();
    } EndIf();
}

// Emit a masked value on the WASM stack.
void JitWasmCodeGen::maskToWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:
        m_emitter.emitI32Const(0xff);
        m_emitter.emitOp(WASM_I32_AND);
        break;
    case JitWidth::b16:
        m_emitter.emitI32Const(0xffff);
        m_emitter.emitOp(WASM_I32_AND);
        break;
    case JitWidth::b32:
    case JitWidth::b64:
    default:
        break; // no masking needed for 32/64-bit
    }
}

// Helper: emit a binary operation that reads dst, applies op with src, writes dst.
void JitWasmCodeGen::emitBinOp(JitWidth w, RegPtr dst, RegPtr src, U8 wasmOp32, U8 wasmOp64) {
    // Stack: push dst, push src, op, (mask), pop dst
    pushRegValue(dst);
    if (w == JitWidth::b8 || w == JitWidth::b16) {
        maskToWidth(w);
    }
    pushRegValue(src);
    if (w == JitWidth::b8 || w == JitWidth::b16) {
        maskToWidth(w);
    }
    m_emitter.emitOp(wasmOp32);
    maskToWidth(w);
    popToReg(w, dst);
}

void JitWasmCodeGen::emitBinOpImm(JitWidth w, RegPtr dst, U32 imm, U8 wasmOp32) {
    pushRegValue(dst);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitOp(wasmOp32);
    maskToWidth(w);
    popToReg(w, dst);
}

// ---------------------------------------------------------------------------
// Register management
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (load) loadGPReg(reg);
    auto r = makeWasmReg((U8)wasmLocalForGPReg(reg), reg);
    m_gpDirty[reg] = true;
    return r;
}

RegPtr JitWasmCodeGen::getReg8(U8 reg, bool load) {
    // x86 encoding: 0-3 = AL/CL/DL/BL (low byte of reg[0-3]),
    //               4-7 = AH/CH/DH/BH (high byte of reg[0-3]).
    bool isHigh = (reg >= 4);
    U8 emReg    = isHigh ? (U8)(reg - 4) : reg;
    if (load) loadGPReg(emReg);
    auto r = makeWasmReg((U8)wasmLocalForGPReg(emReg), emReg, isHigh);
    m_gpDirty[emReg] = true;
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (!delayed) loadGPReg(reg);
    return makeWasmReg((U8)wasmLocalForGPReg(reg), reg);
}

RegPtr JitWasmCodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    bool isHigh = (reg >= 4);
    U8 emReg    = isHigh ? (U8)(reg - 4) : reg;
    if (!delayed) loadGPReg(emReg);
    return makeWasmReg((U8)wasmLocalForGPReg(emReg), emReg, isHigh);
}

RegPtr JitWasmCodeGen::getTmpReg() {
    U32 local = allocScratch();
    return makeWasmReg((U8)local, 0xff);
}

RegPtr JitWasmCodeGen::getTmpReg8()                  { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpRegWithHint(S8 hint)    { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpRegForCallResult()      { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {
    auto r = getTmpReg();
    if (!delayed) {
        loadGPReg(reg);
        // copy: push local, pop to tmp
        m_emitter.emitLocalGet(wasmLocalForGPReg(reg));
        m_emitter.emitLocalSet(r->hardwareReg());
    }
    return r;
}
RegPtr JitWasmCodeGen::getTmpReg8(U8 reg, bool delayed, S8 hint) {
    auto r = getTmpReg();
    if (!delayed) {
        bool isHigh = (reg >= 4);
        U8 emReg = isHigh ? (U8)(reg - 4) : reg;
        loadGPReg(emReg);
        m_emitter.emitLocalGet(wasmLocalForGPReg(emReg));
        if (isHigh) {
            m_emitter.emitI32Const(8);
            m_emitter.emitOp(WASM_I32_SHR_U);
        }
        m_emitter.emitI32Const(0xff);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalSet(r->hardwareReg());
    }
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlySegAddress(U8 seg) {
    U32 local = WASM_SEG_LOCAL_BASE + (seg & 3);
    if (!m_segLoaded[seg & 3]) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load(CPU::offsetofSegAddress(seg));
        m_emitter.emitLocalSet(local);
        m_segLoaded[seg & 3] = true;
    }
    return makeWasmReg((U8)local, 0xfe);
}

RegPtr JitWasmCodeGen::getTmpSegAddress(U8 seg) {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(CPU::offsetofSegAddress(seg));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlySegValue(U8 seg) {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(CPU::offsetofSegValue(seg));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

RegPtr JitWasmCodeGen::getStringRegEcx() { return getReg(1); }
RegPtr JitWasmCodeGen::getStringRegEsi() { return getReg(6); }
RegPtr JitWasmCodeGen::getStringRegEdi() { return getReg(7); }

bool JitWasmCodeGen::isTmpRegAvailable() {
    for (U32 i = 0; i < WASM_TMP_LOCAL_COUNT; i++)
        if (!m_scratchInUse[i]) return true;
    return false;
}

void JitWasmCodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (!reg) return;
    U8 emReg = reg->emulatedReg;
    if (emReg < WASM_GP_LOCAL_COUNT && m_gpDirty[emReg]) {
        storeGPReg(emReg);
        m_gpDirty[emReg] = false;
    }
}

// ---------------------------------------------------------------------------
// EIP
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::readEip() {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, eip.u32));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

void JitWasmCodeGen::writeEip(RegPtr eip) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(eip);
    m_emitter.emitI32Store((U32)offsetof(CPU, eip.u32));
}

void JitWasmCodeGen::writeEip(U32 eip) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)eip);
    m_emitter.emitI32Store((U32)offsetof(CPU, eip.u32));
}

// ---------------------------------------------------------------------------
// CPU state read/write (JitCodeGen pure virtuals)
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::readCPU(JitWidth w, U32 offset, RegPtr res) {
    if (!res) res = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(offset); break;
    default:            m_emitter.emitI32Load(offset);     break;
    }
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

RegPtr JitWasmCodeGen::readCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr res) {
    // Compute address: sib << lsl + offset, add to cpu_ptr
    if (!res) res = getTmpReg();
    U32 scratch = allocScratch();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(sib);
    if (lsl) {
        m_emitter.emitI32Const(lsl);
        m_emitter.emitOp(WASM_I32_SHL);
    }
    m_emitter.emitI32Const((S32)offset);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(0);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(0); break;
    default:            m_emitter.emitI32Load(0);    break;
    }
    m_emitter.emitLocalSet(res->hardwareReg());
    freeScratch(scratch);
    return res;
}

void JitWasmCodeGen::writeCPU(JitWidth w, U32 offset, RegPtr src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(offset); break;
    default:            m_emitter.emitI32Store(offset);    break;
    }
}

void JitWasmCodeGen::writeCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(sib);
    if (lsl) {
        m_emitter.emitI32Const(lsl);
        m_emitter.emitOp(WASM_I32_SHL);
    }
    m_emitter.emitI32Const((S32)offset);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(0); break;
    default:            m_emitter.emitI32Store(0);    break;
    }
}

void JitWasmCodeGen::writeCPUValue(JitWidth w, U32 offset, DYN_PTR_SIZE src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(offset); break;
    default:            m_emitter.emitI32Store(offset);    break;
    }
}

void JitWasmCodeGen::writeCPUValue(JitWidth w, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)src);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    writeCPU(w, sib, lsl, offset, tmp);
    freeScratch(tmp->hardwareReg());
}

// MMU read: get host page base pointer for emulated memory page index
void JitWasmCodeGen::readMMU(RegPtr dest, RegPtr index, U32 offset) {
    // Reads a 4-byte value at `index + offset` in emulated memory via the
    // dedicated memHelperAddr/memHelperValue scratch fields so that lazy
    // flag state survives the call.
    auto r = dest ? dest : getTmpReg();
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), index);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_READ_MEM32);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalSet(r->hardwareReg());
}

void JitWasmCodeGen::readMMU(RegPtr dest, U32 index) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)index);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    readMMU(dest, tmp, 0);
    freeScratch(tmp->hardwareReg());
}

// readHost/writeHost access WASM linear memory directly (host-side C++
// pointers live there). The WASM memarg offset is ADDED to the pushed
// address, so both the dynamic base (`rm`) and the static displacement
// (`offset`) must flow through correctly. Previously we passed 0 as the
// load/store offset, which silently discarded the MemPtr's offset field
// for rm-based addressing (e.g. readHost(op + offsetof(DecodedOp, next))).
void JitWasmCodeGen::readHost(JitWidth w, MemPtr address, RegPtr result, bool emulatedMemory) {
    if (!result) result = getTmpReg();
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(memOffset); break;
    default:            m_emitter.emitI32Load(memOffset);     break;
    }
    m_emitter.emitLocalSet(result->hardwareReg());
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, RegPtr src, bool emulatedMemory) {
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(memOffset); break;
    default:            m_emitter.emitI32Store(memOffset);    break;
    }
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, U32 imm, bool emulatedMemory) {
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    m_emitter.emitI32Const((S32)imm);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(memOffset); break;
    default:            m_emitter.emitI32Store(memOffset);    break;
    }
}

void JitWasmCodeGen::clearMMUPermissionIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) {
    // No-op for WASM; memory safety is handled by the WASM runtime itself.
}
void JitWasmCodeGen::clearIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) {
    // No-op for WASM.
}

// ---------------------------------------------------------------------------
// Emulated memory read/write (with softMMU)
//
// Plain reads and writes use an inline TLB fast path: load
// `wasmReadPageBase[addr>>12]` (or wasmWritePageBase for writes),
// branch on zero or width-aware boundary cross to the existing helper
// call, otherwise direct i32.load/store from `entry + (addr & MASK)`.
// CodePages have wasmWritePageBase==0 so writes that could hit JIT'd
// code always slow-path through the bailout-checking helper.
//
// The RMW path (readWriteMem, used by add/and/or/xchg/bts/btr/btc and
// shift-of-memory ops) uses the writable-page array for both its load
// and store. A zero entry covers code pages, protected/MMIO memory, and
// other special mappings, so those accesses retain the helper path and
// its self-modifying-code bailout checks.
// ---------------------------------------------------------------------------
// Helper-import index for emulated-memory read/write of the given width.
// Picking the wrong width corrupts neighboring bytes: writeMem32 on a
// 16-bit push would clobber the two bytes above the stack slot.
static U32 wasmJitWidthBytes(JitWidth w) {
    switch (w) {
    case JitWidth::b8:   return 1;
    case JitWidth::b16:  return 2;
    case JitWidth::b32:  return 4;
    case JitWidth::b64:  return 8;
    case JitWidth::b128: return 16;
    case JitWidth::b256: return 32;
    }
    kpanic("WASM JIT unsupported JitWidth");
    return K_PAGE_SIZE;
}

static U32 readHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_READ_MEM8;
    case JitWidth::b16: return HELPER_READ_MEM16;
    default:            return HELPER_READ_MEM32;
    }
}
static U32 writeHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_WRITE_MEM8;
    case JitWidth::b16: return HELPER_WRITE_MEM16;
    default:            return HELPER_WRITE_MEM32;
    }
}

// Index of the bailout-checking write helper for a given width. The
// inline-RMW path (readWriteMem) and direct-write path (write) end with a
// helper call followed by an `if (cpu->wasmJitBailout) blockExit()` check
// — only when the write potentially lands on the active block's code page.
static U32 writeCheckHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_WRITE_MEM8_CHECK;
    case JitWidth::b16: return HELPER_WRITE_MEM16_CHECK;
    default:            return HELPER_WRITE_MEM32_CHECK;
    }
}

// Reserve slot 0 (the block-start DecodedOp*, read by the SMC-arming code)
// the first time any reloc site is emitted.
void JitWasmCodeGen::ensureRelocSlot0() {
    if (m_relocValues.empty()) {
        m_relocValues.push_back((U32)(uintptr_t)m_wasmBlockStartOp);
    }
}

// Append a relocation slot holding `value` (resolved against the current
// session's decoded ops) and return its byte offset for use as an i32.load
// memarg offset off the relocBase parameter.
U32 JitWasmCodeGen::addRelocSlot(U32 value) {
    ensureRelocSlot0();
    U32 offset = (U32)(m_relocValues.size() * sizeof(U32));
    m_relocValues.push_back(value);
    return offset;
}

void JitWasmCodeGen::emitArmSmcBailout() {
    if (wasmJitPersistenceActive()) {
        // Relocatable modules read the block-start DecodedOp* from reloc
        // slot 0 at runtime instead of embedding it. When the caller passed
        // no slot array (relocBase == 0, e.g. an unresolved direct-call
        // edge in an offline-merged group), fall back to pre-arming
        // wasmJitBailout: every slow-path write then exits the block
        // conservatively (correct for SMC; only slow-path writes pay).
        // No scratch local: the write paths calling this run with the
        // pool nearly exhausted, so re-read the param instead of tee'ing.
        // Neither arm touches GP locals, so no tracker save/restore needed.
        ensureRelocSlot0();
        m_emitter.emitLocalGet(WASM_RELOC_LOCAL);
        m_emitter.emitIf();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(WASM_RELOC_LOCAL);
        m_emitter.emitI32Load(0);                  // slot 0 = block-start op
        m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitActiveBlock));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const(0);
        m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitBailout));
        m_emitter.emitElse();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const(0);
        m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitActiveBlock));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const(1);
        m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitBailout));
        m_emitter.emitEnd();
        return;
    }
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((U32)(uintptr_t)m_wasmBlockStartOp);
    m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitActiveBlock));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const(0);
    m_emitter.emitI32Store((U32)offsetof(CPU, wasmJitBailout));
}

// Self-modifying-code bailout check: emitted after every JIT-inline mem
// write. If the write hit the active block's bytes, the helper set
// cpu->wasmJitBailout. We exit by writing the *next* op's offset to
// cpu->eip and calling blockExit so the dispatcher resumes there with a
// fresh decode.
//
// The if-body's blockExit() runs syncDirtyRegsToHost which compile-time-
// clears m_gpDirty[]; that's wrong for the no-bailout path where the
// body didn't run. Save/restore the tracker around the emit.
void JitWasmCodeGen::emitBailoutCheck() {
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmJitBailout));
    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    // currentEip is the *current* op's start (postCompile bumps it after
    // compile returns), so add op->len = lastCompiledOpLen to land at the
    // next op so the dispatcher resumes there.
    writeEip(this->currentEip + this->lastCompiledOpLen
             - cpu->seg[CS].address);
    blockExit();
    m_emitter.emitEnd();
    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
}

// Store `reg` into cpu->memHelperAddr (our dedicated mem-helper scratch).
// Also used for memHelperValue by varying the offset.
void JitWasmCodeGen::storeMemHelperField(U32 offset, RegPtr reg) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(reg);
    m_emitter.emitI32Store(offset);
}

RegPtr JitWasmCodeGen::readWriteMem(JitWidth w, RegPtr addressReg,
                                     std::function<void(RegPtr)> prepareWrite, S8 hint) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    emitProfileSampledCall(HELPER_PROFILE_RMW,
        m_currentWasmOp ? (U32)m_currentWasmOp->inst : InstructionCount);
#endif
    m_needsWasmMemoryPageArrays = true;
    auto result = getTmpReg();
    U32 entryLocal = allocScratch();

    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    // A writable-page entry is also readable and points directly at the
    // page's bytes. Mark cross-page accesses as slow by clearing the entry
    // in that arm; the same test can then select the write path below.
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmWritePageBaseArray));
    pushRegValue(addressReg);
    m_emitter.emitI32Const(K_PAGE_SHIFT);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(2);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);
    m_emitter.emitLocalTee(entryLocal);
    m_emitter.emitOp(WASM_I32_EQZ);

    if (w != JitWidth::b8) {
        U32 widthBytes = wasmJitWidthBytes(w);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    {
        m_emitter.emitI32Const(0);
        m_emitter.emitLocalSet(entryLocal);
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        syncStateBeforeFaultingMemoryHelper();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(readHelperForWidth(w));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
        m_emitter.emitLocalSet(result->hardwareReg());
    }
    m_emitter.emitElse();
    {
        m_emitter.emitLocalGet(entryLocal);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_ADD);
        switch (w) {
        case JitWidth::b8:  m_emitter.emitI32Load8U(0);  break;
        case JitWidth::b16: m_emitter.emitI32Load16U(0); break;
        default:            m_emitter.emitI32Load(0, 0); break;
        }
        m_emitter.emitLocalSet(result->hardwareReg());
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;

    if (prepareWrite) {
        prepareWrite(result);

        savedGpDirty   = m_gpDirty;
        savedGpLoaded  = m_gpLoaded;
        savedSegLoaded = m_segLoaded;

        m_emitter.emitLocalGet(entryLocal);
        m_emitter.emitOp(WASM_I32_EQZ);
        m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
        m_emitter.emitIf();
        {
            storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
            storeMemHelperField((U32)offsetof(CPU, memHelperValue), result);
            syncStateBeforeFaultingMemoryHelper();
            m_emitter.emitLocalGet(WASM_CPU_LOCAL);
            emitArmSmcBailout();
            m_emitter.emitCall(writeCheckHelperForWidth(w));
            emitBailoutCheck();
        }
        m_emitter.emitElse();
        {
            m_emitter.emitLocalGet(entryLocal);
            pushRegValue(addressReg);
            m_emitter.emitI32Const(K_PAGE_MASK);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_ADD);
            pushRegValue(result);
            switch (w) {
            case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
            case JitWidth::b16: m_emitter.emitI32Store16(0); break;
            default:            m_emitter.emitI32Store(0, 0); break;
            }
        }
        m_emitter.emitEnd();

        m_gpDirty   = savedGpDirty;
        m_gpLoaded  = savedGpLoaded;
        m_segLoaded = savedSegLoaded;
    }
    freeScratch(entryLocal);
    if (canReleaseScratchReg(addressReg))
        freeScratch(addressReg->hardwareReg());
    return result;
}

// Inline TLB fast path for emulated reads. Compiles down to (in pseudo-WASM):
//
//   entry = wasmReadPageBase[addr >> 12]
//   if (entry == 0 || (addr & 0xfff) > 0x1000 - width) {
//       // slow: existing helper
//       memHelperAddr = addr; call read_helper(cpu); tmp = memHelperValue;
//   } else {
//       tmp = i32.load{8u,16u,_}(entry + addr)
//   }
//
// The boundary check is omitted for b8 (every byte is in some page).
// Save/restore the dirty/loaded trackers around the if since only one
// arm runs and `if`/`end` is treated as a branch boundary structurally
// even though neither branch invalidates the JIT register file.
RegPtr JitWasmCodeGen::read(JitWidth w, RegPtr addressReg,
                             std::function<void(MemPtr)> customOp,
                             std::function<void()> failedOp,
                             RegPtr tmp, bool checkAlignment) {
    if (!tmp) tmp = getTmpReg();
    if (customOp) {
        m_needsWasmMemoryPageArrays = true;
        U32 entryLocal = allocScratch();
        U32 hostLocal = allocScratch();
        auto savedGpDirty   = m_gpDirty;
        auto savedGpLoaded  = m_gpLoaded;
        auto savedSegLoaded = m_segLoaded;

        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, wasmReadPageBaseArray));
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_SHIFT);
        m_emitter.emitOp(WASM_I32_SHR_U);
        m_emitter.emitI32Const(2);
        m_emitter.emitOp(WASM_I32_SHL);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitI32Load(0);
        m_emitter.emitLocalTee(entryLocal);
        m_emitter.emitOp(WASM_I32_EQZ);

        if (w != JitWidth::b8) {
            U32 widthBytes = wasmJitWidthBytes(w);
            pushRegValue(addressReg);
            m_emitter.emitI32Const(K_PAGE_MASK);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
            m_emitter.emitOp(WASM_I32_GT_U);
            m_emitter.emitOp(WASM_I32_OR);
        }

        m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
        m_emitter.emitIf();
        {
            if (failedOp) {
                failedOp();
            } else {
                emulateSingleOp();
            }
        }
        m_emitter.emitElse();
        {
            m_emitter.emitLocalGet(entryLocal);
            pushRegValue(addressReg);
            m_emitter.emitI32Const(K_PAGE_MASK);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_ADD);
            m_emitter.emitLocalSet(hostLocal);
            customOp(createMemPtr(makeWasmReg((U8)hostLocal, 0xff), 0, false));
        }
        m_emitter.emitEnd();

        m_gpDirty   = savedGpDirty;
        m_gpLoaded  = savedGpLoaded;
        m_segLoaded = savedSegLoaded;
        freeScratch(hostLocal);
        freeScratch(entryLocal);
        if (canReleaseScratchReg(addressReg))
            freeScratch(addressReg->hardwareReg());
        return tmp;
    }

    m_needsWasmMemoryPageArrays = true;
    U32 entryLocal = allocScratch();
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    // Push: missCondition (entry == 0 [|| boundary])
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmReadPageBaseArray));
    pushRegValue(addressReg);
    m_emitter.emitI32Const(K_PAGE_SHIFT);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(2);                  // sizeof(U32)
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);                   // entry = wasmReadPageBase[page]
    m_emitter.emitLocalTee(entryLocal);
    m_emitter.emitOp(WASM_I32_EQZ);

    if (w != JitWidth::b8) {
        U32 widthBytes = wasmJitWidthBytes(w);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    {
        // Slow path: existing helper-based load.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        syncStateBeforeFaultingMemoryHelper();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(readHelperForWidth(w));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
        m_emitter.emitLocalSet(tmp->hardwareReg());
    }
    m_emitter.emitElse();
    {
        // Fast path: tmp = i32.load{8u,16u,_}(entry + (addr & K_PAGE_MASK))
        // entry holds the raw host base of the page; mask the address to
        // get the in-page offset before adding (we don't use the
        // (host_base - virt_page_base) trick the host-exception readCache
        // does because under WASM the no-access sentinel can't be a low
        // address that traps).
        m_emitter.emitLocalGet(entryLocal);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_ADD);
        switch (w) {
        case JitWidth::b8:  m_emitter.emitI32Load8U(0);  break;
        case JitWidth::b16: m_emitter.emitI32Load16U(0); break;
        default:            m_emitter.emitI32Load(0, 0); break; // align=0 (unaligned-safe)
        }
        m_emitter.emitLocalSet(tmp->hardwareReg());
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
    freeScratch(entryLocal);
    if (canReleaseScratchReg(addressReg))
        freeScratch(addressReg->hardwareReg());
    return tmp;
}

// Inline TLB fast path for emulated writes. Mirrors read():
//
//   entry = wasmWritePageBase[addr >> 12]
//   if (entry == 0 || (addr & 0xfff) > 0x1000 - width) {
//       slow: bailout-checking helper + emitBailoutCheck()
//   } else {
//       i32.store{8,16,}(entry + (addr & K_PAGE_MASK), src)
//   }
//
// Crucially, CodePage::canWriteRam returns false, so
// wasmWritePageBase[CodePage] == 0 → all writes to JIT'd code take the
// slow path, which preserves the SMC bailout. Plain RWPage writes can
// take the fast path because they can't be on an active block.
void JitWasmCodeGen::write(JitWidth w, RegPtr addressReg, RegPtr src,
                            std::function<void(MemPtr)> customOp,
                            std::function<void()> failedOp, bool checkAlignment) {
    if (customOp) {
        m_needsWasmMemoryPageArrays = true;
        U32 entryLocal = allocScratch();
        U32 hostLocal = allocScratch();
        auto savedGpDirty   = m_gpDirty;
        auto savedGpLoaded  = m_gpLoaded;
        auto savedSegLoaded = m_segLoaded;

        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, wasmWritePageBaseArray));
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_SHIFT);
        m_emitter.emitOp(WASM_I32_SHR_U);
        m_emitter.emitI32Const(2);
        m_emitter.emitOp(WASM_I32_SHL);
        m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitI32Load(0);
        m_emitter.emitLocalTee(entryLocal);
        m_emitter.emitOp(WASM_I32_EQZ);

        if (w != JitWidth::b8) {
            U32 widthBytes = wasmJitWidthBytes(w);
            pushRegValue(addressReg);
            m_emitter.emitI32Const(K_PAGE_MASK);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
            m_emitter.emitOp(WASM_I32_GT_U);
            m_emitter.emitOp(WASM_I32_OR);
        }

        m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
        m_emitter.emitIf();
        {
            if (failedOp) {
                failedOp();
            } else {
                emulateSingleOp();
            }
        }
        m_emitter.emitElse();
        {
            m_emitter.emitLocalGet(entryLocal);
            pushRegValue(addressReg);
            m_emitter.emitI32Const(K_PAGE_MASK);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_ADD);
            m_emitter.emitLocalSet(hostLocal);
            customOp(createMemPtr(makeWasmReg((U8)hostLocal, 0xff), 0, false));
        }
        m_emitter.emitEnd();

        m_gpDirty   = savedGpDirty;
        m_gpLoaded  = savedGpLoaded;
        m_segLoaded = savedSegLoaded;
        freeScratch(hostLocal);
        freeScratch(entryLocal);
        return;
    }

    m_needsWasmMemoryPageArrays = true;
    U32 entryLocal = allocScratch();
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    // missCondition = (entry == 0) [|| boundary]
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmWritePageBaseArray));
    pushRegValue(addressReg);
    m_emitter.emitI32Const(K_PAGE_SHIFT);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(2);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);
    m_emitter.emitLocalTee(entryLocal);
    m_emitter.emitOp(WASM_I32_EQZ);

    if (w != JitWidth::b8) {
        U32 widthBytes = wasmJitWidthBytes(w);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    {
        // Slow path: bailout-checking helper. Writes to CodePages, RO
        // pages, on-demand pages, and cross-page accesses all land here.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), src);
        syncStateBeforeFaultingMemoryHelper();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        emitArmSmcBailout();
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
    }
    m_emitter.emitElse();
    {
        // Fast path: i32.store{8,16,}(entry + (addr & K_PAGE_MASK), src)
        // No bailout check needed: by construction, fast-pathed writes
        // can't land on the active block (which lives on a CodePage,
        // and CodePage::canWriteRam() == false → wasmWritePageBase == 0).
        m_emitter.emitLocalGet(entryLocal);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_ADD);
        pushRegValue(src);
        switch (w) {
        case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
        case JitWidth::b16: m_emitter.emitI32Store16(0); break;
        default:            m_emitter.emitI32Store(0, 0); break;
        }
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
    freeScratch(entryLocal);
}

// Materialize a MemPtr (base + index*scale + disp) into a scratch reg as a
// 32-bit virtual address. Only used for emulated-memory ops; DYN_PTR reads
// go through readHost which loads directly from WASM linear memory.
RegPtr JitWasmCodeGen::memPtrToAddressReg(MemPtr address) {
    auto addr = getTmpReg();
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        if (address->offset) {
            m_emitter.emitI32Const((S32)address->offset);
            m_emitter.emitOp(WASM_I32_ADD);
        }
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    m_emitter.emitLocalSet(addr->hardwareReg());
    return addr;
}

RegPtr JitWasmCodeGen::read(JitWidth w, MemPtr address, RegPtr result) {
    if (!address->emulatedAddress) {
        if (!result) result = getTmpReg();
        readHost(w, address, result, true);
        return result;
    }
    auto addr = memPtrToAddressReg(address);
    // addr scratch freed by the inner read() call (it frees any scratch addressReg)
    return read(w, addr, nullptr, nullptr, result);
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, RegPtr src) {
    if (!address->emulatedAddress) {
        writeHost(w, address, src, true);
        return;
    }
    auto addr = memPtrToAddressReg(address);
    write(w, addr, src);
    freeScratch(addr->hardwareReg());
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, U32 imm) {
    if (!address->emulatedAddress) {
        writeHost(w, address, imm, true);
        return;
    }
    auto addr = memPtrToAddressReg(address);
    auto val = getTmpReg();
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitLocalSet(val->hardwareReg());
    write(w, addr, val);
    freeScratch(val->hardwareReg());
    freeScratch(addr->hardwareReg());
}

// ---------------------------------------------------------------------------
// Arithmetic operations
// ---------------------------------------------------------------------------
void JitWasmCodeGen::addReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_ADD); }
void JitWasmCodeGen::addValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_ADD); }
void JitWasmCodeGen::addValueWithDest(JitWidth w, RegPtr dst, RegPtr reg, U32 imm) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        pushRegValue(reg);
        maskToWidth(w);
        popToReg(w, dst);
    }
    addValue(w, dst, imm);
}
void JitWasmCodeGen::subReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SUB); }
void JitWasmCodeGen::subValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SUB); }
void JitWasmCodeGen::orReg(JitWidth w, RegPtr reg, RegPtr rm)    { emitBinOp(w, reg, rm,  WASM_I32_OR); }
void JitWasmCodeGen::orValue(JitWidth w, RegPtr reg, U32 imm)    { emitBinOpImm(w, reg, imm, WASM_I32_OR); }
void JitWasmCodeGen::andReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_AND); }
void JitWasmCodeGen::andValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_AND); }
void JitWasmCodeGen::xorReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_XOR); }
void JitWasmCodeGen::xorValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_XOR); }
void JitWasmCodeGen::shlReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SHL); }
void JitWasmCodeGen::shlValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SHL); }
void JitWasmCodeGen::shrReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SHR_U); }
void JitWasmCodeGen::shrValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SHR_U); }
// Signed shift needs the value sign-extended from its width before SHR_S.
// emitBinOp masks the low byte/word with AND, which would zero the sign bit
// and turn a negative 8/16-bit into a positive 32-bit, making SHR_S wrong.
static void emitSignExtendTo32(WasmEmitter& e, JitWidth w) {
    if (w == JitWidth::b8) {
        e.emitI32Const(24); e.emitOp(WASM_I32_SHL);
        e.emitI32Const(24); e.emitOp(WASM_I32_SHR_S);
    } else if (w == JitWidth::b16) {
        e.emitI32Const(16); e.emitOp(WASM_I32_SHL);
        e.emitI32Const(16); e.emitOp(WASM_I32_SHR_S);
    }
}
void JitWasmCodeGen::sarReg(JitWidth w, RegPtr reg, RegPtr rm) {
    pushRegValue(reg);
    emitSignExtendTo32(m_emitter, w);
    pushRegValue(rm);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitOp(WASM_I32_SHR_S);
    maskToWidth(w);
    popToReg(w, reg);
}
void JitWasmCodeGen::sarValue(JitWidth w, RegPtr reg, U32 imm) {
    pushRegValue(reg);
    emitSignExtendTo32(m_emitter, w);
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitOp(WASM_I32_SHR_S);
    maskToWidth(w);
    popToReg(w, reg);
}
#ifdef BOXEDWINE_64
void JitWasmCodeGen::andValue64(RegPtr reg, U64 imm) { andValue(JitWidth::b32, reg, (U32)imm); }
#endif

static LazyFlagType lazyTypeForSub(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return FLAGS_SUB8;
    case JitWidth::b16: return FLAGS_SUB16;
    default:            return FLAGS_SUB32;
    }
}

void JitWasmCodeGen::notReg2(JitWidth w, RegPtr reg) {
    pushRegValue(reg);
    m_emitter.emitI32Const(-1);
    m_emitter.emitOp(WASM_I32_XOR);
    maskToWidth(w);
    popToReg(w, reg);
}

void JitWasmCodeGen::negReg2(JitWidth w, RegPtr reg) {
    m_emitter.emitI32Const(0);
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I32_SUB);
    maskToWidth(w);
    popToReg(w, reg);
}

void JitWasmCodeGen::clzReg(JitWidth w, RegPtr result, RegPtr reg) {
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I32_CLZ);
    m_emitter.emitLocalSet(result->hardwareReg());
}

static bool narrowRotateWidth(JitWidth w, U32& bits, U32& countMask) {
    if (w == JitWidth::b8) {
        bits = 8;
        countMask = 7;
        return true;
    }
    if (w == JitWidth::b16) {
        bits = 16;
        countMask = 15;
        return true;
    }
    return false;
}

void JitWasmCodeGen::emitNarrowRotate(JitWidth w, RegPtr reg, RegPtr count, U32 imm, bool countIsImm, bool left) {
    U32 bits = 0;
    U32 countMask = 0;
    if (!narrowRotateWidth(w, bits, countMask)) {
        if (countIsImm) emitBinOpImm(w, reg, imm & 31, left ? WASM_I32_ROTL : WASM_I32_ROTR);
        else emitBinOp(w, reg, count, left ? WASM_I32_ROTL : WASM_I32_ROTR);
        return;
    }

    U32 valueLocal = allocScratch();
    U32 countLocal = allocScratch();

    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitLocalSet(valueLocal);

    if (countIsImm) {
        m_emitter.emitI32Const((S32)(imm & countMask));
    } else {
        pushRegValue(count);
        m_emitter.emitI32Const((S32)countMask);
        m_emitter.emitOp(WASM_I32_AND);
    }
    m_emitter.emitLocalSet(countLocal);

    if (left) {
        m_emitter.emitLocalGet(valueLocal);
        m_emitter.emitLocalGet(countLocal);
        m_emitter.emitOp(WASM_I32_SHL);
        m_emitter.emitLocalGet(valueLocal);
        m_emitter.emitI32Const((S32)bits);
        m_emitter.emitLocalGet(countLocal);
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitOp(WASM_I32_SHR_U);
    } else {
        m_emitter.emitLocalGet(valueLocal);
        m_emitter.emitLocalGet(countLocal);
        m_emitter.emitOp(WASM_I32_SHR_U);
        m_emitter.emitLocalGet(valueLocal);
        m_emitter.emitI32Const((S32)bits);
        m_emitter.emitLocalGet(countLocal);
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitOp(WASM_I32_SHL);
    }
    m_emitter.emitOp(WASM_I32_OR);
    maskToWidth(w);
    popToReg(w, reg);

    freeScratch(countLocal);
    freeScratch(valueLocal);
}

// WASM i32.rotl/i32.rotr wrap at 32 bits. Emit explicit 8/16-bit rotate
// expressions for narrow forms; the shared dynamic op still falls back when
// those forms need CF/OF.
void JitWasmCodeGen::rolReg(JitWidth w, RegPtr reg, RegPtr rm) {
    emitNarrowRotate(w, reg, rm, 0, false, true);
}
void JitWasmCodeGen::rolValue(JitWidth w, RegPtr reg, U32 imm) {
    emitNarrowRotate(w, reg, nullptr, imm, true, true);
}
void JitWasmCodeGen::rorReg(JitWidth w, RegPtr reg, RegPtr rm) {
    emitNarrowRotate(w, reg, rm, 0, false, false);
}
void JitWasmCodeGen::rorValue(JitWidth w, RegPtr reg, U32 imm) {
    emitNarrowRotate(w, reg, nullptr, imm, true, false);
}

// Complex rotate-through-carry and div: fall back to single-op emulation
void JitWasmCodeGen::rclReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rclValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rcrReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rcrValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::shrdReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl) {
    U32 countLocal = allocScratch();
    U32 bits = w == JitWidth::b16 ? 16 : 32;
    U32 dstLocal = allocScratch();
    U32 srcLocal = allocScratch();

    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitLocalSet(dstLocal);
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitLocalSet(srcLocal);
    pushRegValue(cl);
    m_emitter.emitI32Const(0x1f);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(countLocal);

    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitI32Const(0);
    m_emitter.emitOp(WASM_I32_NE);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitI32Const((S32)bits);
    m_emitter.emitOp(WASM_I32_LE_U);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf();

    m_emitter.emitLocalGet(dstLocal);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitLocalGet(srcLocal);
    m_emitter.emitI32Const((S32)bits);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_OR);
    maskToWidth(w);
    popToReg(w, reg);

    m_emitter.emitEnd();
    freeScratch(srcLocal);
    freeScratch(dstLocal);
    freeScratch(countLocal);
}
void JitWasmCodeGen::shrdValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm) {
    U32 count = imm & 0x1f;
    if (!count || count > (w == JitWidth::b16 ? 16u : 32u)) {
        return;
    }

    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitI32Const((S32)count);
    m_emitter.emitOp(WASM_I32_SHR_U);
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitI32Const((S32)((w == JitWidth::b16 ? 16u : 32u) - count));
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_OR);
    maskToWidth(w);
    popToReg(w, reg);
}
void JitWasmCodeGen::shldReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl) {
    U32 countLocal = allocScratch();
    U32 bits = w == JitWidth::b16 ? 16 : 32;
    U32 dstLocal = allocScratch();
    U32 srcLocal = allocScratch();

    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitLocalSet(dstLocal);
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitLocalSet(srcLocal);
    pushRegValue(cl);
    m_emitter.emitI32Const(0x1f);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(countLocal);

    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitI32Const(0);
    m_emitter.emitOp(WASM_I32_NE);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitI32Const((S32)bits);
    m_emitter.emitOp(WASM_I32_LE_U);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf();

    m_emitter.emitLocalGet(dstLocal);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitLocalGet(srcLocal);
    m_emitter.emitI32Const((S32)bits);
    m_emitter.emitLocalGet(countLocal);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitOp(WASM_I32_OR);
    maskToWidth(w);
    popToReg(w, reg);

    m_emitter.emitEnd();
    freeScratch(srcLocal);
    freeScratch(dstLocal);
    freeScratch(countLocal);
}
void JitWasmCodeGen::shldValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm) {
    U32 count = imm & 0x1f;
    if (!count || count > (w == JitWidth::b16 ? 16u : 32u)) {
        return;
    }

    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitI32Const((S32)count);
    m_emitter.emitOp(WASM_I32_SHL);
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitI32Const((S32)((w == JitWidth::b16 ? 16u : 32u) - count));
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitOp(WASM_I32_OR);
    maskToWidth(w);
    popToReg(w, reg);
}
void JitWasmCodeGen::mulReg(JitWidth w, RegPtr reg) {
    if (w == JitWidth::b32) {
        pushRegValue(getReg(0));
        m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
        pushRegValue(reg);
        m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
        m_emitter.emitOp(WASM_I64_MUL);
        m_emitter.emitLocalTee(WASM_I64_SCRATCH);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        popToReg(JitWidth::b32, getReg(0));
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Const(32);
        m_emitter.emitOp(WASM_I64_SHR_U);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        popToReg(JitWidth::b32, getReg(2));
        return;
    }

    U32 productLocal = allocScratch();
    pushRegValue(getReg(0));
    maskToWidth(w);
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_MUL);
    m_emitter.emitLocalTee(productLocal);
    popToReg(JitWidth::b16, getReg(0));

    if (w == JitWidth::b16) {
        m_emitter.emitLocalGet(productLocal);
        m_emitter.emitI32Const(16);
        m_emitter.emitOp(WASM_I32_SHR_U);
        popToReg(JitWidth::b16, getReg(2));
    }
    freeScratch(productLocal);
}

void JitWasmCodeGen::imulReg(JitWidth w, RegPtr reg) {
    if (w == JitWidth::b32) {
        pushRegValue(getReg(0));
        m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
        pushRegValue(reg);
        m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
        m_emitter.emitOp(WASM_I64_MUL);
        m_emitter.emitLocalTee(WASM_I64_SCRATCH);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        popToReg(JitWidth::b32, getReg(0));
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Const(32);
        m_emitter.emitOp(WASM_I64_SHR_S);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        popToReg(JitWidth::b32, getReg(2));
        return;
    }

    U32 productLocal = allocScratch();
    pushRegValue(getReg(0));
    maskToWidth(w);
    emitSignExtendTo32(m_emitter, w);
    pushRegValue(reg);
    maskToWidth(w);
    emitSignExtendTo32(m_emitter, w);
    m_emitter.emitOp(WASM_I32_MUL);
    m_emitter.emitLocalTee(productLocal);
    popToReg(JitWidth::b16, getReg(0));

    if (w == JitWidth::b16) {
        m_emitter.emitLocalGet(productLocal);
        m_emitter.emitI32Const(16);
        m_emitter.emitOp(WASM_I32_SHR_S);
        popToReg(JitWidth::b16, getReg(2));
    }
    freeScratch(productLocal);
}
void JitWasmCodeGen::imulRRI(JitWidth w, RegPtr dst, RegPtr src, U32 s2, RegPtr ov) {
    if (!ov) {
        // No overflow tracking: 32-bit multiply is sufficient.
        pushRegValue(src);
        m_emitter.emitI32Const((S32)s2);
        m_emitter.emitOp(WASM_I32_MUL);
        maskToWidth(w);
        popToReg(w, dst);
        return;
    }
    // With overflow (b32 only): sign-extend operands to i64, multiply, extract halves.
    pushRegValue(src);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    m_emitter.emitI64Const((S64)(S32)s2);
    m_emitter.emitOp(WASM_I64_MUL);
    m_emitter.emitLocalTee(WASM_I64_SCRATCH);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // low 32 bits
    popToReg(w, dst);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const(32);
    m_emitter.emitOp(WASM_I64_SHR_S);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // high 32 bits
    popToReg(JitWidth::b32, ov);
}
void JitWasmCodeGen::imulRR(JitWidth w, RegPtr dst, RegPtr src, RegPtr ov) {
    if (!ov) {
        emitBinOp(w, dst, src, WASM_I32_MUL);
        return;
    }
    // With overflow: sign-extend operands to i64, multiply, extract halves.
    pushRegValue(dst);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    pushRegValue(src);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    m_emitter.emitOp(WASM_I64_MUL);
    m_emitter.emitLocalTee(WASM_I64_SCRATCH);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // low 32 bits
    popToReg(w, dst);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const(32);
    m_emitter.emitOp(WASM_I64_SHR_S);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // high 32 bits
    popToReg(JitWidth::b32, ov);
}
void JitWasmCodeGen::divRegRegWithRemainder(JitWidth w, RegPtr d, RegPtr dh, RegPtr s) { emulateSingleOp(); }
void JitWasmCodeGen::idivRegRegWithRemainder(JitWidth w, RegPtr d, RegPtr dh, RegPtr s) { emulateSingleOp(); }

void JitWasmCodeGen::dynamic_divR32(DecodedOp* op) {
    dynamic_div32(op, getReadOnlyReg(op->reg));
}

void JitWasmCodeGen::dynamic_divE32(DecodedOp* op) {
    dynamic_div32(op, read(JitWidth::b32, calculateEaa(op), nullptr, nullptr, getTmpReg()));
}

static void emitSignedDiv32Dividend(WasmEmitter& emitter, RegPtr eax, RegPtr edx, const std::function<void(RegPtr)>& pushRegValue) {
    pushRegValue(edx);
    emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitter.emitI64Const(32);
    emitter.emitOp(WASM_I64_SHL);
    pushRegValue(eax);
    emitter.emitOp(WASM_I64_EXTEND_I32_U);
    emitter.emitOp(WASM_I64_OR);
}

void JitWasmCodeGen::dynamic_div32(DecodedOp* op, RegPtr src) {
    (void)op;
    RegPtr eax = getReg(0);
    RegPtr edx = getReg(2);
    U32 divisorLocal = allocScratch();
    RegPtr divisor = makeWasmReg((U8)divisorLocal, WASM_GP_LOCAL_COUNT);

    pushRegValue(src);
    m_emitter.emitLocalSet(divisorLocal);

    // x86 raises #DE for divisor zero or quotient overflow. Fallback to the
    // interpreter for those rare paths so WASM never executes a trapping div.
    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    IfNot(JitWidth::b32, divisor); {
        emulateSingleOp();
        blockExit();
    } StartElse(); {
        m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
        IfGreaterThanOrEqual(JitWidth::b32, ComparisonType::Unsigned, edx, divisor); {
            emulateSingleOp();
            blockExit();
        } StartElse(); {
            pushRegValue(edx);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitI64Const(32);
            m_emitter.emitOp(WASM_I64_SHL);
            pushRegValue(eax);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_OR);
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);

            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            pushRegValue(divisor);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_DIV_U);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            popToReg(JitWidth::b32, eax);

            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            pushRegValue(divisor);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_REM_U);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            popToReg(JitWidth::b32, edx);

            storeLazyFlagType(FLAGS_NONE);
            currentLazyFlags = FLAGS_NONE;
        } EndIf();
    } EndIf();

    freeScratch(divisorLocal);
}

void JitWasmCodeGen::dynamic_idivR32(DecodedOp* op) {
    dynamic_idiv32(op, getReadOnlyReg(op->reg));
}

void JitWasmCodeGen::dynamic_idivE32(DecodedOp* op) {
    dynamic_idiv32(op, read(JitWidth::b32, calculateEaa(op), nullptr, nullptr, getTmpReg()));
}

void JitWasmCodeGen::dynamic_idiv32(DecodedOp* op, RegPtr src) {
    (void)op;
    RegPtr eax = getReg(0);
    RegPtr edx = getReg(2);
    U32 divisorLocal = allocScratch();
    U32 remainderLocal = allocScratch();
    RegPtr divisor = makeWasmReg((U8)divisorLocal, WASM_GP_LOCAL_COUNT);
    RegPtr remainder = makeWasmReg((U8)remainderLocal, WASM_GP_LOCAL_COUNT);

    pushRegValue(src);
    m_emitter.emitLocalSet(divisorLocal);
    emitSignedDiv32Dividend(m_emitter, eax, edx, [this](RegPtr reg) { pushRegValue(reg); });
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);

    branchBoundary();
    pushRegValue(divisor);
    m_emitter.emitOp(WASM_I32_EQZ);
    pushRegValue(divisor);
    m_emitter.emitI32Const(-1);
    m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const((S64)(-9223372036854775807LL - 1));
    m_emitter.emitOp(WASM_I64_EQ);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    {
        emulateSingleOp();
        blockExit();
    } StartElse(); {
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        pushRegValue(divisor);
        m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
        m_emitter.emitOp(WASM_I64_DIV_S);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);

        branchBoundary();
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
        m_emitter.emitOp(WASM_I64_NE);
        m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
        m_emitter.emitIf();
        {
            emulateSingleOp();
            blockExit();
        } StartElse(); {
            emitSignedDiv32Dividend(m_emitter, eax, edx, [this](RegPtr reg) { pushRegValue(reg); });
            pushRegValue(divisor);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
            m_emitter.emitOp(WASM_I64_REM_S);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitLocalSet(remainderLocal);

            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I32_WRAP_I64);
            popToReg(JitWidth::b32, eax);

            pushRegValue(remainder);
            popToReg(JitWidth::b32, edx);

            storeLazyFlagType(FLAGS_NONE);
            currentLazyFlags = FLAGS_NONE;
        } EndIf();
    } EndIf();

    freeScratch(remainderLocal);
    freeScratch(divisorLocal);
}

void JitWasmCodeGen::bsfReg(JitWidth w, RegPtr reg, RegPtr rm) {
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_CTZ);
    popToReg(w, reg);
}

void JitWasmCodeGen::bsrReg(JitWidth w, RegPtr reg, RegPtr rm) {
    m_emitter.emitI32Const(31);
    pushRegValue(rm);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_CLZ);
    m_emitter.emitOp(WASM_I32_SUB);
    popToReg(w, reg);
}

void JitWasmCodeGen::absReg(JitWidth w, RegPtr reg)                          { emulateSingleOp(); }

void JitWasmCodeGen::byteSwapReg32(RegPtr reg) {
    // bswap: ((v & 0xff)<<24) | ((v & 0xff00)<<8) | ((v>>8)&0xff00) | (v>>24)
    // Fall back to single-op emulation for correctness.
    emulateSingleOp();
}

void JitWasmCodeGen::xchgReg(JitWidth w, RegPtr dest, RegPtr src) {
    U32 tmp = allocScratch();
    pushRegValue(dest);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitLocalSet(tmp);
    pushRegValue(src);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    popToReg(w, dest);
    m_emitter.emitLocalGet(tmp);
    popToReg(w, src);
    freeScratch(tmp);
}

void JitWasmCodeGen::xaddReg(JitWidth w, RegPtr reg, RegPtr rm) {
    // tmp = rm; rm = rm + reg; reg = tmp
    if (reg->hardwareReg() == rm->hardwareReg() && reg->isHigh == rm->isHigh) {
        addReg(w, rm, reg);
        return;
    }
    U32 tmp = allocScratch();
    pushRegValue(rm);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitLocalSet(tmp);
    addReg(w, rm, reg);
    m_emitter.emitLocalGet(tmp);
    popToReg(w, reg);
    freeScratch(tmp);
}

static LazyFlagType cmpFlagsForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8: return FLAGS_CMP8;
    case JitWidth::b16: return FLAGS_CMP16;
    default: return FLAGS_CMP32;
    }
}

void JitWasmCodeGen::dynamic_cmpxchgr8r8(DecodedOp* op) {
    dynamic_cmpxchgReg(JitWidth::b8, op->reg, op->rm);
}

void JitWasmCodeGen::dynamic_cmpxchgr16r16(DecodedOp* op) {
    dynamic_cmpxchgReg(JitWidth::b16, op->reg, op->rm);
}

void JitWasmCodeGen::dynamic_cmpxchgr32r32(DecodedOp* op) {
    dynamic_cmpxchgReg(JitWidth::b32, op->reg, op->rm);
}

void JitWasmCodeGen::dynamic_cmpxchge32r32(DecodedOp* op) {
    dynamic_cmpxchgMem32(op);
}

void JitWasmCodeGen::dynamic_cmpxchgReg(JitWidth w, U8 dstReg, U8 srcReg) {
    U32 accLocal = allocScratch();
    U32 dstLocal = allocScratch();
    U32 srcLocal = allocScratch();
    U32 resultLocal = allocScratch();

    RegPtr acc = w == JitWidth::b8 ? getReadOnlyReg8(0) : getReadOnlyReg(0);
    RegPtr dst = w == JitWidth::b8 ? getReadOnlyReg8(dstReg) : getReadOnlyReg(dstReg);
    RegPtr src = w == JitWidth::b8 ? getReadOnlyReg8(srcReg) : getReadOnlyReg(srcReg);

    pushRegValue(acc);
    maskToWidth(w);
    m_emitter.emitLocalSet(accLocal);
    pushRegValue(dst);
    maskToWidth(w);
    m_emitter.emitLocalSet(dstLocal);
    pushRegValue(src);
    maskToWidth(w);
    m_emitter.emitLocalSet(srcLocal);

    m_emitter.emitLocalGet(accLocal);
    m_emitter.emitLocalGet(dstLocal);
    m_emitter.emitOp(WASM_I32_SUB);
    maskToWidth(w);
    m_emitter.emitLocalSet(resultLocal);

    RegPtr accTmp = makeWasmReg((U8)accLocal, 0xff);
    RegPtr dstTmp = makeWasmReg((U8)dstLocal, 0xff);
    RegPtr srcTmp = makeWasmReg((U8)srcLocal, 0xff);
    RegPtr resultTmp = makeWasmReg((U8)resultLocal, 0xff);

    storeLazyFlagType(cmpFlagsForWidth(w));
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, dst.u32), accTmp);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, src.u32), dstTmp);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, result.u32), resultTmp);
    currentLazyFlags = cmpFlagsForWidth(w);

    IfEqual(w, accTmp, dstTmp); {
        m_emitter.emitLocalGet(srcLocal);
        popToReg(w, w == JitWidth::b8 ? getReg8(dstReg) : getReg(dstReg));
    } StartElse(); {
        m_emitter.emitLocalGet(dstLocal);
        popToReg(w, w == JitWidth::b8 ? getReg8(0) : getReg(0));
    } EndIf();

    freeScratch(resultLocal);
    freeScratch(srcLocal);
    freeScratch(dstLocal);
    freeScratch(accLocal);
}

void JitWasmCodeGen::dynamic_cmpxchgMem32(DecodedOp* op) {
    U32 addrLocal = allocScratch();
    U32 accLocal = allocScratch();
    U32 memLocal = allocScratch();
    U32 srcLocal = allocScratch();
    U32 resultLocal = allocScratch();

    RegPtr eaa = calculateEaa(op);
    pushRegValue(eaa);
    m_emitter.emitLocalSet(addrLocal);
    if (eaa && eaa->emulatedReg == 0xff) {
        freeScratch(eaa->hardwareReg());
    }

    RegPtr addrTmp = makeWasmReg((U8)addrLocal, WASM_GP_LOCAL_COUNT);
    RegPtr acc = getReadOnlyReg(0);
    RegPtr src = getReadOnlyReg(op->reg);
    RegPtr memTmp = makeWasmReg((U8)memLocal, WASM_GP_LOCAL_COUNT);

    read(JitWidth::b32, addrTmp, nullptr, nullptr, memTmp);

    pushRegValue(acc);
    m_emitter.emitLocalSet(accLocal);
    pushRegValue(src);
    m_emitter.emitLocalSet(srcLocal);

    m_emitter.emitLocalGet(accLocal);
    m_emitter.emitLocalGet(memLocal);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(resultLocal);

    RegPtr accTmp = makeWasmReg((U8)accLocal, WASM_GP_LOCAL_COUNT);
    RegPtr srcTmp = makeWasmReg((U8)srcLocal, WASM_GP_LOCAL_COUNT);
    RegPtr resultTmp = makeWasmReg((U8)resultLocal, WASM_GP_LOCAL_COUNT);

    storeLazyFlagType(FLAGS_CMP32);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, dst.u32), accTmp);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, src.u32), memTmp);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, result.u32), resultTmp);
    currentLazyFlags = FLAGS_CMP32;

    IfEqual(JitWidth::b32, accTmp, memTmp); {
        write(JitWidth::b32, addrTmp, srcTmp);
    } StartElse(); {
        m_emitter.emitLocalGet(memLocal);
        popToReg(JitWidth::b32, getReg(0));
    } EndIf();

    freeScratch(resultLocal);
    freeScratch(srcLocal);
    freeScratch(memLocal);
    freeScratch(accLocal);
    freeScratch(addrLocal);
}

// ---------------------------------------------------------------------------
// Compare / test
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::compareReg(JitWidth w, RegPtr r1, RegPtr r2, JitEvaluate cond, RegPtr res) {
    if (!res) res = getTmpReg();
    pushRegValue(r1);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    pushRegValue(r2);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    switch (cond) {
    case JitEvaluate::EQUALS:                   m_emitter.emitOp(WASM_I32_EQ);   break;
    case JitEvaluate::NOT_EQUALS:               m_emitter.emitOp(WASM_I32_NE);   break;
    case JitEvaluate::LESS_THAN_UNSIGNED:       m_emitter.emitOp(WASM_I32_LT_U); break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED: m_emitter.emitOp(WASM_I32_LE_U); break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:    m_emitter.emitOp(WASM_I32_GT_U); break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED: m_emitter.emitOp(WASM_I32_GE_U); break;
    case JitEvaluate::LESS_THAN_SIGNED:         m_emitter.emitOp(WASM_I32_LT_S); break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:   m_emitter.emitOp(WASM_I32_LE_S); break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:m_emitter.emitOp(WASM_I32_GE_S); break;
    case JitEvaluate::GREATER_THAN_SIGNED:      m_emitter.emitOp(WASM_I32_GT_S); break;
    default: m_emitter.emitOp(WASM_I32_EQ); break;
    }
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

RegPtr JitWasmCodeGen::compareValue(JitWidth w, RegPtr reg, U32 val, JitEvaluate cond, RegPtr res) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)val);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    auto result = compareReg(w, reg, tmp, cond, res);
    freeScratch(tmp->hardwareReg());
    return result;
}

RegPtr JitWasmCodeGen::testZeroReg(JitWidth w, RegPtr reg, RegPtr res) {
    if (!res) res = getTmpReg();
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

// ---------------------------------------------------------------------------
// Move operations
// ---------------------------------------------------------------------------
void JitWasmCodeGen::mov(JitWidth w, RegPtr dest, RegPtr src) {
    pushRegValue(src);
    maskToWidth(w);
    popToReg(w, dest);
    if (src && src->emulatedReg == 0xff) freeScratch(src->hardwareReg());
}

void JitWasmCodeGen::movzx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) {
    pushRegValue(src);
    maskToWidth(sw);
    // result is zero-extended to dw (source bits at low, upper dw bits are 0)
    popToReg(dw, dest);
    if (src && src->emulatedReg == 0xff) freeScratch(src->hardwareReg());
}

void JitWasmCodeGen::movsx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) {
    U32 srcLocal = src ? src->hardwareReg() : 0;
    pushRegValue(src);
    switch (sw) {
    case JitWidth::b8:  m_emitter.emitOp(WASM_I32_EXTEND8_S);  break;
    case JitWidth::b16: m_emitter.emitOp(WASM_I32_EXTEND16_S); break;
    default: break;
    }
    if (dw == JitWidth::b16) {
        // Mask to 16-bit for the merge in popToReg
        m_emitter.emitI32Const(0xFFFF);
        m_emitter.emitOp(WASM_I32_AND);
    }
    popToReg(dw, dest);
    if (src && src->emulatedReg == 0xff && (!dest || dest->hardwareReg() != srcLocal)) {
        freeScratch(srcLocal);
    }
}

void JitWasmCodeGen::movValue(JitWidth w, RegPtr dst, DYN_PTR_SIZE imm) {
    m_emitter.emitI32Const((S32)imm);
    popToReg(w, dst);
}

// ---------------------------------------------------------------------------
// Flags – delegated to JitCodeGen base which stores to CPU struct fields
// (the generated WASM reads/writes these via readCPU/writeCPU which delegate
//  to emitI32Load/emitI32Store with the appropriate offsets)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::storeLazyFlagType(LazyFlagType flags) {
    writeCPUValue(JitWidth::b8, (U32)offsetof(CPU, lazyFlagType), (DYN_PTR_SIZE)flags);
}
void JitWasmCodeGen::storeLazyFlagsDest(RegPtr reg) {
    // isHigh regs point at the high byte of reg[0..3]; pushRegValue already
    // shifts right 8 for them, so a b8 store at dst.u32's low byte is
    // correct (the lazy-flag code reads dst.u8). Non-high regs go in as b32.
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, dst.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, src.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(U32 value) {
    writeCPUValue(JitWidth::b32, (U32)offsetof(CPU, src.u32), value);
}
void JitWasmCodeGen::storeLazyFlagsResult(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, result.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsOldCF(RegPtr reg) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, oldCF), reg);
}
void JitWasmCodeGen::fillFlags(U32 flags) {
    // Materialize lazy flags: call common_fillFlags(cpu).
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FILL_FLAGS);
    m_gpLoaded.fill(false);
    currentLazyFlags = FLAGS_NONE;
}
RegPtr JitWasmCodeGen::getZF() {
    // Sync dirty GP regs so helper sees correct CPU state, then call
    // wasmHelper_computeZF(cpu) which stores ZF in cpu->tmpReg.
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COMPUTE_ZF);
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(r->hardwareReg());
    // Any register values we had cached may have been invalidated if helper
    // touched state; invalidate and let callers re-load as needed.
    m_gpLoaded.fill(false);
    return r;
}
RegPtr JitWasmCodeGen::getCF() {
    auto r = getTmpReg();

    auto emitHelperFallback = [this, &r]() {
        // The helper only reads lazy-flag fields and writes cpu->tmpReg; GP
        // locals do not need to be flushed or invalidated around this call.
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_COMPUTE_CF);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
        m_emitter.emitLocalSet(r->hardwareReg());
    };

    if (currentLazyFlags == FLAGS_NULL) {
        emitHelperFallback();
        return r;
    }

    // Most calls know the preceding lazy-flag formula at compile time. Keep
    // the runtime check because a compiled block can be entered from a path
    // with different lazy state, but compute the overwhelmingly common arm
    // directly in WASM instead of crossing an imported C++ helper.
    RegPtr runtimeType = getLazyFlagType();
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    pushRegValue(runtimeType);
    m_emitter.emitI32Const((S32)currentLazyFlags);
    m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
    m_emitter.emitIf();
    {
        JitCodeGen::getCF(currentLazyFlags, r);
    }
    m_emitter.emitElse();
    {
        emitHelperFallback();
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
    if (canReleaseScratchReg(runtimeType)) {
        freeScratch(runtimeType->hardwareReg());
    }
    return r;
}
void JitWasmCodeGen::orCPUFlags(RegPtr reg) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    orReg(JitWidth::b32, cur, reg);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::xorCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    xorValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::andCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    andValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::orCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    orValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
RegPtr JitWasmCodeGen::getReadOnlyFlags(RegPtr tmp) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags), tmp);
}
RegPtr JitWasmCodeGen::getFlagsInTmp(RegPtr reg) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags), reg);
}
void JitWasmCodeGen::setFlags(RegPtr flags, U32 mask) {
    // flags = (existing & ~mask) | (flags & mask)
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    andValue(JitWidth::b32, cur, ~mask);
    auto masked = getTmpReg();
    mov(JitWidth::b32, masked, flags);
    andValue(JitWidth::b32, masked, mask);
    orReg(JitWidth::b32, cur, masked);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
    freeScratch(masked->hardwareReg());
}
void JitWasmCodeGen::writeFlags(RegPtr flags) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), flags);
}
RegPtr JitWasmCodeGen::getCondition(JitConditional cond, RegPtr res) {
    bool inlineLogicCond32 = currentLazyFlags == FLAGS_TEST32 ||
                             currentLazyFlags == FLAGS_AND32 ||
                             currentLazyFlags == FLAGS_OR32 ||
                             currentLazyFlags == FLAGS_XOR32;
    bool inlineLogicCond8 = currentLazyFlags == FLAGS_TEST8 ||
                            currentLazyFlags == FLAGS_AND8 ||
                            currentLazyFlags == FLAGS_OR8 ||
                            currentLazyFlags == FLAGS_XOR8;
    if ((inlineLogicCond32 || inlineLogicCond8) &&
        (cond == JitConditional::B || cond == JitConditional::NB ||
         cond == JitConditional::BE || cond == JitConditional::NBE ||
         cond == JitConditional::L || cond == JitConditional::NL ||
         cond == JitConditional::LE || cond == JitConditional::NLE)) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_INLINE_COND);
#endif
        U32 condLocal = allocScratch();
        if (cond == JitConditional::B) {
            m_emitter.emitI32Const(0);
        } else if (cond == JitConditional::NB) {
            m_emitter.emitI32Const(1);
        } else {
            U32 resultLocal = allocScratch();
            U32 valueMask = inlineLogicCond8 ? 0xff : 0xffffffffu;
            U32 signMask = inlineLogicCond8 ? 0x80 : 0x80000000u;
            m_emitter.emitLocalGet(WASM_CPU_LOCAL);
            m_emitter.emitI32Load((U32)offsetof(CPU, result.u32));
            m_emitter.emitI32Const((S32)valueMask);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalSet(resultLocal);
            switch (cond) {
            case JitConditional::BE:
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitOp(WASM_I32_EQZ);
                break;
            case JitConditional::NBE:
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitI32Const(0);
                m_emitter.emitOp(WASM_I32_NE);
                break;
            case JitConditional::L:
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitI32Const((S32)signMask);
                m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitI32Const(0);
                m_emitter.emitOp(WASM_I32_NE);
                break;
            case JitConditional::NL:
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitI32Const((S32)signMask);
                m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitOp(WASM_I32_EQZ);
                break;
            case JitConditional::LE:
            case JitConditional::NLE:
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitOp(WASM_I32_EQZ);
                m_emitter.emitLocalGet(resultLocal);
                m_emitter.emitI32Const((S32)signMask);
                m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitI32Const(0);
                m_emitter.emitOp(WASM_I32_NE);
                m_emitter.emitOp(WASM_I32_OR);
                if (cond == JitConditional::NLE) {
                    m_emitter.emitOp(WASM_I32_EQZ);
                }
                break;
            default:
                m_emitter.emitI32Const(0);
                break;
            }
            freeScratch(resultLocal);
        }
        m_emitter.emitLocalSet(condLocal);

        RegPtr tmp = makeWasmReg((U8)condLocal, 0xff);
        if (res && res != tmp) {
            mov(JitWidth::b8, res, tmp);
            freeScratch(condLocal);
            return res;
        }
        return tmp;
    }

    bool inlineSubCond = currentLazyFlags == FLAGS_SUB8 ||
                         currentLazyFlags == FLAGS_SUB16 ||
                         currentLazyFlags == FLAGS_SUB32;
    if (inlineSubCond &&
        (cond == JitConditional::B || cond == JitConditional::NB ||
         cond == JitConditional::BE || cond == JitConditional::NBE ||
         cond == JitConditional::L || cond == JitConditional::NL ||
         cond == JitConditional::LE || cond == JitConditional::NLE)) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_INLINE_COND);
#endif
        U32 bitWidth = currentLazyFlags == FLAGS_SUB8 ? 8 : (currentLazyFlags == FLAGS_SUB16 ? 16 : 32);
        U32 valueMask = bitWidth == 8 ? 0xff : (bitWidth == 16 ? 0xffff : 0xffffffffu);
        U32 condLocal = allocScratch();
        U32 dstLocal = allocScratch();
        U32 srcLocal = allocScratch();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
        if (valueMask != 0xffffffffu) {
            m_emitter.emitI32Const((S32)valueMask);
            m_emitter.emitOp(WASM_I32_AND);
        }
        m_emitter.emitLocalSet(dstLocal);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, src.u32));
        if (valueMask != 0xffffffffu) {
            m_emitter.emitI32Const((S32)valueMask);
            m_emitter.emitOp(WASM_I32_AND);
        }
        m_emitter.emitLocalSet(srcLocal);
        auto emitSubOperand = [this, bitWidth](U32 local, bool signExtend) {
            m_emitter.emitLocalGet(local);
            if (signExtend && bitWidth != 32) {
                m_emitter.emitI32Const((S32)(32 - bitWidth));
                m_emitter.emitOp(WASM_I32_SHL);
                m_emitter.emitI32Const((S32)(32 - bitWidth));
                m_emitter.emitOp(WASM_I32_SHR_S);
            }
        };
        switch (cond) {
        case JitConditional::B:
            emitSubOperand(dstLocal, false);
            emitSubOperand(srcLocal, false);
            m_emitter.emitOp(WASM_I32_LT_U);
            break;
        case JitConditional::NB:
            emitSubOperand(dstLocal, false);
            emitSubOperand(srcLocal, false);
            m_emitter.emitOp(WASM_I32_GE_U);
            break;
        case JitConditional::BE:
            emitSubOperand(dstLocal, false);
            emitSubOperand(srcLocal, false);
            m_emitter.emitOp(WASM_I32_LE_U);
            break;
        case JitConditional::NBE:
            emitSubOperand(dstLocal, false);
            emitSubOperand(srcLocal, false);
            m_emitter.emitOp(WASM_I32_GT_U);
            break;
        case JitConditional::L:
            emitSubOperand(dstLocal, true);
            emitSubOperand(srcLocal, true);
            m_emitter.emitOp(WASM_I32_LT_S);
            break;
        case JitConditional::NL:
            emitSubOperand(dstLocal, true);
            emitSubOperand(srcLocal, true);
            m_emitter.emitOp(WASM_I32_GE_S);
            break;
        case JitConditional::LE:
            emitSubOperand(dstLocal, true);
            emitSubOperand(srcLocal, true);
            m_emitter.emitOp(WASM_I32_LE_S);
            break;
        case JitConditional::NLE:
            emitSubOperand(dstLocal, true);
            emitSubOperand(srcLocal, true);
            m_emitter.emitOp(WASM_I32_GT_S);
            break;
        default:
            m_emitter.emitI32Const(0);
            break;
        }
        m_emitter.emitLocalSet(condLocal);
        freeScratch(srcLocal);
        freeScratch(dstLocal);

        RegPtr tmp = makeWasmReg((U8)condLocal, 0xff);
        if (res && res != tmp) {
            mov(JitWidth::b8, res, tmp);
            freeScratch(condLocal);
            return res;
        }
        return tmp;
    }

    bool inlineZeroCond32 = currentLazyFlags == FLAGS_SUB32 ||
                            currentLazyFlags == FLAGS_TEST32 ||
                            currentLazyFlags == FLAGS_AND32 ||
                            currentLazyFlags == FLAGS_ADD32 ||
                            currentLazyFlags == FLAGS_INC32 ||
                            currentLazyFlags == FLAGS_DEC32 ||
                            currentLazyFlags == FLAGS_OR32 ||
                            currentLazyFlags == FLAGS_XOR32;
    bool inlineZeroCond16 = currentLazyFlags == FLAGS_SUB16 ||
                            currentLazyFlags == FLAGS_TEST16 ||
                            currentLazyFlags == FLAGS_AND16 ||
                            currentLazyFlags == FLAGS_OR16 ||
                            currentLazyFlags == FLAGS_XOR16 ||
                            currentLazyFlags == FLAGS_ADD16 ||
                            currentLazyFlags == FLAGS_INC16 ||
                            currentLazyFlags == FLAGS_DEC16;
    bool inlineZeroCond8 = currentLazyFlags == FLAGS_SUB8 ||
                           currentLazyFlags == FLAGS_TEST8 ||
                           currentLazyFlags == FLAGS_AND8 ||
                           currentLazyFlags == FLAGS_OR8 ||
                           currentLazyFlags == FLAGS_XOR8 ||
                           currentLazyFlags == FLAGS_ADD8 ||
                           currentLazyFlags == FLAGS_INC8 ||
                           currentLazyFlags == FLAGS_DEC8;
    if ((inlineZeroCond32 || inlineZeroCond16 || inlineZeroCond8) && (cond == JitConditional::Z || cond == JitConditional::NZ)) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_INLINE_COND);
#endif
        U32 condLocal = allocScratch();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, result.u32));
        if (inlineZeroCond16 || inlineZeroCond8) {
            m_emitter.emitI32Const(inlineZeroCond8 ? 0xff : 0xffff);
            m_emitter.emitOp(WASM_I32_AND);
        }
        if (cond == JitConditional::Z) {
            m_emitter.emitOp(WASM_I32_EQZ);
        } else {
            m_emitter.emitI32Const(0);
            m_emitter.emitOp(WASM_I32_NE);
        }
        m_emitter.emitLocalSet(condLocal);

        RegPtr tmp = makeWasmReg((U8)condLocal, 0xff);
        if (res && res != tmp) {
            mov(JitWidth::b8, res, tmp);
            freeScratch(condLocal);
            return res;
        }
        return tmp;
    }

    // Per-condition helper stashes 0/1 in cpu->tmpReg. Done this way (rather
    // than one helper taking a condition parameter) because our helper
    // import signature is (i32)->() — a condition arg would have to go
    // through a CPU scratch field, and every candidate (src.u32, dst.u32,
    // etc.) is already reserved for lazy-flag state.
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COND_BASE + (U32)cond);

    // Always load the helper result into a scratch first. `res` may be a
    // GP register; writing its local directly via emitLocalSet would
    // clobber the upper bytes. If the caller gave us a `res`, copy via
    // mov(b8) which merges correctly for GP regs and scratch alike.
    auto tmp = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(tmp->hardwareReg());
    m_gpLoaded.fill(false);
    if (res && res != tmp) {
        mov(JitWidth::b8, res, tmp);
        freeScratch(tmp->hardwareReg());
        return res;
    }
    return tmp;
}
RegPtr JitWasmCodeGen::getConditionCalculationReg(U32 index) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
}

// ---------------------------------------------------------------------------
// Control flow (If/Else/End/Goto)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::finishIf() {
    m_emitter.emitIf();
}

void JitWasmCodeGen::branchBoundary() {
    syncDirtyRegsToHost();
    m_gpLoaded.fill(false);
    m_segLoaded.fill(false);
}

void JitWasmCodeGen::If(JitWidth w, RegPtr reg) {
    branchBoundary();
    pushRegValue(reg);
    maskToWidth(w);
    finishIf();
}
void JitWasmCodeGen::IfNot(JitWidth w, RegPtr reg) {
    branchBoundary();
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, RegPtr mask) {
    branchBoundary();
    pushRegValue(reg);
    pushRegValue(mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, U32 mask) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfNotTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, RegPtr r2) {
    branchBoundary();
    pushRegValue(reg); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfLessThan(JitWidth w, ComparisonType type, RegPtr reg, U32 value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_LT_S : WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfLessThan(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_LT_S : WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_GE_S : WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, ComparisonType type, RegPtr reg, U32 value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_GE_S : WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThan(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_GT_S : WASM_I32_GT_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThan(JitWidth w, ComparisonType type, RegPtr reg, U32 value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(type == ComparisonType::Signed ? WASM_I32_GT_S : WASM_I32_GT_U);
    finishIf();
}
void JitWasmCodeGen::IfNotCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset) {
    // read from CPU struct and If-not
    branchBoundary();
    auto tmp = readCPU(w, sib, lsl, offset);
    pushRegValue(tmp);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
    freeScratch(tmp->hardwareReg());
}
void JitWasmCodeGen::IfCondition(JitConditional cond) {
    // Base JitCodeGen handles: it calls getCondition and emits the right check
    auto r = getCondition(cond);
    If(JitWidth::b32, r);
    // getCondition allocates a scratch local; If() consumes its value on the
    // WASM stack, so we can release the scratch now. Without this, nested
    // codegen (dynamic_loopnz / dynamic_loopz) exhausts the 8-slot pool.
    freeScratch(r->hardwareReg());
}
void JitWasmCodeGen::JumpIfCondition(JitConditional cond, U32 address) {
    // Called when canJumpInBlock() is true. Native backends do a direct
    // branch inside the generated code; we don't have structural
    // intra-block jumps, so treat it as a block exit to the target. The
    // `address` parameter is the *linear* EIP (CS.address + offset), but
    // cpu->eip.u32 stores only the offset relative to CS — subtract here.
    m_manifestJumpCount++;
    if (!m_manifestJumpTarget) {
        m_manifestJumpTarget = address - cpu->seg[CS].address;
    }
    IfCondition(cond);
    emitDirectLoopBackedge(address);
    DecodedOp* targetOp = cpu->memory->getDecodedOp(address);
    if (wasmJitPersistenceActive()) {
        // Relocatable variant: the target DecodedOp* comes from a reloc slot
        // instead of an embedded i32.const. The slot is 0 when the target
        // isn't decoded yet (or the module was instantiated without a slot
        // array); the guards then fall through to the blockExit slow path.
        U32 slotOffset = addRelocSlot((U32)(uintptr_t)targetOp);
        syncDirtyRegsToHost();
        writeEip(address - cpu->seg[CS].address);

        U32 targetLocal = allocScratch();
        m_emitter.emitLocalGet(WASM_RELOC_LOCAL);
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // relocBase != 0
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Load(slotOffset);         // target DecodedOp*
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // target != 0
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_JUMP);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
        m_emitter.emitEnd();
        m_emitter.emitEnd();
        freeScratch(targetLocal);
        blockExit();
    } else if (targetOp) {
        syncDirtyRegsToHost();
        writeEip(address - cpu->seg[CS].address);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_JUMP);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const((S32)(uintptr_t)targetOp);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
    } else {
        writeEip(address - cpu->seg[CS].address);
        blockExit();
    }
    EndIf();
}
void JitWasmCodeGen::IfDF() {
    branchBoundary();
    auto cur = getReadOnlyFlags();
    m_emitter.emitLocalGet(cur->hardwareReg());
    m_emitter.emitI32Const(DF);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfSmallStack() {
    // Small-stack mode is indicated by a non-zero cpu->stackNotMask. Loaded
    // inline rather than via readCPU()+If(reg) so branchBoundary runs once.
    branchBoundary();
    U32 scratch = allocScratch();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, stackNotMask));
    m_emitter.emitLocalSet(scratch);
    m_emitter.emitLocalGet(scratch);
    finishIf();
    freeScratch(scratch);
}
void JitWasmCodeGen::StartElse() {
    branchBoundary();
    m_emitter.emitElse();
}
void JitWasmCodeGen::EndIf() {
    branchBoundary();
    m_emitter.emitEnd();
}
void JitWasmCodeGen::JumpInBlock(U32 address) {
    // A selected hot backward edge can remain in this WASM activation for a
    // bounded number of iterations. Other intra-block jumps retain the
    // dispatcher exit used by the general, unstructured control-flow case.
    m_manifestJumpCount++;
    if (!m_manifestJumpTarget) {
        m_manifestJumpTarget = address - cpu->seg[CS].address;
    }
    emitDirectLoopBackedge(address);
    writeEip(address - cpu->seg[CS].address);
    emitBlockExitWithProfile(HELPER_PROFILE_EXIT_JUMP);
}
#ifdef BOXEDWINE_WASM_JIT_PROFILE
void JitWasmCodeGen::emitProfileSampledCall(U32 helperIdx, U32 detail) {
    U32 counterOffset = (U32)offsetof(CPU, wasmJitProfileSampleCounter);

    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(counterOffset);
    m_emitter.emitI32Const(1);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Store(counterOffset);

    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(counterOffset);
    m_emitter.emitI32Const(WASM_JIT_PROFILE_SAMPLE_MASK);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.setNextBranchHint(WasmBranchHint::Unlikely);
    m_emitter.emitIf();
    if (detail < InstructionCount) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const(detail);
        m_emitter.emitI32Store((U32)offsetof(CPU, tmpReg));
    }
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(helperIdx);
    m_emitter.emitEnd();
}
#endif
void JitWasmCodeGen::emitBlockExitWithProfile(U32 profileHelperIdx) {
    syncDirtyRegsToHost();
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    emitProfileSampledCall(profileHelperIdx);
#else
    (void)profileHelperIdx;
#endif
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperGetNextOpIdx);
    m_emitter.emitReturn();
}
void JitWasmCodeGen::blockExit() {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((U32)(m_currentWasmOp ? m_currentWasmOp->inst : InstructionCount));
    m_emitter.emitI32Store((U32)offsetof(CPU, tmpReg));
#endif
    emitBlockExitWithProfile(HELPER_PROFILE_EXIT_GENERIC);
}
// blockNext1/2 are block-terminating branch destinations (taken vs. fall-
// through). Mirror the base JitCodeGen::blockNext{1,2}: write the CS-offset
// EIP and emit a blockExit so the dispatcher picks up the next op.
void JitWasmCodeGen::blockNext1(U32 eip, DecodedOp* op) {
    m_manifestNext1Count++;
    if (!m_manifestNext1Target) {
        m_manifestNext1Target = eip - cpu->seg[CS].address;
    }
    if (wasmJitPersistenceActive()) {
        // Relocatable variant of the fast path below: the nextJump cache-slot
        // address comes from a reloc slot instead of an embedded i32.const.
        // Emitted unconditionally (even when op->data.nextJump is currently
        // null) so the module shape stays deterministic across sessions; the
        // runtime guards fall through to the dispatcher exit when relocBase,
        // the slot value, or the live target is 0.
        U32 slotOffset = addRelocSlot((U32)(uintptr_t)op->data.nextJump);
        syncDirtyRegsToHost();
        writeEip(eip - cpu->seg[CS].address);

        U32 targetLocal = allocScratch();
        m_emitter.emitLocalGet(WASM_RELOC_LOCAL);
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // relocBase != 0
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Load(slotOffset);         // nextJump cache-slot ptr
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // slot ptr != 0
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Load(0);                  // live target DecodedOp*
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // target != 0
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_NEXT1);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
        m_emitter.emitEnd();
        m_emitter.emitEnd();
        m_emitter.emitEnd();
        freeScratch(targetLocal);
    } else if (op->data.nextJump) {
        syncDirtyRegsToHost();
        writeEip(eip - cpu->seg[CS].address);

        U32 nextJumpLocal = allocScratch();
        U32 targetLocal = allocScratch();

        m_emitter.emitI32Const((S32)(uintptr_t)op->data.nextJump);
        m_emitter.emitLocalSet(nextJumpLocal);
        m_emitter.emitLocalGet(nextJumpLocal);
        m_emitter.emitI32Load(0);
        m_emitter.emitLocalSet(targetLocal);

        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitIf();
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_NEXT1);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
        m_emitter.emitEnd();

        freeScratch(targetLocal);
        freeScratch(nextJumpLocal);
    }
    writeCPUValue(DYN_PTR, (U32)offsetof(CPU, nextOp), 0);
    writeEip(eip - cpu->seg[CS].address);
    emitBlockExitWithProfile(HELPER_PROFILE_EXIT_NEXT1);
}
void JitWasmCodeGen::blockNext2(U32 eip, DecodedOp* op) {
    m_manifestNext2Count++;
    if (!m_manifestNext2Target) {
        m_manifestNext2Target = eip - cpu->seg[CS].address;
    }
    if (wasmJitPersistenceActive()) {
        // Relocatable variant: op->next comes from a reloc slot instead of
        // an embedded i32.const. Guarded (unlike the unconditional embedded
        // path below) because the slot is 0 when op->next was null at
        // compile time or the module was instantiated without a slot array.
        U32 slotOffset = addRelocSlot((U32)(uintptr_t)op->next);
        syncDirtyRegsToHost();
        writeEip(eip - cpu->seg[CS].address);

        U32 targetLocal = allocScratch();
        m_emitter.emitLocalGet(WASM_RELOC_LOCAL);
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // relocBase != 0
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Load(slotOffset);         // op->next DecodedOp*
        m_emitter.emitLocalTee(targetLocal);
        m_emitter.emitIf();                        // target != 0
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_NEXT2);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(targetLocal);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
        m_emitter.emitEnd();
        m_emitter.emitEnd();
        freeScratch(targetLocal);
    } else if (op->next) {
        syncDirtyRegsToHost();
        writeEip(eip - cpu->seg[CS].address);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        emitProfileSampledCall(HELPER_PROFILE_EXIT_NEXT2);
#endif
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const((S32)(uintptr_t)op->next);
        m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
        m_emitter.emitReturn();
    }
    writeCPUValue(DYN_PTR, (U32)offsetof(CPU, nextOp), 0);
    writeEip(eip - cpu->seg[CS].address);
    emitBlockExitWithProfile(HELPER_PROFILE_EXIT_NEXT2);
}
void JitWasmCodeGen::jumpEip(RegPtr reg) {
    writeEip(reg);
    blockExit();
}
bool JitWasmCodeGen::canJumpInBlock(DecodedOp* op) {
    return JitCodeGen::canJumpInBlock(op);
}
bool JitWasmCodeGen::canJumpInBlock(U32 opEip, DecodedOp* op) {
    return JitCodeGen::canJumpInBlock(opEip, op);
}
void JitWasmCodeGen::onTestEnd(DecodedOp* op) {
    syncDirtyRegsToHost();
    // Set cpu->nextOp = op (the TestEnd op)
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)(uintptr_t)op);
    m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
}
void JitWasmCodeGen::jmpHost(RegPtr reg)          { /* no native jumps in WASM */ }
void JitWasmCodeGen::jmpHost(DYN_PTR_SIZE address) { /* no native jumps in WASM */ }
void JitWasmCodeGen::nakedCall(RegPtr reg)         { /* no bare calls in WASM */ }
void JitWasmCodeGen::nakedReturn()                 { m_emitter.emitReturn(); }

void JitWasmCodeGen::dynamic_pause(DecodedOp* op) {
    (void)op;
    // x86 PAUSE is a spin-loop hint. It has no architecturally visible state,
    // so WASM can compile it as a no-op and avoid interpreter fallback.
}

void JitWasmCodeGen::dynamic_loopnz(DecodedOp* op) {
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;
    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx);
        mov(width, reg, cx);
    }
    IfCondition(JitConditional::Z);
        movValue(width, reg, 0);
    EndIf();
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
    If(width, reg);
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(currentEip + op->len + (S32)((S8)op->imm), op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        blockNext2(currentEip + op->len, op);
    }
}

void JitWasmCodeGen::dynamic_loopz(DecodedOp* op) {
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;
    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx);
        mov(width, reg, cx);
    }
    IfCondition(JitConditional::NZ);
        movValue(width, reg, 0);
    EndIf();
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
    If(width, reg);
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(currentEip + op->len + (S32)((S8)op->imm), op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        blockNext2(currentEip + op->len, op);
    }
}

void JitWasmCodeGen::dynamic_loop(DecodedOp* op) {
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;
    decReg(width, getReg(1));
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
    If(width, getReadOnlyReg(1));
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(currentEip + op->len + (S32)((S8)op->imm), op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        blockNext2(currentEip + op->len, op);
    }
}

void JitWasmCodeGen::hintLikelyStringLoopContinue() {
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
}

// ---------------------------------------------------------------------------
// Function calls to C++ helpers
// ---------------------------------------------------------------------------
void JitWasmCodeGen::callHostFunction(void* address, const std::vector<DynParam>& params,
                                       bool restoreCache, bool saveCache) {
    // WASM calling convention: all helpers have the signature void(CPU*).
    // Any additional operands are pre-stored in CPU fields (dst, src, etc.) before
    // the call by the caller, so params must contain exactly one CPU entry.
    // If a future helper needs a non-CPU argument on the WASM stack, this backend
    // must be updated to emit the extra locals before emitCallIndirect.
    if (params.size() != 1 || params[0].type != JitCallParamType::CPU) {
        kpanic_fmt("JitWasmCodeGen::callHostFunction: expected a single CPU param but got %zu params", params.size());
    }
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCallIndirect(m_typeVoidI32, 0);
    m_gpLoaded.fill(false);
}

void JitWasmCodeGen::callHostFunctionWithResult(RegPtr result, void* address,
                                                  const std::vector<DynParam>& params) {
    callHostFunction(address, params, true, true);
    // read result from cpu->dst.u32
    if (result) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
        m_emitter.emitLocalSet(result->hardwareReg());
    }
}


void JitWasmCodeGen::emulateSingleOp() {
    // Sync state, point cpu->eip.u32 at this op's offset (only when we may
    // be in a mixed JIT/emulate block — for pure-emulate blocks each
    // helper's NEXT() advances eip naturally and no writeEip is needed),
    // call runNextSingleOp, reload.
    //
    // Mixed blocks (an inline JIT op followed by emulateSingleOp) need this:
    // the JIT op doesn't update cpu->eip, so the helper would re-decode at
    // the previous emulated op's eip and run the wrong instruction.
    //
    // We can't easily know at compile time whether an inline op preceded
    // this one (the previous op may have used emulateSingleOp), so emit
    // writeEip when the current op is *not* the first op in the block.
    // When it's the first op, cpu->eip was set by the dispatcher.
    if (this->currentEip != this->startingEip) {
        writeEip(this->currentEip - cpu->seg[CS].address);
    }
    syncDirtyRegsToHost();
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((U32)(m_currentWasmOp ? m_currentWasmOp->inst : InstructionCount));
    m_emitter.emitI32Store((U32)offsetof(CPU, tmpReg));
    if (m_currentWasmOp && m_currentWasmOp->inst == Movsd) {
        U32 movsdShape = 0;
        if (m_currentWasmOp->repZero || m_currentWasmOp->repNotZero) {
            movsdShape |= WASM_JIT_PROFILE_MOVSD_REP;
        }
        if (m_currentWasmOp->ea16) {
            movsdShape |= WASM_JIT_PROFILE_MOVSD_EA16;
        }
        if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[m_currentWasmOp->base]) {
            movsdShape |= WASM_JIT_PROFILE_MOVSD_SEGMENTED;
        }
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Const(movsdShape);
        m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    }
#endif
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperEmulateSingleOpIdx);
    m_gpLoaded.fill(false);
    m_segLoaded.fill(false);
    // The interpreter may have updated cpu->lazyFlagType to any value.
    // Reset the compile-time cache so getCondition uses the safe runtime-read
    // path rather than emitting a guard based on a stale expected flag type.
    currentLazyFlags = FLAGS_NULL;
}

void JitWasmCodeGen::fallbackToEmulateSingleOp(const char* family) {
#ifdef BOXEDWINE_WASM_JIT_FALLBACK_STATS
    static std::mutex fallbackStatsMutex;
    static std::unordered_map<const char*, U64> fallbackStats;
    static constexpr U32 FALLBACK_TOP_COUNT = 8;
    static U64 fallbackTotal = 0;
    U64 total;
    bool shouldLog;
    const char* topFamilies[FALLBACK_TOP_COUNT] = {};
    U64 topCounts[FALLBACK_TOP_COUNT] = {};
    char topText[256] = {};

    {
        std::lock_guard<std::mutex> lock(fallbackStatsMutex);
        ++fallbackStats[family];
        total = ++fallbackTotal;
        shouldLog = (total % 5) == 0;

        if (shouldLog) {
            for (const auto& entry : fallbackStats) {
                for (U32 i = 0; i < FALLBACK_TOP_COUNT; ++i) {
                    if (entry.second > topCounts[i]) {
                        for (U32 j = FALLBACK_TOP_COUNT - 1; j > i; --j) {
                            topCounts[j] = topCounts[j - 1];
                            topFamilies[j] = topFamilies[j - 1];
                        }
                        topCounts[i] = entry.second;
                        topFamilies[i] = entry.first;
                        break;
                    }
                }
            }
            U32 offset = 0;
            for (U32 i = 0; i < FALLBACK_TOP_COUNT && topCounts[i]; ++i) {
                int written = std::snprintf(
                    topText + offset,
                    sizeof(topText) - offset,
                    "%s%s:%llu",
                    i ? "," : "",
                    topFamilies[i],
                    (unsigned long long)topCounts[i]);
                if (written < 0) {
                    break;
                }
                if ((U32)written >= sizeof(topText) - offset) {
                    offset = sizeof(topText) - 1;
                    break;
                }
                offset += (U32)written;
            }
        }
    }

    if (shouldLog) {
        klog_fmt("[WASM JIT fallback] total=%llu top=%s",
            (unsigned long long)total, topText);
    }
#else
    (void)family;
#endif
    emulateSingleOp();
}

// ---------------------------------------------------------------------------
// Direct operations (cmp/test/jmp for JIT optimization paths)
// ---------------------------------------------------------------------------
// Stage a JitWidth::bN store of dst/src/result into cpu->{dst,src,result}.u32
// plus cpu->lazyFlagType. The helper-based condition accessors read these to
// compute the flag, so direct-mode needs to populate them even though it
// normally would skip lazy-flag storage.
static LazyFlagType lazyTypeForTest(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return FLAGS_TEST8;
    case JitWidth::b16: return FLAGS_TEST16;
    default:            return FLAGS_TEST32;
    }
}
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, RegPtr right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    if (left && left->emulatedReg == 0xff) freeScratch(left->hardwareReg());
    pushRegValue(right);
    if (right && right->emulatedReg == 0xff) freeScratch(right->hardwareReg());
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForSub(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForSub(w);
}
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, U32 right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    if (left && left->emulatedReg == 0xff) freeScratch(left->hardwareReg());
    m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForSub(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForSub(w);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, RegPtr right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    pushRegValue(right);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForTest(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForTest(w);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, U32 right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForTest(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForTest(w);
}
void JitWasmCodeGen::direct_jump(JitConditional cond, U32 address) {
    JumpIfCondition(cond, address);
}
void JitWasmCodeGen::direct_cmov(JitWidth w, JitConditional cond, RegPtr dst, RegPtr src) {
    IfCondition(cond);
    mov(w, dst, src);
    EndIf();
}
void JitWasmCodeGen::direct_setcc(JitConditional cond, RegPtr dst) {
    auto r = getCondition(cond, dst);
    if (r != dst) mov(JitWidth::b8, dst, r);
}
bool JitWasmCodeGen::directDoesAffectFlags(DecodedOp* op) { return false; }


// ---------------------------------------------------------------------------
// Code management (buffer tracking for branch patching)
// For WASM: we don't do binary patching; use structural control flow.
// These are stubs required by the JitCodeGen interface.
// ---------------------------------------------------------------------------
U32  JitWasmCodeGen::getBufferSize()             { return (U32)m_patchBuffer.size(); }
U32  JitWasmCodeGen::markBufferLocation()        { U32 id = (U32)m_patchLocations.size(); m_patchLocations.push_back((U32)m_patchBuffer.size()); return id; }
U32  JitWasmCodeGen::getBufferLocation(U32 id)   { return id < m_patchLocations.size() ? m_patchLocations[id] : 0; }
void JitWasmCodeGen::copyBuffer(U8* dst, U32 sz) { /* WASM binary managed separately */ }
// Base postCompile checks getIfJumpSize() == 0 to verify all `If`s emitted
// during the op had matching `EndIf`s (in native JITs the size of pending
// branch fixups is non-zero while an If is open). WASM uses structural
// control flow (`if … end`) where the validator catches mismatches itself,
// so we always report 0 here.
U32  JitWasmCodeGen::getIfJumpSize()             { return 0; }

// MarkJumpLocation / Goto: used by JitCodeGen for intra-block branch patching
// in the x86/ARM backends. WASM uses structural control flow, so for
// MarkJumpLocation/Goto specifically there is no patching to do — the loops
// that actually need backward branches use LoopBegin/Goto/LoopEnd instead
// (see the override of those below). Goto remains generic: it dispatches on
// whether `location` is a real loop token (encoded by LoopBegin) or a
// MarkJumpLocation buffer offset (in which case nothing to do).
//
// To distinguish, LoopBegin returns a token tagged with the high bit set
// (kLoopTokenTag); MarkJumpLocation's buffer-offset return value never sets
// it because compiled bodies are far smaller than 2 GiB.
static constexpr U32 kLoopTokenTag = 0x80000000u;

U32 JitWasmCodeGen::MarkJumpLocation() { return markBufferLocation(); }

void JitWasmCodeGen::Goto(U32 location) {
    if ((location & kLoopTokenTag) == 0) {
        // MarkJumpLocation token — x86/ARM patching path; nothing to do.
        return;
    }
    U32 loopDepth = location & ~kLoopTokenTag;
    U32 currentDepth = m_emitter.currentCtrlDepth();
    // The loop frame sits `currentDepth - loopDepth` levels above the
    // current innermost frame. `br N` targets that frame's label, which
    // for a `loop` is the *top* of the loop — i.e., a backward branch.
    branchBoundary();
    m_emitter.emitBr(currentDepth - loopDepth);
}

U32 JitWasmCodeGen::LoopBegin() {
    // Emit `loop` and capture the depth right after it's pushed. The
    // returned token encodes the loop frame's depth so Goto can compute
    // a relative `br` argument from wherever it's called inside the body.
    branchBoundary();
    m_emitter.emitLoop();
    U32 depth = m_emitter.currentCtrlDepth();
    return kLoopTokenTag | depth;
}

void JitWasmCodeGen::LoopEnd() {
    branchBoundary();
    m_emitter.emitEnd();
}

// ---------------------------------------------------------------------------
// Module instantiation hooks
// ---------------------------------------------------------------------------
U8* JitWasmCodeGen::createSyncToHost()    { return nullptr; } // handled inline
U8* JitWasmCodeGen::createSyncFromHost()  { return nullptr; }
U8* JitWasmCodeGen::createBlockExit()     { return nullptr; }
// doJIT panics if calculateCF[0] == nullptr. createCalculationCF (non-virtual)
// calls createDynamicExecutableMemory, so returning a non-null stub here
// satisfies the check. The pointer is never actually called from WASM since
// nakedCall() is a no-op in this backend.
static void wasmDummyCode(CPU*) {}
U8* JitWasmCodeGen::createDynamicExecutableMemory(U32* pSize) {
    return (U8*)(void*)wasmDummyCode;
}

void JitWasmCodeGen::createHelpers() {
    // JitSSE helper generation builds XMM-based cos helpers and requires real
    // XMM hook implementations. Keep the previous empty WASM/JitMMX behavior
    // until the SSE hooks are lowered instead of panic stubs.
}

U8* JitWasmCodeGen::createStartJITCode() {
    // Return the address of the static wasmStartJITOp function.
    // NormalCPU::run() uses this as process->startJITOp.
    return (U8*)wasmStartJITOp;
}

// ---------------------------------------------------------------------------
// Compilation lifecycle
// ---------------------------------------------------------------------------
void JitWasmCodeGen::findDirectLoopCandidate(DecodedOp* op) {
    m_directLoopTargetEip = 0;
    m_directLoopSourceEip = 0;
    m_directLoopOpCount = 0;
    m_directLoopToken = 0;
    m_hasDirectLoopCandidate = false;
    m_directLoopOpen = false;

    U32 eip = this->startingEip;
    DecodedOp* cur = op;
    while (cur && eip <= this->lastOpEip) {
        if (cur->isDirectJumpBranch()) {
            U32 targetEip = eip + cur->len + cur->imm;
            if (targetEip >= this->startingEip && targetEip < eip &&
                targetEip <= this->lastOpEip &&
                (!m_hasDirectLoopCandidate || targetEip > m_directLoopTargetEip)) {
                U32 checkEip = this->startingEip;
                DecodedOp* check = op;
                while (check && checkEip < targetEip) {
                    checkEip += check->len;
                    check = check->next;
                }
                if (check && checkEip == targetEip) {
                    m_directLoopTargetEip = targetEip;
                    m_directLoopSourceEip = eip;
                    m_hasDirectLoopCandidate = true;
                }
            }
        }
        eip += cur->len;
        cur = cur->next;
    }

    if (m_hasDirectLoopCandidate) {
        eip = this->startingEip;
        cur = op;
        while (cur && eip <= m_directLoopSourceEip) {
            if (eip >= m_directLoopTargetEip) {
                m_directLoopOpCount++;
            }
            eip += cur->len;
            cur = cur->next;
        }
    }
}

bool JitWasmCodeGen::emitDirectLoopBackedge(U32 address) {
    if (!m_directLoopOpen || address != m_directLoopTargetEip ||
        currentEip != m_directLoopSourceEip) {
        return false;
    }

    // Make the loop header's reloads observe all guest-register changes from
    // this iteration. EIP is also kept architecturally current for helpers,
    // signals, and the budget-exhausted dispatcher path.
    syncDirtyRegsToHost();
    writeEip(address - cpu->seg[CS].address);

    m_emitter.emitLocalGet(WASM_DIRECT_LOOP_BUDGET_LOCAL);
    m_emitter.emitI32Const(1);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalTee(WASM_DIRECT_LOOP_BUDGET_LOCAL);
    // Booleanize before the bitwise AND with !wasmJitBailout.
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmJitBailout));
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.setNextBranchHint(WasmBranchHint::Likely);
    m_emitter.emitIf();

    // The outer dispatcher accounts for the function's first pass. Each
    // successful backedge starts one additional pass through this loop body.
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, blockInstructionCount));
    m_emitter.emitI32Const((S32)m_directLoopOpCount);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Store((U32)offsetof(CPU, blockInstructionCount));

    U32 currentDepth = m_emitter.currentCtrlDepth();
    m_emitter.emitBr(currentDepth - m_directLoopToken);
    m_emitter.emitEnd();
    return true;
}

void JitWasmCodeGen::preCompile(DecodedOp* op, bool skippedOp) {
    // Reset per-instruction scratch state, then run base bookkeeping (block
    // op count, eip→buffer-pos map, preOp hook). Stash op->len for
    // emitBailoutCheck — it needs the post-write next-op EIP.
    if (this->blockOpCount == 0) {
        m_wasmBlockStartOp = op;
        m_preCompileBlockHash = 2166136261u;
        m_manifestNext1Target = 0;
        m_manifestNext2Target = 0;
        m_manifestJumpTarget = 0;
        m_manifestNext1Count = 0;
        m_manifestNext2Count = 0;
        m_manifestJumpCount = 0;
        findDirectLoopCandidate(op);
        if (m_profileSplitBlockStartEip != this->startingEip) {
            m_profileSplitTargetOp = nullptr;
            m_profileSplitBlockStartEip = 0;
            m_profileSplitTargetEip = 0;
        }
    }
    if (wasmJitPersistenceActive()) {
        if (op->isStringOp()) {
            // String ops reuse decoded fields as runtime profiling/adaptation
            // counters before JIT kicks in. For persisted modules those values
            // make cache keys depend on warmup history, and direction-
            // specialized cached code could be wrong if DF differs on a later
            // run. Resetting these fields makes the wasm string-op emitters
            // choose their generic both-direction path and gives us a stable
            // decoded-op hash.
            op->STR_COUNT = 0;
            op->STR_TOTAL = 0;
            op->DF0 = 0;
            op->DF1 = 0;
        }
        m_preCompileBlockHash = wasmJitHashDecodedOp(m_preCompileBlockHash, op);
    }
    m_currentWasmOp = op;
    m_scratchInUse.fill(false);
    m_f64ScratchInUse.fill(false);
    m_v128ScratchInUse.fill(false);
    lastCompiledOpLen = op->len;

    if (!skippedOp && m_hasDirectLoopCandidate && !m_directLoopOpen &&
        this->currentEip == m_directLoopTargetEip) {
            // Synchronize the one-time linear entry before opening the loop.
            // The compile-time caches are then cleared so loads emitted in
            // the body execute on every backedge and observe flushed values.
            branchBoundary();
            m_emitter.emitI32Const((S32)WASM_DIRECT_LOOP_ITERATIONS);
            m_emitter.emitLocalSet(WASM_DIRECT_LOOP_BUDGET_LOCAL);
            m_emitter.emitLoop();
            m_directLoopToken = m_emitter.currentCtrlDepth();
            m_directLoopOpen = true;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
            g_wasmJitProfileDirectLoopsCreated.fetch_add(1, std::memory_order_relaxed);
            g_wasmJitProfileDirectLoopOps.fetch_add(m_directLoopOpCount, std::memory_order_relaxed);
            g_wasmJitProfileDirectLoopIterations.fetch_add(WASM_DIRECT_LOOP_ITERATIONS, std::memory_order_relaxed);
            if (m_directLoopOpCount <= 4) {
                g_wasmJitProfileDirectLoopOps1To4.fetch_add(1, std::memory_order_relaxed);
            } else if (m_directLoopOpCount <= 8) {
                g_wasmJitProfileDirectLoopOps5To8.fetch_add(1, std::memory_order_relaxed);
            } else if (m_directLoopOpCount <= 16) {
                g_wasmJitProfileDirectLoopOps9To16.fetch_add(1, std::memory_order_relaxed);
            } else {
                g_wasmJitProfileDirectLoopOps17Plus.fetch_add(1, std::memory_order_relaxed);
            }
#endif
            m_gpDirty.fill(false);
            m_gpLoaded.fill(false);
            m_segLoaded.fill(false);
            currentLazyFlags = FLAGS_NULL;
        }
    JitCodeGen::preCompile(op, skippedOp);
}

void JitWasmCodeGen::compile(DecodedOp* op) {
    if (op->inst == Pause) {
        dynamic_pause(op);
        return;
    }
    if (op->inst == Int80) {
        emulateSingleOp();
        blockExit();
        return;
    }
    if (op->inst == MovsdXmmE64) {
        dynamic_movsdXmmE64(op);
        return;
    }
    if (op->inst == MovsdE64Xmm) {
        dynamic_movsdE64Xmm(op);
        return;
    }
    JitCodeGen::compile(op); // delegates to dynamic_* dispatch
}

void JitWasmCodeGen::postCompile(DecodedOp* op) {
    // Critical: base class advances currentEip by op->len. Without delegating,
    // every branch op compiled later in the block computes its target from a
    // stale block-start currentEip, not the op's own position. That made
    // CallJw with imm=0x123 land at start+0x123 instead of start+0x126.
    JitCodeGen::postCompile(op);
}

bool JitWasmCodeGen::shouldStopBlockBefore(U32 eip, DecodedOp* op) {
#if !defined(BOXEDWINE_MULTI_THREADED) && !defined(__TEST)
    // Honor profile-guided split hints from the grouped manifest: end the
    // block before a hot interior target so that target compiles (and is
    // cached) as its own block-start entry, making it reachable by the
    // grouped direct-call machinery. The JS side keeps the hint map; without
    // an active persistence session there are no hints and no EM_JS call.
    if (!wasmJitPersistenceActive() || !op || eip == this->startingEip) {
        return false;
    }
    if (boxedwine_wasm_profile_split_before(
        this->startingEip - cpu->seg[CS].address,
        eip - cpu->seg[CS].address) != 0) {
        m_profileSplitTargetOp = op;
        m_profileSplitBlockStartEip = this->startingEip;
        m_profileSplitTargetEip = eip;
        return true;
    }
    return false;
#else
    return false;
#endif
}

void JitWasmCodeGen::commitJIT(DecodedOp* op) {
    // Finalize the WASM function body and the module binary.
    if (m_directLoopOpen) {
        m_emitter.emitEnd();
        m_directLoopOpen = false;
    }
    m_emitter.endFunction();
    m_wasmBinary = m_emitter.finalize();

    if (m_wasmBinary.empty()) return;

    // Cache identity for the persistence mode. Cheap relative to module
    // emission; computed unconditionally so the flow below stays simple.
    // m_preCompileBlockHash only accumulates while persistence is active, but
    // it is never consulted unless the mode is active either.
    const U32 blockHash = wasmJitFinalizeBlockHash(m_preCompileBlockHash, this->blockOpCount, this->emulatedLen);
    const U32 cacheEip = this->startingEip - cpu->seg[CS].address;

    // Long-lived copy of the relocation slot values (persistence mode only;
    // empty otherwise). Supplied to the instance as the relocBase global.
    // On a cached-bytes hit the values still apply: the cached module was
    // emitted by this same build from the identical decoded chain (the block
    // hash guards the chain; cache zips are per-build artifacts), so its slot
    // layout matches the one this codegen pass just produced.
    U32* relocArr = nullptr;
    if (!m_relocValues.empty()) {
        relocArr = new U32[m_relocValues.size()];
        memcpy(relocArr, m_relocValues.data(), m_relocValues.size() * sizeof(U32));
    }

    // Pass the helper function table to the JS instantiator.
    // In pthreads builds use the Atomics-based allocator; in single-threaded
    // builds use the standard addFunction() wrapper.
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    U64 instantiateStartUs = wasmJitProfileNowNs();
#endif
#ifdef BOXEDWINE_MULTI_THREADED
    // When persistence is active, check the piped-group registry first, then
    // the flat persistent cache (primed from the server zip before main()
    // ran, extended by every worker as it compiles). On a group hit the
    // merged module's export is published at a fresh slot and the group's
    // per-(entry, process) reloc array becomes the block's array — table
    // calls and intra-group direct calls then read the same slots, so no
    // ST-style refresh copy is needed.
    std::vector<U8> cachedBytes;
    bool usedGroup = false;
    int tableIdx = -1;
    if (wasmJitPersistenceActive()) {
        const U8* groupBytes = nullptr;
        int groupSize = 0;
        const char* groupExportName = nullptr;
        U32* groupArr = nullptr;
        U32 groupIdx = 0, entryIdx = 0;
        {
            std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
            const uint64_t key = wasmJitCacheKey(cacheEip, blockHash);
            auto git = g_wasmMtGroupByKey.find(key);
            if (git != g_wasmMtGroupByKey.end()) {
                groupIdx = git->second.first;
                entryIdx = git->second.second;
                const WasmJitMtGroupEntry& entry = g_wasmMtGroups[groupIdx].entries[entryIdx];
                if (entry.relocCount == (U32)m_relocValues.size()) {
                    WasmJitMtGroupProcState& st =
                        wasmJitMtGroupProcStateLocked(groupIdx, (U32)(uintptr_t)cpu->memory);
                    groupArr = st.entryArrays[entryIdx];
                    if (groupArr && !m_relocValues.empty()) {
                        memcpy(groupArr, m_relocValues.data(), m_relocValues.size() * sizeof(U32));
                    }
                    groupBytes = st.patchedBytes.data();
                    groupSize = (int)st.patchedBytes.size();
                    groupExportName = entry.exportName.c_str();
                } else {
                    static bool warned = false;
                    if (!warned) {
                        warned = true;
                        klog_fmt("[WASM JIT MT group] relocCount mismatch eip=%x manifest=%u runtime=%u (using flat path)",
                            cacheEip, entry.relocCount, (U32)m_relocValues.size());
                    }
                }
            }
            if (!groupBytes) {
                auto it = g_wasmCacheByKey.find(key);
                if (it != g_wasmCacheByKey.end()) {
                    cachedBytes = it->second;
                }
            }
        }
        if (groupBytes) {
            // Compile/instantiate outside the mutex — first touch of a large
            // merged group should not serialize other workers' commits.
            tableIdx = boxedwine_wasm_instantiate_group_mt(
                groupBytes, groupSize,
                g_wasmHelperTable, WASM_HELPER_COUNT,
                &g_wasmTableNextSlot,
                groupExportName, (int)groupIdx, (U32)(uintptr_t)cpu->memory);
            if (tableIdx >= 0) {
                usedGroup = true;
                std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
                if (groupArr) {
                    g_wasmRelocArrays[tableIdx] = groupArr;
                    g_wasmHasRelocArrays.store(true, std::memory_order_relaxed);
                }
                g_wasmMtGroupSlotRefs[tableIdx] = { groupIdx, entryIdx, (U32)(uintptr_t)cpu->memory };
            }
            // On instantiation failure fall through to the flat/fresh path
            // (cachedBytes was not read for a group hit; the fresh binary
            // still works — the group entry array simply stays live for any
            // direct calls that already target it).
        }
    }
    const bool usingCached = !cachedBytes.empty();
    const void* instBytes = usingCached ? cachedBytes.data() : m_wasmBinary.data();
    const int instSize = usingCached ? (int)cachedBytes.size() : (int)m_wasmBinary.size();
    if (tableIdx < 0) {
        tableIdx = boxedwine_wasm_instantiate_mt(
            instBytes,
            instSize,
            g_wasmHelperTable,
            WASM_HELPER_COUNT,
            &g_wasmTableNextSlot
        );
    }
#else
  #ifndef __TEST
    // When persistence is active, try the persistent cache first and save
    // fresh compiles; a plain session goes straight to instantiation.
    int tableIdx = -1;
    if (wasmJitPersistenceActive()) {
        tableIdx = boxedwine_wasm_lookup_cached(
            cacheEip, blockHash, g_wasmHelperTable, WASM_HELPER_COUNT,
            (U32)(uintptr_t)relocArr, (U32)m_relocValues.size(),
            // Per-process identity: DecodedOp pointers are only meaningful
            // within one guest address space, so grouped reloc state is
            // keyed by the KMemory that owns this block.
            (U32)(uintptr_t)cpu->memory);
    }
    if (tableIdx < 0) {
        tableIdx = boxedwine_wasm_instantiate(
            m_wasmBinary.data(),
            (int)m_wasmBinary.size(),
            g_wasmHelperTable,
            WASM_HELPER_COUNT
        );
        if (tableIdx >= 0 && wasmJitPersistenceActive()) {
            boxedwine_wasm_save_block(
                cacheEip,
                blockHash,
                m_wasmBinary.data(),
                (int)m_wasmBinary.size(),
                this->blockOpCount,
                this->emulatedLen,
                m_manifestNext1Target,
                m_manifestNext2Target,
                m_manifestJumpTarget,
                m_manifestNext1Count,
                m_manifestNext2Count,
                m_manifestJumpCount,
                (U32)offsetof(CPU, blockInstructionCount),
                (U32)offsetof(CPU, yield),
                (U32)(uintptr_t)&contextTimeRemaining,
                (U32)m_relocValues.size());
        }
    }
  #else
    int tableIdx = boxedwine_wasm_instantiate(
        m_wasmBinary.data(),
        (int)m_wasmBinary.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT
    );
  #endif
#endif
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileAddElapsed(g_wasmJitProfileInstantiateUs, instantiateStartUs);
#endif

    if (tableIdx < 0) {
        delete[] relocArr;
        // Instantiation failed — fall back to the normal CPU interpreter
        // for every op in the block. Without this, op->pfn stays as
        // firstDynamicOp, which would recompile-then-call-self every time
        // the op runs → infinite loop.
        DecodedOp* cur = op;
        while (cur) {
            cur->pfn = NormalCPU::getFunctionForOp(cur);
            cur->pfnJitCode = nullptr;
            if (cur->next == nullptr) break;
            cur = cur->next;
        }
        return;
    }

    if (relocArr) {
        // Register the slot array under the table index so every call can
        // pass it as the block's second parameter (wasmJitRelocBaseForTable)
        // and clearJitBlock can free it with the block (ST). A merged-group
        // install can hand back a table index that already has an array from
        // an earlier commit of the same key; keep the existing one (its
        // values are identical — same key, same build, same decode).
#ifdef BOXEDWINE_MULTI_THREADED
        if (usedGroup) {
            // The group's per-(entry, process) array is already registered
            // for this slot and holds the same values; the standalone copy
            // is not needed.
            delete[] relocArr;
            relocArr = nullptr;
        } else {
            std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
            if (!g_wasmRelocArrays.emplace(tableIdx, relocArr).second) {
                delete[] relocArr;
            }
            g_wasmHasRelocArrays.store(true, std::memory_order_relaxed);
        }
#else
        if ((size_t)tableIdx >= g_wasmRelocBaseByTable.size()) {
            g_wasmRelocBaseByTable.resize(tableIdx + 1, nullptr);
        }
        if (g_wasmRelocBaseByTable[tableIdx]) {
            delete[] relocArr;
        } else {
            g_wasmRelocBaseByTable[tableIdx] = relocArr;
        }
#endif
    }

#ifdef BOXEDWINE_MULTI_THREADED
    {
        std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
        // Keep the per-slot bytes consistent with what was instantiated so
        // lazy cross-worker installs run the same module. Group slots are
        // covered by g_wasmMtGroupSlotRefs instead (lazy installs route to
        // the per-worker group instance).
        if (!usedGroup) {
            g_wasmBlockBinaries[tableIdx] = usingCached ? cachedBytes : m_wasmBinary;
        }
        if (wasmJitPersistenceActive() && !usingCached && !usedGroup) {
            g_wasmCacheByKey[wasmJitCacheKey(cacheEip, blockHash)] = m_wasmBinary;
            // Exit metadata for the exported manifest (the ST build records
            // this in Module.wasmJitBlockMeta from boxedwine_wasm_save_block;
            // workers can't reach that Map, so keep it here and mirror it in
            // wasm_jit_mt_prepare_export).
            g_wasmCacheMetaByKey[wasmJitCacheKey(cacheEip, blockHash)] = {
                this->blockOpCount,
                this->emulatedLen,
                m_manifestNext1Target,
                m_manifestNext2Target,
                m_manifestJumpTarget,
                m_manifestNext1Count,
                m_manifestNext2Count,
                m_manifestJumpCount,
                (U32)m_relocValues.size(),
            };
        }
    }
#endif

    // Walk all ops in this block. The first op gets the startJITOp entry;
    // interior ops keep normal dispatch and only record the owner block.
    // The owner's invalidation sweep frees any subblock table entries it
    // finds in the covered range. Only callable entries get OP_FLAG_JIT.
    op->blockLen     = this->emulatedLen;
    op->blockOpCount = this->blockOpCount;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileCompiledBlock(this->blockOpCount);
#endif
    DecodedOp* cur = op;
    for (U32 i = 0; i < this->blockOpCount && cur; i++) {
        DecodedOp* owner = cur->blockStart;
        if (!owner || owner->blockLen < this->emulatedLen) {
            owner = op;
        }
        cur->blockStart = owner;

        if (cur == op) {
            cur->pfnJitCode = (void*)(uintptr_t)(U32)tableIdx;
            cur->flags |= OP_FLAG_JIT;
            cur->pfn = cpu->thread->process->startJITOp;
            if (m_needsWasmMemoryPageArrays) {
                cur->flags2 |= OP_FLAG2_WASM_JIT_MEM_ARRAYS;
            } else {
                cur->flags2 &= ~OP_FLAG2_WASM_JIT_MEM_ARRAYS;
            }
        } else if (!cur->pfnJitCode || cur->pfn != cpu->thread->process->startJITOp) {
            cur->pfnJitCode = nullptr;
            cur->flags &= ~OP_FLAG_JIT;
            cur->flags2 &= ~OP_FLAG2_WASM_JIT_MEM_ARRAYS;
            cur->pfn = NormalCPU::getFunctionForOp(cur);
        }
        cur = cur->next;
    }

#ifndef BOXEDWINE_MULTI_THREADED
    // The split-shaped prefix block has committed; the hinted target op keeps
    // its normal pfn and compiles as its own block-start entry when execution
    // reaches it.
    m_profileSplitTargetOp = nullptr;
    m_profileSplitBlockStartEip = 0;
    m_profileSplitTargetEip = 0;
#endif
}

// ---------------------------------------------------------------------------
// Factory function: create a new WASM JIT backend instance.
// This is called by the shared JIT infrastructure in jitCodeGen.cpp.
// ---------------------------------------------------------------------------
JitCodeGen* startNewJIT(CPU* cpu) {
    return new JitWasmCodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    U64 jitStartUs = wasmJitProfileNowNs();
#endif
    JitCodeGen* jit = startNewJIT(cpu);
    jit->doJIT(address, op);
    delete jit;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileAddElapsed(g_wasmJitProfileJitUs, jitStartUs);
#endif
}

// ---------------------------------------------------------------------------
// clearJitBlock: called by kmemory when evicting JIT blocks.
// For WASM, release the compiled function from wasmTable.
// ---------------------------------------------------------------------------
void clearJitBlock(const std::vector<void*>& jitOps) {
    // A parent block can contain subblock entries with distinct wasmTable
    // indexes, so dedupe before freeing.
    std::set<int> seen;
    for (void* p : jitOps) {
        if (!p) continue;
        int tableIdx = (int)(uintptr_t)p;
        if (seen.insert(tableIdx).second) {
#ifdef BOXEDWINE_MULTI_THREADED
            boxedwine_wasm_free_block_mt(tableIdx);
            // The reloc array intentionally follows the monotonic MT slot
            // lifetime (never freed) — a racing worker may still run the
            // instance once after eviction, exactly like the bytes in
            // g_wasmBlockBinaries.
#else
            boxedwine_wasm_free_block(tableIdx);
            // The function is now unreachable (removeFunction); release its
            // relocation slot array. Every pointer in it shared this block's
            // lifetime, so nothing else can reference the array.
            if ((size_t)tableIdx < g_wasmRelocBaseByTable.size() &&
                g_wasmRelocBaseByTable[tableIdx]) {
                delete[] g_wasmRelocBaseByTable[tableIdx];
                g_wasmRelocBaseByTable[tableIdx] = nullptr;
            }
#endif
        }
    }
}

// ---------------------------------------------------------------------------
// JIT module persistence - zip import (single-threaded builds only).
//
// Export is handled in JavaScript by saveJitModules(). This import hook reads
// a zip staged at WASM_JIT_IMPORT_PATH on the virtual FS and stores each
// v5-xxxxxxxx-hhhhhhhh.wasm entry into the JS cache via
// wasm_jit_js_store_entry(). It doubles as the exported sentinel
// (Module._wasm_jit_import_from_file) that boxedwine-shell.js uses to detect
// a persistence-capable single-threaded build.
// ---------------------------------------------------------------------------
#if !defined(BOXEDWINE_MULTI_THREADED) && !defined(__TEST)

static const char WASM_JIT_IMPORT_PATH[] = "/tmp-jit-modules/wasm-jit-import.zip";

extern "C" void wasm_jit_import_from_file() {
    unzFile uf = unzOpen(WASM_JIT_IMPORT_PATH);
    if (!uf) return;
    int ret = unzGoToFirstFile(uf);
    while (ret == UNZ_OK) {
        char filename[32];
        unz_file_info fi;
        if (unzGetCurrentFileInfo(uf, &fi, filename, sizeof(filename),
                                   nullptr, 0, nullptr, 0) == UNZ_OK
            && fi.uncompressed_size > 0) {
            if (strncmp(filename, "v5-", 3) != 0) {
                ret = unzGoToNextFile(uf);
                continue;
            }
            uint32_t eip = (uint32_t)strtoul(filename + 3, nullptr, 16);
            const char* dash = strchr(filename + 3, '-');
            uint32_t blockHash = dash ? (uint32_t)strtoul(dash + 1, nullptr, 16) : 0;
            if (!blockHash) {
                ret = unzGoToNextFile(uf);
                continue;
            }
            std::vector<U8> data(fi.uncompressed_size);
            if (unzOpenCurrentFile(uf) == UNZ_OK) {
                unzReadCurrentFile(uf, data.data(), (unsigned)data.size());
                unzCloseCurrentFile(uf);
                wasm_jit_js_store_entry(eip, blockHash, data.data(), (int)data.size());
            }
        }
        ret = unzGoToNextFile(uf);
    }
    unzClose(uf);
}

#endif // !BOXEDWINE_MULTI_THREADED && !__TEST

#endif // BOXEDWINE_WASM_JIT
