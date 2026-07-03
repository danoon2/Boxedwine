/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_XCHG_H__
#define __TEST_XCHG_H__

void testXchgR8R8_0x086();
void testXchgE8R8_0x086();
void testXchgR8R8_0x286();
void testXchgE8R8_0x286();
void testXchgR16R16_0x087();
void testXchgE16R16_0x087();
void testXchgR32R32_0x287();
void testXchgE32R32_0x287();
void testXchgR16Ax_0x090();
void testXchgR32Eax_0x290();
void testXchgR16Ax_0x091();
void testXchgR32Eax_0x291();
void testXchgR16Ax_0x092();
void testXchgR32Eax_0x292();
void testXchgR16Ax_0x093();
void testXchgR32Eax_0x293();
void testXchgR16Ax_0x094();
void testXchgR32Eax_0x294();
void testXchgR16Ax_0x095();
void testXchgR32Eax_0x295();
void testXchgR16Ax_0x096();
void testXchgR32Eax_0x296();
void testXchgR16Ax_0x097();
void testXchgR32Eax_0x297();
void testCmpXchgE8R8_0x1b0();
void testCmpXchgE8R8_0x3b0();
void testCmpXchgE16R16_0x1b1();
void testCmpXchgE32R32_0x3b1();
void testCmpXchg8b_0x3c7();
void testJitCmpXchgBranchAfterEmulatedOp();
void testJitCmpXchgEmulateSync();
void testJitCmpXchgPrefixSkippedEntry();
#ifdef BOXEDWINE_MULTI_THREADED
void testLockedCmpXchgAgainstPlainStore();
void testImplicitLockedXchgAgainstPlainStore();
void testLockedCmpXchgFailureAgainstPlainStore();
void testLockedXaddAgainstPlainStore();
void testPlainReadAgainstLockedWrite();
void testLockedAdditionalFamilies();
void testLockedCmpXchg8bAgainstPlainStore();
void testLockedPageBoundaryNoPartialWrite();
void testImplicitLockedXchgSelfModifyingCode();
void testPlainMemoryOrdering();
void testLockedWidthsAndAlignmentAgainstPlainStore();
void testLockedMemoryOrdering();
#endif

#endif
