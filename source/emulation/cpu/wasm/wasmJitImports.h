/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifndef __WASM_JIT_IMPORTS_H__
#define __WASM_JIT_IMPORTS_H__

// Import indices shared by the module builder and instruction lowering.
// Keep these values in sync with g_wasmHelperTable in jitWasmCodeGen.cpp.
enum WasmHelperIdx {
    HELPER_READ_MEM8         = 0,
    HELPER_WRITE_MEM8        = 1,
    HELPER_READ_MEM16        = 2,
    HELPER_WRITE_MEM16       = 3,
    HELPER_READ_MEM32        = 4,
    HELPER_WRITE_MEM32       = 5,
    HELPER_FETCH_NEXT        = 6,
    HELPER_SYNC_FLAGS        = 7,
    HELPER_EMULATE_SINGLE_OP = 8,
    HELPER_COMPUTE_CF        = 9,
    HELPER_COMPUTE_ZF        = 10,
    HELPER_FILL_FLAGS        = 11,
    HELPER_COND_BASE         = 12, // add JitConditional index to this
    HELPER_SPECIAL_OP        = 28, // legacy block-entry helper slot
    HELPER_WRITE_MEM8_CHECK  = 29,
    HELPER_WRITE_MEM16_CHECK = 30,
    HELPER_WRITE_MEM32_CHECK = 31,
    HELPER_CACHE_FLOAT      = 32,
    HELPER_MOVSD_XMM_E64    = 33,
    HELPER_MOVSD_E64_XMM    = 34,
    HELPER_MOVSD32R           = 35,
    HELPER_READ_WRITE_MEM8    = 36,
    HELPER_READ_WRITE_MEM16   = 37,
    HELPER_READ_WRITE_MEM32   = 38,
    HELPER_STORE_EXTENDED_I64 = 39,
    HELPER_TRANSFER_FPU80 = 40,
    HELPER_FPU_SIN = 41,
    HELPER_FPU_COS = 42,
    HELPER_FPU_TAN = 43,
    HELPER_FPU_LOG = 44,
    HELPER_FPU_POW = 45,
    HELPER_FPU_ATAN2 = 46,
    HELPER_FPU_SLOT_ARITHMETIC = 47,
    HELPER_PROFILE_BLOCK_EXIT = 48,
    HELPER_PROFILE_EXIT_NEXT1 = 49,
    HELPER_PROFILE_EXIT_NEXT2 = 50,
    HELPER_PROFILE_EXIT_JUMP = 51,
    HELPER_PROFILE_EXIT_GENERIC = 52,
    HELPER_PROFILE_INLINE_COND = 53,
    HELPER_PROFILE_RMW = 54,
};

#endif
