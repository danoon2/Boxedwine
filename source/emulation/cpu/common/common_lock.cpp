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

#ifdef BOXEDWINE_MULTI_THREADED

 // for unaligned cases, this won't be 100% guaranteed accurate if another aligned lock access references the same memory (hopefully that would be rare to have overlapped lock instructions of different memory size)
BOXEDWINE_MUTEX lockMutex;

/*
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = cpu->memory->readq(address);
    cpu->fillFlags();
    if (value1 == value2) {
        cpu->addZF();
        cpu->memory->writed(address, EBX);
        cpu->memory->writed(address + 4, ECX);
    } else {
        cpu->removeZF();
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
*/
void common_cmpxchg8b_lock(CPU* cpu, U32 address) {
    U64 expected = ((U64)EDX) << 32 | EAX;
    if (address & 7) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        U64 oldValue = cpu->memory->readq(address);
        cpu->fillFlags();
        if (expected == oldValue) {
            cpu->addZF();
            cpu->memory->writed(address, EBX);
            cpu->memory->writed(address + 4, ECX);
        } else {
            cpu->removeZF();
            EDX = (U32)(oldValue >> 32);
            EAX = (U32)oldValue;
        }
    } else {
        U64 value = ((U64)ECX) << 32 | EBX;

        cpu->fillFlags();

        LockData64* p = (LockData64*)cpu->memory->getRamPtr(address, 8, true);
        std::atomic_ref<U64> mem(p->data);

        if (mem.compare_exchange_strong(expected, value)) {
            cpu->addZF();
        } else {
            cpu->removeZF();
            EDX = (U32)(expected >> 32);
            EAX = (U32)expected;
        }
    }
}

/*
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->memory->writed(address, cpu->reg[srcReg].u32);
    } else {
        EAX = cpu->src.u32;
    }
*/
void common_cmpxchge32r32_lock(CPU* cpu, U32 address, U32 srcReg) {
    cpu->dst.u32 = EAX;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->src.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        if (EAX == cpu->src.u32) {
            cpu->memory->writed(address, cpu->reg[srcReg].u32);
        } else {
            EAX = cpu->src.u32;
        }
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        U32 expected = EAX;
        U32 desired = cpu->reg[srcReg].u32;

        cpu->dst.u32 = EAX;
        if (!mem.compare_exchange_strong(expected, desired)) {
            EAX = expected;
        }
        cpu->src.u32 = expected;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_CMP32;
}

/*
   cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->memory->writew(address, cpu->reg[srcReg].u16);
    } else {
        AX = cpu->src.u16;
    }
*/
void common_cmpxchge16r16_lock(CPU* cpu, U32 address, U32 srcReg) {
    cpu->dst.u16 = AX;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->src.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        if (AX == cpu->src.u16) {
            cpu->memory->writew(address, cpu->reg[srcReg].u16);
        } else {
            AX = cpu->src.u16;
        }
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        U16 expected = AX;
        U16 desired = cpu->reg[srcReg].u16;

        if (!mem.compare_exchange_strong(expected, desired)) {
            AX = expected;
        }
        cpu->src.u16 = expected;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_CMP16;
}

/*
    cpu->dst.u8 = AL;
    cpu->src.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    if (AL == cpu->src.u8) {
        cpu->memory->writeb(address, *cpu->reg8[srcReg]);
    } else {
        AL = cpu->src.u8;
    }
*/
void common_cmpxchge8r8_lock(CPU* cpu, U32 address, U32 srcReg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    U8 expected = AL;
    U8 desired = *cpu->reg8[srcReg];

    cpu->dst.u8 = AL;
    if (!mem.compare_exchange_strong(expected, desired)) {
        AL = expected;
    }
    cpu->src.u8 = expected;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}

/*
    U32 address = eaa(cpu, op);
    U32 tmp = cpu->memory->readd(address);
    cpu->memory->writed(address, cpu->reg[op->reg].u32);
    cpu->reg[op->reg].u32 = tmp;
*/
void common_xchge32r32_lock(CPU* cpu, U32 address, U32 reg) {
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        U32 tmp = cpu->memory->readd(address);
        cpu->memory->writed(address, cpu->reg[reg].u32);
        cpu->reg[reg].u32 = tmp;
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->reg[reg].u32 = mem.exchange(cpu->reg[reg].u32);
    }
}

/*
    U32 address = eaa(cpu, op);
    U16 tmp = cpu->memory->readw(address);
    cpu->memory->writew(address, cpu->reg[op->reg].u16);
    cpu->reg[op->reg].u16 = tmp;
*/
void common_xchge16r16_lock(CPU* cpu, U32 address, U32 reg) {
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        U16 tmp = cpu->memory->readw(address);
        cpu->memory->writew(address, cpu->reg[reg].u16);
        cpu->reg[reg].u16 = tmp;
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->reg[reg].u16 = mem.exchange(cpu->reg[reg].u16);
    }
}

/*
    U32 address = eaa(cpu, op);
    U8 tmp = cpu->memory->readb(address);
    cpu->memory->writeb(address, *cpu->reg8[op->reg]);
    *cpu->reg8[op->reg] = tmp;
*/
void common_xchge8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    *cpu->reg8[reg] = mem.exchange(*cpu->reg8[reg]);
}

/*
    U32 address = eaa(cpu, op);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->dst.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 = cpu->dst.u32;
    cpu->memory->writed(address, cpu->result.u32);
*/
void common_xaddr32e32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(cpu->reg[reg].u32);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    }
    cpu->reg[reg].u32 = cpu->dst.u32;
    cpu->lazyFlags = FLAGS_ADD32;
}

/*
    U32 address = eaa(cpu, op);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->dst.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 = cpu->dst.u16;
    cpu->memory->writew(address, cpu->result.u16);
*/
void common_xaddr16e16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(cpu->reg[reg].u16);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    }
    cpu->reg[reg].u16 = cpu->dst.u16;
    cpu->lazyFlags = FLAGS_ADD16;
}

/*
    U32 address = eaa(cpu, op);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->dst.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] = cpu->dst.u8;
    cpu->memory->writeb(address, cpu->result.u8);
*/
void common_xaddr8e8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_add(*cpu->reg8[reg]);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    *cpu->reg8[reg] = cpu->dst.u8;
    cpu->lazyFlags = FLAGS_ADD8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_adde32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_ADD32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_adde16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_ADD16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_adde8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_add(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_add32_mem_lock(CPU* cpu, U32 address, U32 value) {
    cpu->src.u32 = value;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_ADD32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_add16_mem_lock(CPU* cpu, U32 address, U32 value) {
    cpu->src.u16 = value;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_ADD16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_add8_mem_lock(CPU* cpu, U32 address, U32 value) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = value;
    cpu->dst.u8 = mem.fetch_add(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_sube32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_sub(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_SUB32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_sube16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_sub(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_SUB16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_sube8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_sub(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_sub32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_sub(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_SUB32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_sub16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_sub(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_SUB16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_sub8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_sub(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_ore32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_or(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_OR32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_ore16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_or(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_OR16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_ore8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_or(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/

void common_or32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_or(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_OR32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_or16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_or(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_OR16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_or8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_or(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_ande32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_and(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_AND32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_ande16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_and(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_AND16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_ande8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_and(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/

void common_and32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_and(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_AND32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_and16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_and(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_AND16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_and8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_and(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_xore32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_xor(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_XOR32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_xore16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_xor(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_XOR16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_xore8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_xor(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/

void common_xor32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_xor(cpu->src.u32);
        cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_XOR32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_xor16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_xor(cpu->src.u16);
        cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_XOR16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_xor8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_xor(cpu->src.u8);
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_adce32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(cpu->src.u32 + cpu->oldCF);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_ADC32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_adce16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(cpu->src.u16 + cpu->oldCF);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_ADC16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_adce8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_add(cpu->src.u8 + cpu->oldCF);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/

void common_adc32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(cpu->src.u32 + cpu->oldCF);
        cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_ADC32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_adc16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(cpu->src.u16 + cpu->oldCF);
        cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_ADC16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_adc8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_add(cpu->src.u8 + cpu->oldCF);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_sbbe32r32_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u32 = cpu->reg[reg].u32;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_sub(cpu->src.u32 + cpu->oldCF);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_SBB32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_sbbe16r16_lock(CPU* cpu, U32 address, U32 reg) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u16 = cpu->reg[reg].u16;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_sub(cpu->src.u16 + cpu->oldCF);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_SBB16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_sbbe8r8_lock(CPU* cpu, U32 address, U32 reg) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->src.u8 = *cpu->reg8[reg];
    cpu->dst.u8 = mem.fetch_sub(cpu->src.u8 + cpu->oldCF);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/

void common_sbb32_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u32 = imm;
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_sub(cpu->src.u32 + cpu->oldCF);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_SBB32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_sbb16_mem_lock(CPU* cpu, U32 address, U32 imm) {
    cpu->oldCF = cpu->getCF();
    cpu->src.u16 = imm;
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_sub(cpu->src.u16 + cpu->oldCF);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    }
    cpu->lazyFlags = FLAGS_SBB16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_sbb8_mem_lock(CPU* cpu, U32 address, U32 imm) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->src.u8 = imm;
    cpu->dst.u8 = mem.fetch_sub(cpu->src.u8 + cpu->oldCF);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32= cpu->memory->readd(eaa);
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    cpu->memory->writed(eaa, cpu->result.u32);
*/
void common_inc32_mem32_lock(CPU* cpu, U32 address) {
    cpu->oldCF = cpu->getCF();

    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best
        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 + 1;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_add(1);
        cpu->result.u32 = cpu->dst.u32 + 1;
    }
    cpu->lazyFlags = FLAGS_INC32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16= cpu->memory->readw(eaa);
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    cpu->memory->writew(eaa, cpu->result.u16);
*/
void common_inc16_mem16_lock(CPU* cpu, U32 address) {
    cpu->oldCF = cpu->getCF();

    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best
        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 + 1;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_add(1);
        cpu->result.u16 = cpu->dst.u16 + 1;
    }
    cpu->lazyFlags = FLAGS_INC16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8= cpu->memory->readb(eaa);
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    cpu->memory->writeb(eaa, cpu->result.u8);
*/
void common_inc8_mem8_lock(CPU* cpu, U32 address) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = mem.fetch_add(1);
    cpu->result.u8 = cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32= cpu->memory->readd(eaa);
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    cpu->memory->writed(eaa, cpu->result.u32);
*/
void common_dec32_mem32_lock(CPU* cpu, U32 address) {
    cpu->oldCF = cpu->getCF();

    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best
        cpu->dst.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - 1;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        cpu->dst.u32 = mem.fetch_sub(1);
        cpu->result.u32 = cpu->dst.u32 - 1;
    }
    cpu->lazyFlags = FLAGS_DEC32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16= cpu->memory->readw(eaa);
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    cpu->memory->writew(eaa, cpu->result.u16);
*/
void common_dec16_mem16_lock(CPU* cpu, U32 address) {
    cpu->oldCF = cpu->getCF();

    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->dst.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - 1;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        cpu->dst.u16 = mem.fetch_sub(1);
        cpu->result.u16 = cpu->dst.u16 - 1;
    }
    cpu->lazyFlags = FLAGS_DEC16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8= cpu->memory->readb(eaa);
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    cpu->memory->writeb(eaa, cpu->result.u8);
*/
void common_dec8_mem8_lock(CPU* cpu, U32 address) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = mem.fetch_sub(1);
    cpu->result.u8 = cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->memory->writed(eaa, ~cpu->memory->readd(eaa));
*/
void common_note32_lock(CPU* cpu, U32 address) {
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best
        cpu->memory->writed(address, ~cpu->memory->readd(address));
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);

        while (true) {
            U32 oldValue = cpu->memory->readd(address);

            if (mem.compare_exchange_weak(oldValue, ~oldValue)) {
                break;
            }
        }
    }
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->memory->writew(eaa, ~cpu->memory->readw(eaa));
*/
void common_note16_lock(CPU* cpu, U32 address) {
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best
        cpu->memory->writew(address, ~cpu->memory->readw(address));
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);

        while (true) {
            U16 oldValue = cpu->memory->readw(address);

            if (mem.compare_exchange_weak(oldValue, ~oldValue)) {
                break;
            }
        }
    }
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->memory->writeb(eaa, ~cpu->memory->readb(eaa));
*/
void common_note8_lock(CPU* cpu, U32 address) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);

    while (true) {
        U8 oldValue = cpu->memory->readd(address);

        if (mem.compare_exchange_weak(oldValue, ~oldValue)) {
            break;
        }
    }
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = 0;
    cpu->src.u32 = cpu->memory->readd(eaa);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    cpu->memory->writed(eaa,  cpu->result.u32);
*/
void common_nege32_lock(CPU* cpu, U32 address) {
    cpu->dst.u32 = 0;

    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->src.u32 = cpu->memory->readd(address);
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->memory->writed(address, cpu->result.u32);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);

        while (true) {
            cpu->src.u32 = cpu->memory->readd(address);

            if (mem.compare_exchange_weak(cpu->src.u32, 0 - cpu->src.u32)) {
                break;
            }
        }
        cpu->result.u16 = cpu->dst.u32 - cpu->src.u32;
    }
    cpu->lazyFlags = FLAGS_NEG32;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = 0;
    cpu->src.u16 = cpu->memory->readw(eaa);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    cpu->memory->writew(eaa,  cpu->result.u16);
*/
void common_nege16_lock(CPU* cpu, U32 address) {
    cpu->dst.u16 = 0;

    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        cpu->src.u16 = cpu->memory->readw(address);
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->lazyFlags = FLAGS_NEG16;
        cpu->memory->writew(address, cpu->result.u16);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);

        while (true) {
            cpu->src.u16 = cpu->memory->readw(address);

            if (mem.compare_exchange_weak(cpu->src.u16, -cpu->src.u16)) {
                break;
            }
        }
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    }
    cpu->lazyFlags = FLAGS_NEG16;
}

/*
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = 0;
    cpu->src.u8 = cpu->memory->readb(eaa);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
*/
void common_nege8_lock(CPU* cpu, U32 address) {
    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);

    cpu->dst.u8 = 0;
    while (true) {
        cpu->src.u8 = cpu->memory->readb(address);

        if (mem.compare_exchange_weak(cpu->src.u8, -cpu->src.u8)) {
            break;
        }
    }
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
}

/*
    U32 value;
    cpu->fillFlagsNoCF();
    value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value | mask);
*/
void common_btse32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue | mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_or(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U16 value;
    cpu->fillFlagsNoCF();
    value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value | mask);
*/
void common_btse16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue | mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_or(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[reg].u32) >> 5) * 4));
    cpu->fillFlagsNoCF();
    U32 value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value | mask);
*/
void common_btse32r32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue | mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_or(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[reg].u16) >> 4) * 2));
    cpu->fillFlagsNoCF();
    U16 value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value | mask);
*/
void common_btse16r16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue | mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_or(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U32 value;
    cpu->fillFlagsNoCF();
    value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value & ~mask);
*/
void common_btre32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue & ~mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_and(~mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    cpu->fillFlagsNoCF();
    U16 value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value & ~mask);
*/
void common_btre16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue & ~mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_and(~mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[reg].u32) >> 5) * 4));
    cpu->fillFlagsNoCF();
    U32 value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value & ~mask);
*/
void common_btre32r32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue & ~mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_and(~mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[reg].u16) >> 4) * 2));
    cpu->fillFlagsNoCF();
    U16 value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value & ~mask);
*/
void common_btre16r16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue & ~mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_and(~mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    cpu->fillFlagsNoCF();
    U32 value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value ^ mask);
*/
void common_btce32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue ^ mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_xor(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    cpu->fillFlagsNoCF();
    U16 value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value ^ mask);
*/
void common_btce16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue ^ mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_xor(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[reg].u32) >> 5) * 4));
    cpu->fillFlagsNoCF();
    U32 value = cpu->memory->readd(address);
    cpu->setCF(value & mask);
    cpu->memory->writed(address, value ^ mask);
*/
void common_btce32r32_lock(CPU* cpu, U32 address, U32 mask) {
    U32 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 3) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readd(address);
        cpu->memory->writed(address, oldValue ^ mask);
    } else {
        LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
        std::atomic_ref<U32> mem(p->data);
        oldValue = mem.fetch_xor(mask);
    }
    cpu->setCF(oldValue & mask);
}

/*
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[reg].u16) >> 4) * 2));
    cpu->fillFlagsNoCF();
    U16 value = cpu->memory->readw(address);
    cpu->setCF(value & mask);
    cpu->memory->writew(address, value ^ mask);
*/
void common_btce16r16_lock(CPU* cpu, U32 address, U16 mask) {
    U16 oldValue;

    cpu->fillFlagsNoCF();
    if (address & 1) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockMutex); // hope for the best

        oldValue = cpu->memory->readw(address);
        cpu->memory->writew(address, oldValue ^ mask);
    } else {
        LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
        std::atomic_ref<U16> mem(p->data);
        oldValue = mem.fetch_xor(mask);
    }
    cpu->setCF(oldValue & mask);
}
#endif
