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
#include "../common/cpu.h"
#include "../common/common_fpu.h"
#include "../common/common_sse2.h"
#include "../normal/normal_strings.h"
#include "../normal/normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../../softmmu/kmemory_soft.h"

#include <bit>      // std::bit_cast (C++20) — used in boxedwine_wasm_call_block
#include <emscripten.h>
#include <emscripten/em_js.h>
#include <mutex>
#include <unordered_map>

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

#ifdef BOXEDWINE_MULTI_THREADED
static constexpr U32 WASM_JIT_CHAIN_BLOCK_LIMIT = 1;
static constexpr U32 WASM_JIT_INTERIOR_BRIDGE_LIMIT = 0;
#else
static constexpr U32 WASM_JIT_CHAIN_BLOCK_LIMIT = 2048;
static constexpr U32 WASM_JIT_INTERIOR_BRIDGE_LIMIT = 128;
#endif

static inline bool wasmJitCanChainTo(CPU* cpu, DecodedOp* nextOp) {
    return nextOp &&
        (nextOp->flags & OP_FLAG_JIT) &&
        nextOp->pfnJitCode &&
        nextOp->blockStart == nextOp &&
        nextOp->pfn == cpu->thread->process->startJITOp;
}

static inline bool wasmJitCanBridgeInterior(CPU* cpu, DecodedOp* nextOp) {
    return nextOp &&
        (nextOp->flags & OP_FLAG_JIT) &&
        nextOp->pfnJitCode &&
        nextOp->blockStart != nextOp &&
        nextOp->pfn &&
        nextOp->pfn != cpu->thread->process->startJITOp;
}

static inline void wasmJitFinishBridgeOp(CPU* cpu, DecodedOp* op) {
    cpu->blockInstructionCount++;
    cpu->eip.u32 += op->len;
    cpu->nextOp = op->next ? op->next : cpu->getNextOp();
}

static inline bool wasmJitTryBridgeFastOp(CPU* cpu, DecodedOp* op) {
#ifdef BOXEDWINE_MULTI_THREADED
    return false;
#else
    if (!op) {
        return false;
    }
    switch (op->inst) {
    case MovR32E32:
        if (op->ea16) {
            return false;
        }
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa3(cpu, op));
        break;
    default:
        return false;
    }
    wasmJitFinishBridgeOp(cpu, op);
    return true;
#endif
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
static std::atomic<U64> g_wasmJitProfileInteriorByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileInteriorBlockStartByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileChainStopPfnByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileChainStopOtherByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileBridgeEntries{0};
static std::atomic<U64> g_wasmJitProfileBridgeToJit{0};
static std::atomic<U64> g_wasmJitProfileBridgeLimit{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen1{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen2{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen3To4{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen5To8{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen9To16{0};
static std::atomic<U64> g_wasmJitProfileBridgeLen17To31{0};
static std::atomic<U64> g_wasmJitProfileBridgeLenCap{0};
static std::atomic<U64> g_wasmJitProfileBridgeFastCalls{0};
static std::atomic<U64> g_wasmJitProfileBridgePfnCalls{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32ToJit{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Interior{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Null{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32NoJitFlag{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32NoTable{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Pfn{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Other{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Limit{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Ea16{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Ea32{0};
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32DstReg[8];
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32Rm[8];
static std::atomic<U64> g_wasmJitProfileBridgeMovR32E32NextByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileBridgeFastByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileBridgePfnByInst[InstructionCount];
static std::atomic<U64> g_wasmJitProfileBridgeNextByInst[InstructionCount];
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
static std::atomic<U64> g_wasmJitProfileHelperCond{0};
static std::atomic<U64> g_wasmJitProfileInlineCond{0};
static std::atomic<U64> g_wasmJitProfileCondByType[16];
static std::atomic<U64> g_wasmJitProfileCondByLazyFlag[52];
static std::atomic<U64> g_wasmJitProfileEmulateByInst[InstructionCount];
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
static std::atomic<U64> g_wasmJitProfileBridgeFastUs{0};
static std::atomic<U64> g_wasmJitProfileBridgePfnUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemReadUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWriteUs{0};
static std::atomic<U64> g_wasmJitProfileHelperMemWriteCheckUs{0};
static std::atomic<U64> g_wasmJitProfileHelperBlockEnterUs{0};
static std::atomic<U64> g_wasmJitProfileHelperEmulateUs{0};
static std::atomic<U64> g_wasmJitProfileHelperFlagsUs{0};
static std::atomic<U64> g_wasmJitProfileHelperCondUs{0};
static std::atomic<U32> g_wasmJitProfileLastLogMs{0};

#define WASM_JIT_PROFILE_TIMING_SAMPLE 1024

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
    explicit WasmJitProfileTimer(std::atomic<U64>& counter) : counter(counter), startNs(wasmJitProfileNowNs()) {}
    ~WasmJitProfileTimer() {
        wasmJitProfileAddElapsed(counter, startNs);
    }
private:
    std::atomic<U64>& counter;
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
    case AddR32R32: return "AddR32R32";
    case AddR32I32: return "AddR32I32";
    case AddE32I32: return "AddE32I32";
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
    U64 memArrayChecks = g_wasmJitProfileMemArrayChecks.load(std::memory_order_relaxed);
    U64 jitUs = g_wasmJitProfileJitUs.load(std::memory_order_relaxed);
    U64 instantiateUs = g_wasmJitProfileInstantiateUs.load(std::memory_order_relaxed);
    U64 startUs = g_wasmJitProfileStartUs.load(std::memory_order_relaxed);
    U64 startPreCallUs = g_wasmJitProfileStartPreCallUs.load(std::memory_order_relaxed);
    U64 startPostCallUs = g_wasmJitProfileStartPostCallUs.load(std::memory_order_relaxed);
    U64 callBlockUs = g_wasmJitProfileCallBlockUs.load(std::memory_order_relaxed);
    U64 fetchNextUs = g_wasmJitProfileFetchNextUs.load(std::memory_order_relaxed);
    U64 bridgeFastUs = g_wasmJitProfileBridgeFastUs.load(std::memory_order_relaxed);
    U64 bridgePfnUs = g_wasmJitProfileBridgePfnUs.load(std::memory_order_relaxed);
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
    char topMovBridgeNext[512];
    char topBridgeFast[512];
    char topBridgePfn[512];
    char topBridgeNext[512];
    char topChainStopPfn[512];
    char topChainStopOther[512];
    char topFetchNextTarget[512];
    char topGenericExit[512];
    wasmJitProfileFormatTopCounters(topCond, sizeof(topCond), g_wasmJitProfileCondByType, 16, wasmJitProfileCondName);
    wasmJitProfileFormatTopCounters(topLazy, sizeof(topLazy), g_wasmJitProfileCondByLazyFlag, 52, wasmJitProfileLazyFlagName);
    wasmJitProfileFormatTopInstCounters(topEmulate, sizeof(topEmulate), g_wasmJitProfileEmulateByInst);
    wasmJitProfileFormatTopInstCounters(topInterior, sizeof(topInterior), g_wasmJitProfileInteriorByInst);
    wasmJitProfileFormatTopInstCounters(topInteriorBlockStart, sizeof(topInteriorBlockStart), g_wasmJitProfileInteriorBlockStartByInst);
    wasmJitProfileFormatTopInstCounters(topMovBridgeNext, sizeof(topMovBridgeNext), g_wasmJitProfileBridgeMovR32E32NextByInst);
    wasmJitProfileFormatTopInstCounters(topBridgeFast, sizeof(topBridgeFast), g_wasmJitProfileBridgeFastByInst);
    wasmJitProfileFormatTopInstCounters(topBridgePfn, sizeof(topBridgePfn), g_wasmJitProfileBridgePfnByInst);
    wasmJitProfileFormatTopInstCounters(topBridgeNext, sizeof(topBridgeNext), g_wasmJitProfileBridgeNextByInst);
    wasmJitProfileFormatTopInstCounters(topChainStopPfn, sizeof(topChainStopPfn), g_wasmJitProfileChainStopPfnByInst);
    wasmJitProfileFormatTopInstCounters(topChainStopOther, sizeof(topChainStopOther), g_wasmJitProfileChainStopOtherByInst);
    wasmJitProfileFormatTopInstCounters(topFetchNextTarget, sizeof(topFetchNextTarget), g_wasmJitProfileFetchNextTargetByInst);
    wasmJitProfileFormatTopInstCounters(topGenericExit, sizeof(topGenericExit), g_wasmJitProfileGenericExitByInst);
    klog_fmt("[WASM JIT profile] start=%llu call_block=%llu block_exit=%llu slot_miss=%llu "
             "compiled=%llu avg_ops=%llu.%llu ops[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17+=%llu] "
             "exits[next1=%llu next2=%llu jump=%llu generic=%llu] "
             "helpers[fetch_next=%llu mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu inline_cond=%llu] "
             "fetch_next_target[jit=%llu interior=%llu null=%llu no_flag=%llu no_table=%llu pfn=%llu other=%llu] "
             "chain[next_jit=%llu plain=%llu mem_arrays=%llu not_jit=%llu null=%llu pct=%llu] "
             "chain_stop[null=%llu no_flag=%llu no_table=%llu interior=%llu pfn=%llu other=%llu] "
             "interior_idx[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17+=%llu] "
             "interior_byte[1-15=%llu 16-31=%llu 32-63=%llu 64-127=%llu 128+=%llu] "
             "bridge[entries=%llu to_jit=%llu limit=%llu] "
             "bridge_len[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17+=%llu cap=%llu] "
             "bridge_exec[fast=%llu pfn=%llu] "
             "bridge_mov_r32_e32[total=%llu to_jit=%llu interior=%llu null=%llu no_flag=%llu no_table=%llu pfn=%llu other=%llu limit=%llu ea16=%llu ea32=%llu dst0=%llu dst1=%llu dst2=%llu dst3=%llu dst4=%llu dst5=%llu dst6=%llu dst7=%llu rm0=%llu rm1=%llu rm2=%llu rm3=%llu rm4=%llu rm5=%llu rm6=%llu rm7=%llu] "
             "loop[entries=%llu extra=%llu avg_blocks=%llu.%llu limit=%llu] "
             "loop_len[1=%llu 2=%llu 3-4=%llu 5-8=%llu 9-16=%llu 17-32=%llu 33-64=%llu 65-128=%llu 129-511=%llu 512-1023=%llu 1024-2047=%llu cap=%llu] "
             "mem_arrays[checks=%llu refresh=%llu] "
             "time_us[jit=%llu instantiate=%llu codegen=%llu start=%llu start_pre=%llu start_post=%llu call_block=%llu fetch_next=%llu bridge_fast=%llu bridge_pfn=%llu "
             "mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu] "
             "avg_ns[start=%llu start_pre=%llu start_post=%llu call_block=%llu fetch_next=%llu bridge_fast=%llu bridge_pfn=%llu mem_r=%llu mem_w=%llu mem_wc=%llu enter=%llu emulate=%llu flags=%llu cond=%llu] "
             "movsd[total=%llu rep=%llu single=%llu ea16=%llu ea32=%llu seg=%llu flat=%llu df1=%llu df0=%llu rep_flat=%llu rep_seg=%llu rep_ea16=%llu] "
             "cond_top[%s] lazy_top[%s] emulate_top[%s] interior_top[%s] interior_block_top[%s] "
             "bridge_mov_next_top[%s] bridge_fast_top[%s] bridge_pfn_top[%s] bridge_next_top[%s] "
             "chain_pfn_top[%s] chain_other_top[%s] fetch_next_top[%s] generic_exit_top[%s]",
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
             (unsigned long long)g_wasmJitProfileBridgeEntries.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeToJit.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLimit.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen1.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen2.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen3To4.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen5To8.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen9To16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLen17To31.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeLenCap.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeFastCalls.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgePfnCalls.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32ToJit.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Interior.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Null.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32NoJitFlag.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32NoTable.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Pfn.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Other.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Limit.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Ea16.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Ea32.load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[0].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[1].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[2].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[3].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[4].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[5].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[6].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32DstReg[7].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[0].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[1].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[2].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[3].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[4].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[5].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[6].load(std::memory_order_relaxed),
             (unsigned long long)g_wasmJitProfileBridgeMovR32E32Rm[7].load(std::memory_order_relaxed),
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
             (unsigned long long)wasmJitProfileUs(bridgeFastUs),
             (unsigned long long)wasmJitProfileUs(bridgePfnUs),
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
             (unsigned long long)wasmJitProfileAvgNs(bridgeFastUs, g_wasmJitProfileBridgeFastCalls.load(std::memory_order_relaxed)),
             (unsigned long long)wasmJitProfileAvgNs(bridgePfnUs, g_wasmJitProfileBridgePfnCalls.load(std::memory_order_relaxed)),
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
             topMovBridgeNext,
             topBridgeFast,
             topBridgePfn,
             topBridgeNext,
             topChainStopPfn,
             topChainStopOther,
             topFetchNextTarget,
             topGenericExit);
}

static inline bool wasmJitProfileStartEntry() {
    U64 count = g_wasmJitProfileStartEntries.fetch_add(1, std::memory_order_relaxed) + 1;
    wasmJitProfileMaybeLog();
    return (count & (WASM_JIT_PROFILE_TIMING_SAMPLE - 1)) == 0;
}

static inline bool wasmJitProfileCallBlock() {
    U64 count = g_wasmJitProfileCallBlockEntries.fetch_add(1, std::memory_order_relaxed) + 1;
    return (count & (WASM_JIT_PROFILE_TIMING_SAMPLE - 1)) == 0;
}

static inline void wasmJitProfileBlockExit() {
    g_wasmJitProfileBlockExits.fetch_add(1, std::memory_order_relaxed);
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

static inline void wasmJitProfileExitNext1() {
    g_wasmJitProfileExitNext1.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitNext2() {
    g_wasmJitProfileExitNext2.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitJump() {
    g_wasmJitProfileExitJump.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileExitGeneric(U32 inst) {
    g_wasmJitProfileExitGeneric.fetch_add(1, std::memory_order_relaxed);
    if (inst < InstructionCount) {
        g_wasmJitProfileGenericExitByInst[inst].fetch_add(1, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileHelperMemRead() {
    g_wasmJitProfileHelperMemRead.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperMemWrite() {
    g_wasmJitProfileHelperMemWrite.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperMemWriteCheck() {
    g_wasmJitProfileHelperMemWriteCheck.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperBlockEnter() {
    g_wasmJitProfileHelperBlockEnter.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperEmulate() {
    g_wasmJitProfileHelperEmulate.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperFlags() {
    g_wasmJitProfileHelperFlags.fetch_add(1, std::memory_order_relaxed);
}

static inline void wasmJitProfileHelperCond(U32 cond, U32 lazyFlagType) {
    g_wasmJitProfileHelperCond.fetch_add(1, std::memory_order_relaxed);
    if (cond < 16) {
        g_wasmJitProfileCondByType[cond].fetch_add(1, std::memory_order_relaxed);
    }
    if (lazyFlagType < 52) {
        g_wasmJitProfileCondByLazyFlag[lazyFlagType].fetch_add(1, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileInlineCond() {
    g_wasmJitProfileInlineCond.fetch_add(1, std::memory_order_relaxed);
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

static inline void wasmJitProfileFetchNextTarget(CPU* cpu, DecodedOp* nextOp) {
    if (!nextOp) {
        g_wasmJitProfileFetchNextTargetNull.fetch_add(1, std::memory_order_relaxed);
    } else if (wasmJitCanChainTo(cpu, nextOp)) {
        g_wasmJitProfileFetchNextTargetJit.fetch_add(1, std::memory_order_relaxed);
    } else if (!(nextOp->flags & OP_FLAG_JIT)) {
        g_wasmJitProfileFetchNextTargetNoJitFlag.fetch_add(1, std::memory_order_relaxed);
    } else if (!nextOp->pfnJitCode) {
        g_wasmJitProfileFetchNextTargetNoTable.fetch_add(1, std::memory_order_relaxed);
    } else if (nextOp->blockStart != nextOp) {
        g_wasmJitProfileFetchNextTargetInterior.fetch_add(1, std::memory_order_relaxed);
    } else if (nextOp->pfn != cpu->thread->process->startJITOp) {
        g_wasmJitProfileFetchNextTargetPfn.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileFetchNextTargetOther.fetch_add(1, std::memory_order_relaxed);
    }
    if (nextOp && nextOp->inst < InstructionCount) {
        g_wasmJitProfileFetchNextTargetByInst[nextOp->inst].fetch_add(1, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileBridge(U32 entries, bool toJit, bool hitLimit, U64 scale) {
    if (entries) {
        g_wasmJitProfileBridgeEntries.fetch_add((U64)entries * scale, std::memory_order_relaxed);
        if (hitLimit) {
            g_wasmJitProfileBridgeLenCap.fetch_add(scale, std::memory_order_relaxed);
        } else if (entries == 1) {
            g_wasmJitProfileBridgeLen1.fetch_add(scale, std::memory_order_relaxed);
        } else if (entries == 2) {
            g_wasmJitProfileBridgeLen2.fetch_add(scale, std::memory_order_relaxed);
        } else if (entries <= 4) {
            g_wasmJitProfileBridgeLen3To4.fetch_add(scale, std::memory_order_relaxed);
        } else if (entries <= 8) {
            g_wasmJitProfileBridgeLen5To8.fetch_add(scale, std::memory_order_relaxed);
        } else if (entries <= 16) {
            g_wasmJitProfileBridgeLen9To16.fetch_add(scale, std::memory_order_relaxed);
        } else {
            g_wasmJitProfileBridgeLen17To31.fetch_add(scale, std::memory_order_relaxed);
        }
    }
    if (toJit) {
        g_wasmJitProfileBridgeToJit.fetch_add(scale, std::memory_order_relaxed);
    }
    if (hitLimit) {
        g_wasmJitProfileBridgeLimit.fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileBridgeFast(U64 startNs, DecodedOp* op, U64 scale) {
    g_wasmJitProfileBridgeFastCalls.fetch_add(scale, std::memory_order_relaxed);
    g_wasmJitProfileBridgeFastUs.fetch_add((wasmJitProfileNowNs() - startNs) * scale, std::memory_order_relaxed);
    if (op && op->inst < InstructionCount) {
        g_wasmJitProfileBridgeFastByInst[op->inst].fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileBridgePfn(U64 startNs, DecodedOp* op, U64 scale) {
    g_wasmJitProfileBridgePfnCalls.fetch_add(scale, std::memory_order_relaxed);
    g_wasmJitProfileBridgePfnUs.fetch_add((wasmJitProfileNowNs() - startNs) * scale, std::memory_order_relaxed);
    if (op && op->inst < InstructionCount) {
        g_wasmJitProfileBridgePfnByInst[op->inst].fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileBridgeStep(CPU* cpu, DecodedOp* bridgeOp, DecodedOp* nextOp, bool hitLimit, U64 scale) {
    if (nextOp && nextOp->inst < InstructionCount) {
        g_wasmJitProfileBridgeNextByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
    }
    if (!bridgeOp || bridgeOp->inst != MovR32E32) {
        return;
    }
    g_wasmJitProfileBridgeMovR32E32.fetch_add(scale, std::memory_order_relaxed);
    if (bridgeOp->ea16) {
        g_wasmJitProfileBridgeMovR32E32Ea16.fetch_add(scale, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileBridgeMovR32E32Ea32.fetch_add(scale, std::memory_order_relaxed);
    }
    if (bridgeOp->reg < 8) {
        g_wasmJitProfileBridgeMovR32E32DstReg[bridgeOp->reg].fetch_add(scale, std::memory_order_relaxed);
    }
    if (bridgeOp->rm < 8) {
        g_wasmJitProfileBridgeMovR32E32Rm[bridgeOp->rm].fetch_add(scale, std::memory_order_relaxed);
    }
    if (hitLimit) {
        g_wasmJitProfileBridgeMovR32E32Limit.fetch_add(scale, std::memory_order_relaxed);
    }
    if (!nextOp) {
        g_wasmJitProfileBridgeMovR32E32Null.fetch_add(scale, std::memory_order_relaxed);
    } else if (wasmJitCanChainTo(cpu, nextOp)) {
        g_wasmJitProfileBridgeMovR32E32ToJit.fetch_add(scale, std::memory_order_relaxed);
    } else if (!(nextOp->flags & OP_FLAG_JIT)) {
        g_wasmJitProfileBridgeMovR32E32NoJitFlag.fetch_add(scale, std::memory_order_relaxed);
    } else if (!nextOp->pfnJitCode) {
        g_wasmJitProfileBridgeMovR32E32NoTable.fetch_add(scale, std::memory_order_relaxed);
    } else if (nextOp->blockStart != nextOp) {
        g_wasmJitProfileBridgeMovR32E32Interior.fetch_add(scale, std::memory_order_relaxed);
        if (nextOp->inst < InstructionCount) {
            g_wasmJitProfileBridgeMovR32E32NextByInst[nextOp->inst].fetch_add(scale, std::memory_order_relaxed);
        }
    } else if (nextOp->pfn != cpu->thread->process->startJITOp) {
        g_wasmJitProfileBridgeMovR32E32Pfn.fetch_add(scale, std::memory_order_relaxed);
    } else {
        g_wasmJitProfileBridgeMovR32E32Other.fetch_add(scale, std::memory_order_relaxed);
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
    U64 count = g_wasmJitProfileLoopSampleEntries.fetch_add(1, std::memory_order_relaxed) + 1;
    return (count & (WASM_JIT_PROFILE_TIMING_SAMPLE - 1)) == 0;
}

static inline void wasmJitProfileMemArrayCheck(bool refreshed, U64 scale) {
    g_wasmJitProfileMemArrayChecks.fetch_add(scale, std::memory_order_relaxed);
    if (refreshed) {
        g_wasmJitProfileMemArrayRefreshes.fetch_add(scale, std::memory_order_relaxed);
    }
}

static inline void wasmJitProfileMovsd(U32 shape, bool df) {
    g_wasmJitProfileMovsdTotal.fetch_add(1, std::memory_order_relaxed);
    bool rep = (shape & WASM_JIT_PROFILE_MOVSD_REP) != 0;
    bool ea16 = (shape & WASM_JIT_PROFILE_MOVSD_EA16) != 0;
    bool segmented = (shape & WASM_JIT_PROFILE_MOVSD_SEGMENTED) != 0;
    (rep ? g_wasmJitProfileMovsdRep : g_wasmJitProfileMovsdSingle).fetch_add(1, std::memory_order_relaxed);
    (ea16 ? g_wasmJitProfileMovsdEa16 : g_wasmJitProfileMovsdEa32).fetch_add(1, std::memory_order_relaxed);
    (segmented ? g_wasmJitProfileMovsdSegmented : g_wasmJitProfileMovsdFlat).fetch_add(1, std::memory_order_relaxed);
    (df ? g_wasmJitProfileMovsdDf1 : g_wasmJitProfileMovsdDf0).fetch_add(1, std::memory_order_relaxed);
    if (rep) {
        (segmented ? g_wasmJitProfileMovsdRepSegmented : g_wasmJitProfileMovsdRepFlat).fetch_add(1, std::memory_order_relaxed);
        if (ea16) {
            g_wasmJitProfileMovsdRepEa16.fetch_add(1, std::memory_order_relaxed);
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
        var fn = inst.exports['execute'];
        if (!fn) return -1;
        var idx = addFunction(fn, 'vi');

#ifndef __TEST
        if (!boxedwine_wasm_instantiate._count) boxedwine_wasm_instantiate._count = 0;
        boxedwine_wasm_instantiate._count++;
        if (boxedwine_wasm_instantiate._count % 1000 === 0) {
            console.log('[WASM JIT] compiled=' + boxedwine_wasm_instantiate._count +
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
typedef void (*WasmBlockFn)(int);
static inline void boxedwine_wasm_call_block(int tableIndex, int cpuPtr) {
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
    fn(cpuPtr);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    if (profileSample) {
        wasmJitProfileAddElapsedScaled(g_wasmJitProfileCallBlockUs, profileStartNs);
    }
#endif
}

// Single-threaded: return the slot to Emscripten's free-list.
EM_JS(void, boxedwine_wasm_free_block, (int tableIndex),
{
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
        U8 wasmJitSetupFlags = op->flags2 & OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        if (wasmJitSetupFlags) {
            KMemoryData* memoryData = nullptr;
            if (wasmMemoryPageArraysNeedRefresh(cpu, &memoryData)) {
                wasmPrepareBlockEnter(cpu, memoryData);
            }
        }
        WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPreCallUs, startPreCallNs); })
        boxedwine_wasm_call_block((int)(uintptr_t)op->pfnJitCode, (int)(uintptr_t)cpu);
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
        boxedwine_wasm_call_block((int)(uintptr_t)op->pfnJitCode, (int)(uintptr_t)cpu);
        WASM_JIT_PROFILE_ONLY(if (profileSample) { U64 startPostCallNs = wasmJitProfileNowNs(); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartPostCallUs, startPostCallNs); wasmJitProfileAddElapsedScaled(g_wasmJitProfileStartUs, profileStartNs); })
        WASM_JIT_PROFILE_ONLY(if (profileSample) { wasmJitProfileChainTarget(cpu, cpu->nextOp, WASM_JIT_PROFILE_TIMING_SAMPLE); })
        // nextOp is updated by the WASM block itself (via helper call).
        if (chainedBlocks) {
            chainedInstructionCount += executedOp->blockOpCount;
        }
        chainedBlocks++;
        op = cpu->nextOp;
        if (!wasmJitCanChainTo(cpu, op)) {
            U32 bridgeEntries = 0;
            while (bridgeEntries < WASM_JIT_INTERIOR_BRIDGE_LIMIT && !cpu->yield && wasmJitCanBridgeInterior(cpu, op)) {
                DecodedOp* bridgeOp = op;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
                U64 bridgeOpStartNs = loopProfileSample ? wasmJitProfileNowNs() : 0;
#endif
                if (wasmJitTryBridgeFastOp(cpu, bridgeOp)) {
                    WASM_JIT_PROFILE_ONLY(if (loopProfileSample) { wasmJitProfileBridgeFast(bridgeOpStartNs, bridgeOp, WASM_JIT_PROFILE_TIMING_SAMPLE); })
                } else {
                    NormalCPU::runWasmJitBridge(cpu, bridgeOp);
                    WASM_JIT_PROFILE_ONLY(if (loopProfileSample) { wasmJitProfileBridgePfn(bridgeOpStartNs, bridgeOp, WASM_JIT_PROFILE_TIMING_SAMPLE); })
                }
                bridgeEntries++;
                op = cpu->nextOp;
                WASM_JIT_PROFILE_ONLY(if (loopProfileSample) { wasmJitProfileBridgeStep(cpu, bridgeOp, op, bridgeEntries == WASM_JIT_INTERIOR_BRIDGE_LIMIT && wasmJitCanBridgeInterior(cpu, op), WASM_JIT_PROFILE_TIMING_SAMPLE); })
                if (wasmJitCanChainTo(cpu, op)) {
                    break;
                }
            }
            WASM_JIT_PROFILE_ONLY(if (loopProfileSample) { wasmJitProfileBridge(bridgeEntries, wasmJitCanChainTo(cpu, op), bridgeEntries == WASM_JIT_INTERIOR_BRIDGE_LIMIT && wasmJitCanBridgeInterior(cpu, op), WASM_JIT_PROFILE_TIMING_SAMPLE); })
            if (!wasmJitCanChainTo(cpu, op)) {
                break;
            }
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
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs);
#endif
    cpu->memHelperValue = cpu->memory->readd(cpu->memHelperAddr);
}
static void wasmHelper_writeMem32(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs);
#endif
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
}
static void wasmHelper_readMem8(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs);
#endif
    cpu->memHelperValue = cpu->memory->readb(cpu->memHelperAddr);
}
static void wasmHelper_writeMem8(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs);
#endif
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
}
static void wasmHelper_readMem16(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemRead();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemReadUs);
#endif
    cpu->memHelperValue = cpu->memory->readw(cpu->memHelperAddr);
}
static void wasmHelper_writeMem16(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWrite();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteUs);
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
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs);
#endif
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem16_check(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs);
#endif
    cpu->memory->writew(cpu->memHelperAddr, (U16)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem8_check(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperMemWriteCheck();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperMemWriteCheckUs);
#endif
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}

// Update cpu->nextOp by decoding the block at cpu->eip.u32
static void wasmHelper_fetchNextOp(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileBlockExit();
    g_wasmJitProfileFetchNextCalls.fetch_add(1, std::memory_order_relaxed);
    WasmJitProfileTimer profileTimer(g_wasmJitProfileFetchNextUs);
#endif
    if (!cpu->thread->terminating) {
        cpu->nextOp = cpu->getNextOp();
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        wasmJitProfileFetchNextTarget(cpu, cpu->nextOp);
#endif
    }
}

// Sync CPU lazy flags — flags are maintained in cpu->flags directly by WASM code
static void wasmHelper_syncFlags(CPU* cpu) {}

// Kept in the helper table for module import-index stability. Block-entry
// setup is now done in wasmStartJITOp before calling the generated block,
// which avoids one generated WASM -> imported C++ helper call per block.
static void wasmHelper_blockEnter(CPU* cpu) {
    (void)cpu;
}

// Fall back to the normal CPU interpreter for one instruction (used for
// operations the WASM JIT doesn't inline, e.g. FPU/SSE/complex shifts).
void jitRunSingleOp(CPU* cpu);   // defined in jitCodeGen.cpp
static void wasmHelper_emulateSingleOp(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperEmulate();
    if (cpu->tmpReg < InstructionCount) {
        g_wasmJitProfileEmulateByInst[cpu->tmpReg].fetch_add(1, std::memory_order_relaxed);
    }
    if (cpu->tmpReg == Movsd) {
        wasmJitProfileMovsd(cpu->memHelperValue, (cpu->flags & DF) != 0);
    }
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperEmulateUs);
#endif
    jitRunSingleOp(cpu);
}

// Compute CF using the C++ lazy-flag machinery and stash the result in
// cpu->tmpReg. Used by the WASM backend's getCF() override since it can't
// do a nakedCall to the per-flag-type computation functions.
static void wasmHelper_computeCF(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperFlags();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs);
#endif
    cpu->tmpReg = cpu->getCF() ? 1 : 0;
}
static void wasmHelper_computeZF(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperFlags();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs);
#endif
    cpu->tmpReg = cpu->getZF() ? 1 : 0;
}

// Materialize all lazy flags into cpu->flags; reset lazyFlagType to FLAGS_NONE.
void common_fillFlags(CPU* cpu);
static void wasmHelper_fillFlags(CPU* cpu) {
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileHelperFlags();
    WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperFlagsUs);
#endif
    common_fillFlags(cpu);
}

// One helper per JitConditional: evaluates the condition against cpu state
// (honoring lazy flags via cpu->getXF()) and stashes 0/1 in cpu->tmpReg.
// Separate helpers avoid any need for an input parameter — using
// cpu->src.u32 would clobber live lazy-flag state.
#define WASM_COND_HELPER(NAME, EXPR)                                           \
    static void wasmHelper_cond_##NAME(CPU* cpu) {                             \
        WASM_JIT_PROFILE_ONLY(wasmJitProfileHelperCond((U32)JitConditional::NAME, (U32)cpu->lazyFlagType);) \
        WASM_JIT_PROFILE_ONLY(WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperCondUs);) \
        cpu->tmpReg = (EXPR) ? 1 : 0;                                          \
    }
WASM_COND_HELPER(O,   cpu->getOF())
WASM_COND_HELPER(NO,  !cpu->getOF())
WASM_COND_HELPER(B,   cpu->getCF())
WASM_COND_HELPER(NB,  !cpu->getCF())
#ifdef BOXEDWINE_WASM_DIAGNOSTICS
static U32 g_condZLogCount = 0;
static void wasmHelper_cond_Z(CPU* cpu) {
    WASM_JIT_PROFILE_ONLY(wasmJitProfileHelperCond((U32)JitConditional::Z, (U32)cpu->lazyFlagType);)
    WASM_JIT_PROFILE_ONLY(WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperCondUs);)
    if (g_condZLogCount < 100) {
        g_condZLogCount++;
        klog_fmt("[WASM-DIAG] cond_Z #%u eip=0x%08x lazyType=%u result=0x%08x dst=0x%08x flags=0x%08x ZF=%d",
                 g_condZLogCount, cpu->eip.u32 + cpu->seg[CS].address,
                 (U32)cpu->lazyFlagType, cpu->result.u32, cpu->dst.u32, cpu->flags,
                 cpu->getZF() ? 1 : 0);
    }
    cpu->tmpReg = cpu->getZF() ? 1 : 0;
}
static void wasmHelper_cond_NZ(CPU* cpu) {
    WASM_JIT_PROFILE_ONLY(wasmJitProfileHelperCond((U32)JitConditional::NZ, (U32)cpu->lazyFlagType);)
    WASM_JIT_PROFILE_ONLY(WasmJitProfileTimer profileTimer(g_wasmJitProfileHelperCondUs);)
    cpu->tmpReg = (!cpu->getZF()) ? 1 : 0;
}
#else
WASM_COND_HELPER(Z,   cpu->getZF())
WASM_COND_HELPER(NZ,  !cpu->getZF())
#endif
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
    wasmJitProfileBlockExit();
}
static void wasmHelper_profileExitNext1(CPU* cpu) {
    (void)cpu;
    wasmJitProfileExitNext1();
}
static void wasmHelper_profileExitNext2(CPU* cpu) {
    (void)cpu;
    wasmJitProfileExitNext2();
}
static void wasmHelper_profileExitJump(CPU* cpu) {
    (void)cpu;
    wasmJitProfileExitJump();
}
static void wasmHelper_profileExitGeneric(CPU* cpu) {
    wasmJitProfileExitGeneric(cpu->tmpReg);
}
static void wasmHelper_profileInlineCond(CPU* cpu) {
    (void)cpu;
    wasmJitProfileInlineCond();
}
#endif

static void wasmHelper_fldSingleReal(CPU* cpu) {
    common_FLD_SINGLE_REAL(cpu, cpu->memHelperAddr);
}

static void wasmHelper_fld1(CPU* cpu) {
    common_FLD1(cpu);
}

static void wasmHelper_fldDoubleReal(CPU* cpu) {
    common_FLD_DOUBLE_REAL(cpu, cpu->memHelperAddr);
}

static void wasmHelper_fcomSingleRealPop(CPU* cpu) {
    common_FCOM_SINGLE_REAL_Pop(cpu, cpu->memHelperAddr);
}

static void wasmHelper_faddSt0Stj(CPU* cpu) {
    common_FADD_ST0_STj(cpu, cpu->memHelperValue);
}

static void wasmHelper_fdivSt0Stj(CPU* cpu) {
    common_FDIV_ST0_STj(cpu, cpu->memHelperValue);
}

static void wasmHelper_fnstswAx(CPU* cpu) {
    common_FNSTSW_AX(cpu);
}

static void wasmHelper_fstSingleRealPop(CPU* cpu) {
    common_FST_SINGLE_REAL_Pop(cpu, cpu->memHelperAddr);
}

static void wasmHelper_fistDwordIntegerPop(CPU* cpu) {
    common_FIST_DWORD_INTEGER_Pop(cpu, cpu->memHelperAddr);
}

static void wasmHelper_movsdXmmE64(CPU* cpu) {
    common_movsdXmmE64(cpu, cpu->memHelperValue, cpu->memHelperAddr);
}

static void wasmHelper_movsdE64Xmm(CPU* cpu) {
    common_movsdE64Xmm(cpu, cpu->memHelperValue, cpu->memHelperAddr);
}

static void wasmHelper_movsd32r(CPU* cpu) {
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
    (void*)wasmHelper_fldSingleReal,
    (void*)wasmHelper_fld1,
    (void*)wasmHelper_fldDoubleReal,
    (void*)wasmHelper_fcomSingleRealPop,
    (void*)wasmHelper_faddSt0Stj,
    (void*)wasmHelper_fdivSt0Stj,
    (void*)wasmHelper_fnstswAx,
    (void*)wasmHelper_fstSingleRealPop,
    (void*)wasmHelper_fistDwordIntegerPop,
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
#endif
};
static constexpr int WASM_HELPER_COUNT = (int)(sizeof(g_wasmHelperTable) / sizeof(g_wasmHelperTable[0]));

#ifdef BOXEDWINE_MULTI_THREADED
static bool lazyInstallWasmJitBlockForWorker(int tableIndex) {
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
    HELPER_FLD_SINGLE_REAL = 32,
    HELPER_FLD1 = 33,
    HELPER_FLD_DOUBLE_REAL = 34,
    HELPER_FCOM_SINGLE_REAL_POP = 35,
    HELPER_FADD_ST0_STJ = 36,
    HELPER_FDIV_ST0_STJ = 37,
    HELPER_FNSTSW_AX = 38,
    HELPER_FST_SINGLE_REAL_POP = 39,
    HELPER_FIST_DWORD_INTEGER_POP = 40,
    HELPER_MOVSD_XMM_E64 = 41,
    HELPER_MOVSD_E64_XMM = 42,
    HELPER_MOVSD32R = 43,
    HELPER_PROFILE_BLOCK_EXIT = 44,
    HELPER_PROFILE_EXIT_NEXT1 = 45,
    HELPER_PROFILE_EXIT_NEXT2 = 46,
    HELPER_PROFILE_EXIT_JUMP = 47,
    HELPER_PROFILE_EXIT_GENERIC = 48,
    HELPER_PROFILE_INLINE_COND = 49,
};

// ---------------------------------------------------------------------------
// JitWasmCodeGen constructor
// ---------------------------------------------------------------------------
JitWasmCodeGen::JitWasmCodeGen(CPU* cpu) : JitCodeGen(cpu) {
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

    // Declare the single local function and export it.
    U32 execFuncIdx = m_emitter.addFunction(m_typeVoidI32);
    m_emitter.addExport("execute", execFuncIdx);

    // Begin the function body. Locals layout:
    //   local 0 (param): cpu_ptr i32
    //   locals 1-8 (i32 x8): GP registers eax-edi
    //   locals 9-12 (i32 x4): segment addresses cs, ds, ss, es
    //   locals 13-44 (i32 x32): scratch temporaries
    //   local 45 (i64 x1): i64 scratch for 64-bit multiply
    m_emitter.beginFunction({
        { 8,  WasmType::I32 },  // GP registers (locals 1-8)
        { 4,  WasmType::I32 },  // segment addresses (locals 9-12)
        { 32, WasmType::I32 },  // scratch temporaries (locals 13-44)
        { 1,  WasmType::I64 },  // i64 scratch for 64-bit multiply (local 45)
    });

    m_gpLoaded.fill(false);
    m_gpDirty.fill(false);
    m_segLoaded.fill(false);
    m_scratchInUse.fill(false);
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

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg);
}

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg, bool isHigh) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg, isHigh);
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
    m_emitter.emitI32Load(0);
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
// shift-of-memory ops) still routes through the helpers — the
// scratch-pool budget for callbacks like btMask + nested IfTest is
// too tight to add an inline if/else without spilling into
// allocScratch's silent slot-0 fallback.
// ---------------------------------------------------------------------------
// Helper-import index for emulated-memory read/write of the given width.
// Picking the wrong width corrupts neighboring bytes: writeMem32 on a
// 16-bit push would clobber the two bytes above the stack slot.
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

void JitWasmCodeGen::emitArmSmcBailout() {
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
    auto result = getTmpReg();
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(readHelperForWidth(w));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalSet(result->hardwareReg());
    if (prepareWrite) {
        prepareWrite(result);
        // The callback may have mutated addressReg's local or rewritten
        // memHelperAddr indirectly — re-stage it before dispatching write.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), result);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        // Use the bailout-checking write so self-modifying code that hits
        // the active block's bytes can be detected; emit the check.
        emitArmSmcBailout();
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
    }
    if (addressReg && addressReg->emulatedReg == 0xff)
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
        // Legacy path: customOp expects a MemPtr it can manipulate; the
        // fast path can't supply one, so always slow-path here.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(readHelperForWidth(w));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
        m_emitter.emitLocalSet(tmp->hardwareReg());
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
        U32 widthBytes = (w == JitWidth::b16) ? 2 : 4;
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.emitIf();
    {
        // Slow path: existing helper-based load.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
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
    if (addressReg && addressReg->emulatedReg == 0xff)
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
        // Legacy path: customOp expects a MemPtr; slow-path only.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), src);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        emitArmSmcBailout();
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
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
        U32 widthBytes = (w == JitWidth::b16) ? 2 : 4;
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.emitIf();
    {
        // Slow path: bailout-checking helper. Writes to CodePages, RO
        // pages, on-demand pages, and cross-page accesses all land here.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), src);
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
    // NEG = 0 - src. Store lazy flag operands before overwriting the register.
    // dst=0 (the implicit minuend), src=original operand, result=0-src.
    // Flag semantics are identical to SUB(0, src).
    storeLazyFlagsSrc(reg);
    writeCPUValue(JitWidth::b32, (U32)offsetof(CPU, dst.u32), 0);
    auto result = getTmpReg();
    m_emitter.emitI32Const(0);
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I32_SUB);
    maskToWidth(w);
    m_emitter.emitLocalTee(result->hardwareReg());
    popToReg(w, reg);
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForSub(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForSub(w);
}

void JitWasmCodeGen::dynamic_salc(DecodedOp* op) {
    (void)op;
    RegPtr cf = getCF();
    m_emitter.emitI32Const(0);
    pushRegValue(cf);
    m_emitter.emitOp(WASM_I32_SUB);
    popToReg(JitWidth::b8, getReg8(0));
    freeScratch(cf->hardwareReg());
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
    IfNot(JitWidth::b32, divisor); {
        emulateSingleOp();
        blockExit();
    } StartElse(); {
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
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COMPUTE_CF);
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(r->hardwareReg());
    m_gpLoaded.fill(false);
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
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_INLINE_COND);
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
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_INLINE_COND);
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
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_INLINE_COND);
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
    IfCondition(cond);
    DecodedOp* targetOp = cpu->memory->getDecodedOp(address);
    if (targetOp) {
        syncDirtyRegsToHost();
        writeEip(address - cpu->seg[CS].address);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_BLOCK_EXIT);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_EXIT_JUMP);
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
    // No native intra-block branching: set EIP (offset within CS, so strip
    // the CS base) and exit. fetchNextOp picks back up at the target op
    // in the next dispatcher round.
    writeEip(address - cpu->seg[CS].address);
    emitBlockExitWithProfile(HELPER_PROFILE_EXIT_JUMP);
}
void JitWasmCodeGen::emitBlockExitWithProfile(U32 profileHelperIdx) {
    syncDirtyRegsToHost();
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(profileHelperIdx);
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
    if (op->data.nextJump) {
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
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_BLOCK_EXIT);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_EXIT_NEXT1);
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
    if (op->next) {
        syncDirtyRegsToHost();
        writeEip(eip - cpu->seg[CS].address);
#ifdef BOXEDWINE_WASM_JIT_PROFILE
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_BLOCK_EXIT);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(HELPER_PROFILE_EXIT_NEXT2);
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

void JitWasmCodeGen::dynamic_FLD_SINGLE_REAL(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FLD_SINGLE_REAL);
}

void JitWasmCodeGen::dynamic_FLD1(DecodedOp* op) {
    (void)op;
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FLD1);
}

void JitWasmCodeGen::dynamic_FLD_DOUBLE_REAL(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FLD_DOUBLE_REAL);
}

void JitWasmCodeGen::dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FCOM_SINGLE_REAL_POP);
}

void JitWasmCodeGen::dynamic_FADD_ST0_STj(DecodedOp* op) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const(op->reg);
    m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FADD_ST0_STJ);
}

void JitWasmCodeGen::dynamic_FDIV_ST0_STj(DecodedOp* op) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const(op->reg);
    m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FDIV_ST0_STJ);
}

void JitWasmCodeGen::dynamic_FNSTSW_AX(DecodedOp* op) {
    (void)op;
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FNSTSW_AX);
}

void JitWasmCodeGen::dynamic_FST_SINGLE_REAL_Pop(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FST_SINGLE_REAL_POP);
}

void JitWasmCodeGen::dynamic_FIST_DWORD_INTEGER_Pop(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FIST_DWORD_INTEGER_POP);
}

void JitWasmCodeGen::dynamic_movsdXmmE64(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const(op->reg);
    m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_MOVSD_XMM_E64);
}

void JitWasmCodeGen::dynamic_movsdE64Xmm(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), address);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const(op->reg);
    m_emitter.emitI32Store((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_MOVSD_E64_XMM);
}

void JitWasmCodeGen::dynamic_movsd_op(DecodedOp* op) {
    (void)op;
    // The direct rep movsd helper removed the fallback cost but caused a
    // long-run WASM stack overflow. Keep profiling this path as an emulated op
    // until the string-copy helper can be made scheduler-safe.
    emulateSingleOp();
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

U8* JitWasmCodeGen::createStartJITCode() {
    // Return the address of the static wasmStartJITOp function.
    // NormalCPU::run() uses this as process->startJITOp.
    return (U8*)wasmStartJITOp;
}

// ---------------------------------------------------------------------------
// Compilation lifecycle
// ---------------------------------------------------------------------------
void JitWasmCodeGen::preCompile(DecodedOp* op, bool skippedOp) {
    // Reset per-instruction scratch state, then run base bookkeeping (block
    // op count, eip→buffer-pos map, preOp hook). Stash op->len for
    // emitBailoutCheck — it needs the post-write next-op EIP.
    if (this->blockOpCount == 0) {
        m_wasmBlockStartOp = op;
    }
    m_currentWasmOp = op;
    m_scratchInUse.fill(false);
    lastCompiledOpLen = op->len;
    JitCodeGen::preCompile(op, skippedOp);
}

void JitWasmCodeGen::compile(DecodedOp* op) {
    if (op->inst == Pause) {
        dynamic_pause(op);
        return;
    }
    if (op->inst == FLD_SINGLE_REAL) {
        dynamic_FLD_SINGLE_REAL(op);
        return;
    }
    if (op->inst == FLD1) {
        dynamic_FLD1(op);
        return;
    }
    if (op->inst == FLD_DOUBLE_REAL) {
        dynamic_FLD_DOUBLE_REAL(op);
        return;
    }
    if (op->inst == FCOM_SINGLE_REAL_Pop) {
        dynamic_FCOM_SINGLE_REAL_Pop(op);
        return;
    }
    if (op->inst == FADD_ST0_STj) {
        dynamic_FADD_ST0_STj(op);
        return;
    }
    if (op->inst == FDIV_ST0_STj) {
        dynamic_FDIV_ST0_STj(op);
        return;
    }
    if (op->inst == FNSTSW_AX) {
        dynamic_FNSTSW_AX(op);
        return;
    }
    if (op->inst == FST_SINGLE_REAL_Pop) {
        dynamic_FST_SINGLE_REAL_Pop(op);
        return;
    }
    if (op->inst == FIST_DWORD_INTEGER_Pop) {
        dynamic_FIST_DWORD_INTEGER_Pop(op);
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

void JitWasmCodeGen::commitJIT(DecodedOp* op) {
    // Finalize the WASM function body and the module binary.
    m_emitter.endFunction();
    m_wasmBinary = m_emitter.finalize();

    if (m_wasmBinary.empty()) return;

    // Pass the helper function table to the JS instantiator.
    // In pthreads builds use the Atomics-based allocator; in single-threaded
    // builds use the standard addFunction() wrapper.
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    U64 instantiateStartUs = wasmJitProfileNowNs();
#endif
#ifdef BOXEDWINE_MULTI_THREADED
    int tableIdx = boxedwine_wasm_instantiate_mt(
        m_wasmBinary.data(),
        (int)m_wasmBinary.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT,
        &g_wasmTableNextSlot
    );
#else
    int tableIdx = boxedwine_wasm_instantiate(
        m_wasmBinary.data(),
        (int)m_wasmBinary.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT
    );
#endif
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileAddElapsed(g_wasmJitProfileInstantiateUs, instantiateStartUs);
#endif

    if (tableIdx < 0) {
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

#ifdef BOXEDWINE_MULTI_THREADED
    {
        std::lock_guard<std::mutex> lock(g_wasmBlockBinariesMutex);
        g_wasmBlockBinaries[tableIdx] = m_wasmBinary;
    }
#endif

    // Walk all ops in this block: the first op gets the startJITOp entry
    // (its pfnJitCode is the wasmTable index the dispatcher invokes). All
    // ops in the block share the same tableIdx so removeCodeBlock's sweep
    // picks them up for cleanup; they also get OP_FLAG_JIT so doJIT skips
    // recompiling. Only the first op's pfn points at startJITOp — control
    // never re-enters the middle of a WASM block.
    op->blockLen     = this->emulatedLen;
    op->blockOpCount = this->blockOpCount;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    wasmJitProfileCompiledBlock(this->blockOpCount);
#endif
    DecodedOp* cur = op;
    while (cur) {
        cur->pfnJitCode = (void*)(uintptr_t)(U32)tableIdx;
        cur->flags     |= OP_FLAG_JIT;
        if (m_needsWasmMemoryPageArrays) {
            cur->flags2 |= OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        } else {
            cur->flags2 &= ~OP_FLAG2_WASM_JIT_MEM_ARRAYS;
        }
        cur->blockStart = op;
        if (cur == op) {
            cur->pfn = cpu->thread->process->startJITOp;
        } else if (cur->pfn == cpu->thread->process->startJITOp) {
            // This interior op was previously the first op of another compiled
            // block. Its pfnJitCode has now been overwritten to this block's
            // tableIdx, so calling startJITOp here would run the wrong WASM
            // block from its beginning with incorrect CPU state.  Reset to the
            // interpreter function so direct entry at this op falls back to
            // the normal interpreter path.
            cur->pfn = NormalCPU::getFunctionForOp(cur);
        }
        if (cur->next == nullptr) break;
        cur = cur->next;
    }
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
    // All ops in a WASM-compiled block share the same wasmTable index
    // (we don't sub-compile individual ops), so dedupe before freeing.
    std::set<int> seen;
    for (void* p : jitOps) {
        if (!p) continue;
        int tableIdx = (int)(uintptr_t)p;
        if (seen.insert(tableIdx).second) {
#ifdef BOXEDWINE_MULTI_THREADED
            boxedwine_wasm_free_block_mt(tableIdx);
#else
            boxedwine_wasm_free_block(tableIdx);
#endif
        }
    }
}

#endif // BOXEDWINE_WASM_JIT
