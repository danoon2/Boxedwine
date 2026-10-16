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

#ifndef __CODEPAGE_DATA_H__
#define __CODEPAGE_DATA_H__

#define MAX_DYNAMIC_COUNT 0xff

#include "soft_ram.h"
#ifdef BOXEDWINE_JIT_ARMV8
#include "../cpu/decoder.h"
#endif
#include <map>
#include <unordered_map>
#include <memory>

class KMemory;
class CPU;

struct PendingDecodedOps {
	std::vector<DecodedOp*> ops;
	U32 retirementEpoch = 0;
};

class DecodedOpPageCache {
public:
	DecodedOpPageCache();
	~DecodedOpPageCache();
	
	DecodedOp* ops[K_PAGE_SIZE];
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    void** jitEntries = nullptr;
    bool ownsJitEntries = true;
    U16 jitEntryXorMask = 0;
#endif
};

typedef void (*OpCacheCallback)(U32 address, DecodedOp* op, void* pData);

class DecodedOpCache {
public:
	DecodedOpCache();
	~DecodedOpCache();
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    void**** getJitPageGroups() const { return jitPageGroups; }
    void** getJitEntryLocation(U32 address);
#endif
#ifdef BOXEDWINE_JIT_ARMV8
    void** getDirectJitEntryLocation(U32 address);
    void** getFlatJitEntries() const { return flatJitEntries; }
    bool hasFlatJitHash() const { return flatJitHash; }
    bool hasFlatJitPageHash() const { return flatJitPageHash; }
    U32 getFlatJitHashShift() const { return flatJitHashShift; }
    U32 getFlatJitEntryIndex(U32 address) const {
        if (!flatJitHash) return address;
        return address ^ (address >> flatJitHashShift) ^
            (flatJitPageHash ? (address & 0xffff0000u) >> 4 : 0);
    }
#endif

	DecodedOp* get(U32 address) {
		U32 pageIndex = address >> K_PAGE_SHIFT;
		DecodedOpPageCache* page = pageData[pageIndex >> 10][pageIndex & 0x3ff];
		return page ? page->ops[address & K_PAGE_MASK] : nullptr;
	}
	DecodedOp** getLocation(U32 address);
	DecodedOp* getPreviousOpAndRemoveIfOverlapping(U32 address);
	void remove(U32 address, U32 len, bool becauseOfWrite);
	void prepareRemoveRanges(const std::vector<std::pair<U32, U32>>& ranges);
	void finishPreparedRemove();
	void iterateOps(U32 address, U32 len, OpCacheCallback callback, void* pData);
#ifdef BOXEDWINE_JIT
	// Walk every cached DecodedOp and collect published JIT entries before the cache is wiped.
	void collectAllJitBlocks(std::vector<void*>& out);
#endif
	void add(DecodedOp* op, U32 address, U32 opCount);
#ifdef __TEST
    std::function<void(U32, DecodedOp*)> testAfterOpPublished;
#endif
	bool isAddressDynamic(U32 address, U32 len);
	void clearPageWriteCounts(U32 pageIndex);
	void threadCleanup(U32 threadId);
	void clear();
#ifdef BOXEDWINE_MULTI_THREADED
	std::atomic<U32>* getEpochAddress() { return &epoch; }
	void registerThread(CPU* cpu);
#endif

private:
	friend class BtCPU;
	friend class CPU;
	void removeAll();
	void removeStartAt(U32 address, U32 len, bool becauseOfWrite);
	void removePageRange(U32 address, U32 len, bool becauseOfWrite);
	DecodedOp* getPreviousOp(U32 address, U32* foundAddress, DecodedOpPageCache** foundPage);
	DecodedOpPageCache* getPageCache(U32 pageIndex, bool create);
	DecodedOpPageCache** pageData[0x400];
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    // 10 group bits, 10 page bits, then a 12-bit instruction-byte offset.
    void**** jitPageGroups = nullptr;
    void** emptyJitPageGroup[0x400] = {};
#endif
#ifdef BOXEDWINE_JIT_ARMV8
    void** flatJitEntries = nullptr;
    bool flatJitHash = false;
    bool flatJitPageHash = false;
    U32 flatJitHashShift = K_PAGE_SHIFT;
    // Pointer chunks remain stable as the index grows. Keep cells until all
    // compiled code and decoded instructions are retired by removeAll().
    struct DirectJitEntry {
        DecodedOpJitEntries binding;
        void* scatteredCode = nullptr;
    };
    std::unordered_map<U32, DirectJitEntry> directJitEntries;
    static constexpr U32 DIRECT_ENTRY_CHUNK_SIZE = 512;
    std::vector<std::unique_ptr<void*[]>> directJitEntryChunks;
    U32 directJitEntryCount = 0;
#endif
	DecodedOpPageCache* emptyPageCacheLevel1[0x400];
	U8* getWriteCounts(U32 pageIndex, bool create);
	U8** writeCounts[0x400];
	void clearPendingDeallocs(U32 threadId);
	void retirePendingDeallocs(PendingDecodedOps& pending, size_t previousSize);
#ifdef BOXEDWINE_MULTI_THREADED
	void reclaimPendingDeallocs();
	std::atomic<U32> epoch{1};
	std::map<U32, CPU*> registeredThreads;
#endif

	// Removed ops remain intact until every CPU that could have observed them
	// has reached a later dispatch boundary.
	std::map<U32, PendingDecodedOps> pendingDeallocs;
	PendingDecodedOps* preparedRemovalPendingDeallocs = nullptr;
	DecodedOp* preparedRemovalDone = nullptr;
};

#endif
