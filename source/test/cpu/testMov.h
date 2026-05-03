/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_MOV_H__
#define __TEST_MOV_H__

void testMovR8R8_0x088();
void testMovE8R8_0x088();
void testMovR8R8_0x288();
void testMovE8R8_0x288();
void testMovR16R16_0x089();
void testMovE16R16_0x089();
void testMovR32R32_0x289();
void testMovE32R32_0x289();
void testMovR8R8_0x08a();
void testMovR8E8_0x08a();
void testMovR8R8_0x28a();
void testMovR8E8_0x28a();
void testMovR16R16_0x08b();
void testMovR16E16_0x08b();
void testMovR32R32_0x28b();
void testMovR32E32_0x28b();
void testMovE16Sreg_0x08c();
void testMovE32Sreg_0x28c();
void testMovSregE16_0x08e();
void testMovSregE32_0x28e();
void testMovAlOb_0x0a0();
void testMovAlOb_0x2a0();
void testMovAxOw_0x0a1();
void testMovEaxOd_0x2a1();
void testMovObAl_0x0a2();
void testMovObAl_0x2a2();
void testMovOwAx_0x0a3();
void testMovOdEax_0x2a3();
void testMovR8Ib_0x0b0();
void testMovR8Ib_0x2b0();
void testMovR16Iw_0x0b8();
void testMovR32Id_0x2b8();
void testMovE8Ib_0x0c6();
void testMovE8Ib_0x2c6();
void testMovE16Iw_0x0c7();
void testMovE32Id_0x2c7();
void testMovzxR16E8_0x1b6();
void testMovzxR32E8_0x3b6();
void testMovzxR32E16_0x3b7();
void testMovsxR16E8_0x1be();
void testMovsxR32E8_0x3be();
void testMovsxR32E16_0x3bf();

#endif
