/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_LOADSEG_H__
#define __TEST_LOADSEG_H__

void testLesR16M16_0x0c4();
void testLesR32M32_0x2c4();
void testLdsR16M16_0x0c5();
void testLdsR32M32_0x2c5();
void testLssR16M16_0x1b2();
void testLssR32M32_0x3b2();
void testLfsR16M16_0x1b4();
void testLfsR32M32_0x3b4();
void testLgsR16M16_0x1b5();
void testLgsR32M32_0x3b5();

#endif
