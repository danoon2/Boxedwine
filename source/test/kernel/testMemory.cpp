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
#include "../../emulation/softmmu/kmemory_soft.h"
#include <cstdlib>
#ifdef BOXEDWINE_MULTI_THREADED
#include <atomic>
#include <thread>
#endif

void testRamPageMetadataConcurrentGrowth() {
#ifdef BOXEDWINE_MULTI_THREADED
    // Grow the RAM table while other threads inspect live pages, as WineD3D
    // and PhysX do during 3DMark Vantage CPU test 2. Even an existing-key
    // lookup must synchronize with the table's insertions and rehashes.
    constexpr U32 readerCount = 3;
    constexpr U32 pageCount = 16384;
    std::vector<U8> storage((pageCount + 1) * K_PAGE_SIZE);
    U8* base = (U8*)(((uintptr_t)storage.data() + K_PAGE_SIZE - 1) & ~(uintptr_t)(K_PAGE_SIZE - 1));
    std::vector<RamPage> pages;
    pages.reserve(pageCount);
    for (U32 i = 0; i < readerCount; i++) {
        pages.push_back(ramPageAllocNative(base + i * K_PAGE_SIZE));
    }
    std::atomic<U32> ready{0};
    std::atomic<bool> done{false};
    std::atomic<bool> failed{false};
    std::vector<std::thread> readers;
    for (U32 i = 0; i < readerCount; i++) {
        const RamPage page = pages[i];
        readers.emplace_back([&, page]() {
            ready.fetch_add(1);
            do {
                ramPageMarkSystem(page, true);
                if (!ramPageIsSystem(page) || !ramPageIsNative(page)) failed = true;
                ramPageMarkSystem(page, false);
                if (ramPageIsSystem(page) || !ramPageIsNative(page)) failed = true;
            } while (!done.load());
        });
    }
    while (ready.load() != readerCount) std::this_thread::yield();
    for (U32 i = readerCount; i < pageCount; i++) {
        pages.push_back(ramPageAllocNative(base + i * K_PAGE_SIZE));
    }
    done = true;
    for (auto& reader : readers) reader.join();
    for (RamPage page : pages) {
        if (ramPageUseCount(page) != 1 || !ramPageIsNative(page) || ramPageIsSystem(page)) failed = true;
        ramPageRelease(page);
    }
    if (failed.load()) testFail("RAM page flags or reference counts changed during concurrent table growth");
#endif
}

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

void testInitialStackLinearAlignment() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    const U32 oldEsp = context.cpu->reg[4].u32;
    context.thread->setupStack();
    const U32 stackTop = context.cpu->reg[4].u32;
    context.cpu->reg[4].u32 = oldEsp;
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    U32 topGuard = stackTop;
    while (memory->canRead(topGuard, 1) && topGuard - stackTop < groupBytes) {
        topGuard += K_PAGE_SIZE;
    }
    const U32 mappingStart = topGuard + K_PAGE_SIZE - MAX_STACK_SIZE;
    if (memory->canRead(topGuard, 1) || memory->canWrite(topGuard, 1) ||
        memory->canRead(mappingStart, 1) || memory->canWrite(mappingStart, 1)) {
        testFail("initial stack guard permissions changed");
    }
    if (!memory->canRead(mappingStart + K_PAGE_SIZE, MAX_STACK_SIZE - 2 * K_PAGE_SIZE) ||
        !memory->canWrite(mappingStart + K_PAGE_SIZE, MAX_STACK_SIZE - 2 * K_PAGE_SIZE)) {
        testFail("initial stack usable mapping changed");
    }
    if (ramPageUseLinearMemory() && !std::getenv("BOXEDWINE_ALIGN_STACK_SLOW")) {
        if (stackTop & (groupBytes - 1)) {
            testFail("initial stack is not aligned to native memory groups");
        }
        KMemoryData* data = getMemData(memory);
        if (!data->linearMemoryMappings[(stackTop - 4) >> K_PAGE_SHIFT]) {
            testFail("initial stack top group has no direct memory mapping");
        } else {
            memory->writed(stackTop - 4, 0x12345678);
            if (*(volatile U32*)(data->linearMemoryBase + stackTop - 4) != 0x12345678) {
                testFail("initial stack direct alias does not match guest RAM");
            }
        }
    }
    memory->unmap(mappingStart, MAX_STACK_SIZE);
}

void testMmapStackEndAlignment() {
    auto& context = testContext();
    KMemory* memory = context.memory;
    KThread* thread = context.thread;
    KMemoryData* data = getMemData(memory);
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    const bool grouped = data->useLinearMemoryJit() && groupBytes > K_PAGE_SIZE;
    const bool enabled = grouped && !std::getenv("BOXEDWINE_ALIGN_PTHREAD_STACK_SLOW");
    constexpr U32 flags = K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_STACK;
    for (U32 guardPages : {1u, 2u, 3u, 4u}) {
        const U32 guardBytes = guardPages * K_PAGE_SIZE;
        const U32 usableBytes = 256 * 1024;
        const U32 bytes = usableBytes + guardBytes;
        const U32 address = memory->mmap(thread, 0, bytes, 0, flags, -1, 0);
        if (address >= (U32)-4095) {
            testFail("anonymous stack mmap failed");
            return;
        }
        if ((enabled && ((address + bytes) & (groupBytes - 1))) ||
            (!enabled && grouped && (address & (groupBytes - 1)))) {
            testFail("anonymous stack mapping has incorrect native alignment");
        }
        if (memory->canRead(address, 1) || memory->canWrite(address + bytes - 1, 1)) {
            testFail("stack placement changed initial PROT_NONE permissions");
        }
        if (memory->mprotect(thread, address + guardBytes, usableBytes, K_PROT_READ | K_PROT_WRITE)) {
            testFail("stack usable range mprotect failed");
        }
        if (memory->canRead(address + guardBytes - 1, 1) || memory->canWrite(address, 1) ||
            !memory->canRead(address + guardBytes, usableBytes) ||
            !memory->canWrite(address + guardBytes, usableBytes)) {
            testFail("stack guard or usable range changed");
        }
        memory->writed(address + bytes - 4, 0x1234abcd);
        if (enabled) {
            if (!data->linearMemoryMappings[(address + bytes - 4) >> K_PAGE_SHIFT] ||
                *(volatile U32*)(data->linearMemoryBase + address + bytes - 4) != 0x1234abcd) {
                testFail("thread stack top has no matching linear alias");
            }
        }
        memory->unmap(address, bytes);
        if (memory->isPageMapped(address >> K_PAGE_SHIFT) ||
            memory->isPageMapped((address + bytes - 1) >> K_PAGE_SHIFT)) {
            testFail("stack unmap did not remove the requested range");
        }
    }
    // A guest-specified address remains authoritative. Fixed mappings may
    // intentionally have a partial native group at either end.
    constexpr U32 fixedAddress = 0x22401000;
    constexpr U32 fixedBytes = 5 * K_PAGE_SIZE;
    for (U32 fixedFlag : {(U32)K_MAP_FIXED, (U32)K_MAP_FIXED_NOREPLACE}) {
        const U32 address = memory->mmap(thread, fixedAddress, fixedBytes,
            K_PROT_READ | K_PROT_WRITE, flags | fixedFlag, -1, 0);
        if (address != fixedAddress) testFail("MAP_STACK changed a fixed guest address");
        if (address < (U32)-4095) memory->unmap(address, fixedBytes);
    }
    constexpr U32 hint = 0x22004000;
    const U32 hinted = memory->mmap(thread, hint, fixedBytes, K_PROT_READ, flags, -1, 0);
    if (hinted != hint) testFail("MAP_STACK changed the existing aligned hint policy");
    if (hinted < (U32)-4095) memory->unmap(hinted, fixedBytes);
    // Non-stack anonymous mappings retain start alignment.
    const U32 ordinary = memory->mmap(thread, 0, fixedBytes, K_PROT_READ,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
    if (ordinary >= (U32)-4095 || (grouped && (ordinary & (groupBytes - 1)))) {
        testFail("ordinary anonymous mmap alignment changed");
    }
    if (ordinary < (U32)-4095) memory->unmap(ordinary, fixedBytes);
}

#endif
