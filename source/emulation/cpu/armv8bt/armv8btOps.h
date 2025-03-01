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

#ifndef __ARMV8BT_OPS_H__
#define __ARMV8BT_OPS_H__

#ifdef BOXEDWINE_ARMV8BT
class Armv8btAsm;

enum Conditional {
    condional_O,
    condional_NO,
    condional_B,
    condional_NB,
    condional_Z,
    condional_NZ,
    condional_BE,
    condional_NBE,
    condional_S,
    condional_NS,
    condional_P,
    condional_NP,
    condional_L,
    condional_NL,
    condional_LE,
    condional_NLE
};

typedef void(*Armv8btOp)(Armv8btAsm* data);
typedef void(*arithReg32)(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags);
typedef void(*arithValue32)(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags);

void writeResultArithReg(Armv8btAsm* data, bool usesResult, bool needsResult, bool resultNeedsZeroExtends, U32 width);
U8 setupRegForArith(Armv8btAsm* data, U8 untranslatedReg, bool usesFlagReg, U8 flagReg, U32 width);
void setupFlagsForArith(Armv8btAsm* data, Arm8BtFlags* lazyFlags, U32& flags, bool& hardwareFlags, bool& usesSrc, bool& usesDst, bool& usesResult, bool& resultNeedsZeroExtends);
void arithRI(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends, bool needRegZeroExtended=false);
void arithEI(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends);
void doCondition(Armv8btAsm* data, Conditional conditional, const std::function<void()>& f);

extern Armv8btOp armv8btEncoder[InstructionCount];

#endif

#endif
