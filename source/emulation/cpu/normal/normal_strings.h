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

#ifndef __STRINGS_OP_H__
#define __STRINGS_OP_H__
#include "../common/cpu.h"
// Byte/word SSE loops win below these conservative measured byte counts.
constexpr U32 MOVS_RAM_MIN_BYTES = sizeof(void*) == 8 ? 1024 : 128;
// Maximum dword count to inline; wider native 64-bit loops amortize calls later.
constexpr U32 MOVSD_INLINE_MAX = sizeof(void*) == 8 ? 511 : 7;

#ifdef BOXEDWINE_JIT
// The legacy 32-bit x86 backend cannot keep both host pointers plus the
// string registers live. Retain its existing loop/helper policy.
#ifdef BOXEDWINE_JIT_X86
constexpr bool MOVSD_UNROLL_SUPPORTED = false;
#else
constexpr bool MOVSD_UNROLL_SUPPORTED = true;
#endif

inline void profileMovsCount(DecodedOp* op, U32 count) {
    // Keep the warmup average bounded without adding storage to DecodedOp.
    if (op->STR_COUNT < 0xffff) {
        ++op->STR_COUNT;
        op->STR_TOTAL += std::min(count, 0xffffu);
    }
}

inline void profileMovsdCount(DecodedOp* op, U32 count, bool forward) {
    if (count > MOVSD_INLINE_MAX) op->STR_FLAGS |= STR_LARGE_COPY;
    if (count >= 8) op->STR_FLAGS |= STR_WIDE_COPY;
    // Bound both packed counters. Only interpreter warmup is sampled; guards
    // remain mandatory because later calls can have entirely different sizes.
    if (op->STR_COUNT < 0xffff) {
        ++op->STR_COUNT;
        if (forward && count == 6) ++op->MOVSD_SIZE_HITS;
        if (forward && count == 7) op->MOVSD_SIZE_HITS += 0x10000;
    }
}

inline U32 movsdExpectedCount(const DecodedOp* op) {
    // Limit code growth to the two sizes measured in the copy benchmarks.
    // The minimum and dominance requirement are conservative selection policy,
    // not a claim that these thresholds are optimal for every application.
    if (!MOVSD_UNROLL_SUPPORTED || op->STR_COUNT < 16 || op->STR_COUNT > 0xffff) return 0;
    if ((op->MOVSD_SIZE_HITS & 0xffff) * 100 >= op->STR_COUNT * 95) return 6;
    if ((op->MOVSD_SIZE_HITS >> 16) * 100 >= op->STR_COUNT * 95) return 7;
    return 0;
}
#endif

void movsb16(CPU* cpu, U32 base);
void movsb16r(CPU* cpu, U32 base);
void movsb32(CPU* cpu, U32 base);
void movsb32r(CPU* cpu, U32 base);
void movsw16(CPU* cpu, U32 base);
void movsw16r(CPU* cpu, U32 base);
void movsw32(CPU* cpu, U32 base);
void movsw32r(CPU* cpu, U32 base);
void movsd16(CPU* cpu, U32 base);
void movsd16r(CPU* cpu, U32 base);
void movsd32(CPU* cpu, U32 base);
// Copies ordinary RAM only; leaves ECX nonzero when regular access is needed.
void movsb32rRam(CPU* cpu, U32 base);
void movsw32rRam(CPU* cpu, U32 base);
void movsd32rRam(CPU* cpu, U32 base);
void movsd32r(CPU* cpu, U32 base);
// RAM-only compare prefixes preserve flags and leave at least one element.
void cmpsb32rPrefix(CPU* cpu, U32 arg);
void cmpsw32rPrefix(CPU* cpu, U32 arg);
void cmpsd32rPrefix(CPU* cpu, U32 arg);
void scasb32rPrefix(CPU* cpu, U32 arg);
void scasw32rPrefix(CPU* cpu, U32 arg);
void scasd32rPrefix(CPU* cpu, U32 arg);

void cmpsb16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb32r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32r(CPU* cpu, U32 rep_zero, U32 base);
// Ordinary RAM only; unfinished accesses leave ECX nonzero.
void stosb32rRam(CPU* cpu, U32 base);
void stosw32rRam(CPU* cpu, U32 base);
void stosd32rRam(CPU* cpu, U32 base);
void lodsb32rRam(CPU* cpu, U32 base);
void lodsw32rRam(CPU* cpu, U32 base);
void lodsd32rRam(CPU* cpu, U32 base);

void stosb16(CPU* cpu);
void stosb16r(CPU* cpu);
void stosb32(CPU* cpu);
void stosb32r(CPU* cpu);
void stosw16(CPU* cpu);
void stosw16r(CPU* cpu);
void stosw32(CPU* cpu);
void stosw32r(CPU* cpu);
void stosd16(CPU* cpu);
void stosd16r(CPU* cpu);
void stosd32(CPU* cpu);
void stosd32r(CPU* cpu);
void lodsb16(CPU* cpu, U32 base);
void lodsb16r(CPU* cpu, U32 base);
void lodsb32(CPU* cpu, U32 base);
void lodsb32r(CPU* cpu, U32 base);
void lodsw16(CPU* cpu, U32 base);
void lodsw16r(CPU* cpu, U32 base);
void lodsw32(CPU* cpu, U32 base);
void lodsw32r(CPU* cpu, U32 base);
void lodsd16(CPU* cpu, U32 base);
void lodsd16r(CPU* cpu, U32 base);
void lodsd32(CPU* cpu, U32 base);
void lodsd32r(CPU* cpu, U32 base);
void scasb16(CPU* cpu, U32 rep_zero);
void scasb16r(CPU* cpu, U32 rep_zero);
void scasb32(CPU* cpu, U32 rep_zero);
void scasb32r(CPU* cpu, U32 rep_zero);
void scasw16(CPU* cpu, U32 rep_zero);
void scasw16r(CPU* cpu, U32 rep_zero);
void scasw32(CPU* cpu, U32 rep_zero);
void scasw32r(CPU* cpu, U32 rep_zero);
void scasd16(CPU* cpu, U32 rep_zero);
void scasd16r(CPU* cpu, U32 rep_zero);
void scasd32(CPU* cpu, U32 rep_zero);
void scasd32r(CPU* cpu, U32 rep_zero);
#endif
