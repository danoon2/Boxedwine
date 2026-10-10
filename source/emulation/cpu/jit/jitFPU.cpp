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

#ifdef BOXEDWINE_JIT
#include "jitFPU.h"

std::vector<JitFPU::FpuRegisterLoop> JitFPU::findFpuRegisterLoops(U8 maxValues) const {
    std::vector<FpuRegisterLoop> result;
    if (!supportsFpuStackCache()) return result;
    bool touchesFpu = false;
    for (const auto& entry : blockInstructions) touchesFpu |= entry.op->isFpuOp();
    if (!touchesFpu) return result;
    for (const auto& edge : findLoopBackedges()) {
        U8 slots = 0;
        U8 eagerSlots = 0;
        bool readsMemory = false;
        bool accepted = true;
        for (const auto& entry : blockInstructions) {
            DecodedOp* op = entry.op;
            // Other compiled edges must not bypass the header initialization.
            if (entry.eip != edge.sourceEip && op->isDirectBranch()) {
                U32 target = entry.eip + op->len + op->imm;
                if (target >= edge.targetEip && target <= edge.sourceEip) accepted = false;
            }
            if (entry.eip < edge.targetEip || entry.eip > edge.sourceEip) continue;
            if (op->lock || instructionInfo[op->inst].writeMemWidth ||
                (entry.eip != edge.targetEip && (op->flags2 & OP_FLAG2_JUMP_TARGET))) {
                accepted = false;
                continue;
            }
            if (entry.eip == edge.sourceEip) {
                accepted &= op->isJumpCC();
                continue;
            }
            if (op->isBranch()) {
                accepted = false;
                continue;
            }
            switch (op->inst) {
            case FADD_SINGLE_REAL: case FADD_DOUBLE_REAL:
            case FMUL_SINGLE_REAL: case FMUL_DOUBLE_REAL:
            case FSUB_SINGLE_REAL: case FSUB_DOUBLE_REAL:
            case FSUBR_SINGLE_REAL: case FSUBR_DOUBLE_REAL:
            case FIADD_DWORD_INTEGER: case FIADD_WORD_INTEGER:
            case FIMUL_DWORD_INTEGER: case FIMUL_WORD_INTEGER:
            case FISUB_DWORD_INTEGER: case FISUB_WORD_INTEGER:
            case FISUBR_DWORD_INTEGER: case FISUBR_WORD_INTEGER:
                slots |= 1;
                readsMemory = true;
                break;
            case FADD_ST0_STj: case FADD_STi_ST0:
            case FMUL_ST0_STj: case FMUL_STi_ST0:
            case FSUB_ST0_STj: case FSUB_STi_ST0:
            case FSUBR_ST0_STj: case FSUBR_STi_ST0:
                slots |= 1u | (1u << op->reg);
                break;
            case FCHS: case FABS:
                slots |= 1;
                break;
            default:
                LoopRegisterUsage registers;
                accepted &= !instructionInfo[op->inst].readMemWidth && accumulateLoopRegisters(op, registers);
                break;
            }
            if (!readsMemory) eagerSlots = slots;
        }
        U8 count = 0;
        for (U8 i = 0; i < 8; ++i) if (slots & (1u << i)) ++count;
        if (!accepted || !count || count > maxValues) continue;
        bool overlaps = false;
        for (const auto& loop : result)
            overlaps |= edge.targetEip <= loop.backedge && edge.sourceEip >= loop.header;
        if (!overlaps) result.push_back({edge.targetEip, edge.sourceEip, slots, readsMemory && (slots & ~eagerSlots) != 0});
    }
    return result;
}

void JitFPU::prepareFpuRegisterLoop(U8 slots) {
    beginFpuCache();
    // Import only the unconditionally used values. Fixed stack slots avoid
    // the all-eight-slot validity/tag register file needed by rotating loops.
    for (U8 i = 0; i < 8; ++i) if (slots & (1u << i)) cachedFpuValue(i);
}

// Slots are indexed relative to the TOP at entry, not the current logical ST(0).
// Push/pop and FXCH change only this compile-time mapping. Unknown incoming
// values are imported lazily; a region need not start with an empty x87 stack.
void JitFPU::beginFpuCache() {
    if (fpuStackCache.entryTop) return;
    fpuStackCache.entryTop = getFpuCacheTopReg();
    readCPU(JitWidth::b32, offsetof(CPU, fpu.top), fpuStackCache.entryTop);
    preserveFpuCacheTop(fpuStackCache.entryTop);
    for (U8 i = 0; i < 8; ++i) {
        fpuStackCache.slots[i].value = getFpuCacheReg(i);
        fpuStackCache.slots[i].tag = getFpuCacheTagReg(i);
    }
}

JitFPU::CachedFpuSlot& JitFPU::cachedFpuSlot(U8 relativeIndex) {
    return fpuStackCache.slots[(fpuStackCache.top + relativeIndex) & 7];
}

FPURegPtr JitFPU::cachedFpuValue(U8 relativeIndex) {
    auto& slot = cachedFpuSlot(relativeIndex);
    if (!slot.loaded || slot.needsValidation) {
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + relativeIndex) & 7);
        // Keep prior dirty state. A backend may leave an import clean when
        // CPU storage is already current; local conversions require writeback.
        slot.dirty |= loadFpuValue(slot.value, index, slot.loaded ? slot.valid : nullptr);
        slot.loaded = true;
        if (slot.valid) movValue(JitWidth::b32, slot.valid, 1);
        slot.needsValidation = false;
    }
    slot.value->hardwareReg(); // Native backends may have spilled this value.
    return slot.value;
}

RegPtr JitFPU::cachedFpuTag(U8 relativeIndex) {
    auto& slot = cachedFpuSlot(relativeIndex);
    if (!slot.tagLoaded) {
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + relativeIndex) & 7);
        mov(JitWidth::b32, slot.tag, readFPUTag(index));
        slot.tagLoaded = true;
    }
    slot.tag->hardwareReg();
    return slot.tag;
}

void JitFPU::cachedFpuPush(FPURegPtr value, RegPtr tag) {
    fpuStackCache.top = (fpuStackCache.top - 1) & 7;
    auto& slot = cachedFpuSlot(0);
    moveFpuReg(slot.value, value);
    if (tag) mov(JitWidth::b32, slot.tag, tag);
    else movValue(JitWidth::b32, slot.tag, TAG_Valid);
    slot.loaded = slot.dirty = slot.tagLoaded = slot.tagDirty = true;
    if (slot.valid) movValue(JitWidth::b32, slot.valid, 1);
    slot.needsValidation = false;
}

void JitFPU::cachedFpuPop() {
    auto& slot = cachedFpuSlot(0);
    movValue(JitWidth::b32, slot.tag, TAG_Empty);
    slot.tagLoaded = slot.tagDirty = true;
    fpuStackCache.top = (fpuStackCache.top + 1) & 7;
}

// This deliberately leaves the compiler state intact: callers can materialize
// inside a cold conditional path without changing the hot path's cache.
void JitFPU::materializeFpuCache(U8 slotMask) {
    if (!fpuStackCache.entryTop) return;
    for (U8 i = 0; i < 8; ++i) {
        auto& slot = fpuStackCache.slots[i];
        if (!(slotMask & (1u << i)) || (!slot.dirty && !slot.tagDirty)) continue;
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, i);
        if (slot.dirty) {
            if (slot.needsValidation) If(JitWidth::b32, slot.valid);
            syncXmmToCPUWithIndexReg(index, slot.value);
            if (slot.needsValidation) EndIf();
        }
        if (slot.tagDirty) writeFPUTag(index, slot.tag);
    }
    if (slotMask == 0xff)
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top));
}

void JitFPU::flushFpuCache() {
    materializeFpuCache();
    fpuStackCache = FpuStackCache{};
}

// Populate the loop's register file without converting unused extended values.
// All slots may have been changed by an earlier iteration when a cold exit is
// taken near the start of this iteration, so writeback must track them all.
void JitFPU::prepareFpuCacheForLoop(bool dynamicValidity, U8 slots, bool trackTags) {
    beginFpuCache();
    for (U8 i = 0; i < 8; ++i) {
        if (!(slots & (1u << ((i - fpuStackCache.top) & 7)))) continue;
        auto& slot = fpuStackCache.slots[i];
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, i);
        slot.valid = getFpuCacheValidReg(i);
        if (!slot.loaded) {
            loadCpuFpuReg(slot.value, index);
            readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached), slot.valid);
            slot.needsValidation = true;
        } else if (!slot.needsValidation) {
            // A preceding conditional move may have left the old extended
            // value untouched. Preserve that runtime validity at loop entry.
            movValue(JitWidth::b32, slot.valid, 1);
        }
        // TOP rotation or an extended load can make a header position lazy
        // on a later iteration, even if it initially contained a valid double.
        slot.needsValidation |= dynamicValidity;
        if (trackTags) {
            if (!slot.tagLoaded) mov(JitWidth::b32, slot.tag, readFPUTag(index));
            slot.tagLoaded = slot.tagDirty = true;
        }
        slot.loaded = slot.dirty = true;
    }
}

void JitFPU::reconcileFpuCacheForLoop(const FpuStackCache& entry) {
    if (!fpuStackCache.entryTop) kpanic("missing cached x87 loop state");
    U8 rotation = (fpuStackCache.top - entry.top) & 7;
    // FXCH renames locals, and a changing TOP rotates the relative slots.
    // Parallel-copy values/tags into the header's mapping on the taken edge.
    // The fall-through arm still uses the end-of-iteration mapping.
    bool visited[8] = {};
    for (U8 start = 0; start < 8; ++start) {
        if (visited[start]) continue;
        if (entry.slots[start].value == fpuStackCache.slots[(start + rotation) & 7].value) {
            visited[start] = true;
            continue;
        }
        FPURegPtr value = getFPUTmp();
        RegPtr tag = getTmpReg();
        moveFpuReg(value, entry.slots[start].value);
        mov(JitWidth::b32, tag, entry.slots[start].tag);
        U8 dst = start;
        while (true) {
            visited[dst] = true;
            U8 src = 0;
            while (src < 8 && entry.slots[src].value != fpuStackCache.slots[(dst + rotation) & 7].value) ++src;
            if (src == 8) kpanic("invalid cached x87 loop register mapping");
            if (src == start) {
                moveFpuReg(entry.slots[dst].value, value);
                mov(JitWidth::b32, entry.slots[dst].tag, tag);
                break;
            }
            moveFpuReg(entry.slots[dst].value, entry.slots[src].value);
            mov(JitWidth::b32, entry.slots[dst].tag, entry.slots[src].tag);
            dst = src;
        }
    }
    if (rotation) {
        // Validity locals stay with physical slots through FXCH (which first
        // validates both operands), so they only need the TOP rotation.
        bool rotated[8] = {};
        RegPtr valid = getTmpReg();
        for (U8 start = 0; start < 8; ++start) {
            if (rotated[start]) continue;
            mov(JitWidth::b32, valid, entry.slots[start].valid);
            U8 dst = start;
            while (true) {
                rotated[dst] = true;
                U8 src = (dst + rotation) & 7;
                mov(JitWidth::b32, entry.slots[dst].valid, src == start ? valid : entry.slots[src].valid);
                if (src == start) break;
                dst = src;
            }
        }
        addValue(JitWidth::b32, entry.entryTop, rotation);
        andValue(JitWidth::b32, entry.entryTop, 7);
    }
}

S32 JitFPU::cachedFpuStackChange(DecodedOp* op) const {
    // Keep stack effects together with the cacheable instruction set. This is
    // used only after canCacheFpuOp has accepted the instruction.
    switch (op->inst) {
    case FLD_SINGLE_REAL: case FLD_DOUBLE_REAL:
    case FILD_DWORD_INTEGER: case FILD_WORD_INTEGER:
    case FLD1: case FLDZ: case FLD_STi:
    case FLDL2T: case FLDL2E: case FLDPI: case FLDLG2: case FLDLN2:
    case FDECSTP: case FILD_QWORD_INTEGER:
    case FLD_EXTENDED_REAL: case FBLD_PACKED_BCD:
    case FPTAN: case FSINCOS: case FXTRACT:
        return -1;
    case FADD_STi_ST0_Pop: case FMUL_STi_ST0_Pop:
    case FSUB_STi_ST0_Pop: case FSUBR_STi_ST0_Pop:
    case FDIV_STi_ST0_Pop: case FDIVR_STi_ST0_Pop:
    case FST_SINGLE_REAL_Pop: case FST_DOUBLE_REAL_Pop:
    case FIST_DWORD_INTEGER_Pop: case FISTTP32:
    case FCOM_STi_Pop: case FUCOM_STi_Pop:
    case FCOM_SINGLE_REAL_Pop: case FCOM_DOUBLE_REAL_Pop:
    case FICOM_DWORD_INTEGER_Pop: case FICOM_WORD_INTEGER_Pop:
    case FFREEP_STi: case FST_STi_Pop: case FINCSTP:
    case FCOMI_ST0_STj_Pop: case FUCOMI_ST0_STj_Pop:
    case FIST_WORD_INTEGER_Pop: case FISTTP16:
    case FISTP_QWORD_INTEGER: case FISTTP64:
    case FSTP_EXTENDED_REAL: case FBSTP_PACKED_BCD:
    case FYL2X: case FYL2XP1: case FPATAN:
        return 1;
    case FCOMPP: case FUCOMPP:
        return 2;
    default:
        return 0;
    }
}

void JitFPU::cachedFpuBinary(U8 dst, U8 src, XmmXmmCallback callback, bool reverse, bool pop) {
    FPURegPtr a = cachedFpuValue(dst);
    FPURegPtr b = cachedFpuValue(src);
    if (reverse) {
        FPURegPtr tmp = getFPUTmp();
        moveFpuReg(tmp, b);
        (this->*callback)(tmp, a);
        moveFpuReg(a, tmp);
    } else {
        (this->*callback)(a, b);
    }
    roundFpuResultToPrecision(a);
    cachedFpuSlot(dst).dirty = true;
    if (pop) cachedFpuPop();
}

void JitFPU::cachedFpuDiv(U8 dst, U8 src, bool reverse, bool pop) {
    // Match the existing division guard, but use the virtual stack's tags.
    // A slow division executes the whole instruction, including any pop, so it
    // must leave the block instead of rejoining the fast arm's cached mapping.
    RegPtr state = readCPU(JitWidth::b8, offsetof(CPU, fpu.divExceptionsUnmasked));
    orReg(JitWidth::b8, state, cachedFpuTag(dst));
    orReg(JitWidth::b8, state, cachedFpuTag(src));
    guardCachedFpuFallback(state);
    cachedFpuBinary(dst, src, &JitFPU::fpuDiv, reverse, pop);
}

void JitFPU::guardCachedFpuFallback(RegPtr state) {
    auto savedCache = fpuStackCache;
    If(JitWidth::b8, state); {
        emulateSingleOp();
        blockExit();
    } EndIf();
    fpuStackCache = savedCache;
}

void JitFPU::cachedFpuMemory(DecodedOp* op, XmmXmmCallback callback, JitWidth width, bool integer, bool reverse, bool division) {
    read(width, calculateEaa(op), [this, callback, width, integer, reverse, division](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        if (integer) {
            RegPtr value = getTmpReg();
            readHost(width, address, value);
            if (width == JitWidth::b16) movsx(JitWidth::b32, value, JitWidth::b16, value);
            regToFpuReg(tmp, value);
        } else {
            loadFpuReg(tmp, address, width == JitWidth::b32 ? DYN_FPU_32_BIT : DYN_FPU_64_BIT);
            if (width == JitWidth::b32) fpuRegExtend32To64(tmp, tmp);
        }
        RegPtr isZero;
        if (division && !reverse) {
            isZero = integer ? getFpuDivMemoryIntZero(address, width) : getFpuDivMemoryFloatZero(address, width);
        }
        // The operand and zero check are now loaded. Release the guarded MMU
        // address before importing tags: Win32 has only four byte registers.
        address = nullptr;
        if (division) {
            RegPtr state = readCPU(JitWidth::b8, offsetof(CPU, fpu.divExceptionsUnmasked));
            if (isZero) {
                orReg(JitWidth::b8, state, isZero);
                isZero = nullptr;
            }
            orReg(JitWidth::b8, state, cachedFpuTag(0));
            guardCachedFpuFallback(state);
        }
        FPURegPtr dst = cachedFpuValue(0);
        if (reverse) {
            (this->*callback)(tmp, dst);
            moveFpuReg(dst, tmp);
        } else {
            (this->*callback)(dst, tmp);
        }
        roundFpuResultToPrecision(dst);
        cachedFpuSlot(0).dirty = true;
    });
}

void JitFPU::cachedFpuStore(U8 dst, bool pop) {
    if (pop) {
        RegPtr empty = getTmpReg8();
        mov(JitWidth::b32, empty, cachedFpuTag(0));
        compareValue(JitWidth::b8, empty, TAG_Empty, JitEvaluate::EQUALS, empty);
        guardCachedFpuFallback(empty);
    }
    FPURegPtr value = cachedFpuValue(0);
    RegPtr tag = cachedFpuTag(0);
    auto& slot = cachedFpuSlot(dst);
    moveFpuReg(slot.value, value);
    mov(JitWidth::b32, slot.tag, tag);
    slot.loaded = slot.dirty = slot.tagLoaded = slot.tagDirty = true;
    if (slot.valid) movValue(JitWidth::b32, slot.valid, 1);
    slot.needsValidation = false;
    if (pop) cachedFpuPop();
}

void JitFPU::cachedFpuConditionalMove(U8 src, JitConditional condition) {
    if (!src) return;
    auto& dst = cachedFpuSlot(0);
    if (!dst.loaded) {
        // Import the raw double plus its validity, without converting an
        // extended value that an untaken move must leave untouched.
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
        dst.valid = getFpuCacheValidReg(fpuStackCache.top);
        loadCpuFpuReg(dst.value, index);
        readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached), dst.valid);
        dst.loaded = dst.needsValidation = true;
    }
    cachedFpuTag(0);
    auto savedCache = fpuStackCache;
    IfCondition(condition); {
        FPURegPtr value = cachedFpuValue(src);
        RegPtr tag = cachedFpuTag(src);
        moveFpuReg(dst.value, value);
        mov(JitWidth::b32, dst.tag, tag);
        if (dst.valid) movValue(JitWidth::b32, dst.valid, 1);
    } EndIf();
    // Source imports only happened on the taken arm. On either arm the
    // destination's validity flag describes whether its double can be used.
    fpuStackCache = savedCache;
    cachedFpuSlot(0).dirty = cachedFpuSlot(0).tagDirty = true;
}

void JitFPU::cachedFpuLoadInt64(MemPtr address) {
    fpuStackCache.top = (fpuStackCache.top - 1) & 7;
    RegPtr index = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    loadInt64ToExtended(address, index);
    markCachedFpuTopExtended();
}

void JitFPU::markCachedFpuTopExtended() {
    auto& slot = cachedFpuSlot(0);
    // The exact value now lives in CPU extended storage. Retire any stale
    // double in this slot, including a dirty value from an earlier iteration.
    slot.valid = getFpuCacheValidReg(fpuStackCache.top);
    movValue(JitWidth::b32, slot.valid, 0);
    movValue(JitWidth::b32, slot.tag, TAG_Valid);
    slot.loaded = slot.needsValidation = slot.tagLoaded = slot.tagDirty = true;
    slot.dirty = false;
}

void JitFPU::cachedFpuTransfer80(DecodedOp* op) {
    bool store = op->inst == FSTP_EXTENDED_REAL || op->inst == FBSTP_PACKED_BCD;
    bool bcd = op->inst == FBLD_PACKED_BCD || op->inst == FBSTP_PACKED_BCD;
    accessFpu80(calculateEaa(op), store, [this, store, bcd](MemPtr address) {
        if (!store) fpuStackCache.top = (fpuStackCache.top - 1) & 7;
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
        if (store) {
            loadCachedFpuState(0);
            auto& slot = cachedFpuSlot(0);
            if (bcd) {
                storeFpuBcd(address, index, slot.value, slot.valid);
            } else {
                saveFpu80(address, index, slot.value, slot.valid);
                // ST80 retains converted raw bits after the pop. BCD stores
                // leave the original representation and cached value intact.
                markCachedFpuTopExtended();
            }
        } else {
            transferFpu80(address, index, store, bcd);
        }
        if (store) cachedFpuPop();
        else markCachedFpuTopExtended();
    });
}

void JitFPU::cachedFpuExamine() {
    auto& slot = cachedFpuSlot(0);
    RegPtr index = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    RegPtr result = getTmpReg();
    auto examineDouble = [this, &slot, index, result] {
        FPURegPtr value = slot.loaded ? slot.value : getFPUTmp();
        if (!slot.loaded) loadCpuFpuReg(value, index);
        mov(JitWidth::b32, result, classifyFpu(value));
    };
    if (slot.loaded && !slot.needsValidation) {
        examineDouble();
    } else {
        RegPtr valid = slot.loaded ? slot.valid : readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached));
        If(JitWidth::b32, valid); {
            examineDouble();
        } StartElse(); {
            mov(JitWidth::b32, result, classifyExtendedFpu(index));
        } EndIf();
    }
    // Empty slots still report the sign of their retained value. Do not load
    // or convert a raw operand merely to classify it.
    IfEqual(JitWidth::b32, cachedFpuTag(0), TAG_Empty); {
        andValue(JitWidth::b32, result, 0x0200);
        orValue(JitWidth::b32, result, 0x4100);
    } EndIf();
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
    andValue(JitWidth::b32, sw, ~0x4700);
    orReg(JitWidth::b32, sw, result);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
}

void JitFPU::cachedFpuMath(DecodedOp* op) {
    bool binary = op->inst == FYL2X || op->inst == FYL2XP1 || op->inst == FPATAN;
    loadCachedFpuState(0);
    auto& xSlot = cachedFpuSlot(0);
    FPURegPtr x = xSlot.value;
    RegPtr st0 = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    auto retainRepresentation = [this](U8 relative) {
        auto& slot = cachedFpuSlot(relative);
        IfNot(JitWidth::b32, slot.valid); {
            RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + relative) & 7);
            storeDoubleAsExtended(slot.value, index);
        } EndIf();
        slot.dirty = slot.needsValidation = true;
    };
    if (binary) {
        loadCachedFpuState(1);
        auto& ySlot = cachedFpuSlot(1);
        FPURegPtr y = ySlot.value;
        RegPtr st1 = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + 1) & 7);
        RegPtr cached = getTmpReg();
        mov(JitWidth::b32, cached, xSlot.valid);
        orReg(JitWidth::b32, cached, ySlot.valid);
        // Match the interpreter's representation choice and conversion order.
        // Either cached operand selects getF64 for both; two raw operands use
        // the current SoftFloat mode and leave the popped input untouched.
        If(JitWidth::b32, cached); {
            loadFpuValue(x, st0, xSlot.valid);
            loadFpuValue(y, st1, ySlot.valid);
            movValue(JitWidth::b32, xSlot.valid, 1);
            movValue(JitWidth::b32, ySlot.valid, 1);
        } StartElse(); {
            loadFpuValue(x, st0, xSlot.valid, false);
            loadFpuValue(y, st1, ySlot.valid, false);
        } EndIf();
        xSlot.dirty = true;
        if (op->inst == FPATAN) {
            fpuAtan2(y, y, x);
        } else {
            FPURegPtr logarithm = getFPUTmp();
            moveFpuReg(logarithm, x);
            if (op->inst == FYL2XP1) {
                FPURegPtr one = getFPUTmp();
                loadCpuFpuRegConst(one, offsetof(CPU, fOne));
                fpuAdd(logarithm, one);
            }
            fpuMath(FpuMath::Log, logarithm, logarithm);
            // Match the interpreter's order: (y * log(x)) / log(2).
            fpuMul(y, logarithm);
            loadCpuFpuRegConst(logarithm, offsetof(CPU, fLN2));
            fpuDiv(y, logarithm);
        }
        retainRepresentation(1);
        cachedFpuPop();
    } else {
        // A raw input is converted for the host math call, but its result must
        // remain raw: later BCD stores and remainders select algorithms by it.
        loadFpuValue(x, st0, xSlot.valid, false);
        if (op->inst == FSINCOS) {
            FPURegPtr cosine = getFPUTmp();
            fpuMath(FpuMath::Cos, cosine, x);
            fpuMath(FpuMath::Sin, x, x);
            retainRepresentation(0);
            cachedFpuPush(cosine);
            auto& pushed = cachedFpuSlot(0);
            pushed.valid = getFpuCacheValidReg(fpuStackCache.top);
            mov(JitWidth::b32, pushed.valid, xSlot.valid);
            retainRepresentation(0);
        } else {
            FpuMath math = op->inst == FSIN ? FpuMath::Sin : op->inst == FCOS ? FpuMath::Cos :
                op->inst == FPTAN ? FpuMath::Tan : FpuMath::Exp2Minus1;
            fpuMath(math, x, x);
            retainRepresentation(0);
            if (op->inst == FPTAN) cachedFpuLoadConst(offsetof(CPU, fOne));
        }
    }
    if (op->inst == FSIN || op->inst == FCOS || op->inst == FPTAN || op->inst == FSINCOS) {
        RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
        andValue(JitWidth::b32, sw, ~0x0400);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
    }
}

void JitFPU::loadCachedFpuState(U8 relativeIndex) {
    auto& slot = cachedFpuSlot(relativeIndex);
    slot.valid = getFpuCacheValidReg((fpuStackCache.top + relativeIndex) & 7);
    if (!slot.loaded) {
        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + relativeIndex) & 7);
        loadCpuFpuReg(slot.value, index);
        readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached), slot.valid);
    } else if (!slot.needsValidation) {
        movValue(JitWidth::b32, slot.valid, 1);
    }
    slot.loaded = slot.needsValidation = true;
    cachedFpuTag(relativeIndex);
}

void JitFPU::cachedFpuSpecial(DecodedOp* op) {
    bool extract = op->inst == FXTRACT;
    bool remainder = op->inst == FPREM || op->inst == FPREM_nearest;
    U8 other = extract ? 7 : 1;
    loadCachedFpuState(0);
    if (!extract) loadCachedFpuState(1);
    auto& x = cachedFpuSlot(0);
    auto& y = cachedFpuSlot(other);
    RegPtr st0 = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    RegPtr st1 = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + other) & 7);
    auto reload = [this](CachedFpuSlot& slot, RegPtr index) {
        loadCpuFpuReg(slot.value, index);
        readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached), slot.valid);
    };
    auto slow = [&, this] {
        U8 mask = (U8)((1u << fpuStackCache.top) | (1u << ((fpuStackCache.top + 1) & 7)));
        materializeFpuCache(mask);
        U8 operation = op->inst == FPREM ? 0 : 1;
        fpuSlotArithmetic(operation, st0, st1);
        reload(x, st0); reload(y, st1);
    };
    if (remainder) {
        // Runtime validity decides whether a result lives in a local or in its
        // raw slot, including paths that widen a dirty cached operand inline.
        x.dirty = y.dirty = true;
        fpuRemainder(x.value, y.value, x.valid, y.valid, st0, st1, op->inst == FPREM_nearest, slow);
        // Fast results live in locals; slow results may still be raw. Runtime
        // validity guards writeback and the next arithmetic use at the join.
        x.dirty = y.dirty = true;
    } else {
        if (extract) fpuExtract(x.value, x.valid, st0, st1);
        else fpuScale(x.value, y.value, x.valid, y.valid, st0, st1);
        // These operations retain the interpreter's extended representation.
        // Retire stale local values without touching the rest of the cache.
        // All inline paths leave these physical slots raw.
        // No need to import stale CPU double values merely to mark them invalid.
        movValue(JitWidth::b32, x.valid, 0);
        x.dirty = false;
        if (extract) {
            fpuStackCache.top = (fpuStackCache.top - 1) & 7;
            markCachedFpuTopExtended();
        } else {
            movValue(JitWidth::b32, y.valid, 0);
            y.dirty = false;
        }
    }
}

void JitFPU::cachedFpuStoreInt64(MemPtr address, bool truncate) {
    auto& slot = cachedFpuSlot(0);
    RegPtr index = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    auto storeDouble = [this, &slot, index, address, truncate]() {
        // Some backends round their input in place. Preserve the original
        // value for later TOP rotations and aliases even though this op pops.
        FPURegPtr tmp = getFPUTmp();
        if (slot.loaded) moveFpuReg(tmp, slot.value);
        else loadCpuFpuReg(tmp, index);
        storeFPUToInt64(tmp, address, truncate);
    };
    if (slot.loaded && !slot.needsValidation) {
        storeDouble();
    } else {
        RegPtr valid = slot.loaded ? slot.valid : readCPU(JitWidth::b8, index, 0, offsetof(CPU, fpu.isRegCached));
        If(JitWidth::b32, valid); {
            storeDouble();
        } StartElse(); {
            // Read the exact physical slot without materializing other slots
            // or converting the operand through a rounded double.
            storeExtendedAsInt64(index, address, truncate);
        } EndIf();
    }
}

void JitFPU::cachedFpuLoadConst(U32 offset) {
    FPURegPtr value = getFPUTmp();
    loadCpuFpuRegConst(value, offset);
    cachedFpuPush(value);
}

void JitFPU::cachedFpuCompare(U8 src, U8 pops, bool integerFlags) {
    FPURegPtr a = cachedFpuValue(0);
    FPURegPtr b = cachedFpuValue(src);
    // Comparison backends may modify ordTags; persistent tags must survive.
    RegPtr tags = getTmpReg8();
    mov(JitWidth::b32, tags, cachedFpuTag(0));
    orReg(JitWidth::b32, tags, cachedFpuTag(src));
    if (integerFlags) doFCOMI(b, a, tags);
    else doFCOM(b, a, tags);
    while (pops--) cachedFpuPop();
}

void JitFPU::cachedFpuCompareMemory(DecodedOp* op, JitWidth width, bool integer, bool pop) {
    read(width, calculateEaa(op), [this, width, integer, pop](MemPtr address) {
        FPURegPtr other = getFPUTmp();
        if (integer) {
            RegPtr value = getTmpReg();
            readHost(width, address, value);
            if (width == JitWidth::b16) movsx(JitWidth::b32, value, JitWidth::b16, value);
            regToFpuReg(other, value);
        } else {
            loadFpuReg(other, address, width == JitWidth::b32 ? DYN_FPU_32_BIT : DYN_FPU_64_BIT);
            if (width == JitWidth::b32) fpuRegExtend32To64(other, other);
        }
        // Win32 needs these address registers for the tag and status update.
        address = nullptr;
        FPURegPtr value = cachedFpuValue(0);
        RegPtr tags = getTmpReg8();
        mov(JitWidth::b32, tags, cachedFpuTag(0));
        doFCOM(other, value, tags);
        if (pop) cachedFpuPop();
    });
}

void JitFPU::cachedFpuInit(DecodedOp* op) {
    // FINIT discards cached doubles but leaves the raw register bits alone.
    // Do not write back values that the instruction is about to invalidate.
    dynamic_FNINIT(op);
    movValue(JitWidth::b32, fpuStackCache.entryTop, 0);
    fpuStackCache.top = 0;
    for (U8 i = 0; i < 8; ++i) {
        auto& slot = fpuStackCache.slots[i];
        slot.valid = getFpuCacheValidReg(i);
        movValue(JitWidth::b32, slot.valid, 0);
        movValue(JitWidth::b32, slot.tag, TAG_Empty);
        // Also valid at a retained-cache loop header: a later iteration may
        // reach this position with an entirely different incoming stack.
        slot.loaded = slot.dirty = slot.tagLoaded = slot.tagDirty = slot.needsValidation = true;
    }
}

void JitFPU::rebaseFpuCache(RegPtr newTop) {
    // Import and pin validity before rebasing, but reclaim temporary indices.
    withFpuScratchScope([this] { prepareFpuCacheForLoop(true); });
    RegPtr rotation = getTmpReg();
    // Keep newTop live; mov may consume a temporary source on some backends.
    addValueWithDest(JitWidth::b32, rotation, newTop, 0);
    subReg(JitWidth::b32, rotation, fpuStackCache.entryTop);
    // Three conditional rotations preserve every value/validity pair,
    // including FXCH-renamed locals. Raw values stay in their physical slots.
    // Tags need no rotation: FLDENV replaces all eight immediately afterwards.
    for (U8 step : {1, 2, 4}) {
        withFpuScratchScope([this, rotation, step] {
            IfTest(JitWidth::b32, rotation, step); {
                FPURegPtr value = getFPUTmp();
                RegPtr valid = getTmpReg();
                for (U8 start = 0; start < step; ++start) {
                    moveFpuReg(value, fpuStackCache.slots[start].value);
                    mov(JitWidth::b32, valid, fpuStackCache.slots[start].valid);
                    U8 dst = start;
                    while (true) {
                        U8 src = (dst + step) & 7;
                        moveFpuReg(fpuStackCache.slots[dst].value, src == start ? value : fpuStackCache.slots[src].value);
                        mov(JitWidth::b32, fpuStackCache.slots[dst].valid, src == start ? valid : fpuStackCache.slots[src].valid);
                        if (src == start) break;
                        dst = src;
                    }
                }
            } EndIf();
        });
    }
    mov(JitWidth::b32, fpuStackCache.entryTop, newTop);
    fpuStackCache.top = 0;
}

RegPtr JitFPU::cachedFpuTagWord() {
    RegPtr tags = getTmpReg();
    movValue(JitWidth::b32, tags, 0);
    RegPtr mmx = readCPU(JitWidth::b8, offsetof(CPU, fpu.isMMXInUse));
    for (U8 i = 0; i < 8; ++i) {
        withFpuScratchScope([this, tags, mmx, i] {
            loadCachedFpuState(i);
            auto& slot = cachedFpuSlot(i);
            RegPtr tag = getTmpReg();
            andValueWithDest(JitWidth::b32, tag, slot.tag, 3);
            If(JitWidth::b32, mmx); {
                movValue(JitWidth::b32, tag, TAG_Valid);
            } EndIf();
            RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + i) & 7);
            IfNotEqual(JitWidth::b32, tag, TAG_Empty); {
                RegPtr classification = getTmpReg();
                If(JitWidth::b32, slot.valid); {
                    mov(JitWidth::b32, classification, classifyFpu(slot.value));
                } StartElse(); {
                    mov(JitWidth::b32, classification, classifyExtendedFpu(index));
                } EndIf();
                // GetTag preserves the stored tag for ordinary finite values.
                // Classification must not convert a raw value or raise exceptions.
                IfTest(JitWidth::b32, classification, 0x4000); {
                    movValue(JitWidth::b32, tag, TAG_Zero);
                } EndIf();
                IfTest(JitWidth::b32, classification, 0x0100); {
                    movValue(JitWidth::b32, tag, TAG_Special);
                } EndIf();
            } EndIf();
            shlValue(JitWidth::b32, index, 1);
            shlReg(JitWidth::b32, tag, index);
            orReg(JitWidth::b32, tags, tag);
        });
    }
    return tags;
}

void JitFPU::cachedFpuEnvironment(DecodedOp* op) {
    bool full = op->inst == FNSAVE || op->inst == FRSTOR;
    bool store = op->inst == FNSTENV || op->inst == FNSAVE;
    U32 stride = cpu->isBig() ? 4 : 2;
    JitWidth width = cpu->isBig() ? JitWidth::b32 : JitWidth::b16;
    accessFpuEnvironment(calculateEaa(op), 7 * stride + (full ? 80 : 0), store, [this, op, full, store, stride, width](MemPtr address) {
        auto field = [address, stride](U32 i) {
            MemPtr result = address->copy();
            result->offset += i * stride;
            return result;
        };
        if (store) {
            RegPtr sw = cachedFpuStatusWord();
            writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
            writeHost(width, field(0), readCPU(JitWidth::b32, offsetof(CPU, fpu.cw)));
            writeHost(width, field(1), sw);
            writeHost(width, field(2), cachedFpuTagWord());
            for (U32 i = 0; i < 4; ++i)
                writeHost(width, field(i + 3), readCPU(JitWidth::b32, offsetof(CPU, fpu.envData) + i * sizeof(U32)));
            if (full) {
                for (U8 i = 0; i < 8; ++i) {
                    withFpuScratchScope([this, address, stride, i] {
                        // Tag generation has imported every slot without
                        // converting the lazy extended values.
                        auto& slot = cachedFpuSlot(i);
                        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, (fpuStackCache.top + i) & 7);
                        MemPtr valueAddress = address->copy();
                        valueAddress->offset += 7 * stride + 10 * i;
                        saveFpu80(valueAddress, index, slot.value, slot.valid);
                    });
                }
                cachedFpuInit(op);
            }
        } else {
            RegPtr tmp = getTmpReg();
            readHost(width, field(0), tmp);
            andValue(JitWidth::b32, tmp, 0xffff); // SetCW takes a 16-bit word.
            writeCPU(JitWidth::b32, offsetof(CPU, fpu.cw), tmp);
            shrValue(JitWidth::b32, tmp, 10);
            andValue(JitWidth::b32, tmp, 3);
            writeCPU(JitWidth::b32, offsetof(CPU, fpu.round), tmp);
            readHost(width, field(1), tmp);
            writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), tmp);
            shrValue(JitWidth::b32, tmp, 11);
            andValue(JitWidth::b32, tmp, 7);
            if (full) {
                // FRSTOR replaces the values too; none of the old locals need
                // importing or rotating. Keep tmp available for later fields.
                addValueWithDest(JitWidth::b32, fpuStackCache.entryTop, tmp, 0);
                fpuStackCache.top = 0;
            } else {
                withFpuScratchScope([this, tmp] { rebaseFpuCache(tmp); });
            }
            RegPtr tags = getTmpReg();
            readHost(width, field(2), tags);
            for (U8 i = 0; i < 8; ++i) {
                withFpuScratchScope([this, tags, i] {
                    RegPtr shift = calculateIndexReg(fpuStackCache.entryTop, i);
                    shlValue(JitWidth::b32, shift, 1);
                    auto& slot = fpuStackCache.slots[i];
                    addValueWithDest(JitWidth::b32, slot.tag, tags, 0);
                    shrReg(JitWidth::b32, slot.tag, shift);
                    andValue(JitWidth::b32, slot.tag, 3);
                    slot.tagLoaded = slot.tagDirty = true;
                });
            }
            for (U32 i = 0; i < 4; ++i) {
                readHost(width, field(i + 3), tmp);
                writeCPU(JitWidth::b32, offsetof(CPU, fpu.envData) + i * sizeof(U32), tmp);
            }
            updateFpuDivExceptionState();
            updateExceptionSummary();
            if (full) {
                for (U8 i = 0; i < 8; ++i) {
                    withFpuScratchScope([this, address, stride, i] {
                        RegPtr index = calculateIndexReg(fpuStackCache.entryTop, i);
                        MemPtr valueAddress = address->copy();
                        valueAddress->offset += 7 * stride + 10 * i;
                        transferFpu80(valueAddress, index, false, false);
                        auto& slot = fpuStackCache.slots[i];
                        slot.valid = getFpuCacheValidReg(i);
                        movValue(JitWidth::b32, slot.valid, 0);
                        slot.loaded = slot.needsValidation = true;
                        slot.dirty = false;
                    });
                }
                writeCPUValue(JitWidth::b32, offsetof(CPU, fpuDirtyFlags), 1);
            }
        }
    });
}

RegPtr JitFPU::cachedFpuStatusWord() {
    RegPtr top = calculateIndexReg(fpuStackCache.entryTop, fpuStackCache.top);
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
    andValue(JitWidth::b32, sw, ~0x3800);
    shlValue(JitWidth::b32, top, 11);
    orReg(JitWidth::b32, sw, top);
    return sw;
}

// Ordinary integer and SSE instructions do not alias the x87 stack. Helpers
// and exits synchronize explicitly; only legacy x87/MMX lowering still needs
// architectural FPU state before it emits an instruction.
bool JitFPU::canKeepFpuCache(DecodedOp* op) const {
    return supportsFpuStackCache() &&
        (canCacheFpuOp(op) || (!op->isFpuOp() && !op->isMmxOp()));
}

bool JitFPU::canCacheNativeFpuOp(DecodedOp* op) const {
    switch (op->inst) {
    case FCMOV_ST0_STj_CF: case FCMOV_ST0_STj_ZF:
    case FCMOV_ST0_STj_CF_OR_ZF: case FCMOV_ST0_STj_PF:
    case FCMOV_ST0_STj_NCF: case FCMOV_ST0_STj_NZF:
    case FCMOV_ST0_STj_NCF_AND_NZF: case FCMOV_ST0_STj_NPF:
    case FXAM: case F2XM1: case FSIN: case FCOS: case FPTAN: case FSINCOS:
    case FYL2X: case FYL2XP1: case FPATAN:
    case FPREM: case FPREM_nearest: case FSCALE: case FXTRACT:
    case FNINIT: case FRNDINT:
    case FILD_QWORD_INTEGER: case FISTP_QWORD_INTEGER: case FISTTP64:
    case FLD_EXTENDED_REAL: case FSTP_EXTENDED_REAL:
    case FBLD_PACKED_BCD: case FBSTP_PACKED_BCD:
    case FLDENV: case FNSTENV: case FNSAVE: case FRSTOR:
        return false;
    default:
        return JitFPU::canCacheFpuOp(op);
    }
}

bool JitFPU::canCacheFpuOp(DecodedOp* op) const {
    if (!supportsFpuStackCache()) return false;
    switch (op->inst) {
    case FADD_ST0_STj:
    case FADD_STi_ST0:
    case FADD_STi_ST0_Pop:
    case FADD_SINGLE_REAL:
    case FADD_DOUBLE_REAL:
    case FIADD_DWORD_INTEGER:
    case FIADD_WORD_INTEGER:
    case FMUL_ST0_STj:
    case FMUL_STi_ST0:
    case FMUL_STi_ST0_Pop:
    case FMUL_SINGLE_REAL:
    case FMUL_DOUBLE_REAL:
    case FIMUL_DWORD_INTEGER:
    case FIMUL_WORD_INTEGER:
    case FSUB_ST0_STj:
    case FSUB_STi_ST0:
    case FSUB_STi_ST0_Pop:
    case FSUB_SINGLE_REAL:
    case FSUB_DOUBLE_REAL:
    case FISUB_DWORD_INTEGER:
    case FISUB_WORD_INTEGER:
    case FSUBR_ST0_STj:
    case FSUBR_STi_ST0:
    case FSUBR_STi_ST0_Pop:
    case FSUBR_SINGLE_REAL:
    case FSUBR_DOUBLE_REAL:
    case FISUBR_DWORD_INTEGER:
    case FISUBR_WORD_INTEGER:
    case FLD_SINGLE_REAL:
    case FLD_DOUBLE_REAL:
    case FILD_DWORD_INTEGER:
    case FILD_WORD_INTEGER:
    case FST_SINGLE_REAL:
    case FST_SINGLE_REAL_Pop:
    case FST_DOUBLE_REAL:
    case FST_DOUBLE_REAL_Pop:
    case FLD1:
    case FLDZ:
    case FLD_STi:
    case FXCH_STi:
    case FDIV_ST0_STj:
    case FDIVR_ST0_STj:
    case FDIV_STi_ST0:
    case FDIVR_STi_ST0:
    case FDIV_STi_ST0_Pop:
    case FDIVR_STi_ST0_Pop:
    case FIST_DWORD_INTEGER:
    case FIST_DWORD_INTEGER_Pop:
    case FISTTP32:
    case FCOM_STi: case FCOM_STi_Pop:
    case FUCOM_STi: case FUCOM_STi_Pop:
    case FCOMPP: case FUCOMPP:
    case FCOM_SINGLE_REAL: case FCOM_SINGLE_REAL_Pop:
    case FCOM_DOUBLE_REAL: case FCOM_DOUBLE_REAL_Pop:
    case FICOM_DWORD_INTEGER: case FICOM_DWORD_INTEGER_Pop:
    case FICOM_WORD_INTEGER: case FICOM_WORD_INTEGER_Pop:
    case FFREE_STi: case FFREEP_STi:
    case FNSTSW: case FNSTSW_AX:
    case FNCLEX: case FNOP:
    case FDIV_SINGLE_REAL: case FDIVR_SINGLE_REAL:
    case FDIV_DOUBLE_REAL: case FDIVR_DOUBLE_REAL:
    case FIDIV_DWORD_INTEGER: case FIDIVR_DWORD_INTEGER:
    case FIDIV_WORD_INTEGER: case FIDIVR_WORD_INTEGER:
    case FRNDINT: case FLDCW: case FNSTCW:
    case FST_STi: case FST_STi_Pop:
    case FCHS: case FABS: case FSQRT: case FTST:
    case FDECSTP: case FINCSTP:
    case FLDL2T: case FLDL2E: case FLDPI: case FLDLG2: case FLDLN2:
    case FCMOV_ST0_STj_CF: case FCMOV_ST0_STj_ZF:
    case FCMOV_ST0_STj_CF_OR_ZF: case FCMOV_ST0_STj_PF:
    case FCMOV_ST0_STj_NCF: case FCMOV_ST0_STj_NZF:
    case FCMOV_ST0_STj_NCF_AND_NZF: case FCMOV_ST0_STj_NPF:
    case FCOMI_ST0_STj: case FUCOMI_ST0_STj:
    case FCOMI_ST0_STj_Pop: case FUCOMI_ST0_STj_Pop:
    case FIST_WORD_INTEGER: case FIST_WORD_INTEGER_Pop: case FISTTP16:
    case FILD_QWORD_INTEGER: case FISTP_QWORD_INTEGER: case FISTTP64:
    case FLD_EXTENDED_REAL: case FSTP_EXTENDED_REAL:
    case FBLD_PACKED_BCD: case FBSTP_PACKED_BCD:
    case FXAM: case F2XM1: case FSIN: case FCOS: case FPTAN: case FSINCOS:
    case FYL2X: case FYL2XP1: case FPATAN:
    case FPREM: case FPREM_nearest: case FSCALE: case FXTRACT:
    case FNINIT: case FLDENV: case FNSTENV: case FNSAVE: case FRSTOR:
        return true;
    default:
        return false;
    }
}

bool JitFPU::compileCachedFpuOp(DecodedOp* op) {
    if (!canCacheFpuOp(op)) return false;
    beginFpuCache();
    switch (op->inst) {
    case FNINIT:
        cachedFpuInit(op);
        break;
    case FLDENV: case FNSTENV: case FNSAVE: case FRSTOR:
        cachedFpuEnvironment(op);
        break;
    case FPREM: case FPREM_nearest: case FSCALE: case FXTRACT:
        cachedFpuSpecial(op);
        break;
    case FXAM:
        cachedFpuExamine();
        break;
    case F2XM1: case FSIN: case FCOS: case FPTAN: case FSINCOS:
    case FYL2X: case FYL2XP1: case FPATAN:
        cachedFpuMath(op);
        break;
    case FLD_EXTENDED_REAL: case FSTP_EXTENDED_REAL:
    case FBLD_PACKED_BCD: case FBSTP_PACKED_BCD:
        cachedFpuTransfer80(op);
        break;
    case FILD_QWORD_INTEGER:
        read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
            cachedFpuLoadInt64(address);
        });
        break;
    case FISTP_QWORD_INTEGER: case FISTTP64:
        write(JitWidth::b64, calculateEaa(op), nullptr, [this, op](MemPtr address) {
            cachedFpuStoreInt64(address, op->inst == FISTTP64);
            cachedFpuPop();
        });
        break;
    case FST_STi: case FST_STi_Pop:
        cachedFpuStore(op->reg, op->inst == FST_STi_Pop);
        break;
    case FCHS: case FABS:
        {
            FPURegPtr value = cachedFpuValue(0);
            if (op->inst == FCHS) fpuNeg(value);
            else fpuAbs(value);
            cachedFpuSlot(0).dirty = true;
        }
        break;
    case FSQRT:
        {
            FPURegPtr value = cachedFpuValue(0);
            fpuSqrt(value, value);
            roundFpuResultToPrecision(value);
            cachedFpuSlot(0).dirty = true;
        }
        break;
    case FTST:
        {
            FPURegPtr value = cachedFpuValue(0);
            FPURegPtr zero = getFPUTmp();
            fpuXor(zero, zero);
            RegPtr tag = getTmpReg8();
            mov(JitWidth::b32, tag, cachedFpuTag(0));
            doFCOM(zero, value, tag);
        }
        break;
    case FDECSTP:
        fpuStackCache.top = (fpuStackCache.top - 1) & 7;
        break;
    case FINCSTP:
        fpuStackCache.top = (fpuStackCache.top + 1) & 7;
        break;
    case FLDL2T: cachedFpuLoadConst(offsetof(CPU, fL2T)); break;
    case FLDL2E: cachedFpuLoadConst(offsetof(CPU, fL2E)); break;
    case FLDPI: cachedFpuLoadConst(offsetof(CPU, fPi)); break;
    case FLDLG2: cachedFpuLoadConst(offsetof(CPU, fLG2)); break;
    case FLDLN2: cachedFpuLoadConst(offsetof(CPU, fLN2)); break;
    case FCMOV_ST0_STj_CF: cachedFpuConditionalMove(op->reg, JitConditional::B); break;
    case FCMOV_ST0_STj_ZF: cachedFpuConditionalMove(op->reg, JitConditional::Z); break;
    case FCMOV_ST0_STj_CF_OR_ZF: cachedFpuConditionalMove(op->reg, JitConditional::BE); break;
    case FCMOV_ST0_STj_PF: cachedFpuConditionalMove(op->reg, JitConditional::P); break;
    case FCMOV_ST0_STj_NCF: cachedFpuConditionalMove(op->reg, JitConditional::NB); break;
    case FCMOV_ST0_STj_NZF: cachedFpuConditionalMove(op->reg, JitConditional::NZ); break;
    case FCMOV_ST0_STj_NCF_AND_NZF: cachedFpuConditionalMove(op->reg, JitConditional::NBE); break;
    case FCMOV_ST0_STj_NPF: cachedFpuConditionalMove(op->reg, JitConditional::NP); break;
    case FCOMI_ST0_STj: case FUCOMI_ST0_STj:
        cachedFpuCompare(op->reg, 0, true);
        break;
    case FCOMI_ST0_STj_Pop: case FUCOMI_ST0_STj_Pop:
        cachedFpuCompare(op->reg, 1, true);
        break;

    case FCOM_STi: case FUCOM_STi:
        cachedFpuCompare(op->reg, 0);
        break;
    case FCOM_STi_Pop: case FUCOM_STi_Pop:
        cachedFpuCompare(op->reg, 1);
        break;
    case FCOMPP: case FUCOMPP:
        cachedFpuCompare(1, 2);
        break;
    case FCOM_SINGLE_REAL: case FCOM_SINGLE_REAL_Pop:
        cachedFpuCompareMemory(op, JitWidth::b32, false, op->inst == FCOM_SINGLE_REAL_Pop);
        break;
    case FCOM_DOUBLE_REAL: case FCOM_DOUBLE_REAL_Pop:
        cachedFpuCompareMemory(op, JitWidth::b64, false, op->inst == FCOM_DOUBLE_REAL_Pop);
        break;
    case FICOM_DWORD_INTEGER: case FICOM_DWORD_INTEGER_Pop:
        cachedFpuCompareMemory(op, JitWidth::b32, true, op->inst == FICOM_DWORD_INTEGER_Pop);
        break;
    case FICOM_WORD_INTEGER: case FICOM_WORD_INTEGER_Pop:
        cachedFpuCompareMemory(op, JitWidth::b16, true, op->inst == FICOM_WORD_INTEGER_Pop);
        break;
    case FFREE_STi: case FFREEP_STi:
        {
            auto& slot = cachedFpuSlot(op->reg);
            movValue(JitWidth::b32, slot.tag, TAG_Empty);
            slot.tagLoaded = slot.tagDirty = true;
            if (op->inst == FFREEP_STi) cachedFpuPop();
        }
        break;
    case FNSTSW:
        write(JitWidth::b16, calculateEaa(op), nullptr, [this](MemPtr address) {
            writeHost(JitWidth::b16, address, cachedFpuStatusWord());
        });
        break;
    case FNSTSW_AX:
        mov(JitWidth::b16, getReg(0), cachedFpuStatusWord());
        break;
    case FNCLEX:
        dynamic_FNCLEX(op);
        break;
    case FNOP:
        break;
    case FDIV_SINGLE_REAL: case FDIVR_SINGLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuDiv, JitWidth::b32, false, op->inst == FDIVR_SINGLE_REAL, true);
        break;
    case FDIV_DOUBLE_REAL: case FDIVR_DOUBLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuDiv, JitWidth::b64, false, op->inst == FDIVR_DOUBLE_REAL, true);
        break;
    case FIDIV_DWORD_INTEGER: case FIDIVR_DWORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuDiv, JitWidth::b32, true, op->inst == FIDIVR_DWORD_INTEGER, true);
        break;
    case FIDIV_WORD_INTEGER: case FIDIVR_WORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuDiv, JitWidth::b16, true, op->inst == FIDIVR_WORD_INTEGER, true);
        break;
    case FRNDINT:
        {
            FPURegPtr value = cachedFpuValue(0);
            updateFPURounding();
            roundFPUToInt64(value);
            restoreFPURounding();
            cachedFpuSlot(0).dirty = true;
        }
        break;
    case FLDCW:
        dynamic_FLDCW(op);
        break;
    case FNSTCW:
        dynamic_FNSTCW(op);
        break;
    case FADD_ST0_STj:
        cachedFpuBinary(0, op->reg, &JitFPU::fpuAdd, false, false);
        break;
    case FADD_STi_ST0:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuAdd, false, false);
        break;
    case FADD_STi_ST0_Pop:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuAdd, false, true);
        break;
    case FADD_SINGLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuAdd, JitWidth::b32, false, false);
        break;
    case FADD_DOUBLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuAdd, JitWidth::b64, false, false);
        break;
    case FIADD_DWORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuAdd, JitWidth::b32, true, false);
        break;
    case FIADD_WORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuAdd, JitWidth::b16, true, false);
        break;
    case FMUL_ST0_STj:
        cachedFpuBinary(0, op->reg, &JitFPU::fpuMul, false, false);
        break;
    case FMUL_STi_ST0:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuMul, false, false);
        break;
    case FMUL_STi_ST0_Pop:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuMul, false, true);
        break;
    case FMUL_SINGLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuMul, JitWidth::b32, false, false);
        break;
    case FMUL_DOUBLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuMul, JitWidth::b64, false, false);
        break;
    case FIMUL_DWORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuMul, JitWidth::b32, true, false);
        break;
    case FIMUL_WORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuMul, JitWidth::b16, true, false);
        break;
    case FSUB_ST0_STj:
        cachedFpuBinary(0, op->reg, &JitFPU::fpuSub, false, false);
        break;
    case FSUB_STi_ST0:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuSub, false, false);
        break;
    case FSUB_STi_ST0_Pop:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuSub, false, true);
        break;
    case FSUB_SINGLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b32, false, false);
        break;
    case FSUB_DOUBLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b64, false, false);
        break;
    case FISUB_DWORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b32, true, false);
        break;
    case FISUB_WORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b16, true, false);
        break;
    case FSUBR_ST0_STj:
        cachedFpuBinary(0, op->reg, &JitFPU::fpuSub, true, false);
        break;
    case FSUBR_STi_ST0:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuSub, true, false);
        break;
    case FSUBR_STi_ST0_Pop:
        cachedFpuBinary(op->reg, 0, &JitFPU::fpuSub, true, true);
        break;
    case FSUBR_SINGLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b32, false, true);
        break;
    case FSUBR_DOUBLE_REAL:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b64, false, true);
        break;
    case FISUBR_DWORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b32, true, true);
        break;
    case FISUBR_WORD_INTEGER:
        cachedFpuMemory(op, &JitFPU::fpuSub, JitWidth::b16, true, true);
        break;
    case FDIV_ST0_STj:
        cachedFpuDiv(0, op->reg, false, false);
        break;
    case FDIVR_ST0_STj:
        cachedFpuDiv(0, op->reg, true, false);
        break;
    case FDIV_STi_ST0:
        cachedFpuDiv(op->reg, 0, false, false);
        break;
    case FDIVR_STi_ST0:
        cachedFpuDiv(op->reg, 0, true, false);
        break;
    case FDIV_STi_ST0_Pop:
        cachedFpuDiv(op->reg, 0, false, true);
        break;
    case FDIVR_STi_ST0_Pop:
        cachedFpuDiv(op->reg, 0, true, true);
        break;
    case FLD_SINGLE_REAL:
        read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
            FPURegPtr tmp = getFPUTmp();
            loadFpuReg(tmp, address, DYN_FPU_32_BIT);
            fpuRegExtend32To64(tmp, tmp);
            cachedFpuPush(tmp);
        });
        break;
    case FLD_DOUBLE_REAL:
        read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
            FPURegPtr tmp = getFPUTmp();
            loadFpuReg(tmp, address, DYN_FPU_64_BIT);
            cachedFpuPush(tmp);
        });
        break;
    case FILD_DWORD_INTEGER:
        read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
            FPURegPtr tmp = getFPUTmp();
            RegPtr value = getTmpReg();
            readHost(JitWidth::b32, address, value);
            regToFpuReg(tmp, value);
            cachedFpuPush(tmp);
        });
        break;
    case FILD_WORD_INTEGER:
        read(JitWidth::b16, calculateEaa(op), [this](MemPtr address) {
            FPURegPtr tmp = getFPUTmp();
            RegPtr value = getTmpReg();
            readHost(JitWidth::b16, address, value);
            movsx(JitWidth::b32, value, JitWidth::b16, value);
            regToFpuReg(tmp, value);
            cachedFpuPush(tmp);
        });
        break;
    case FST_SINGLE_REAL:
        write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
            FPURegPtr value = cachedFpuValue(0);
            FPURegPtr tmp = getFPUTmp();
            fpuReg64To32(tmp, value);
            storeFpuReg(tmp, address, DYN_FPU_32_BIT);
        });
        break;
    case FST_SINGLE_REAL_Pop:
        write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
            FPURegPtr value = cachedFpuValue(0);
            FPURegPtr tmp = getFPUTmp();
            fpuReg64To32(tmp, value);
            storeFpuReg(tmp, address, DYN_FPU_32_BIT);
            cachedFpuPop();
        });
        break;
    case FST_DOUBLE_REAL:
        write(JitWidth::b64, calculateEaa(op), nullptr, [this](MemPtr address) {
            FPURegPtr value = cachedFpuValue(0);
            storeFpuReg(value, address);
        });
        break;
    case FST_DOUBLE_REAL_Pop:
        write(JitWidth::b64, calculateEaa(op), nullptr, [this](MemPtr address) {
            FPURegPtr value = cachedFpuValue(0);
            storeFpuReg(value, address);
            cachedFpuPop();
        });
        break;
    case FIST_DWORD_INTEGER:
    case FIST_DWORD_INTEGER_Pop:
    case FISTTP32:
    case FIST_WORD_INTEGER:
    case FIST_WORD_INTEGER_Pop:
    case FISTTP16:
        {
            bool truncate = op->inst == FISTTP32 || op->inst == FISTTP16;
            bool pop = op->inst != FIST_DWORD_INTEGER && op->inst != FIST_WORD_INTEGER;
            JitWidth width = (op->inst == FIST_WORD_INTEGER || op->inst == FIST_WORD_INTEGER_Pop || op->inst == FISTTP16) ? JitWidth::b16 : JitWidth::b32;
            write(width, calculateEaa(op), nullptr, [this, truncate, pop, width](MemPtr address) {
                if (!truncate) updateFPURounding();
                // Conversion must leave the cached float unchanged for FIST
                // and any aliases created by FLD ST(i).
                RegPtr result = fpuRegToInt32(cachedFpuValue(0), truncate);
                if (!truncate) restoreFPURounding();
                writeHost(width, address, result);
                // The memory wrapper handles fault/SMC fallback before this
                // fast arm; commit the virtual pop only after the store.
                if (pop) cachedFpuPop();
            });
        }
        break;
    case FLD1:
        {
            RegPtr one = getTmpReg();
            movValue(JitWidth::b32, one, 1);
            FPURegPtr value = getFPUTmp();
            regToFpuReg(value, one);
            cachedFpuPush(value);
        }
        break;
    case FLDZ:
        {
            FPURegPtr value = getFPUTmp();
            fpuXor(value, value);
            cachedFpuPush(value);
        }
        break;
    case FLD_STi:
        {
            FPURegPtr value = cachedFpuValue(op->reg);
            RegPtr tag = cachedFpuTag(op->reg);
            cachedFpuPush(value, tag);
        }
        break;
    case FXCH_STi:
        {
            // Pin both values/tags while a bounded native cache reloads them.
            FPURegPtr firstValue = cachedFpuValue(0);
            FPURegPtr secondValue = cachedFpuValue(op->reg);
            RegPtr firstTag = cachedFpuTag(0);
            RegPtr secondTag = cachedFpuTag(op->reg);
            auto& a = cachedFpuSlot(0);
            auto& b = cachedFpuSlot(op->reg);
            std::swap(a.value, b.value);
            std::swap(a.tag, b.tag);
            a.dirty = b.dirty = a.tagDirty = b.tagDirty = true;
        }
        break;
    default:
        return false;
    }
    return true;
}

RegPtr JitFPU::calculateIndexReg(RegPtr topReg, U32 index) {
    RegPtr result = getTmpReg();
    addValueWithDest(JitWidth::b32, result, topReg, index);
    andValue(JitWidth::b32, result, 7);
    return result;
}

RegPtr JitFPU::readFPUTag(RegPtr indexReg) {
    return readCPU(JitWidth::b8, indexReg, 0, offsetof(CPU, fpu.tags[0]));
}

void JitFPU::writeFPUTag(RegPtr indexReg, RegPtr valueReg) {
    writeCPU(JitWidth::b8, indexReg, 0, offsetof(CPU, fpu.tags[0]), valueReg);
}

void JitFPU::dynamic_FPU_PREP_PUSH(RegPtr topReg, bool writeTag) {
    subValue(JitWidth::b32, topReg, 1);
    andValue(JitWidth::b32, topReg, 7);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), topReg);

    if (writeTag) {
        writeCPUValue(JitWidth::b8, topReg, 0, offsetof(CPU, fpu.tags[0]), TAG_Valid);
    }
}

void JitFPU::IfNotRegCached(RegPtr indexReg) {
    static_assert(sizeof(cpu->fpu.isRegCached) == 9, "false");
    IfNotCPU(JitWidth::b8, indexReg, 0, offsetof(CPU, fpu.isRegCached));
}

void JitFPU::setRegIsCached(RegPtr indexReg, bool regIsCached) {
    writeCPUValue(JitWidth::b8, indexReg, 0, offsetof(CPU, fpu.isRegCached), regIsCached ? 1 : 0);
}

// movsd qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)], xmm
void JitFPU::syncXmmToCPU(RegPtr topReg, FPURegPtr fpuReg, U8 regIndex) {
    if (regIndex == 0) {
        syncXmmToCPUWithIndexReg(topReg, fpuReg);
    } else {
        syncXmmToCPUWithIndexReg(calculateIndexReg(topReg, regIndex), fpuReg);
    }
}

void JitFPU::syncXmmToCPUWithIndexReg(RegPtr indexReg, FPURegPtr xmm) {
    static_assert(sizeof(cpu->fpu.regCache) == 72, "false");
    storeCpuFpuReg(xmm, indexReg);
    setRegIsCached(indexReg, true);
}

static void dynamic_cache_float(CPU* cpu, U32 index) {
    cpu->fpu.getF64(cpu->fpu.STV(index));
    cpu->fpu.getF64(cpu->fpu.STV(0));
}

void JitFPU::cacheFpuReg(U32 regIndex) {
    call_I(dynamic_cache_float, regIndex);
}

// movsd xmm, qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)]
RegPtr JitFPU::syncCPUToXmm(RegPtr topReg, FPURegPtr fpuReg, U8 regIndex) {
    RegPtr indexReg = topReg;
    if (regIndex != 0) {
        indexReg = calculateIndexReg(topReg, regIndex);
    }
    IfNotRegCached(indexReg);
    cacheFpuReg((U32)regIndex);
    EndIf();
    loadCpuFpuReg(fpuReg, indexReg);
    return indexReg;
}

RegPtr JitFPU::getTopReg() {
    return readCPU(JitWidth::b32, offsetof(CPU, fpu.top));
}

void JitFPU::updateFpuDivExceptionState() {
    constexpr U32 REQUIRED_MASKS = FPU_SW_IE | FPU_SW_ZE;
    RegPtr state = getTmpReg8();
    readCPU(JitWidth::b32, offsetof(CPU, fpu.cw), state);
    xorValue(JitWidth::b32, state, REQUIRED_MASKS);
    andValue(JitWidth::b32, state, REQUIRED_MASKS);
    compareValue(JitWidth::b32, state, 0, JitEvaluate::NOT_EQUALS, state);
    writeCPU(JitWidth::b8, offsetof(CPU, fpu.divExceptionsUnmasked), state);
}

void JitFPU::updateExceptionSummary() {
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
    RegPtr cw = readCPU(JitWidth::b32, offsetof(CPU, fpu.cw));
    xorValue(JitWidth::b32, cw, FPU_SW_EXCEPTION_MASK);
    andReg(JitWidth::b32, cw, sw);
    andValue(JitWidth::b32, cw, FPU_SW_EXCEPTION_MASK);

    If(JitWidth::b32, cw); {
        orValue(JitWidth::b32, sw, FPU_SW_ES);
    } StartElse(); {
        andValue(JitWidth::b32, sw, ~FPU_SW_ES);
    } EndIf();

    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
}

RegPtr JitFPU::getFpuDivSlowPathState(RegPtr indexReg) {
    RegPtr state = readCPU(JitWidth::b8, offsetof(CPU, fpu.divExceptionsUnmasked));
    orReg(JitWidth::b8, state, readFPUTag(indexReg));
    return state;
}

void JitFPU::guardFpuDivSlowPath(RegPtr state) {
    If(JitWidth::b8, state); {
        emulateSingleOp();
    } EndIf();
}

void JitFPU::guardFpuDivRegTags(RegPtr stIndex, RegPtr otherIndex, bool reverse) {
    (void)reverse;
    RegPtr state = getFpuDivSlowPathState(stIndex);
    orReg(JitWidth::b8, state, readFPUTag(otherIndex));
    guardFpuDivSlowPath(state);
}

RegPtr JitFPU::getFpuDivMemoryFloatZero(MemPtr address, JitWidth width) {
    // The address base can be a persistent backend register (R15 is the
    // Windows linear-memory base), so it must never double as the load result.
    RegPtr value = getTmpReg();
    if (width == JitWidth::b64) {
        MemPtr highAddress = address->copy();
        highAddress->offset += 4;
        RegPtr high = getTmpReg();
        readHost(JitWidth::b32, highAddress, high);
        readHost(JitWidth::b32, address, value);
        andValue(JitWidth::b32, high, 0x7fffffff);
        orReg(JitWidth::b32, value, high);
    } else {
        readHost(JitWidth::b32, address, value);
        andValue(JitWidth::b32, value, 0x7fffffff);
    }
    compareValue(JitWidth::b32, value, 0, JitEvaluate::EQUALS, value);
    return value;
}

RegPtr JitFPU::getFpuDivMemoryIntZero(MemPtr address, JitWidth width) {
    RegPtr value = getTmpReg();
    readHost(width, address, value);
    compareValue(width, value, 0, JitEvaluate::EQUALS, value);
    return value;
}

void JitFPU::guardFpuDivMemory(RegPtr top, RegPtr isZero) {
    RegPtr state = getFpuDivSlowPathState(top);
    if (isZero) {
        orReg(JitWidth::b8, state, isZero);
    }
    guardFpuDivSlowPath(state);
}

class FPUReg {
public:
    FPUReg(JitFPU* data, RegPtr topReg, U32 regIndex, RegPtr& calculatedIndexReg) {
        this->reg = data->getFPUTmp();
        calculatedIndexReg = data->syncCPUToXmm(topReg, this->reg, regIndex);
    }
    FPUReg(JitFPU* data, RegPtr topReg, U32 regIndex) {
        this->reg = data->getFPUTmp();
        data->syncCPUToXmm(topReg, this->reg, regIndex);
    }
    FPURegPtr reg;
};

void JitFPU::roundFpuResultToPrecision(FPURegPtr result) {
    RegPtr precision = readCPU(JitWidth::b32, offsetof(CPU, fpu.cw));
    andValue(JitWidth::b32, precision, 0x0300);
    IfEqual(JitWidth::b32, precision, 0); {
        precision = nullptr;
        FPURegPtr rounded = getFPUTmp();
        updateFPURounding();
        fpuReg64To32(rounded, result);
        restoreFPURounding();
        fpuRegExtend32To64(result, rounded);
    } EndIf();
}

void JitFPU::dynamic_SINGLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    read(JitWidth::b32, calculateEaa(op), [reverse, callback, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address, DYN_FPU_32_BIT);
        fpuRegExtend32To64(tmp, tmp);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(tmp, dst.reg);
        } else {
            (this->*callback)(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_DIV_SINGLE_REAL(DecodedOp* op, bool reverse) {
    read(JitWidth::b32, calculateEaa(op), [reverse, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address, DYN_FPU_32_BIT);
        fpuRegExtend32To64(tmp, tmp);
        RegPtr isZero;
        if (!reverse) {
            isZero = getFpuDivMemoryFloatZero(address, JitWidth::b32);
        }
        address = nullptr;
        RegPtr top = getTopReg();
        guardFpuDivMemory(top, isZero);
        FPUReg dst(this, top, 0);

        if (reverse) {
            fpuDiv(tmp, dst.reg);
        } else {
            fpuDiv(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_FCOM_SINGLE_REAL(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address, DYN_FPU_32_BIT);
        fpuRegExtend32To64(tmp, tmp);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(std::move(top)));
    });
}

void JitFPU::dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address, DYN_FPU_32_BIT);
        fpuRegExtend32To64(tmp, tmp);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);
        doFCOM(tmp, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_FCOMPP(DecodedOp* op) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg reg1(this, top, 1, index);
    FPUReg reg2(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(JitWidth::b8, tag, readFPUTag(index));
    doFCOM(reg1.reg, reg2.reg, tag);
    tag = nullptr;
    index = nullptr;
    dynamic_FPU_POP(top, 2);
}

void JitFPU::dynamic_STi_ST0(DecodedOp* op, XmmXmmCallback callback, bool reverse, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);

    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        roundFpuResultToPrecision(src.reg);
        syncXmmToCPUWithIndexReg(index, src.reg);
    } else {
        (this->*callback)(dst.reg, src.reg);
        roundFpuResultToPrecision(dst.reg);
        syncXmmToCPUWithIndexReg(index, dst.reg);
    }
    if (pop) {
        dynamic_FPU_POP(top);
    }
}

void JitFPU::doFCOM_STi(DecodedOp* op, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(JitWidth::b8, tag, readFPUTag(std::move(index)));
    doFCOM(dst.reg, src.reg, tag);
    if (pop) {
        dynamic_FPU_POP(top);
    }
}

void JitFPU::dynamic_FCOM_STi(DecodedOp* op) {
    doFCOM_STi(op, false);
}

void JitFPU::dynamic_FCOM_STi_Pop(DecodedOp* op) {
    doFCOM_STi(op, true);
}

void JitFPU::dynamic_FUCOM_STi(DecodedOp* op) {
    doFCOM_STi(op, false);
}

void JitFPU::dynamic_FUCOM_STi_Pop(DecodedOp* op) {
    doFCOM_STi(op, true);
}

void JitFPU::dynamic_FPU_POP(RegPtr topReg, U8 amount) {
    // this->tags[this->top] = TAG_Empty;
    // this->top = ((this->top + 1) & 7);
    writeCPUValue(JitWidth::b8, topReg, 0, offsetof(CPU, fpu.tags[0]), TAG_Empty);
    addValue(JitWidth::b32, topReg, amount);
    andValue(JitWidth::b32, topReg, 7);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), topReg);
}

void JitFPU::dynamic_ST0_STj(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    RegPtr top = getTopReg();
    FPUReg src(this, top, op->reg);
    FPUReg dst(this, top, 0);
    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        roundFpuResultToPrecision(src.reg);
        syncXmmToCPU(top, src.reg, 0);
    } else {
        (this->*callback)(dst.reg, src.reg);
        roundFpuResultToPrecision(dst.reg);
        syncXmmToCPU(top, dst.reg, 0);
    }
}

void JitFPU::dynamic_FDIVR_ST0_STj(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(top, calculateIndexReg(top, op->reg), true);
    dynamic_ST0_STj(op, &JitFPU::fpuDiv, true);
}

void JitFPU::dynamic_FDIV_ST0_STj(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(top, calculateIndexReg(top, op->reg), false);
    dynamic_ST0_STj(op, &JitFPU::fpuDiv);
}

void JitFPU::dynamic_FDIVR_STi_ST0(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(calculateIndexReg(top, op->reg), top, true);
    dynamic_STi_ST0(op, &JitFPU::fpuDiv, true);
}

void JitFPU::dynamic_FDIV_STi_ST0(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(calculateIndexReg(top, op->reg), top, false);
    dynamic_STi_ST0(op, &JitFPU::fpuDiv);
}

void JitFPU::dynamic_FDIVR_STi_ST0_Pop(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(calculateIndexReg(top, op->reg), top, true);
    dynamic_STi_ST0(op, &JitFPU::fpuDiv, true, true);
}

void JitFPU::dynamic_FDIV_STi_ST0_Pop(DecodedOp* op) {
    RegPtr top = getTopReg();
    guardFpuDivRegTags(calculateIndexReg(top, op->reg), top, false);
    dynamic_STi_ST0(op, &JitFPU::fpuDiv, false, true);
}

void JitFPU::dynamic_DOUBLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    read(JitWidth::b64, calculateEaa(op), [reverse, callback, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(tmp, dst.reg);
        } else {
            (this->*callback)(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_DIV_DOUBLE_REAL(DecodedOp* op, bool reverse) {
    read(JitWidth::b64, calculateEaa(op), [reverse, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address);
        RegPtr isZero;
        if (!reverse) {
            isZero = getFpuDivMemoryFloatZero(address, JitWidth::b64);
        }
        address = nullptr;
        RegPtr top = getTopReg();
        guardFpuDivMemory(top, isZero);
        FPUReg dst(this, top, 0);

        if (reverse) {
            fpuDiv(tmp, dst.reg);
        } else {
            fpuDiv(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_FCOM_DOUBLE_REAL(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
    });
}

void JitFPU::dynamic_FCOM_DOUBLE_REAL_Pop(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_DWORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    read(JitWidth::b32, calculateEaa(op), [reverse, callback, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromInt(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(tmp, dst.reg);
        } else {
            (this->*callback)(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_IDIV_DWORD_INTEGER(DecodedOp* op, bool reverse) {
    read(JitWidth::b32, calculateEaa(op), [reverse, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromInt(tmp, address);
        RegPtr isZero;
        if (!reverse) {
            isZero = getFpuDivMemoryIntZero(address, JitWidth::b32);
        }
        address = nullptr;
        RegPtr top = getTopReg();
        guardFpuDivMemory(top, isZero);
        FPUReg dst(this, top, 0);

        if (reverse) {
            fpuDiv(tmp, dst.reg);
        } else {
            fpuDiv(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_FICOM_DWORD_INTEGER(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromInt(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
    });
}

void JitFPU::dynamic_FICOM_DWORD_INTEGER_Pop(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromInt(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::loadFpuRegFromShort(FPURegPtr reg, MemPtr address) {
    RegPtr result = getTmpReg();
    readHost(JitWidth::b16, address, result);
    movsx(JitWidth::b32, result, JitWidth::b16, result);
    regToFpuReg(reg, std::move(result));
}

void JitFPU::dynamic_WORD_INTEGER(DecodedOp* op, XmmXmmCallback callback,  bool reverse) {
    read(JitWidth::b16, calculateEaa(op), [reverse, callback, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromShort(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(tmp, dst.reg);
        } else {
            (this->*callback)(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_IDIV_WORD_INTEGER(DecodedOp* op, bool reverse) {
    read(JitWidth::b16, calculateEaa(op), [reverse, this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromShort(tmp, address);
        RegPtr isZero;
        if (!reverse) {
            isZero = getFpuDivMemoryIntZero(address, JitWidth::b16);
        }
        address = nullptr;
        RegPtr top = getTopReg();
        guardFpuDivMemory(top, isZero);
        FPUReg dst(this, top, 0);

        if (reverse) {
            fpuDiv(tmp, dst.reg);
        } else {
            fpuDiv(dst.reg, tmp);
        }
        FPURegPtr result = reverse ? tmp : dst.reg;
        roundFpuResultToPrecision(result);
        syncXmmToCPU(top, result, 0);
    });
}

void JitFPU::dynamic_FICOM_WORD_INTEGER(DecodedOp* op) {
    read(JitWidth::b16, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromShort(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
    });
}

void JitFPU::dynamic_FICOM_WORD_INTEGER_Pop(DecodedOp* op) {
    read(JitWidth::b16, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromShort(tmp, address);
        address = nullptr;
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(tmp, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::fpuNeg(FPURegPtr dst) {
    FPURegPtr mask = getFPUTmp();
    loadCpuFpuRegConst(mask, offsetof(CPU, fNeg));
    fpuXor(dst, mask);
}

void JitFPU::fpuAbs(FPURegPtr dst) {
    FPURegPtr mask = getFPUTmp();
    loadCpuFpuRegConst(mask, offsetof(CPU, fAbs));
    fpuAnd(dst, mask);
}

void JitFPU::dynamic_FCHS(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);
    fpuNeg(dst.reg);
    syncXmmToCPU(top, dst.reg, 0);
}

void JitFPU::dynamic_FABS(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);
    fpuAbs(dst.reg);
    syncXmmToCPU(top, dst.reg, 0);
}

void JitFPU::dynamic_FTST(DecodedOp* op) {
    // this->regs[8].d = 0.0;
    // FCOM(this->top, 8);

    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);
    FPURegPtr tmp = getFPUTmp();

    fpuXor(tmp, tmp);
    doFCOM(tmp, dst.reg, readFPUTag(top));
}

void JitFPU::dynamic_FLD_STi(DecodedOp* op) {
    // int reg_from = cpu->fpu.STV(reg);
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FST(reg_from, cpu->fpu.STV(0));

    RegPtr top = getTopReg();
    FPUReg fromTmp(this, top, op->reg);
    RegPtr tag;
    if (op->reg) {
        tag = readFPUTag(calculateIndexReg(top, op->reg));
    } else {
        tag = readFPUTag(top);
    }
    dynamic_FPU_PREP_PUSH(top, false); // will change topReg

    syncXmmToCPU(top, fromTmp.reg, 0);
    writeFPUTag(top, tag);
}

void JitFPU::dynamic_FXCH_STi(DecodedOp* op) {
    // int tag = this->tags[other];
    // struct FPU_Reg reg = this->regs[other];
    // this->tags[other] = this->tags[st];
    // this->regs[other] = this->regs[st];
    // this->tags[st] = tag;
    // this->regs[st] = reg;

    RegPtr top = getTopReg();
    FPUReg from(this, top, op->reg);
    FPUReg to(this, top, 0);

    // exchange xmm
    syncXmmToCPU(top, from.reg, 0);
    syncXmmToCPU(top, to.reg, op->reg);

    RegPtr index = calculateIndexReg(top, op->reg);

    // exchange tags
    RegPtr topIndex = readFPUTag(top);
    writeFPUTag(top, readFPUTag(index));
    writeFPUTag(index, topIndex);
}

void JitFPU::dynamic_FNOP(DecodedOp* op) {

}

void JitFPU::doFST_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    // cpu->fpu.FPOP();    
    RegPtr top = getTopReg();
    RegPtr index = calculateIndexReg(top, op->reg);

    FPUReg src(this, top, 0);

    // copy tag
    writeFPUTag(index, readFPUTag(top));

    syncXmmToCPUWithIndexReg(index, src.reg);

    if (pop) {
        dynamic_FPU_POP(top);
    }
}

void JitFPU::dynamic_FST_STi(DecodedOp* op) {
    doFST_STi(op, false);
}

void JitFPU::dynamic_FST_STi_Pop(DecodedOp* op) {
    {
        RegPtr top = getTopReg();
        RegPtr tag = readFPUTag(top);
        IfEqual(JitWidth::b8, tag, TAG_Empty); {
            emulateSingleOp();
        } EndIf();
    }
    doFST_STi(op, true);
}

void JitFPU::dynamic_FLD1(DecodedOp* op) {
    RegPtr top = getTopReg();
    RegPtr reg = getTmpReg();
    FPURegPtr tmp = getFPUTmp();

    dynamic_FPU_PREP_PUSH(top, true);
    movValue(JitWidth::b32, reg, 1);
    regToFpuReg(tmp, reg);
    syncXmmToCPU(top, tmp, 0);
}

void JitFPU::fpuLoadConst(U32 offset) {
    RegPtr top = getTopReg();
    FPURegPtr tmp = getFPUTmp();

    dynamic_FPU_PREP_PUSH(top, true);
    loadCpuFpuRegConst(tmp, offset);
    syncXmmToCPU(top, tmp, 0);
}

void JitFPU::dynamic_FLDL2T(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fL2T));
}

void JitFPU::dynamic_FLDL2E(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fL2E));
}

void JitFPU::dynamic_FLDPI(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fPi));
}

void JitFPU::dynamic_FLDLG2(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fLG2));
}

void JitFPU::dynamic_FLDLN2(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fLN2));
}

void JitFPU::dynamic_FLDZ(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPURegPtr tmp = getFPUTmp();

    dynamic_FPU_PREP_PUSH(top, true);
    fpuXor(tmp, tmp);
    syncXmmToCPU(top, tmp, 0);
}

void JitFPU::dynamic_FLD_SINGLE_REAL(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));

    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address, DYN_FPU_32_BIT);
        fpuRegExtend32To64(tmp, tmp);
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true);
        syncXmmToCPU(top, tmp, 0);
    });
}

void JitFPU::dynamic_FST_SINGLE_REAL(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, DYN_FPU_32_BIT);
    });
}

void JitFPU::dynamic_FST_SINGLE_REAL_Pop(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, DYN_FPU_32_BIT);
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_FST_DOUBLE_REAL(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        storeFpuReg(src.reg, address);
    });
}

void JitFPU::dynamic_FST_DOUBLE_REAL_Pop(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        storeFpuReg(src.reg, address);
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_FNSTCW(DecodedOp* op) {
    // cpu->memory->writew(address, cpu->fpu.CW());
    write(JitWidth::b16, calculateEaa(op), readCPU(JitWidth::b16, offsetof(CPU, fpu.cw)));
}

void JitFPU::dynamic_FLDCW(DecodedOp* op) {
    RegPtr cw = read(JitWidth::b16, calculateEaa(op));
    movzx(JitWidth::b32, cw, JitWidth::b16, cw);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.cw), cw);

    shrValue(JitWidth::b32, cw, 10);
    andValue(JitWidth::b32, cw, 3);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.round), cw);
    cw = nullptr;
    updateFpuDivExceptionState();
    updateExceptionSummary();
}

void JitFPU::dynamic_FLDENV(DecodedOp* op) {
    RegPtr tmp = getTmpReg8();
    RegPtr addressReg = calculateEaa(op);
    RegPtr tag;
    
    if (!cpu->isBig()) {
        xorReg(JitWidth::b32, tmp, tmp);
        read(JitWidth::b16, addressReg, nullptr, nullptr, tmp);
        addValue(JitWidth::b32, addressReg, 2);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.cw), tmp);
        shrValue(JitWidth::b32, tmp, 10);
        andValue(JitWidth::b32, tmp, 3);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.round), tmp);

        // sw
        read(JitWidth::b16, addressReg, nullptr, nullptr, tmp);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), tmp);
        addValue(JitWidth::b32, addressReg, 2);
        shrValue(JitWidth::b32, tmp, 11);
        andValue(JitWidth::b32, tmp, 7);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), tmp);

        // tag
        tag = read(JitWidth::b16, addressReg, nullptr, nullptr);
        addValue(JitWidth::b32, addressReg, 2);

        for (int i = 0; i < 4; i++) {
            read(JitWidth::b16, addressReg, nullptr, nullptr, tmp);
            writeCPU(JitWidth::b16, offsetof(CPU, fpu.envData) + sizeof(U32) * i, tmp);
            addValue(JitWidth::b32, addressReg, 2);
        }
    } else {
        read(JitWidth::b32, addressReg, nullptr, nullptr, tmp);
        addValue(JitWidth::b32, addressReg, 4);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.cw), tmp);
        shrValue(JitWidth::b32, tmp, 10);
        andValue(JitWidth::b32, tmp, 3);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.round), tmp);

        // sw
        read(JitWidth::b32, addressReg, nullptr, nullptr, tmp);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), tmp);
        addValue(JitWidth::b32, addressReg, 4);
        shrValue(JitWidth::b32, tmp, 11);
        andValue(JitWidth::b32, tmp, 7);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), tmp);

        // tag
        tag = read(JitWidth::b32, addressReg, nullptr, nullptr);
        addValue(JitWidth::b32, addressReg, 4);

        for (int i = 0; i < 4; i++) {
            read(JitWidth::b32, addressReg, nullptr, nullptr, tmp);
            writeCPU(JitWidth::b32, offsetof(CPU, fpu.envData) + sizeof(U32) * i, tmp);
            addValue(JitWidth::b32, addressReg, 4);
        }
    }

    for (int i = 0; i < 8; i++) {
        if (i != 0) {
            shrValue(JitWidth::b32, tag, 2);
        }
        andValueWithDest(JitWidth::b32, tmp, tag, 3);
        writeCPU(JitWidth::b8, offsetof(CPU, fpu.tags[0]) + i, tmp);
    }
    tmp = nullptr;
    tag = nullptr;
    addressReg = nullptr;
    updateFpuDivExceptionState();
    updateExceptionSummary();
}

void JitFPU::dynamic_FNSTENV(DecodedOp* op) {
    RegPtr tmp = getTmpReg8();
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));

    readCPU(JitWidth::b32, offsetof(CPU, fpu.top), tmp);
    shlValue(JitWidth::b32, tmp, 11);
    andValue(JitWidth::b32, sw, ~0x3800);
    orReg(JitWidth::b32, sw, tmp);
    
    RegPtr addressReg = calculateEaa(op);
    
    if (!cpu->isBig()) {
        // cw
        readCPU(JitWidth::b32, offsetof(CPU, fpu.cw), tmp);
        write(JitWidth::b16, addressReg, tmp);
        addValue(JitWidth::b32, addressReg, 2);

        // sw
        write(JitWidth::b16, addressReg, sw);
        addValue(JitWidth::b32, addressReg, 2);

        // tag
        xorReg(JitWidth::b32, sw, sw);
        for (int i = 0; i < 8; i++) {
            readCPU(JitWidth::b8, offsetof(CPU, fpu.tags[0]) + i, tmp);
            IfEqual(JitWidth::b8, tmp, 3); {
                orValue(JitWidth::b32, sw, 3 << (i * 2));
            } EndIf();
        }
        write(JitWidth::b16, addressReg, sw);
        addValue(JitWidth::b32, addressReg, 2);

        // instruction pointer
        // op code
        // data pointer
        // data pointer selector
        for (int i = 0; i < 4; i++) {
            readCPU(JitWidth::b32, offsetof(CPU, fpu.envData) + sizeof(U32) * i, tmp);
            write(JitWidth::b16, addressReg, tmp);
            addValue(JitWidth::b32, addressReg, 2);
        }
    } else {
        // cw
        readCPU(JitWidth::b32, offsetof(CPU, fpu.cw), tmp);
        write(JitWidth::b32, addressReg, tmp);
        addValue(JitWidth::b32, addressReg, 4);

        // sw
        write(JitWidth::b32, addressReg, sw);
        addValue(JitWidth::b32, addressReg, 4);

        // tag
        xorReg(JitWidth::b32, sw, sw);
        for (int i = 0; i < 8; i++) {
            readCPU(JitWidth::b8, offsetof(CPU, fpu.tags[0]) + i, tmp);
            IfEqual(JitWidth::b8, tmp, 3); {
                orValue(JitWidth::b32, sw, 3 << (i * 2));
            } EndIf();
        }
        write(JitWidth::b32, addressReg, sw);
        addValue(JitWidth::b32, addressReg, 4);

        // instruction pointer
        // op code
        // data pointer
        // data pointer selector
        for (int i = 0; i < 4; i++) {
            readCPU(JitWidth::b32, offsetof(CPU, fpu.envData) + sizeof(U32) * i, tmp);
            write(JitWidth::b32, addressReg, tmp);
            addValue(JitWidth::b32, addressReg, 4);
        }
    }

}

// motorhead uses this
void JitFPU::dynamic_FRNDINT(DecodedOp* op) {
    // double value = this->regCache[this->top].d;
    // this->regCache[this->top].d = (double)(S64)FROUND(value);
    RegPtr top = getTopReg();
    FPUReg src(this, top, 0);
    updateFPURounding();
    roundFPUToInt64(src.reg);
    restoreFPURounding();
    syncXmmToCPU(top, src.reg, 0);
}

void JitFPU::dynamic_FDECSTP(DecodedOp* op) {
    // this->top = (this->top - 1) & 7;
    RegPtr top = getTopReg();
    subValue(JitWidth::b32, top, 1);
    andValue(JitWidth::b32, top, 7);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), top);
}
void JitFPU::dynamic_FINCSTP(DecodedOp* op) {
    // this->top = (this->top + 1) & 7;
    RegPtr top = getTopReg();
    addValue(JitWidth::b32, top, 1);
    andValue(JitWidth::b32, top, 7);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.top), top);
}

void JitFPU::dynamic_doCMov(U8 regIndex) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg src(this, top, regIndex, index);

    syncXmmToCPUWithIndexReg(top, src.reg);
    writeFPUTag(top, readFPUTag(index));
}

void JitFPU::dynamic_FCMOV_ST0_STj_CF(DecodedOp* op) {
    IfCondition(JitConditional::B);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_ZF(DecodedOp* op) {
    IfCondition(JitConditional::Z);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_CF_OR_ZF(DecodedOp* op) {
    IfCondition(JitConditional::BE);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_PF(DecodedOp* op) {
    IfCondition(JitConditional::P);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_NCF(DecodedOp* op) {
    IfCondition(JitConditional::NB);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_NZF(DecodedOp* op) {
    IfCondition(JitConditional::NZ);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_NCF_AND_NZF(DecodedOp* op) {
    IfCondition(JitConditional::NBE);
        dynamic_doCMov(op->reg);
    EndIf();
}

void JitFPU::dynamic_FCMOV_ST0_STj_NPF(DecodedOp* op) {
    IfCondition(JitConditional::NP);
        dynamic_doCMov(op->reg);
    EndIf();
}

// age of empires uses this for path finding
void JitFPU::dynamic_FSQRT(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg reg(this, top, 0);
    fpuSqrt(reg.reg, reg.reg);
    roundFpuResultToPrecision(reg.reg);
    syncXmmToCPU(top, reg.reg, 0);
}

void JitFPU::dynamic_FILD_DWORD_INTEGER(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));

    read(JitWidth::b32, calculateEaa(op), [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPURegPtr tmp = getFPUTmp();

        loadFpuRegFromInt(tmp, address);
        dynamic_FPU_PREP_PUSH(top, true); // will change topReg
        syncXmmToCPUWithIndexReg(top, tmp);
    });
}

// #define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
// #define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
// #define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
// #define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000

void JitFPU::doFCOM(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags) {
    // if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
    // 	((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
    // 	FPU_SET_C3(this, 1);
    // 	FPU_SET_C2(this, 1);
    // 	FPU_SET_C0(this, 1);
    // 	return;
    // }
    // if (this->regs[st].d == this->regs[other].d) {
    // 	FPU_SET_C3(this, 1);
    // 	FPU_SET_C2(this, 0);
    // 	FPU_SET_C0(this, 0);
    // 	return;
    // }
    // if (this->regs[st].d < this->regs[other].d) {
    // 	FPU_SET_C3(this, 0);
    // 	FPU_SET_C2(this, 0);
    // 	FPU_SET_C0(this, 1);
    // 	return;
    // }
    // st > other
    // FPU_SET_C3(this, 0);
    // FPU_SET_C2(this, 0);
    // FPU_SET_C0(this, 0);	

    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
    fcompare(fpuReg1, fpuReg2, ordTags, [sw, this] {
        // equal
        andValue(JitWidth::b32, sw, ~0x0700);
        orValue(JitWidth::b32, sw, 0x4000);
    }, [sw, this] {
        // less than
        andValue(JitWidth::b32, sw, ~0x4600);
        orValue(JitWidth::b32, sw, 0x0100);
    }, [sw, this] {
        // greater than
        andValue(JitWidth::b32, sw, ~0x4700);
    }, [sw, this] {
        // invalid
        andValue(JitWidth::b32, sw, ~0x0200);
        orValue(JitWidth::b32, sw, 0x4500);
    });
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
}

void JitFPU::doFCOMI(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags) {
    // if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
    //     ((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
    //     setFlags(cpu, ZF | PF | CF);
    //     return;
    // }
    // if (this->regs[st].d == this->regs[other].d) {
    //     setFlags(cpu, ZF);
    //     return;
    // }
    // if (this->regs[st].d < this->regs[other].d) {
    //     setFlags(cpu, CF);
    //     return;
    // }
    // st > other
    // setFlags(cpu, 0);

    andCPUFlagsImmV2(~FMASK_TEST);
    // shift 8 because popFlagsFromReg expects flags in AH
    fcompare(fpuReg1, fpuReg2, ordTags, [this] {
        // equal
        orCPUFlagsImmV2(ZF);
    }, [this] {
        // less than
        orCPUFlagsImmV2(CF);
    }, [] {
        // greater than
        // nothing
    }, [this] {
        // invalid
        orCPUFlagsImmV2(CF | PF | ZF);
    });
    storeLazyFlagType(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
}

void JitFPU::dynamic_FUCOMPP(DecodedOp* op) {
    dynamic_FCOMPP(op);
}

void JitFPU::dynamic_FNCLEX(DecodedOp* op) {
    // this->sw &= 0x7f00;
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));

    andValue(JitWidth::b32, sw, 0x7f00);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
}

void JitFPU::dynamic_FNSTSW(DecodedOp* op) {
    // fpu.sw &= ~0x3800; 
    // fpu.sw |= (fpu.top & 7) << 11
    // writew(address, fpu.SW());

    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));

        andValue(JitWidth::b32, sw, ~0x3800);
        shlValue(JitWidth::b32, top, 11);
        orReg(JitWidth::b32, sw, top);
        writeHost(JitWidth::b16, address, sw);
    });
}

void JitFPU::dynamic_FNSTSW_AX(DecodedOp* op) {
    RegPtr top = getTopReg();
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));

    andValue(JitWidth::b32, sw, ~0x3800);
    shlValue(JitWidth::b32, top, 11);
    orReg(JitWidth::b32, sw, top);
    mov(JitWidth::b16, getReg(0), sw);
}

void JitFPU::dynamic_FNINIT(DecodedOp* op) {
    /*
    SetCW(0x37F);
    this->sw = 0;
    this->top = FPU_GET_TOP(this);
    this->tags[0] = TAG_Empty;
    this->tags[1] = TAG_Empty;
    this->tags[2] = TAG_Empty;
    this->tags[3] = TAG_Empty;
    this->tags[4] = TAG_Empty;
    this->tags[5] = TAG_Empty;
    this->tags[6] = TAG_Empty;
    this->tags[7] = TAG_Empty;
    this->isMMXInUse = false;
    memset(isRegCached, 0, sizeof(isRegCached));
    */
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.cw), 0x37f);
    writeCPUValue(JitWidth::b8, offsetof(CPU, fpu.divExceptionsUnmasked), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.sw), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.top), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.round), 0);
    writeCPUValue(JitWidth::b8, offsetof(CPU, fpu.isMMXInUse), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached) + 4, 0);
    writeCPUValue(JitWidth::b8, offsetof(CPU, fpu.isRegCached) + 8, 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.envData[0]), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.envData[1]), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.envData[2]), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.envData[3]), 0);

    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]), TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]) + 4, TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
}

void JitFPU::doFCOMI_ST0_STj(DecodedOp* op, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(JitWidth::b32, tag, readFPUTag(std::move(index)));
    doFCOMI(dst.reg, src.reg, tag);
    if (pop) {
        dynamic_FPU_POP(top);
    }
}

void JitFPU::dynamic_FUCOMI_ST0_STj(DecodedOp* op) {
    doFCOMI_ST0_STj(op, false);
}

void JitFPU::dynamic_FCOMI_ST0_STj(DecodedOp* op) {
    dynamic_FUCOMI_ST0_STj(op);
}

void JitFPU::dynamic_FISTTP32(DecodedOp* op) {
    // cpu->fpu.FSTT_I32(cpu, address);
    // cpu->fpu.FPOP();    
    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        writeHost(JitWidth::b32, address, fpuRegToInt32(src.reg, true));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_FISTTP64(DecodedOp* op) {
    RegPtr top = getTopReg();

    // some apps seem to do a memcpy like thing with data pushed in and out
    // we don't want the loaded 64-bit int to change because of rounding if its writen directly back out
    // so if its not already cached (64-bit format vs 80-bit), then do the slow way to keep the 64-bit precision
    // 
    // see FPU::FLD_I64
    IfNotRegCached(top); {
        JitCodeGen::dynamic_FISTTP64(op);
    } StartElse(); {
        write(JitWidth::b64, calculateEaa(op), nullptr, [top, this](MemPtr address) {
            FPUReg src(this, top, 0);
            storeFPUToInt64(src.reg, address, true);
            dynamic_FPU_POP(top);
        });
    } EndIf();
}

void JitFPU::dynamic_FIST_DWORD_INTEGER(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        updateFPURounding(); // set rounding first since it needs 2 tmp regs

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        RegPtr reg = fpuRegToInt32(src.reg, false);
        restoreFPURounding(); // restore before the write in case it has an exception

        writeHost(JitWidth::b32, address, reg);
    });
}

void JitFPU::dynamic_FIST_DWORD_INTEGER_Pop(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [this](MemPtr address) {
        updateFPURounding(); // set rounding first since it needs 2 tmp regs

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        RegPtr reg = fpuRegToInt32(src.reg, false);
        restoreFPURounding(); // restore before the write in case it has an exception

        writeHost(JitWidth::b32, address, reg);        
        dynamic_FPU_POP(top);
    });
}

void JitFPU::doFFREE_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FFREE_STi(cpu->fpu.STV(reg)); this->tags[st] = TAG_Empty;
    RegPtr top = getTopReg();
    RegPtr index = calculateIndexReg(top, op->reg);
    writeCPUValue(JitWidth::b8, index, 0, offsetof(CPU, fpu.tags[0]), TAG_Empty);
    if (pop) {
        dynamic_FPU_POP(top);
    }
}

void JitFPU::dynamic_FFREE_STi(DecodedOp* op) {
    doFFREE_STi(op, false);
}

void JitFPU::dynamic_FLD_DOUBLE_REAL(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuReg(tmp, address);
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true);
        syncXmmToCPU(top, tmp, 0);
    });
}

void JitFPU::dynamic_FFREEP_STi(DecodedOp* op) {
    doFFREE_STi(op, true);
}

void JitFPU::dynamic_FUCOMI_ST0_STj_Pop(DecodedOp* op) {
    doFCOMI_ST0_STj(op, true);
}

void JitFPU::dynamic_FCOMI_ST0_STj_Pop(DecodedOp* op) {
    doFCOMI_ST0_STj(op, true);
}

// bang bang uses this
void JitFPU::dynamic_FILD_WORD_INTEGER(DecodedOp* op) {
    // S16 value = (S16)cpu->memory->readw(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_I16(value, cpu->fpu.STV(0));
    read(JitWidth::b16, calculateEaa(op), [this](MemPtr address) {
        FPURegPtr tmp = getFPUTmp();
        loadFpuRegFromShort(tmp, address);
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true); // will change topReg
        syncXmmToCPUWithIndexReg(top, tmp);
    });
}

// SSE3 instruction
void JitFPU::dynamic_FISTTP16(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), nullptr, [this](MemPtr address) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        writeHost(JitWidth::b16, address, fpuRegToInt32(src.reg, true));
        dynamic_FPU_POP(top);
    });
}

void JitFPU::dynamic_FIST_WORD_INTEGER(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), nullptr, [this](MemPtr address) {
        updateFPURounding();

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        RegPtr reg = fpuRegToInt32(src.reg, false); 

        restoreFPURounding(); // restore before the write in case it has an exception
        writeHost(JitWidth::b16, address, reg);        
    });
}

void JitFPU::dynamic_FIST_WORD_INTEGER_Pop(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), nullptr, [this](MemPtr address) {
        updateFPURounding();

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        RegPtr reg = fpuRegToInt32(src.reg, false);

        restoreFPURounding(); // restore before the write in case it has an exception
        writeHost(JitWidth::b16, address, reg);
        dynamic_FPU_POP(top);
    });
}

/*
extFloat80_t i64_to_extF80( int64_t a )
{
    uint_fast16_t uiZ64;
    uint_fast64_t absA;
    bool sign;
    int_fast8_t shiftDist;
    union { struct extFloat80M s; extFloat80_t f; } uZ;

    uiZ64 = 0;
    absA = 0;
    if ( a ) {
        sign = (a < 0);
        absA = sign ? -(uint_fast64_t) a : (uint_fast64_t) a;
        shiftDist = softfloat_countLeadingZeros64( absA );
        uiZ64 = packToExtF80UI64( sign, 0x403E - shiftDist );
        absA <<= shiftDist;
    }
    uZ.s.signExp = uiZ64;
    uZ.s.signif  = absA;
    return uZ.f;

}
*/
void JitFPU::dynamic_FILD_QWORD_INTEGER(DecodedOp* op) {
#ifndef BOXEDWINE_64
    JitCodeGen::dynamic_FILD_QWORD_INTEGER(op);
#else
    // adds about 1% to Quake 2

    RegPtr absA = read(JitWidth::b64, calculateEaa(op));
    RegPtr top = getTopReg();

    dynamic_FPU_PREP_PUSH(top, true); // will change topReg

    RegPtr uiZ64 = getTmpReg();
    xorReg(JitWidth::b32, uiZ64, uiZ64);
    If(JitWidth::b64, absA); {

        RegPtr signReg = uiZ64;
        shrValueWithDest(JitWidth::b64, signReg, absA, 63);
        shlValue(JitWidth::b32, signReg, 15);
        absReg(JitWidth::b64, absA);

        RegPtr dist = getTmpReg();

        clzReg(JitWidth::b64, dist, absA);
        shlReg(JitWidth::b64, absA, dist);

        // dist = 0x403e - dist
        subValue(JitWidth::b32, dist, 0x403e);
        negReg2(JitWidth::b32, dist);

        orReg(JitWidth::b32, signReg, dist);
    } EndIf();

    // cpu->fpu.regs[top].signif = absA
    // cpu->fpu.regs[top].signExp = uiZ64
    setRegIsCached(top, false);
    shlValue(JitWidth::b32, top, 4); // fpu reg is 16-bytes
    writeCPU(JitWidth::b16, top, 0, (U32)(offsetof(CPU, fpu.regs[0].signExp)), uiZ64);
    writeCPU(JitWidth::b64, top, 0, (U32)(offsetof(CPU, fpu.regs[0].signif)), absA);
#endif
}

void JitFPU::dynamic_FISTP_QWORD_INTEGER(DecodedOp* op) {
    RegPtr top = getTopReg();

    // some apps seem to do a memcpy like thing with data pushed in and out
    // we don't want the loaded 64-bit int to change because of rounding if its writen directly back out
    // so if its not already cached (64-bit format vs 80-bit), then do the slow way to keep the 64-bit precision
    // 
    // see FPU::FLD_I64
    IfNotRegCached(top); {
        JitCodeGen::dynamic_FISTP_QWORD_INTEGER(op);
    } StartElse(); {
        write(JitWidth::b64, calculateEaa(op), nullptr, [top, this](MemPtr address) {
            FPUReg src(this, top, 0);
            storeFPUToInt64(src.reg, address, false);            
            dynamic_FPU_POP(top);
        });
    } EndIf();
}

#endif
