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
#include <map>

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
};

typedef void (*OpCacheCallback)(U32 address, DecodedOp* op, void* pData);

class DecodedOpCache {
public:
	DecodedOpCache();
	~DecodedOpCache();

	DecodedOp* get(U32 address);
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
	DecodedOp* getPreviousOp(U32 address, U32* foundAddress, DecodedOpPageCache** foundPage);
	DecodedOpPageCache* getPageCache(U32 pageIndex, bool create);
	DecodedOpPageCache** pageData[0x400];
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
