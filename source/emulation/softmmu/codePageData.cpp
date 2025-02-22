#include "boxedwine.h"
#include "codePageData.h"
#include "../../util/ptrpool.h"
#include "kmemory_soft.h"

#define FIRST_INDEX_SIZE 0x400
#define SECOND_INDEX_SIZE 0x400
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
	if (writeCounts) {
		delete[] writeCounts;
	}
}

DecodedOpCache::DecodedOpCache() {
	memset(pageData, 0, sizeof(pageData));
}

DecodedOpCache::~DecodedOpCache() {
	removeAll();
}

DecodedOp** DecodedOpCache::getLocation(U32 address) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
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

void DecodedOpCache::removeStartAt(U32 address, U32 len, bool becauseOfWrite) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (!page) {
		return;
	}
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		U32 todo = K_PAGE_SIZE - offset;
		removeStartAt((pageIndex + 1) << K_PAGE_SHIFT, len - todo, becauseOfWrite);
		end = K_PAGE_SIZE;
	}
	if (becauseOfWrite && !page->writeCounts) {
		page->writeCounts = new U8[K_PAGE_SIZE];
		memset(page->writeCounts, 0, K_PAGE_SIZE);
	}
	for (U32 i = offset; i < end; i++) {
		if (page->ops[i]) {
			page->ops[i]->dealloc();
			page->ops[i] = nullptr;
			activeOps--;
		}
		if (becauseOfWrite && (page->writeCounts[i] < MAX_DYNAMIC_COUNT)) {
			page->writeCounts[i]++;
		}
	}
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

DecodedOp* DecodedOpCache::getPreviousOpAndRemoveIfOverlapping(U32 address, bool* removedOverlappingOp) {
	U32 previousOpAddress = 0;
	DecodedOpPageCache* previousPageCache = nullptr;
	DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &previousPageCache);

	if (previousOp) {
		// does previousOp span into address, if so then remove it
		if (previousOpAddress + previousOp->len > address) {
			previousPageCache->ops[previousOpAddress & K_PAGE_MASK] = nullptr;
			previousOp->dealloc();
			activeOps--;
			previousOp = getPreviousOp(previousOpAddress, &previousOpAddress, &previousPageCache);
		}
	}
	return previousOp;
}

void DecodedOpCache::remove(U32 address, U32 len, bool becauseOfWrite) {
	DecodedOp* prev = getPreviousOpAndRemoveIfOverlapping(address);
	if (prev) {
		prev->next = DecodedOp::allocDone();
	}
	removeStartAt(address, len, becauseOfWrite);
}

void DecodedOpCache::removePage(U32 pageIndex) {
	U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(pageIndex);
	U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(pageIndex);
	DecodedOpPageCache** first = pageData[firstIndex];
	DecodedOpPageCache* result = nullptr;
	if (first) {
		result = first[secondIndex];
	}
	if (result) {
		for (U32 i = 0; i < K_PAGE_SIZE; i++) {
			if (result->ops[i]) {
				int ii = 0;
			}
		}
		delete first[secondIndex];
		first[secondIndex] = nullptr;
		bool found = false;
		for (U32 i = 0; i < SECOND_INDEX_SIZE; i++) {
			if (first[i]) {
				found = true;
				break;
			}
		}
		if (!found) {
			delete[] first;
			pageData[firstIndex] = nullptr;
		}
	}
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
	}
}

void DecodedOpCache::add(DecodedOp* op, U32 address, bool followOpNext) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, true);
	U32 offset = address & K_PAGE_MASK;
	DecodedOp* prevOp = nullptr;

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
		page->ops[offset] = op;
		activeOps++;
		if (!followOpNext) {
			break;
		}
		offset += op->len;
		prevOp = op;
		op = op->next;
		if (offset >= K_PAGE_SIZE) {
			pageIndex++;
			page = getPageCache(pageIndex, true);
			offset -= K_PAGE_SIZE;
		}
	}
	if (followOpNext) {
		prevOp->next = get((pageIndex << K_PAGE_SHIFT) +  offset);
	}
}

void DecodedOpCache::incrementWriteCounts(U32 address, U32 len) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(address, false);
	if (!page) {
		return;
	}
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		U32 pageIndex = address >> K_PAGE_SHIFT;
		U32 todo = K_PAGE_SIZE - offset;
		incrementWriteCounts((pageIndex + 1) << K_PAGE_SHIFT, len - todo);
		end = K_PAGE_SIZE;
	}
	for (U32 i = offset; i < end; i++) {
		if (page->writeCounts[i] < MAX_DYNAMIC_COUNT) {
			page->writeCounts[i]++;
		}
	}
}

void DecodedOpCache::markAddressDynamic(U32 address, U32 len) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (!page) {
		return;
	}
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		U32 pageIndex = address >> K_PAGE_SHIFT;
		U32 todo = K_PAGE_SIZE - offset;
		markAddressDynamic((pageIndex + 1) << K_PAGE_SHIFT, len - todo);
		end = K_PAGE_SIZE;
	}
	if (!page->writeCounts) {
		page->writeCounts = new U8[K_PAGE_SIZE];
		memset(page->writeCounts, 0, K_PAGE_SIZE);
	}
	for (U32 i = offset; i < end; i++) {
		page->writeCounts[i] = MAX_DYNAMIC_COUNT;
	}
}

bool DecodedOpCache::isAddressDynamic(U32 address, U32 len) {
	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (!page) {
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
	if (page->writeCounts) {
		for (U32 i = offset; i < end; i++) {
			if (page->writeCounts[i] == MAX_DYNAMIC_COUNT) {
				return true;
			}
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
