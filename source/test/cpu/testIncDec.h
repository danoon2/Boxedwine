/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_INC_DEC_H__
#define __TEST_INC_DEC_H__

void testIncR16_0x040();
void testIncR32_0x240();
void testDecR16_0x048();
void testDecR32_0x248();
void testIncR8_0x0fe();
void testIncE8_0x0fe();
void testDecR8_0x0fe();
void testDecE8_0x0fe();
void testIncE16_0x0ff();
void testIncE32_0x2ff();
void testDecE16_0x0ff();
void testDecE32_0x2ff();
#ifdef BOXEDWINE_MULTI_THREADED
void testLockedInc();
#endif

#endif
