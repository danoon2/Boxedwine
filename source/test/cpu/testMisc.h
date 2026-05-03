/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_MISC_H__
#define __TEST_MISC_H__

void testBoundR16M16_0x062();
void testBoundR32M32_0x262();
void testOpSizePrefix_0x066();
void testOpSizePrefix_0x266();
void testAddressPrefix_0x067();
void testAddressPrefix_0x267();
void testMemoryAccess16();
void testMemoryAccess32();
void testLeaR16M_0x08d();
void testLeaR32M_0x28d();
void testCbw_0x098();
void testCwde_0x298();
void testCwd_0x099();
void testCdq_0x299();
void testSahf_0x09e();
void testSahf_0x29e();
void testLahf_0x09f();
void testLahf_0x29f();
void testEnter16_0x0c8();
void testEnter32_0x2c8();
void testLeave16_0x0c9();
void testLeave32_0x2c9();
void testSalc_0x0d6();
void testSalc_0x2d6();
void testXlat_0x0d7();
void testXlat_0x2d7();
void testCmc_0x0f5();
void testCmc_0x2f5();
void testClc_0x0f8();
void testClc_0x2f8();
void testStc_0x0f9();
void testStc_0x2f9();
void testBswap_0x3c8_0x3cf();

#endif
