/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_ADD_H__
#define __TEST_ADD_H__

void testAddR8R8_0x000();
void testAddE8R8_0x000();
void testAddR16R16_0x001();
void testAddE16R16_0x001();
void testAddR32R32_0x001();
void testAddE32R32_0x001();
void testAddR8R8_0x002();
void testAddR8E8_0x002();
void testAddR16R16_0x003();
void testAddR16E16_0x003();
void testAddR32R32_0x003();
void testAddR32E32_0x003();
void testAddAlIb_0x004();
void testAddAxIw_0x005();
void testAddEaxId_0x005();
void testAddE8Ib_0x080();
void testAddE8Ib_0x280();
void testAddE16Iw_0x081();
void testAddE32Id_0x281();
void testAddE8Ib_0x082();
void testAddE8Ib_0x282();
void testAddE16Ib_0x083();
void testAddE32Ib_0x283();
void testXaddE8R8_0x1c0();
void testXaddE8R8_0x3c0();
void testXaddE16R16_0x1c1();
void testXaddE32R32_0x3c1();

#endif
