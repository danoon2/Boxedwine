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

U8 DynReg::hardwareReg() {
    if (reg != 0xff) {
        return reg;
    }
    reg = delayedLoading();
    return reg;
}

void DynamicData::dynamic_sidt(DecodedOp* op) {
}

static void dynamic_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void DynamicData::dynamic_callback(DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        call(dynamic_onExitSignal);
    } else {
        kpanic("DynamicData::dynamic_callback unhandled callback");
    }
}

void DynamicData::dynamic_invalid_op(DecodedOp* op) {
    //kpanic_fmt("Invalid instruction %x\n", op->inst);
    call(common_ud2);
    blockDone(true);
}

void DynamicData::dynamic_onTestEnd(DecodedOp* op) {
    onTestEnd(op);
}

void DynamicData::pushParam(std::vector<DynParam>& params, DynWidth width, RegPtr reg) {
    if (width == DYN_32bit) {
        params.push_back(DynParam(DYN_PARAM_REG_32, reg));
    } else if (width == DYN_16bit) {
        params.push_back(DynParam(DYN_PARAM_REG_16, reg));
    } else if (width == DYN_8bit) {
        params.push_back(DynParam(DYN_PARAM_REG_8, reg));
    } else {
        kpanic("DynamicData::callAndReturn");
    }
}

RegPtr DynamicData::callAndReturn_R(CallReturnR address, DynWidth width, RegPtr reg, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    pushParam(params, width, reg);
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr DynamicData::callAndReturn_RS(CallReturnRS address, DynWidth width, RegPtr reg, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    pushParam(params, width, reg);
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr DynamicData::callAndReturn_I(CallReturnI address, U32 value, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value));
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

void DynamicData::call_RR(CallRR address, DynWidth width, RegPtr reg, DynWidth width2, RegPtr reg2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    pushParam(params, width, reg);
    pushParam(params, width2, reg2);
    callHostFunction(address, params);
}

void DynamicData::call_I(CallI address, U32 value) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value));
    callHostFunction(address, params);
}

void DynamicData::call_II(CallII address, U32 value1, U32 value2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value2));
    callHostFunction(address, params);
}

void DynamicData::call_III(CallIII address, U32 value1, U32 value2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value2));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value3));
    callHostFunction(address, params);
}

void DynamicData::call_IRI(CallIRI address, U32 value1, DynWidth width2, RegPtr reg2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    pushParam(params, width2, reg2);
    params.push_back(DynParam(DYN_PARAM_CONST_32, value3));
    callHostFunction(address, params);
}

void DynamicData::call_IIR(CallIRI address, U32 value1, U32 value2, DynWidth width3, RegPtr reg3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));    
    params.push_back(DynParam(DYN_PARAM_CONST_32, value2));
    pushParam(params, width3, reg3);
    callHostFunction(address, params);
}

void DynamicData::call_IIIR(CallIIIR address, U32 value1, U32 value2, U32 value3, DynWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value2));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value3));
    pushParam(params, width, reg);
    callHostFunction(address, params);
}

void DynamicData::call_IRRR(CallIIIR address, U32 value1, DynWidth width1, RegPtr reg1, DynWidth width2, RegPtr reg2, DynWidth width3, RegPtr reg3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    pushParam(params, width1, reg1);
    pushParam(params, width2, reg2);
    pushParam(params, width3, reg3);
    callHostFunction(address, params);
}

void DynamicData::call_IR(CallIR address, U32 value, DynWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value));
    pushParam(params, width, reg);    
    callHostFunction(address, params);
}

void DynamicData::call_RI(CallRI address, DynWidth width, RegPtr reg, U32 value) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    pushParam(params, width, reg);
    params.push_back(DynParam(DYN_PARAM_CONST_32, value));
    callHostFunction(address, params);
}

RegPtr DynamicData::callAndReturn_IR(CallReturnIR address, U32 value, DynWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value));
    pushParam(params, width, reg);
    RegPtr result = getTmpRegForCallResult();
    callHostFunctionWithResult(result, address, params);
    return result;
}

RegPtr DynamicData::callAndReturn(CallReturn address) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    RegPtr result = getTmpRegForCallResult();
    callHostFunctionWithResult(result, address, params);
    return result;
}

RegPtr DynamicData::callAndReturn_II(CallReturnII address, U32 value1, U32 value2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value1));
    params.push_back(DynParam(DYN_PARAM_CONST_32, value2));
    RegPtr result = getTmpRegForCallResult();
    callHostFunctionWithResult(result, address, params);
    return result;
}

void DynamicData::call_R(CallR address, DynWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    pushParam(params, width, reg);
    callHostFunction(address, params);
}

void DynamicData::call(CallNoArgs address) {
    std::vector<DynParam> params;
    params.push_back(DynParam(DYN_PARAM_CPU));
    callHostFunction(address, params);
}

#endif