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
#include "../../softmmu/kmemory_soft.h"
// Copy ordinary RAM a page at a time; special pages use normal accessors.
template<U32 size>
static U32 copyMovsRam(CPU* cpu, U32 source, U32 dest, U32 count, bool backward) {
    const U32 sourceOffset = source & K_PAGE_MASK;
    const U32 destOffset = dest & K_PAGE_MASK;
    if (cpu->debugTrapActive) return 0;
    auto* pages = getMemData(cpu->memory);
    if (sourceOffset > K_PAGE_SIZE - size || destOffset > K_PAGE_SIZE - size) {
        // A single unaligned element may touch two pages. Validate the whole
        // element before reading or writing, retaining sequential MOVS order.
        if (!pages->mmu[source >> K_PAGE_SHIFT].canReadRam ||
            !pages->mmu[(U32)(source + size - 1) >> K_PAGE_SHIFT].canReadRam ||
            !pages->mmu[dest >> K_PAGE_SHIFT].canWriteRam ||
            !pages->mmu[(U32)(dest + size - 1) >> K_PAGE_SHIFT].canWriteRam) return 0;
        U8 value[size];
        for (U32 i = 0; i < size; ++i) {
            const U32 address = source + i;
            value[i] = ramPageGet((RamPage)pages->mmu[address >> K_PAGE_SHIFT].ramIndex)[address & K_PAGE_MASK];
        }
        for (U32 i = 0; i < size; ++i) {
            const U32 address = dest + i;
            ramPageGet((RamPage)pages->mmu[address >> K_PAGE_SHIFT].ramIndex)[address & K_PAGE_MASK] = value[i];
        }
        return 1;
    }
    const MMU& srcPage = pages->mmu[source >> K_PAGE_SHIFT];
    const MMU& dstPage = pages->mmu[dest >> K_PAGE_SHIFT];
    if (!srcPage.canReadRam || !dstPage.canWriteRam) {
        return 0;
    }

    count = std::min(count, backward
        ? std::min(sourceOffset / size + 1, destOffset / size + 1)
        : std::min((K_PAGE_SIZE - sourceOffset) / size, (K_PAGE_SIZE - destOffset) / size));
    U8* src = ramPageGet((RamPage)srcPage.ramIndex) + sourceOffset;
    U8* dst = ramPageGet((RamPage)dstPage.ramIndex) + destOffset;
    const U32 bytes = count * size;
    U8* lowSrc = backward ? src - (bytes - size) : src;
    U8* lowDst = backward ? dst - (bytes - size) : dst;
    const uintptr_t s = (uintptr_t)lowSrc;
    const uintptr_t d = (uintptr_t)lowDst;
    if ((s <= d && d - s >= bytes) || (d < s && s - d >= bytes)) {
        ::memcpy(lowDst, lowSrc, bytes);
    } else {
        // Compare host ranges so guest aliases also preserve REP's sequential
        // overlap behavior. Element-sized memcpy permits unaligned addresses.
        for (U32 i = 0; i < count; ++i) {
            U32 value;
            const S32 offset = backward ? -(S32)(i * size) : (S32)(i * size);
            ::memcpy(&value, src + offset, size);
            ::memcpy(dst + offset, &value, size);
        }
    }
    return count;
}

template<U32 size, bool ramOnly>
static void movs32r(CPU* cpu, U32 base) {
    const U32 dBase = cpu->seg[ES].address;
    const U32 sBase = cpu->seg[base].address;
    const bool backward = cpu->getDirection() < 0;
    const U32 step = backward ? 0u - size : size;
    while (ECX) {
        const U32 source = sBase + ESI;
        const U32 dest = dBase + EDI;
        U32 count = copyMovsRam<size>(cpu, source, dest, ECX, backward);
        if (!count) {
            // The JIT helper cannot fault or invalidate its caller. Leave
            // special accesses for an interpreter exit with precise progress.
            if (ramOnly) return;
            if (size == 1) cpu->memory->writeb(dest, cpu->memory->readb(source));
            else if (size == 2) cpu->memory->writew(dest, cpu->memory->readw(source));
            else cpu->memory->writed(dest, cpu->memory->readd(source));
            count = 1;
        } else if (!ramOnly) {
            const U32 bytes = count * size;
            cpu->memory->checkDebugTrapOnMemoryWrite(backward ? dest - (bytes - size) : dest, bytes);
        }
        // Only committed elements advance the architectural registers.
        ESI += count * step;
        EDI += count * step;
        ECX -= count;
    }
}

void movsb32rRam(CPU* cpu, U32 base) { movs32r<1, true>(cpu, base); }
void movsw32rRam(CPU* cpu, U32 base) { movs32r<2, true>(cpu, base); }
void movsd32rRam(CPU* cpu, U32 base) { movs32r<4, true>(cpu, base); }

void movsb16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    cpu->memory->writeb(dBase+DI, cpu->memory->readb(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsb16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+DI, cpu->memory->readb(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsb32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    cpu->memory->writeb(dBase+EDI, cpu->memory->readb(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsb32r(CPU* cpu, U32 base) {
    movs32r<1, false>(cpu, base);
}

void movsw16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    cpu->memory->writew(dBase+DI, cpu->memory->readw(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsw16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;

    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+DI, cpu->memory->readw(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsw32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    cpu->memory->writew(dBase+EDI, cpu->memory->readw(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsw32r(CPU* cpu, U32 base) {
    movs32r<2, false>(cpu, base);
}

void movsd16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    cpu->memory->writed(dBase+DI, cpu->memory->readd(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsd16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        cpu->memory->writed(dBase+DI, cpu->memory->readd(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsd32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    cpu->memory->writed(dBase+EDI, cpu->memory->readd(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsd32r(CPU* cpu, U32 base) {
    movs32r<4, false>(cpu, base);
}

// The RAM prefix preserves entry flags and leaves at least one element
// for ordinary access and the final subtraction, including on faults.
template<U32 size, bool scan>
static void comparePrefixRam(CPU* cpu, U32 arg);

void cmpsb16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase + DI);
    U8 v2 = cpu->memory->readb(sBase+SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u8 = v2;
    cpu->src.u8 = v1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlagType = FLAGS_SUB8;
}
void cmpsb16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;

    if (count) {
        U8 v1=0;
        U8 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+DI);
            v2 = cpu->memory->readb(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u8 = v2;
        cpu->src.u8 = v1;
        cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
        cpu->lazyFlagType = FLAGS_SUB8;
    }
}
void cmpsb32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase + EDI);
    U8 v2 = cpu->memory->readb(sBase+ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u8 = v2;
    cpu->src.u8 = v1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlagType = FLAGS_SUB8;
}
void cmpsb32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    if (ECX > 16) comparePrefixRam<1, false>(cpu, base | (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U8 v1=0;
        U8 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+EDI);
            v2 = cpu->memory->readb(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u8 = v2;
        cpu->src.u8 = v1;
        cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
        cpu->lazyFlagType = FLAGS_SUB8;
    }
}
void cmpsw16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase + DI);
    U16 v2 = cpu->memory->readw(sBase + SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u16 = v2;
    cpu->src.u16 = v1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlagType = FLAGS_SUB16;
}
void cmpsw16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    if (count) {
        U16 v1=0;
        U16 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+DI);
            v2 = cpu->memory->readw(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u16 = v2;
        cpu->src.u16 = v1;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->lazyFlagType = FLAGS_SUB16;
    }
}
void cmpsw32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase + EDI);
    U16 v2 = cpu->memory->readw(sBase + ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u16 = v2;
    cpu->src.u16 = v1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlagType = FLAGS_SUB16;
}
void cmpsw32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    if (ECX > 16) comparePrefixRam<2, false>(cpu, base | (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U16 v1=0;
        U16 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+EDI);
            v2 = cpu->memory->readw(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u16 = v2;
        cpu->src.u16 = v1;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->lazyFlagType = FLAGS_SUB16;
    }
}
void cmpsd16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase + DI);
    U32 v2 = cpu->memory->readd(sBase + SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u32 = v2;
    cpu->src.u32 = v1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlagType = FLAGS_SUB32;
}
void cmpsd16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    if (count) {
        U32 v1=0;
        U32 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+DI);
            v2 = cpu->memory->readd(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u32 = v2;
        cpu->src.u32 = v1;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->lazyFlagType = FLAGS_SUB32;
    }
}
void cmpsd32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase + EDI);
    U32 v2 = cpu->memory->readd(sBase + ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u32 = v2;
    cpu->src.u32 = v1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlagType = FLAGS_SUB32;
}
void cmpsd32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    if (ECX > 16) comparePrefixRam<4, false>(cpu, base | (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U32 v1=0;
        U32 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+EDI);
            v2 = cpu->memory->readd(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u32 = v2;
        cpu->src.u32 = v1;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->lazyFlagType = FLAGS_SUB32;
    }
}
// Batched STOS/LODS for ordinary RAM only. Return before any access that can
// fault, invoke a debug callback, or invalidate the caller's generated code.
template<U32 size, bool store>
static void fillLoad32rRam(CPU* cpu, U32 base) {
    if (cpu->debugTrapActive) return;
    const U32 segment = cpu->seg[base].address;
    const bool backward = cpu->getDirection() < 0;
    const U32 step = backward ? 0u - size : size;
    auto* pages = getMemData(cpu->memory);
    while (ECX) {
        const U32 address = segment + (store ? EDI : ESI);
        const U32 offset = address & K_PAGE_MASK;
        const MMU& page = pages->mmu[address >> K_PAGE_SHIFT];
        if (store ? !page.canWriteRam : !page.canReadRam) return;
        U32 count;
        U32 value = EAX;
        if (offset > K_PAGE_SIZE - size) {
            // Validate the whole unaligned element before committing any byte.
            const MMU& last = pages->mmu[(U32)(address + size - 1) >> K_PAGE_SHIFT];
            if (store ? !last.canWriteRam : !last.canReadRam) return;
            for (U32 i = 0; i < size; ++i) {
                const U32 at = address + i;
                U8* ptr = ramPageGet((RamPage)pages->mmu[at >> K_PAGE_SHIFT].ramIndex) + (at & K_PAGE_MASK);
                if (store) *ptr = ((U8*)&value)[i];
                else ((U8*)&value)[i] = *ptr;
            }
            count = 1;
        } else {
            count = std::min(ECX, backward ? offset / size + 1 : (K_PAGE_SIZE - offset) / size);
            U8* ptr = ramPageGet((RamPage)page.ramIndex) + offset;
            if (store) {
                if (backward) ptr -= (count - 1) * size;
                if (size == 1) {
                    ::memset(ptr, value, count);
                } else {
                    const simde__m128i pattern = size == 2 ? simde_mm_set1_epi16((S16)value) : simde_mm_set1_epi32(value);
                    U32 i = 0;
                    for (; i + 64 / size <= count; i += 64 / size) {
                        simde_mm_storeu_si128((simde__m128i*)(ptr + i * size), pattern);
                        simde_mm_storeu_si128((simde__m128i*)(ptr + i * size + 16), pattern);
                        simde_mm_storeu_si128((simde__m128i*)(ptr + i * size + 32), pattern);
                        simde_mm_storeu_si128((simde__m128i*)(ptr + i * size + 48), pattern);
                    }
                    for (; i + 16 / size <= count; i += 16 / size)
                        simde_mm_storeu_si128((simde__m128i*)(ptr + i * size), pattern);
                    for (; i < count; ++i) ::memcpy(ptr + i * size, &value, size);
                }
            } else {
                // Intermediate ordinary RAM loads cannot fault or have side
                // effects. Only the last value in this page remains in AL/AX/EAX.
                const S32 last = backward ? -(S32)((count - 1) * size) : (S32)((count - 1) * size);
                ::memcpy(&value, ptr + last, size);
            }
        }
        if (store) {
            EDI += count * step;
        } else {
            ESI += count * step;
            EAX = value;
        }
        ECX -= count;
    }
}

void stosb32rRam(CPU* cpu, U32 base) { fillLoad32rRam<1, true>(cpu, base); }
void stosw32rRam(CPU* cpu, U32 base) { fillLoad32rRam<2, true>(cpu, base); }
void stosd32rRam(CPU* cpu, U32 base) { fillLoad32rRam<4, true>(cpu, base); }
void lodsb32rRam(CPU* cpu, U32 base) { fillLoad32rRam<1, false>(cpu, base); }
void lodsw32rRam(CPU* cpu, U32 base) { fillLoad32rRam<2, false>(cpu, base); }
void lodsd32rRam(CPU* cpu, U32 base) { fillLoad32rRam<4, false>(cpu, base); }

void stosb16(CPU* cpu) {
    cpu->memory->writeb(cpu->seg[ES].address+DI, AL);
    DI += cpu->getDirection();
}
void stosb16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+DI, AL);
        DI+=inc;
        CX--;
    }
}
void stosb32(CPU* cpu) {
    cpu->memory->writeb(cpu->seg[ES].address+EDI, AL);
    EDI += cpu->getDirection();
}
void stosb32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+EDI, AL);
        EDI+=inc;
        ECX--;
    }
}
void stosw16(CPU* cpu) {
    cpu->memory->writew(cpu->seg[ES].address+DI, AX);
    DI += cpu->getDirection() << 1;
}
void stosw16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+DI, AX);
        DI+=inc;
        CX--;
    }
}
void stosw32(CPU* cpu) {
    cpu->memory->writew(cpu->seg[ES].address+EDI, AX);
    EDI += cpu->getDirection() << 1;
}
void stosw32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+EDI, AX);
        EDI+=inc;
        ECX--;
    }
}
void stosd16(CPU* cpu) {
    cpu->memory->writed(cpu->seg[ES].address+DI, EAX);
    DI += cpu->getDirection() << 2;
}
void stosd16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writed(dBase+DI, EAX);
        DI+=inc;
        CX--;
    }
}
void stosd32(CPU* cpu) {
    cpu->memory->writed(cpu->seg[ES].address+EDI, EAX);
    EDI += cpu->getDirection() << 2;
}
void stosd32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writed(dBase+EDI, EAX);
        EDI+=inc;
        ECX--;
    }
}
void lodsb16(CPU* cpu, U32 base) {
    AL = cpu->memory->readb(cpu->seg[base].address+SI);
    SI += cpu->getDirection();
}
void lodsb16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        AL = cpu->memory->readb(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsb32(CPU* cpu, U32 base) {
    AL = cpu->memory->readb(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection();
}
void lodsb32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        AL = cpu->memory->readb(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsw16(CPU* cpu, U32 base) {
    AX = cpu->memory->readw(cpu->seg[base].address+SI);
    SI += cpu->getDirection() << 1;
}
void lodsw16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        AX = cpu->memory->readw(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsw32(CPU* cpu, U32 base) {
    AX = cpu->memory->readw(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection() << 1;
}
void lodsw32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        AX = cpu->memory->readw(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsd16(CPU* cpu, U32 base) {
    EAX = cpu->memory->readd(cpu->seg[base].address+SI);
    SI += cpu->getDirection() << 2;
}
void lodsd16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        EAX = cpu->memory->readd(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsd32(CPU* cpu, U32 base) {
    EAX = cpu->memory->readd(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection() << 2;
}
void lodsd32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        EAX = cpu->memory->readd(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void scasb16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase+DI);
    DI+=inc;
    cpu->dst.u8 = AL;
    cpu->src.u8 = v1;
    cpu->result.u8 = AL - v1;
    cpu->lazyFlagType = FLAGS_SUB8;
}
void scasb16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    if (count) {
        U8 v1 = 0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+DI);
            DI+=inc;
            CX--;
            if ((AL==v1)!=rep_zero) break;
        }
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlagType = FLAGS_SUB8;
    }
}
void scasb32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase+EDI);
    EDI+=inc;
    cpu->dst.u8 = AL;
    cpu->src.u8 = v1;
    cpu->result.u8 = AL - v1;
    cpu->lazyFlagType = FLAGS_SUB8;
}
void scasb32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    if (ECX > 16) comparePrefixRam<1, true>(cpu, (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U8 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((AL==v1)!=rep_zero) break;
        }
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlagType = FLAGS_SUB8;
    }
}
void scasw16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase+DI);
    DI+=inc;
    cpu->dst.u16 = AX;
    cpu->src.u16 = v1;
    cpu->result.u16 = AX - v1;
    cpu->lazyFlagType = FLAGS_SUB16;
}
void scasw16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    if (count) {
        U16 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+DI);
            DI+=inc;
            CX--;
            if ((AX==v1)!=rep_zero) break;
        }
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlagType = FLAGS_SUB16;
    }
}
void scasw32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase+EDI);
    EDI+=inc;
    cpu->dst.u16 = AX;
    cpu->src.u16 = v1;
    cpu->result.u16 = AX - v1;
    cpu->lazyFlagType = FLAGS_SUB16;
}
void scasw32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    if (ECX > 16) comparePrefixRam<2, true>(cpu, (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U16 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((AX==v1)!=rep_zero) break;
        }
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlagType = FLAGS_SUB16;
    }
}
void scasd16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase+DI);
    DI+=inc;
    cpu->dst.u32 = EAX;
    cpu->src.u32 = v1;
    cpu->result.u32 = EAX - v1;
    cpu->lazyFlagType = FLAGS_SUB32;
}
void scasd16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    if (count) {
        U32 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+DI);
            DI+=inc;
            CX--;
            if ((EAX==v1)!=rep_zero) break;
        }
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlagType = FLAGS_SUB32;
    }
}
void scasd32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase+EDI);
    EDI+=inc;
    cpu->dst.u32 = EAX;
    cpu->src.u32 = v1;
    cpu->result.u32 = EAX - v1;
    cpu->lazyFlagType = FLAGS_SUB32;
}
void scasd32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    if (ECX > 16) comparePrefixRam<4, true>(cpu, (rep_zero ? 8 : 0));
    U32 count = ECX;
    if (count) {
        U32 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((EAX==v1)!=rep_zero) break;
        }
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlagType = FLAGS_SUB32;
    }
}

// Skip comparisons that continue without changing flags. Leave the stopping
// chunk, and always at least one element, to scalar execution for exact flags.
template<U32 size, bool scan>
static void comparePrefixRam(CPU* cpu, U32 arg) {
    if (cpu->debugTrapActive) return;
    const U32 sBase = scan ? 0 : cpu->seg[arg & 7].address;
    const U32 dBase = cpu->seg[ES].address;
    const bool keepEqual = (arg & 8) != 0;
    const bool backward = cpu->getDirection() < 0;
    const U32 step = backward ? 0u - size : size;
    const U32 mask = size == 1 ? 0xff : size == 2 ? 0xffff : 0xffffffff;
    const U32 accumulator = scan ? EAX & mask : 0;
    const U64 ones = size == 1 ? 0x0101010101010101ull : size == 2 ? 0x0001000100010001ull : 0x0000000100000001ull;
    const U64 highBits = ones << (size * 8 - 1);
    const U64 pattern = accumulator * ones;
    auto* pages = getMemData(cpu->memory);
    auto element = [&](U32 address, U32& value) {
        if (!pages->mmu[address >> K_PAGE_SHIFT].canReadRam ||
                !pages->mmu[(U32)(address + size - 1) >> K_PAGE_SHIFT].canReadRam) return false;
        value = 0;
        for (U32 i = 0; i < size; ++i) {
            const U32 at = address + i;
            value |= (U32)ramPageGet((RamPage)pages->mmu[at >> K_PAGE_SHIFT].ramIndex)[at & K_PAGE_MASK] << (i * 8);
        }
        return true;
    };
    while (ECX > 1) {
        const U32 src = scan ? 0 : sBase + ESI;
        const U32 dst = dBase + EDI;
        const U32 so = src & K_PAGE_MASK, d = dst & K_PAGE_MASK;
        if ((!scan && so > K_PAGE_SIZE - size) || d > K_PAGE_SIZE - size) {
            U32 a = accumulator, b;
            if ((!scan && !element(src, a)) || !element(dst, b) || ((a == b) != keepEqual)) return;
            if (!scan) ESI += step;
            EDI += step; --ECX;
            continue;
        }
        const MMU& dest = pages->mmu[dst >> K_PAGE_SHIFT];
        if (!dest.canReadRam) return;
        U32 count = std::min(ECX - 1, backward ? d / size + 1 : (K_PAGE_SIZE - d) / size);
        U8* dp = ramPageGet((RamPage)dest.ramIndex) + d;
        U8* sp = nullptr;
        if (!scan) {
            const MMU& source = pages->mmu[src >> K_PAGE_SHIFT];
            if (!source.canReadRam) return;
            count = std::min(count, backward ? so / size + 1 : (K_PAGE_SIZE - so) / size);
            sp = ramPageGet((RamPage)source.ramIndex) + so;
        }
        // The common equality/byte-search forms can use tuned libc kernels.
        if (sizeof(void*) == 8 && !scan && keepEqual) {
            const S32 start = backward ? -(S32)((count - 1) * size) : 0;
            if (::memcmp(sp + start, dp + start, count * size) == 0) {
                ESI += count * step; EDI += count * step; ECX -= count;
                continue;
            }
        }
        if (sizeof(void*) == 8 && scan && size == 1 && !keepEqual && !backward) {
            const U8* match = (const U8*)::memchr(dp, accumulator, count);
            const U32 done = match ? (U32)(match - dp) : count;
            EDI += done; ECX -= done;
            if (match) return;
            continue;
        }
        U32 done = 0;
        bool stop = false;
        for (; done + 16 / size <= count; done += 16 / size) {
            const S32 offset = backward ? -(S32)(done * size + 16 - size) : (S32)(done * size);
            U64 a0 = pattern, a1 = pattern, b0, b1;
            if (!scan) { ::memcpy(&a0, sp + offset, 8); ::memcpy(&a1, sp + offset + 8, 8); }
            ::memcpy(&b0, dp + offset, 8); ::memcpy(&b1, dp + offset + 8, 8);
            const U64 x0 = a0 ^ b0, x1 = a1 ^ b1;
            const bool continues = keepEqual ? !(x0 | x1) :
                !((((x0 - ones) & ~x0) | ((x1 - ones) & ~x1)) & highBits);
            if (!continues) { stop = true; break; }
        }
        if (!stop) for (; done < count; ++done) {
            const S32 offset = backward ? -(S32)(done * size) : (S32)(done * size);
            U32 a = accumulator, b = 0;
            if (!scan) ::memcpy(&a, sp + offset, size);
            ::memcpy(&b, dp + offset, size);
            if ((a == b) != keepEqual) { stop = true; break; }
        }
        if (!scan) ESI += done * step;
        EDI += done * step; ECX -= done;
        if (stop) return;
    }
}

void cmpsb32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<1, false>(cpu, arg); }
void cmpsw32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<2, false>(cpu, arg); }
void cmpsd32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<4, false>(cpu, arg); }
void scasb32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<1, true>(cpu, arg); }
void scasw32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<2, true>(cpu, arg); }
void scasd32rPrefix(CPU* cpu, U32 arg) { comparePrefixRam<4, true>(cpu, arg); }
