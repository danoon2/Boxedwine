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
#include "codePageData.h"
#include "../../util/ptrpool.h"
#include "kmemory_soft.h"
#include "../cpu/normal/normalCPU.h"

#define FIRST_INDEX_SIZE 0x400
#define SECOND_INDEX_SIZE 0x400
// if these change, fix X64Asm::jmpReg and any other binary translators
#define GET_FIRST_INDEX_FROM_PAGE(page) (page >> 10)
#define GET_SECOND_INDEX_FROM_PAGE(page) (page & 0x3ff);

std::atomic_int activeOps;

DecodedOpPageCache::DecodedOpPageCache() {
	memset(ops, 0, sizeof(ops));
}

DecodedOpPageCache::~DecodedOpPageCache() {
	for (U32 i = 0; i < K_PAGE_SIZE; i++) {
		if (ops[i]) {
			ops[i]->dealloc();
			activeOps--;
		}
	}
}

#ifdef BOXEDWINE_DYNAMIC
BOXEDWINE_MUTEX DecodedOpCache::lock;
#endif

DecodedOpCache::DecodedOpCache() {
	memset(pageData, 0, sizeof(pageData));
	memset(writeCounts, 0, sizeof(writeCounts));
}

DecodedOpCache::~DecodedOpCache() {
	removeAll();
}

DecodedOp** DecodedOpCache::getLocation(U32 address) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, true);
	if (page) {
		U32 offset = address & K_PAGE_MASK;
		return &page->ops[offset];
	}
	return nullptr;
}

DecodedOp* DecodedOpCache::get(U32 address) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (page) {
		U32 offset = address & K_PAGE_MASK;
		if (page->ops[offset] == nullptr) {
			if (offset > 0 && page->ops[offset - 1] && page->ops[offset - 1]->lock) {
				return page->ops[offset - 1];
			}
		}
		return page->ops[offset];
	}
	return nullptr;
}

bool DecodedOpCache::removeStartAt(U32 address, U32 len, bool becauseOfWrite) {
	U32 pageIndex = address >> K_PAGE_SHIFT;		
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	bool result = false;

	if (end > K_PAGE_SIZE) {
		U32 todo = K_PAGE_SIZE - offset;
		result = removeStartAt((pageIndex + 1) << K_PAGE_SHIFT, len - todo, becauseOfWrite);
		len = todo;
		end = K_PAGE_SIZE;
	}

	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (!page) {
		return false;
	}
	U8* pageWriteCounts = nullptr;
	if (becauseOfWrite) {
		pageWriteCounts = getWriteCounts(pageIndex, true);
	}
#ifdef BOXEDWINE_DYNAMIC	
	if (removeJITCode(address, len)) { // remove JIT before op
		result = true;
	}
#endif
	KThread* thread = KThread::currentThread();
	for (U32 i = offset; i < end; i++) {
		if (page->ops[i]) {
			pendingDeallocs[thread->id].push_back(page->ops[i]);
			page->ops[i] = nullptr;
			activeOps--;
		}
		if (becauseOfWrite) {
			if ((pageWriteCounts[i] < MAX_DYNAMIC_COUNT)) {
				pageWriteCounts[i]++;
			}
		}
	}
	return result;
}

DecodedOp* DecodedOpCache::getPreviousOp(U32 address, U32* foundAddress, DecodedOpPageCache** foundPage) {
	S32 offset = (address - 1) & K_PAGE_MASK;
	U32 pageIndex = (address - 1) >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);

	if (!page) {
		return nullptr;
	}
	// x86 instructions can be at most 15 bytes
	for (U32 i = 0; i <= 15; i++) {
		if (page->ops[offset]) {
			*foundAddress = address - i - 1;
			*foundPage = page;
			return page->ops[offset];
		}
		offset--;
		if (offset < 0) {
			offset = K_PAGE_SIZE - 1;
			pageIndex--;
			page = getPageCache(pageIndex, false);
			if (!page) {
				return nullptr;
			}
		}
	}
	return nullptr;
}

DecodedOp* DecodedOpCache::getPreviousOpAndRemoveIfOverlapping(U32 address, bool* removedCurrentJitBlock) {
	U32 previousOpAddress = 0;
	DecodedOpPageCache* previousPageCache = nullptr;
	DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &previousPageCache);

	if (previousOp) {
		// does previousOp span into address, if so then remove it
		if (previousOpAddress + previousOp->len > address) {
			DecodedOp* op = previousPageCache->ops[previousOpAddress & K_PAGE_MASK];
			previousPageCache->ops[previousOpAddress & K_PAGE_MASK] = nullptr;			
#ifdef BOXEDWINE_DYNAMIC
			if (removeJITCode(previousOpAddress, previousOp->len) && removedCurrentJitBlock) {
				*removedCurrentJitBlock = true;
			}
#endif
			pendingDeallocs[KThread::currentThread()->id].push_back(previousOp);
			activeOps--;
			previousOp = getPreviousOp(previousOpAddress, &previousOpAddress, &previousPageCache);
		}
	}
	return previousOp;
}

void DecodedOpCache::threadCleanup(U32 threadId) {
	clearPendingDeallocs(threadId);
}

void DecodedOpCache::clear() {
	removeAll();
	for (auto& it : pendingDeallocs) {
		for (auto& op : it.second) {
			op->dealloc();
		}
	}
	pendingDeallocs.clear();
}

void DecodedOpCache::clearPendingDeallocs(U32 threadId) {
	if (pendingDeallocs.count(threadId)) {
		for (auto& op : pendingDeallocs[threadId]) {
			op->dealloc();
		}
		pendingDeallocs.erase(threadId);
	}
}

void DecodedOpCache::iterateOps(U32 address, U32 len, OpCacheCallback callback, void* pData) {
	U32 previousOpAddress = 0;
	DecodedOpPageCache* prevPage = nullptr;
	DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &prevPage);

	if (previousOp) {
		// does previousOp span into address
		if (previousOpAddress + previousOp->len > address) {
			callback(previousOp, pData);
		}
	}

	while (len) {
		U32 pageIndex = address >> K_PAGE_SHIFT;
		U32 offset = address & K_PAGE_MASK;
		U32 todo = len;
		if (todo > K_PAGE_SIZE - offset) {
			todo = K_PAGE_SIZE - offset;
		}
		DecodedOpPageCache* page = getPageCache(pageIndex, false);
		if (page) {			
			for (U32 i = offset; i < offset + todo; i++) {
				if (page->ops[i]) {
					callback(page->ops[i], pData);
				}
			}
		}
		address += todo;
		len -= todo;
	}
}

bool DecodedOpCache::remove(U32 address, U32 len, bool becauseOfWrite) {
	bool result = false;
	DecodedOp* prev = getPreviousOpAndRemoveIfOverlapping(address, &result);
	if (prev) {
		prev->next = DecodedOp::allocDone();
	}
	if (removeStartAt(address, len, becauseOfWrite)) {
		result = true;
	}
	return result;
}

void DecodedOpCache::removeAll() {
	for (U32 firstIndex = 0; firstIndex < FIRST_INDEX_SIZE; firstIndex++) {
		if (pageData[firstIndex]) {
			for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
				delete pageData[firstIndex][secondIndex];
			}
			delete[] pageData[firstIndex];
			pageData[firstIndex] = nullptr;
		}
		if (writeCounts[firstIndex]) {
			for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
				delete writeCounts[firstIndex][secondIndex];
			}
			delete[] writeCounts[firstIndex];
			writeCounts[firstIndex] = nullptr;
		}
	}
}

void DecodedOpCache::add(DecodedOp* op, U32 address, U32 opCount) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, true);
	U32 offset = address & K_PAGE_MASK;
	DecodedOp* prevOp = nullptr;

	clearPendingDeallocs(KThread::currentThread()->id);

	while (op) {
#ifdef _DEBUG
		if (page->ops[offset]) {
			kpanic("DecodedOpCache::add");
		}
#endif

#ifdef __TEST
		if (op->inst == TestEnd)
			return;
#endif
		// not a real instruction so don't map into page->ops
		if (op->inst == Done)
			return;
#ifdef _DEBUG
		if (op->eip != address) {
			kpanic("oops");
		}
#endif
		page->ops[offset] = op;
		address += op->len;
		activeOps++;
		opCount--;		
		offset += op->len;
		prevOp = op;
		if (opCount == 0) {
			break;
		}
		op = op->next;
		if (offset >= K_PAGE_SIZE) {
			pageIndex++;
			page = getPageCache(pageIndex, true);
			offset -= K_PAGE_SIZE;
		}
	}
	if (!(prevOp->flags & OP_FLAG_END_OF_LONG_CHAIN)) {
		prevOp->next = get((pageIndex << K_PAGE_SHIFT) + offset);
	}
}

bool DecodedOpCache::isAddressDynamic(U32 address, U32 len) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	U8* pageWriteCounts = getWriteCounts(pageIndex, false);
	if (!pageWriteCounts) {
		return false;
	}
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		U32 pageIndex = address >> K_PAGE_SHIFT;
		U32 todo = K_PAGE_SIZE - offset;
		if (isAddressDynamic((pageIndex + 1) << K_PAGE_SHIFT, len - todo)) {
			return true;
		}
		end = K_PAGE_SIZE;
	}
	for (U32 i = offset; i < end; i++) {
		if (pageWriteCounts[i] == MAX_DYNAMIC_COUNT) {
			return true;
		}
	}
	return false;
}

DecodedOpPageCache* DecodedOpCache::getPageCache(U32 pageIndex, bool create) {
	U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(pageIndex);
	U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(pageIndex);
	DecodedOpPageCache** first = pageData[firstIndex];
	DecodedOpPageCache* result = nullptr;
	if (first) {
		result = first[secondIndex];
	}
	if (!result && create) {
		BOXEDWINE_CRITICAL_SECTION;
		if (!first) {
			first = pageData[firstIndex];
		}
		if (first) {
			result = first[secondIndex];
		}
		if (!first) {
			pageData[firstIndex] = new DecodedOpPageCache * [SECOND_INDEX_SIZE];
			memset(pageData[firstIndex], 0, sizeof(DecodedOpPageCache*) * SECOND_INDEX_SIZE);
		}
		if (!result) {
			result = new DecodedOpPageCache();
			pageData[firstIndex][secondIndex] = result;
		}
	}
	return result;
}

void DecodedOpCache::clearPageWriteCounts(U32 pageIndex) {
	U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(pageIndex);
	U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(pageIndex);
	U8** first = writeCounts[firstIndex];
	U8* result = nullptr;
	if (first && first[secondIndex]) {
		memset(first[secondIndex], 0, sizeof(K_PAGE_SIZE));
	}
}

U8* DecodedOpCache::getWriteCounts(U32 pageIndex, bool create) {
	U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(pageIndex);
	U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(pageIndex);
	U8** first = writeCounts[firstIndex];
	U8* result = nullptr;
	if (first) {
		result = first[secondIndex];
	}
	if (!result && create) {
		if (!first) {
			first = writeCounts[firstIndex];
		}
		if (first) {
			result = first[secondIndex];
		}
		if (!first) {
			writeCounts[firstIndex] = new U8* [SECOND_INDEX_SIZE];
			memset(writeCounts[firstIndex], 0, sizeof(U8*) * SECOND_INDEX_SIZE);
		}
		if (!result) {
			result = new U8[K_PAGE_SIZE];
			memset(result, 0, sizeof(U8) * K_PAGE_SIZE);
			writeCounts[firstIndex][secondIndex] = result;
		}
	}
	return result;
}

#ifdef BOXEDWINE_DYNAMIC

void DecodedOpCache::addJITCode_nolock(DecodedOp* op, U32 eip, U32 len) {
	jitCode[eip] = DecodedOpJIT(op, eip, len);
}

bool DecodedOpCache::removeJITCode(U32 eip, U32 len) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(DecodedOpCache::lock);

	auto it = jitCode.upper_bound(eip + len - 1);
	if (it == jitCode.begin()) {
		return false;
	}
	it--;
	bool result = false;
	KThread* thread = KThread::currentThread();
	while (true) {
		DecodedOpJIT& data = it->second;
		U32 x1 = data.eip;
		U32 x2 = x1 + data.eipLen - 1;
		U32 y1 = eip;
		U32 y2 = y1 + len - 1;

		// !(x2 < y1 || x1 > y2) // easier to think of when they are not overlapped
		if (x2 >= y1 && x1 <= y2) {
			// :TODO: dynamic memory leaks
			U32 address = data.eip;
			DecodedOp* op = data.op;	
			// this can happen during an execv, so cpu->eip does not need to be valid, to don't call cpu->getOp since that will decode the op if its not in the cache
			U32 eip = thread->cpu->getEipAddress(); // current instruction
			DecodedOp* currentOp = thread->memory->getDecodedOp(eip);
			DecodedOp* nextOp = nullptr;

			if (currentOp) {
				nextOp = currentOp->next;
			}

			while (op && address < data.eip + data.eipLen) {
				op->pfn = NormalCPU::getFunctionForOp(op);
				if (nextOp == op) {
					result = true;
				}
				if (nextOp == op && op->pfnJitCode) {
					// this is important if the current jit block modifies itself
					U8* p = (U8*)op->pfnJitCode;
					p[0] = 0x5f;
					p[1] = 0x5b;					
#ifdef _DEBUG
					p[2] = 0x89; // mov esp, ebp
					p[3] = 0xec;
					p[4] = 0x5d; // pop ebp
					p[5] = 0xc3;
#else
					p[2] = 0xc3; // ret
#endif
				}
				op->pfnJitCode = nullptr;
				op->flags &= ~OP_FLAG_JIT;
				address += op->len;
				if (!op->next && address < data.eip + data.eipLen) {
					op = get(address);
					continue;
				}
				op = op->next;
			}
			if (it == jitCode.begin()) {
				jitCode.erase(it);
				return result;
			}
			jitCode.erase(it--);
		}
		else {
			return result;
		}
	}
}

bool DecodedOpCache::hasJITCode(U32 eip, U32 len) {
	auto it = jitCode.upper_bound(eip + len - 1);
	if (it != jitCode.begin()) {
		it--;
		DecodedOpJIT& data = it->second;
		U32 x1 = data.eip;
		U32 x2 = x1 + data.eipLen - 1;
		U32 y1 = eip;
		U32 y2 = y1 + len - 1;

		return x2 >= y1 && x1 <= y2;
	}
	return false;
}
#endif
