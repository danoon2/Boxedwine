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

#include "boxedwine.h"

#ifdef BOXEDWINE_DYNAMIC
#include "dynamicData.h"

#include "../normal/instructions.h"
#include "../common/common_arith.h"
#include "../common/common_pushpop.h"
#include "../dynamic/dynamic_func.h"
#include "../dynamic/dynamic_arith.h"
#include "../dynamic/dynamic_mov.h"
#include "../dynamic/dynamic_incdec.h"
#include "../dynamic/dynamic_jump.h"
#include "../dynamic/dynamic_pushpop.h"
#include "../dynamic/dynamic_strings.h"
#include "../dynamic/dynamic_shift.h"
#include "../dynamic/dynamic_conditions.h"
#include "../dynamic/dynamic_setcc.h"
#include "../dynamic/dynamic_xchg.h"
#include "../dynamic/dynamic_bit.h"
#include "../dynamic/dynamic_other.h"
#include "../dynamic/dynamic_mmx.h"
#include "../dynamic/dynamic_sse.h"
#include "../dynamic/dynamic_sse2.h"
#include "../dynamic/dynamic_fpu.h"
#include "../dynamic/dynamic_lock.h"

void DynamicData::dynamic_sidt(DecodedOp* op) {
}

static void dynamic_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void DynamicData::dynamic_callback(DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction(dynamic_onExitSignal, false, 1, 0, DYN_PARAM_CPU);
    } else {
        kpanic("x32CPU::x32_callback unhandled callback");
    }
}

void DynamicData::dynamic_invalid_op(DecodedOp* op) {
    //kpanic_fmt("Invalid instruction %x\n", op->inst);
    callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(true);
}

void DynamicData::dynamic_onTestEnd(DecodedOp* op) {
    onTestEnd(op);
}

#endif