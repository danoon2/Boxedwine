/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "../cpu/testCPU.h"

void testMlockValidatesMappedAccessibleRange() {
    constexpr U32 MAP_ADDRESS = 0xc1000000;
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    KThread* thread = context.thread;

    if (memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != MAP_ADDRESS) {
        testFail("mlock readable page setup failed");
        return;
    }
    if (memory->mmap(thread, MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE, 0,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != MAP_ADDRESS + K_PAGE_SIZE) {
        testFail("mlock inaccessible page setup failed");
        return;
    }

    if (memory->mlock(MAP_ADDRESS + 32, K_PAGE_SIZE - 64) != 0) {
        testFail("mlock rejected a mapped readable range");
    }
    if (memory->mlock(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE) != (U32)-K_ENOMEM) {
        testFail("mlock accepted a PROT_NONE page");
    }
    if (memory->mlock(MAP_ADDRESS, K_PAGE_SIZE * 2) != (U32)-K_ENOMEM) {
        testFail("mlock accepted a range containing a PROT_NONE page");
    }
    if (memory->mlock(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE) != (U32)-K_ENOMEM) {
        testFail("mlock accepted an unmapped page");
    }
    if (memory->mlock(0xfffff000, K_PAGE_SIZE * 2) != (U32)-K_ENOMEM) {
        testFail("mlock accepted a range that overflows the guest address space");
    }
    if (memory->mlock(MAP_ADDRESS, 0) != 0) {
        testFail("mlock rejected an empty range");
    }

    if (memory->munlock(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE) != 0) {
        testFail("munlock rejected a mapped PROT_NONE page");
    }
    if (memory->munlock(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE) != (U32)-K_ENOMEM) {
        testFail("munlock accepted an unmapped page");
    }
    if (memory->munlock(0xfffff000, K_PAGE_SIZE * 2) != (U32)-K_ENOMEM) {
        testFail("munlock accepted a range that overflows the guest address space");
    }
    if (memory->munlock(MAP_ADDRESS, 0) != 0) {
        testFail("munlock rejected an empty range");
    }
}

#endif
