/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_CALLRET_H__
#define __TEST_CALLRET_H__

void testRetn16Iw_0x0c2();
void testRetn32Iw_0x2c2();
void testRetn16_0x0c3();
void testRetn32_0x2c3();
void testCallJw_0x0e8();
void testCallJd_0x2e8();
void testCallFar16_0x09a();
void testCallFar32_0x29a();
void testCallR16_0x0ff();
void testCallE16_0x0ff();
void testCallFarE16_0x0ff();
void testCallR32_0x2ff();
void testCallE32_0x2ff();
void testCallFarE32_0x2ff();

#endif
