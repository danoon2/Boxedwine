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

#ifndef __TEST_CPU_H__
#define __TEST_CPU_H__

void newInstruction(int flags);
void pushCode8(int value);
void pushCode16(int value);
void pushCode32(int value);
void runTestCPU();
void failed(const char* msg, ...);

extern CPU* cpu;

#define FLAG_MASK (AF|CF|SF|PF|ZF|OF)

#define STACK_ADDRESS 0xE0000000
#define HEAP_ADDRESS 0xF0000000
#define CODE_ADDRESS 0xD0000000
#define HEAP_SEG 0x213
#define CODE_SEG 0x223
#define STACK_SEG 0x233
#define CODE_SEG_16 0x243

#define DEFAULT 0xDEADBEEF

struct Test_Float {
    union {
        U32   i;
        float f;
    };
};

struct TestDouble {
    union {
        U64   i;
        double d;
    };
};

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)   
__m128i floatTo128(float f1, float f2, float f3, float f4);
#endif

extern const U32 FLOAT_POSITIVE_INFINITY_BITS;
extern const U32 FLOAT_NEGATIVE_INFINITY_BITS;
extern const U32 FLOAT_QUIET_NAN_BITS;
extern const U64 DOUBLE_QUIET_NAN_BITS;

extern const float POSITIVE_INFINITY;
extern const float NEGATIVE_INFINITY;
extern const float TEST_NAN;
extern const double TEST_NAN_DOUBLE;

#endif
