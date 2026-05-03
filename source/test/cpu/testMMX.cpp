/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "testMMX.h"
#include "testCPU.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 MEM_BASE = 0x10000;
constexpr U32 MEM_SRC = MEM_BASE + 0x100;
constexpr U32 MEM_DST = MEM_BASE + 0x180;
constexpr U64 MMX_DEFAULT = 0x1234567890abcdefULL;
constexpr U64 MMX_SRC = 0xaabbccddeeff2468ULL;
constexpr U32 GPR_SRC = 0x84726ac1;
constexpr U64 MEM_GUARD = 0x1122334455667788ULL;

enum RegId {
    R_AX,
    R_CX,
    R_DX,
    R_BX,
    R_SP,
    R_BP,
    R_SI,
    R_DI
};

void emitDirectAddressModRM(int regField, U32 address) {
    pushCode8(0x04 | (regField << 3));
    pushCode8(0x25);
    pushCode32(address);
}

void initMmx() {
    newInstruction(0);
    cpu->big = 1;
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        cpu->fpu.getMMX(i)->q = MMX_DEFAULT;
    }
    memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, MMX_SRC);
    memory->writeq(TEST_HEAP_ADDRESS + MEM_DST, MEM_GUARD);
}

void verifyOnlyMmxChanged(int changed, U64 expected, const char* name) {
    for (int i = 0; i < 8; ++i) {
        U64 value = cpu->fpu.getMMX(i)->q;
        if (i == changed) {
            if (value != expected) {
                failed("%s mmx value", name);
            }
        } else if (value != MMX_DEFAULT) {
            failed("%s mmx unchanged", name);
        }
    }
}

void verifyMmxMove(int dst, U64 dstExpected, int src, U64 srcExpected, const char* name) {
    for (int i = 0; i < 8; ++i) {
        U64 expected = MMX_DEFAULT;
        if (i == dst) {
            expected = dstExpected;
        } else if (i == src) {
            expected = srcExpected;
        }
        if (cpu->fpu.getMMX(i)->q != expected) {
            failed("%s mmx move", name);
        }
    }
}

void emitMovdMmxFromReg(int mmx, int reg) {
    pushCode8(0x0f);
    pushCode8(0x6e);
    pushCode8(0xc0 | (mmx << 3) | reg);
}

void emitMovdRegFromMmx(int reg, int mmx) {
    pushCode8(0x0f);
    pushCode8(0x7e);
    pushCode8(0xc0 | (mmx << 3) | reg);
}

void emitMovdMmxFromMem(int mmx, U32 address) {
    pushCode8(0x0f);
    pushCode8(0x6e);
    emitDirectAddressModRM(mmx, address);
}

void emitMovdMemFromMmx(int mmx, U32 address) {
    pushCode8(0x0f);
    pushCode8(0x7e);
    emitDirectAddressModRM(mmx, address);
}

void emitMovqMmxFromMmx(int dst, int src) {
    pushCode8(0x0f);
    pushCode8(0x6f);
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitMovqMmxFromMmxStoreForm(int dst, int src) {
    pushCode8(0x0f);
    pushCode8(0x7f);
    pushCode8(0xc0 | (src << 3) | dst);
}

void emitMovqMmxFromMem(int dst, U32 address) {
    pushCode8(0x0f);
    pushCode8(0x6f);
    emitDirectAddressModRM(dst, address);
}

void emitMovqMemFromMmx(int src, U32 address) {
    pushCode8(0x0f);
    pushCode8(0x7f);
    emitDirectAddressModRM(src, address);
}

void emitMmxRegReg(U8 opcode, int dst, int src) {
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitMmxRegMem(U8 opcode, int dst, U32 address) {
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(dst, address);
}

void emitMmxMemReg(U8 opcode, int src, U32 address) {
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(src, address);
}

void emitMmxRegRegRmDst(U8 opcode, int dstRm, int srcReg) {
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (srcReg << 3) | dstRm);
}

void emitStoreReg32ToMem(int reg, U32 address) {
    pushCode8(0x89);
    emitDirectAddressModRM(reg, address);
}

void emitRestoreEsp() {
    pushCode8(0xbc);
    pushCode32(4096);
}

U64 paddBytes(U64 a, U64 b) {
    U64 result = 0;
    for (int i = 0; i < 8; ++i) {
        U64 value = ((a >> (i * 8)) + (b >> (i * 8))) & 0xff;
        result |= value << (i * 8);
    }
    return result;
}

U64 paddWords(U64 a, U64 b) {
    U64 result = 0;
    for (int i = 0; i < 4; ++i) {
        U64 value = ((a >> (i * 16)) + (b >> (i * 16))) & 0xffff;
        result |= value << (i * 16);
    }
    return result;
}

U64 paddDwords(U64 a, U64 b) {
    U64 low = ((a & 0xffffffffULL) + (b & 0xffffffffULL)) & 0xffffffffULL;
    U64 high = (((a >> 32) + (b >> 32)) & 0xffffffffULL) << 32;
    return low | high;
}

struct MmxBinaryCase {
    U8 opcode;
    U64 dst;
    U64 src;
    U64 expected;
    const char* name;
};

struct MmxImmCase {
    U8 opcode;
    U8 group;
    U64 dst;
    U8 imm;
    U64 expected;
    const char* name;
};

void emitMmxImm(U8 opcode, U8 group, int dst, U8 imm) {
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (group << 3) | dst);
    pushCode8(imm);
}

void runMmxBinaryCase(const MmxBinaryCase& c) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initMmx();
            cpu->fpu.getMMX(dst)->q = c.dst;
            cpu->fpu.getMMX(src)->q = c.src;
            emitMmxRegReg(c.opcode, dst, src);
            runTestCPU();
            verifyMmxMove(dst, c.expected, src, c.src, c.name);
        }

        initMmx();
        cpu->fpu.getMMX(dst)->q = c.dst;
        memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, c.src);
        emitMmxRegMem(c.opcode, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyMmxChanged(dst, c.expected, c.name);
    }
}

void runMmxBinaryCases(const MmxBinaryCase* cases, int count) {
    for (int i = 0; i < count; ++i) {
        runMmxBinaryCase(cases[i]);
    }
}

void runMmxImmCase(const MmxImmCase& c) {
    for (int dst = 0; dst < 8; ++dst) {
        initMmx();
        cpu->fpu.getMMX(dst)->q = c.dst;
        emitMmxImm(c.opcode, c.group, dst, c.imm);
        runTestCPU();
        verifyOnlyMmxChanged(dst, c.expected, c.name);
    }
}

void runMmxImmCases(const MmxImmCase* cases, int count) {
    for (int i = 0; i < count; ++i) {
        runMmxImmCase(cases[i]);
    }
}

void runPmovmskbCase(U64 srcValue, U32 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initMmx();
            cpu->fpu.getMMX(src)->q = srcValue;
            emitMmxRegReg(0xd7, dst, src);
            if (dst == R_SP) {
                emitStoreReg32ToMem(R_SP, MEM_DST);
                emitRestoreEsp();
            }
            runTestCPU();
            U32 actual = dst == R_SP ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
            if (actual != expected) {
                failed("%s reg", name);
            }
        }
    }
}

void runMovntqCase() {
    for (int src = 0; src < 8; ++src) {
        initMmx();
        cpu->fpu.getMMX(src)->q = MMX_SRC;
        memory->writeq(TEST_HEAP_ADDRESS + MEM_DST, MEM_GUARD);
        emitMmxMemReg(0xe7, src, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != MMX_SRC) {
            failed("mmx movntq");
        }
    }
}

void runMaskmovqCase(U64 data, U64 mask, U64 expected, const char* name) {
    for (int dsMode = 0; dsMode < 2; ++dsMode) {
        for (int dataReg = 0; dataReg < 8; ++dataReg) {
            for (int maskReg = 0; maskReg < 8; ++maskReg) {
                if (dataReg == maskReg) {
                    continue;
                }
                initMmx();
                cpu->fpu.getMMX(dataReg)->q = data;
                cpu->fpu.getMMX(maskReg)->q = mask;
                U32 address = TEST_HEAP_ADDRESS + MEM_DST;
                if (dsMode) {
                    cpu->seg[DS].address = TEST_HEAP_ADDRESS;
                    cpu->seg[DS].value = TEST_HEAP_SEG;
                    cpu->thread->process->hasSetSeg[DS] = true;
                    cpu->reg[R_DI].u32 = MEM_DST;
                } else {
                    cpu->seg[DS].address = 0;
                    cpu->seg[DS].value = 0;
                    cpu->thread->process->hasSetSeg[DS] = false;
                    cpu->reg[R_DI].u32 = address;
                }
                memory->writeq(address, 0x9999999999999999ULL);
                memory->writeq(address + 8, 0x9999999999999999ULL);
                emitMmxRegRegRmDst(0xf7, maskReg, dataReg);
                runTestCPU();
                if (memory->readq(address) != expected ||
                        memory->readq(address + 8) != 0x9999999999999999ULL) {
                    failed("%s memory", name);
                }
            }
        }
    }
}

} // namespace

void testMmxMovd_0x36e_0x37e() {
    for (int mmx = 0; mmx < 8; ++mmx) {
        for (int reg = 0; reg < 8; ++reg) {
            initMmx();
            cpu->reg[reg].u32 = GPR_SRC;
            emitMovdMmxFromReg(mmx, reg);
            runTestCPU();
            verifyOnlyMmxChanged(mmx, GPR_SRC, "mmx movd from reg");

            initMmx();
            cpu->fpu.getMMX(mmx)->q = MMX_SRC;
            cpu->reg[reg].u32 = 0;
            emitMovdRegFromMmx(reg, mmx);
            runTestCPU();
            if (cpu->reg[reg].u32 != (U32)MMX_SRC) {
                failed("mmx movd to reg");
            }
        }

        initMmx();
        emitMovdMmxFromMem(mmx, MEM_SRC);
        runTestCPU();
        verifyOnlyMmxChanged(mmx, (U32)MMX_SRC, "mmx movd from mem");

        initMmx();
        cpu->fpu.getMMX(mmx)->q = MMX_SRC;
        emitMovdMemFromMmx(mmx, MEM_DST);
        runTestCPU();
        if (memory->readd(TEST_HEAP_ADDRESS + MEM_DST) != (U32)MMX_SRC ||
                memory->readd(TEST_HEAP_ADDRESS + MEM_DST + 4) != (U32)(MEM_GUARD >> 32)) {
            failed("mmx movd to mem");
        }
    }
}

void testMmxMovq_0x36f_0x37f() {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initMmx();
            cpu->fpu.getMMX(src)->q = MMX_SRC;
            emitMovqMmxFromMmx(dst, src);
            runTestCPU();
            verifyMmxMove(dst, MMX_SRC, src, MMX_SRC, "mmx movq reg");

            initMmx();
            cpu->fpu.getMMX(src)->q = MMX_SRC;
            emitMovqMmxFromMmxStoreForm(dst, src);
            runTestCPU();
            verifyMmxMove(dst, MMX_SRC, src, MMX_SRC, "mmx movq reg store form");
        }

        initMmx();
        emitMovqMmxFromMem(dst, MEM_SRC);
        runTestCPU();
        verifyOnlyMmxChanged(dst, MMX_SRC, "mmx movq from mem");

        initMmx();
        cpu->fpu.getMMX(dst)->q = MMX_SRC;
        emitMovqMemFromMmx(dst, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != MMX_SRC) {
            failed("mmx movq to mem");
        }
    }
}

void testMmxPadd_0x3fc_0x3fd_0x3fe() {
    const MmxBinaryCase cases[] = {
        {0xfc, MMX_SRC, MMX_DEFAULT, paddBytes(MMX_SRC, MMX_DEFAULT), "mmx paddb default"},
        {0xfc, 0x33445566778899aaULL, 0x5566778899aaddccULL, 0x88aaccee10327676ULL, "mmx paddb carry wrap"},
        {0xfd, MMX_SRC, MMX_DEFAULT, paddWords(MMX_SRC, MMX_DEFAULT), "mmx paddw default"},
        {0xfd, 0x33445566778899aaULL, 0x5566778899aaddccULL, 0x88aaccee11327776ULL, "mmx paddw carry wrap"},
        {0xfe, MMX_SRC, MMX_DEFAULT, paddDwords(MMX_SRC, MMX_DEFAULT), "mmx paddd default"},
        {0xfe, 0x33445566778899aaULL, 0x5566778899aaddccULL, 0x88aaccee11337776ULL, "mmx paddd carry wrap"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxLogic_0x3db_0x3df_0x3eb_0x3ef() {
    const MmxBinaryCase cases[] = {
        {0xdb, MMX_SRC, MMX_DEFAULT, MMX_SRC & MMX_DEFAULT, "mmx pand default"},
        {0xdb, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x1100220044446688ULL, "mmx pand mask"},
        {0xdf, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x880088bb88998877ULL, "mmx pandn mask"},
        {0xeb, MMX_SRC, MMX_DEFAULT, MMX_SRC | MMX_DEFAULT, "mmx por default"},
        {0xeb, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x9922bbffddffffffULL, "mmx por mask"},
        {0xef, MMX_SRC, MMX_DEFAULT, MMX_SRC ^ MMX_DEFAULT, "mmx pxor default"},
        {0xef, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x882299ff99bb9977ULL, "mmx pxor mask"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxUnpackPackCompare_0x360_0x361_0x362_0x363_0x364_0x365_0x366_0x367_0x368_0x369_0x36a_0x36b_0x374_0x375_0x376() {
    const MmxBinaryCase cases[] = {
        {0x60, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0xcc55dd66ee77ff88ULL, "mmx punpcklbw"},
        {0x61, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0xccdd5566eeff7788ULL, "mmx punpcklwd"},
        {0x62, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0xccddeeff55667788ULL, "mmx punpckldq"},
        {0x68, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x99110022aa33bb44ULL, "mmx punpckhbw"},
        {0x69, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x99001122aabb3344ULL, "mmx punpckhwd"},
        {0x6a, 0x1122334455667788ULL, 0x9900aabbccddeeffULL, 0x9900aabb11223344ULL, "mmx punpckhdq"},
        {0x63, 0x000300300300fffeULL, 0x00eb00000fff0034ULL, 0x7f007f3403307ffeULL, "mmx packsswb saturation"},
        {0x67, 0x000300300300fffeULL, 0x00eb00000fff0034ULL, 0xeb00ff340330ff00ULL, "mmx packuswb saturation"},
        {0x6b, 0x00000030ffee1234ULL, 0x00030000fffff234ULL, 0x7ffff23400308000ULL, "mmx packssdw saturation"},
        {0x64, 0x030100feff011234ULL, 0x03000001fffe1234ULL, 0x00ff000000ff0000ULL, "mmx pcmpgtb signed edges"},
        {0x65, 0x0301fffefffe1334ULL, 0x03000001fffe1234ULL, 0xffff00000000ffffULL, "mmx pcmpgtw signed edges"},
        {0x66, 0x03010001ff011234ULL, 0x03000ffefffe1234ULL, 0xffffffff00000000ULL, "mmx pcmpgtd signed edges"},
        {0x74, 0x030000feff011234ULL, 0x03010001fffe1234ULL, 0xff00ff00ff00ffffULL, "mmx pcmpeqb mixed equality"},
        {0x75, 0x030000feff011234ULL, 0x03010001fffe1234ULL, 0x000000000000ffffULL, "mmx pcmpeqw mixed equality"},
        {0x76, 0x030000feff011234ULL, 0x03010001ff011234ULL, 0x00000000ffffffffULL, "mmx pcmpeqd mixed equality"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxShiftImm_0x371_0x372_0x373() {
    const MmxImmCase cases[] = {
        {0x71, 2, 0x0102f007f2345678ULL, 1, 0x00817803791a2b3cULL, "mmx psrlw imm 1"},
        {0x71, 2, 0x0102f007f2345678ULL, 8, 0x000100f000f20056ULL, "mmx psrlw imm byte"},
        {0x71, 4, 0xf102f007f2345678ULL, 1, 0xf881f803f91a2b3cULL, "mmx psraw imm 1"},
        {0x71, 4, 0xf102f007f2345678ULL, 8, 0xfff1fff0fff20056ULL, "mmx psraw imm byte"},
        {0x71, 6, 0x0102f00712345678ULL, 1, 0x0204e00e2468acf0ULL, "mmx psllw imm 1"},
        {0x71, 6, 0x0102f00712345678ULL, 8, 0x0200070034007800ULL, "mmx psllw imm byte"},
        {0x72, 2, 0x0102f007f2345678ULL, 1, 0x00817803791a2b3cULL, "mmx psrld imm 1"},
        {0x72, 2, 0x0102f007f2345678ULL, 16, 0x000001020000f234ULL, "mmx psrld imm word"},
        {0x72, 4, 0xf102f00772345678ULL, 1, 0xf8817803391a2b3cULL, "mmx psrad imm 1"},
        {0x72, 4, 0xf102f00772345678ULL, 16, 0xfffff10200007234ULL, "mmx psrad imm word"},
        {0x72, 6, 0x0102f007f2345678ULL, 1, 0x0205e00ee468acf0ULL, "mmx pslld imm 1"},
        {0x72, 6, 0x0102f00712345678ULL, 16, 0xf007000056780000ULL, "mmx pslld imm word"},
        {0x73, 2, 0xf102f007f2345678ULL, 1, 0x78817803f91a2b3cULL, "mmx psrlq imm 1"},
        {0x73, 2, 0xf102f007f2345678ULL, 32, 0x00000000f102f007ULL, "mmx psrlq imm dword"},
        {0x73, 6, 0x0102f00712345678ULL, 1, 0x0205e00e2468acf0ULL, "mmx psllq imm 1"},
        {0x73, 6, 0x0102f007f2345678ULL, 32, 0xf234567800000000ULL, "mmx psllq imm dword"},
    };
    runMmxImmCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxShiftReg_0x3d1_0x3d2_0x3d3_0x3e1_0x3e2_0x3f1_0x3f2_0x3f3() {
    const MmxBinaryCase cases[] = {
        {0xd1, 0x0102f007f2345678ULL, 1, 0x00817803791a2b3cULL, "mmx psrlw reg 1"},
        {0xd1, 0x0102f007f2345678ULL, 8, 0x000100f000f20056ULL, "mmx psrlw reg byte"},
        {0xd2, 0x0102f007f2345678ULL, 1, 0x00817803791a2b3cULL, "mmx psrld reg 1"},
        {0xd2, 0x0102f007f2345678ULL, 16, 0x000001020000f234ULL, "mmx psrld reg word"},
        {0xd3, 0xf102f007f2345678ULL, 1, 0x78817803f91a2b3cULL, "mmx psrlq reg 1"},
        {0xd3, 0xf102f007f2345678ULL, 32, 0x00000000f102f007ULL, "mmx psrlq reg dword"},
        {0xe1, 0xf102f007f2345678ULL, 1, 0xf881f803f91a2b3cULL, "mmx psraw reg 1"},
        {0xe1, 0xf102f007f2345678ULL, 8, 0xfff1fff0fff20056ULL, "mmx psraw reg byte"},
        {0xe2, 0xf102f00772345678ULL, 1, 0xf8817803391a2b3cULL, "mmx psrad reg 1"},
        {0xe2, 0xf102f00772345678ULL, 16, 0xfffff10200007234ULL, "mmx psrad reg word"},
        {0xf1, 0x0102f00712345678ULL, 1, 0x0204e00e2468acf0ULL, "mmx psllw reg 1"},
        {0xf1, 0x0102f00712345678ULL, 8, 0x0200070034007800ULL, "mmx psllw reg byte"},
        {0xf2, 0x0102f007f2345678ULL, 1, 0x0205e00ee468acf0ULL, "mmx pslld reg 1"},
        {0xf2, 0x0102f00712345678ULL, 16, 0xf007000056780000ULL, "mmx pslld reg word"},
        {0xf3, 0x0102f007f2345678ULL, 1, 0x0205e00fe468acf0ULL, "mmx psllq reg 1"},
        {0xf3, 0x0102f007f2345678ULL, 32, 0xf234567800000000ULL, "mmx psllq reg dword"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxSaturatingArithmetic_0x3d8_0x3d9_0x3dc_0x3dd_0x3e8_0x3e9_0x3ec_0x3ed() {
    const MmxBinaryCase cases[] = {
        {0xd8, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x2200330000000000ULL, "mmx psubusb saturation"},
        {0xd9, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x21bc330000000000ULL, "mmx psubusw saturation"},
        {0xdc, 0x33445566778899aaULL, 0x5566778899aa11ccULL, 0x88aacceeffffaaffULL, "mmx paddusb saturation"},
        {0xdd, 0x33445566778899aaULL, 0x5566778899aa11ccULL, 0x88aacceeffffab76ULL, "mmx paddusw saturation"},
        {0xe8, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x227f33007fddccbbULL, "mmx psubsb saturation"},
        {0xe9, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x21bc33007fffcbbbULL, "mmx psubsw saturation"},
        {0xec, 0x33445566778899aaULL, 0x5566778899aa11ccULL, 0x7f7f7fee1080aa80ULL, "mmx paddsb saturation"},
        {0xed, 0x33445566778899aaULL, 0x5566778899aa11ccULL, 0x7fff7fff1132ab76ULL, "mmx paddsw saturation"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxMultiplySubtract_0x3d4_0x3d5_0x3e5_0x3f5_0x3f8_0x3f9_0x3fa() {
    const MmxBinaryCase cases[] = {
        {0xd4, 0xf2345678f0abcdefULL, 0x5555555566666666ULL, 0x4789abce57123455ULL, "mmx paddq wrap"},
        {0xd5, 0x03e8fc1800007fffULL, 0x03e803e855557fffULL, 0x4240bdc000000001ULL, "mmx pmullw signed lanes"},
        {0xe5, 0x03e8fc1800007fffULL, 0x03e803e855557fffULL, 0x000ffff000003fffULL, "mmx pmulhw signed lanes"},
        {0xf5, 0x03e9fc1800007fffULL, 0x03e803e855557fffULL, 0x000003e83fff0001ULL, "mmx pmaddwd signed pairs"},
        {0xf8, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x22bc3300deddccbbULL, "mmx psubb wrap"},
        {0xf9, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x21bc3300ddddcbbbULL, "mmx psubw wrap"},
        {0xfa, 0x33445566778899aaULL, 0x1188226699abcdefULL, 0x21bc3300dddccbbbULL, "mmx psubd wrap"},
    };
    runMmxBinaryCases(cases, sizeof(cases) / sizeof(cases[0]));
}

void testMmxPmovmskb_0x3d7() {
    runPmovmskbCase(0x1122804455667788ULL, 0x21, "mmx pmovmskb");
}

void testMmxPminub_0x3da() {
    const MmxBinaryCase c[] = {{0xda, 0x1122804455667788ULL, 0x2211704377ff3211ULL, 0x1111704355663211ULL, "mmx pminub"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPmaxub_0x3de() {
    const MmxBinaryCase c[] = {{0xde, 0x1122804455667788ULL, 0x2211704377ff3211ULL, 0x2222804477ff7788ULL, "mmx pmaxub"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPavgb_0x3e0() {
    const MmxBinaryCase c[] = {{0xe0, 0x1122804455667788ULL, 0x2312704277fe3112ULL, 0x1a1a784366b2544dULL, "mmx pavgb"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPavgw_0x3e3() {
    const MmxBinaryCase c[] = {{0xe3, 0x1122804455667788ULL, 0x2212704277fe3112ULL, 0x199a784366b2544dULL, "mmx pavgw"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPmulhuw_0x3e4() {
    const MmxBinaryCase c[] = {{0xe4, 0x1122804455660001ULL, 0x2212ffff77fe3112ULL, 0x0247804328070000ULL, "mmx pmulhuw"}};
    runMmxBinaryCases(c, 1);
}

void testMmxMovntq_0x3e7() {
    runMovntqCase();
}

void testMmxPminsw_0x3ea() {
    const MmxBinaryCase c[] = {{0xea, 0x11112345ffff8000ULL, 0x11121234fffe7fffULL, 0x11111234fffe8000ULL, "mmx pminsw"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPmaxsw_0x3ee() {
    const MmxBinaryCase c[] = {{0xee, 0x11112345ffff8000ULL, 0x11121234fffe7fffULL, 0x11122345ffff7fffULL, "mmx pmaxsw"}};
    runMmxBinaryCases(c, 1);
}

void testMmxPsadbw_0x3f6() {
    const MmxBinaryCase c[] = {{0xf6, 0x12112345ffff8000ULL, 0x11121234fffe7fffULL, 0x0000000000000125ULL, "mmx psadbw"}};
    runMmxBinaryCases(c, 1);
}

void testMmxMaskmovq_0x3f7() {
    runMaskmovqCase(0x1122334455667788ULL, 0x807fff00808001f0ULL, 0x1199339955669988ULL, "mmx maskmovq");
}

#endif
