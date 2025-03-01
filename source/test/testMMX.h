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

#ifndef __TEST_MMX_H__
#define __TEST_MMX_H__

void initMmxTest();
void loadMMX(U8 reg, U32 index, U64 value);

void testMmx64imm8(U8 preOp1, U8 op, U64 value1, U64 value2, U64 result, U8 imm8);
void testMmx64Eimm8(U8 preOp1, U8 op, U64 value1, U32 value2, U64 result, U8 imm8);
void testRegMmx64imm8(U8 preOp1, U8 op, U32 value1, U64 value2, U32 result, U8 imm8);
void testMmx64Reg(U8 op, U64 value1, U32 value2, U32 result);
void testMmx64(U8 op, U64 value1, U64 value2, U64 result);

#define MMX_MEM_VALUE64_DEFAULT 0x1234567890abcdefl
#define MMX_MEM_VALUE64 0xaabbccddeeff2468l
#define MMX_MEM_VALUE64_OFFSET 16
#define MMX_MEM_VALUE32 0x84726ac1
#define MMX_MEM_VALUE32_OFFSET 24
#define MMX_MEM_VALUE_TMP_OFFSET 32

void testMmxMovdToE();
void testMmxMovdToMmx();
void testMmxEmms();
void testMmxMovqToMmx();
void testMmxMovqToE();
void testMmxPaddb();
void testMmxPaddw();
void testMmxPaddd();
void testMmxPaddsb();
void testMmxPaddsw();
void testMmxPaddusb();
void testMmxPaddusw();
void testMmxPsubb();
void testMmxPsubw();
void testMmxPsubd();
void testMmxPsubsb();
void testMmxPsubsw();
void testMmxPsubusb();
void testMmxPsubusw();
void testMmxPmulhw();
void testMmxPmullw();
void testMmxPmaddwd();
void testMmxPcmpeqb();
void testMmxPcmpeqw();
void testMmxPcmpeqd();
void testMmxPcmpgtb();
void testMmxPcmpgtw();
void testMmxPcmpgtd();
void testMmxPackssdw();
void testMmxPacksswb();
void testMmxPackuswb();
void testMmxPunpckhbw();
void testMmxPunpckhdq();
void testMmxPunpckhwd();
void testMmxPunpcklbw();
void testMmxPunpckldq();
void testMmxPunpcklwd();
void testMmxPxor();
void testMmxPor();
void testMmxPand();
void testMmxPandn();
void testMmxPsllwImm8();
void testMmxPsllw();
void testMmxPslldImm8();
void testMmxPslld();
void testMmxPsllqImm8();
void testMmxPsllq();
void testMmxPsrlwImm8();
void testMmxPsrlw();
void testMmxPsrldImm8();
void testMmxPsrld();
void testMmxPsrlqImm8();
void testMmxPsrlq();
void testMmxPsrawImm8();
void testMmxPsraw();
void testMmxPsradImm8();
void testMmxPsrad();
void testMmxPaddq3d4();
void testSse2Pmuludq3f4();
void testSse2Psubq3fb();

#endif