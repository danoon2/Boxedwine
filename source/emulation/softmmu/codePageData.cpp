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

#if defined(BOXEDWINE_JIT_ARMV8) && (defined(__APPLE__) || defined(__linux__))
#include <sys/mman.h>
static constexpr U64 FLAT_JIT_ENTRY_BYTES = ((U64)1 << 32) * sizeof(void*);
static constexpr size_t FLAT_JIT_LEAF_BYTES = K_PAGE_SIZE * sizeof(void*);

static void adviseFlatJitLeaf(void** entries, bool reuse) {
    static const bool disabled = std::getenv("BOXEDWINE_ARM_FLAT_RECLAIM_SLOW") != nullptr;
    const size_t granularity = (size_t)Platform::getPagePermissionGranularity() << K_PAGE_SHIFT;
    // In particular, a 64 KiB host page can contain two 32 KiB leaves. Never
    // discard a neighboring leaf that may still hold published entries.
    if (disabled || !entries || !granularity || FLAT_JIT_LEAF_BYTES % granularity) return;
#ifdef __APPLE__
    madvise(entries, FLAT_JIT_LEAF_BYTES, reuse ? MADV_FREE_REUSE : MADV_FREE_REUSABLE);
#else
    if (!reuse) madvise(entries, FLAT_JIT_LEAF_BYTES, MADV_DONTNEED);
#endif
}
#endif

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
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
            ops[i]->detachJitEntry();
#endif
			ops[i]->dealloc();
			activeOps--;
		}
	}
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    if (ownsJitEntries) std::free(jitEntries);
#if defined(BOXEDWINE_JIT_ARMV8) && (defined(__APPLE__) || defined(__linux__))
    else adviseFlatJitLeaf(jitEntries, false);
#endif
#endif
}

DecodedOpCache::DecodedOpCache() {
	memset(emptyPageCacheLevel1, 0, sizeof(emptyPageCacheLevel1));
	for (int i = 0; i < 0x400; i++) {
		pageData[i] = emptyPageCacheLevel1;
	}
	memset(writeCounts, 0, sizeof(writeCounts));
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    if (!std::getenv("BOXEDWINE_JIT_ENTRY_CACHE_SLOW")) {
        jitPageGroups = (void****)std::malloc(FIRST_INDEX_SIZE * sizeof(void***));
        if (jitPageGroups) {
            for (U32 first = 0; first < FIRST_INDEX_SIZE; ++first) {
                jitPageGroups[first] = emptyJitPageGroup;
            }
        }
    }
#endif
#if defined(BOXEDWINE_JIT_ARMV8) && (defined(__APPLE__) || defined(__linux__))
    if (jitPageGroups && !std::getenv("BOXEDWINE_ARM_FLAT_CACHE_SLOW")) {
        // A 32-bit guest has a bounded address space. Reserve virtual storage
        // for one host pointer per guest byte; anonymous pages are demand-zero
        // and consume physical memory only as lookup slots are touched.
        void* entries = mmap(nullptr, FLAT_JIT_ENTRY_BYTES, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if (entries != MAP_FAILED) {
            flatJitEntries = (void**)entries;
            flatJitHash = !std::getenv("BOXEDWINE_ARM_FLAT_HASH_SLOW");
            if (const char* value = std::getenv("BOXEDWINE_ARM_FLAT_HASH_SHIFT")) {
                U32 shift = (U32)std::strtoul(value, nullptr, 0);
                // A shift of at least twelve preserves a contiguous,
                // independently reclaimable leaf for every guest page.
                if (shift >= K_PAGE_SHIFT && shift <= 24) flatJitHashShift = shift;
            }
            flatJitPageHash = flatJitHash && std::getenv("BOXEDWINE_ARM_FLAT_PAGE_HASH") &&
                !std::getenv("BOXEDWINE_ARM_FLAT_PAGE_HASH_SLOW");
        }
    }
#endif
}

DecodedOpCache::~DecodedOpCache() {
	removeAll();
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    std::free(jitPageGroups);
#endif
#if defined(BOXEDWINE_JIT_ARMV8) && (defined(__APPLE__) || defined(__linux__))
    if (flatJitEntries) munmap(flatJitEntries, FLAT_JIT_ENTRY_BYTES);
#endif
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

#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
void** DecodedOpCache::getJitEntryLocation(U32 address) {
    if (!jitPageGroups) return nullptr;
    U32 pageIndex = address >> K_PAGE_SHIFT;
    U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(pageIndex);
    if (jitPageGroups[firstIndex] == emptyJitPageGroup) {
        void*** group = (void***)std::calloc(SECOND_INDEX_SIZE, sizeof(void**));
        if (!group) return nullptr;
#ifdef BOXEDWINE_JIT_ARMV8
        // Native lookups follow address dependencies from this pointer.
        // Publish zeroed children before exposing a newly allocated group.
        std::atomic_thread_fence(std::memory_order_release);
#endif
        jitPageGroups[firstIndex] = group;
    }
    DecodedOpPageCache* page = getPageCache(pageIndex, true);
    if (!page->jitEntries) {
        void** entries;
#ifdef BOXEDWINE_JIT_ARMV8
        if (flatJitEntries) {
            // XOR-shift is a permutation of all 32-bit addresses. Mixing page
            // bits into the byte offset spreads regularly spaced targets over
            // host cache sets while preserving a contiguous leaf per guest page.
            entries = flatJitEntries + (getFlatJitEntryIndex(pageIndex << K_PAGE_SHIFT) & ~K_PAGE_MASK);
            page->jitEntryXorMask = getFlatJitEntryIndex(pageIndex << K_PAGE_SHIFT) & K_PAGE_MASK;
            page->ownsJitEntries = false;
#if defined(__APPLE__) || defined(__linux__)
            // Cleared leaves remain at stable virtual addresses. Make their
            // zeroed storage active again before publishing new code pointers.
            adviseFlatJitLeaf(entries, true);
#endif
        } else
#endif
            entries = (void**)std::calloc(K_PAGE_SIZE, sizeof(void*));
        if (entries) {
            // Decoding alone does not need a compiled-entry array. Bind any
            // existing instructions before publishing this page to the JIT.
            for (U32 offset = 0; offset < K_PAGE_SIZE; ++offset) {
                DecodedOp* op = page->ops[offset];
                if (op) {
                    op->bindJitEntry(&entries[offset ^ page->jitEntryXorMask]);
                }
            }
            page->jitEntries = entries;
#ifdef BOXEDWINE_JIT_ARMV8
            std::atomic_thread_fence(std::memory_order_release);
#endif
            jitPageGroups[firstIndex][pageIndex & 0x3ff] = entries;
        }
    }
    return page->jitEntries ? &page->jitEntries[(address & K_PAGE_MASK) ^ page->jitEntryXorMask] : nullptr;
}
#endif

#ifdef BOXEDWINE_JIT_ARMV8
void** DecodedOpCache::getDirectJitEntryLocation(U32 address) {
    static const bool disabled = std::getenv("BOXEDWINE_ARM_COMPACT_DIRECT_SLOW") != nullptr;
    if (!flatJitEntries || disabled) return getJitEntryLocation(address);
    // Direct branches already know the guest target at translation time. Their
    // cells can be packed independently of the sparse indirect-lookup table.
    auto result = directJitEntries.try_emplace(address);
    auto& entry = result.first->second;
    if (result.second) {
        if (std::getenv("BOXEDWINE_ARM_PACKED_DIRECT_SLOW")) {
            entry.binding.direct = &entry.scatteredCode;
        } else {
            // Pack hot pointer cells separately from hash nodes/DecodedOps.
            // Growing the chunk list never moves an existing cell.
            U32 offset = directJitEntryCount % DIRECT_ENTRY_CHUNK_SIZE;
            if (!offset) directJitEntryChunks.push_back(std::make_unique<void*[]>(DIRECT_ENTRY_CHUNK_SIZE));
            entry.binding.direct = directJitEntryChunks.back().get() + offset;
            ++directJitEntryCount;
        }
    }
    void** slot = entry.binding.direct;
    if (DecodedOp* op = get(address)) {
        op->bindDirectJitEntry(&entry.binding);
    }
    return slot;
}
#endif

void DecodedOpCache::removeStartAt(U32 address, U32 len, bool becauseOfWrite) {
	// Preserve the old last-page-first order without one host stack frame per
	// guest page. Large sparse unmaps can cover hundreds of thousands of pages.
	U64 end = (U64)address + len;
	while (end > address) {
		U64 start = std::max((U64)address, (end - 1) & ~(U64)K_PAGE_MASK);
		removePageRange((U32)start, (U32)(end - start), becauseOfWrite);
		end = start;
	}
}

void DecodedOpCache::removePageRange(U32 address, U32 len, bool becauseOfWrite) {
	U32 pageIndex = address >> K_PAGE_SHIFT;		
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;

	U8* pageWriteCounts = nullptr;
	if (becauseOfWrite) {
		pageWriteCounts = getWriteCounts(pageIndex, true);
	}
	DecodedOpPageCache* page = getPageCache(pageIndex, false);
	if (!page && !pageWriteCounts) {
		return;
	}

	KThread* thread = page ? KThread::currentThread() : nullptr;
	for (U32 i = offset; i < end; i++) {
		if (page && page->ops[i]) {
			if (preparedRemovalPendingDeallocs) {
				preparedRemovalPendingDeallocs->ops.push_back(page->ops[i]);
			} else {
				pendingDeallocs[thread->id].ops.push_back(page->ops[i]);
			}
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
            page->ops[i]->detachJitEntry();
#endif
			page->ops[i] = nullptr;
			activeOps--;
		}
		if (pageWriteCounts) {
#if defined(BOXEDWINE_WASM_JIT) && !defined(BOXEDWINE_MULTI_THREADED)
			if (KSystem::disableWasmJitForWrittenCode) {
				pageWriteCounts[i] = MAX_DYNAMIC_COUNT;
				continue;
			}
#endif
			if ((pageWriteCounts[i] < MAX_DYNAMIC_COUNT)) {
				pageWriteCounts[i]++;
			}
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

DecodedOp* DecodedOpCache::getPreviousOpAndRemoveIfOverlapping(U32 address) {
	U32 previousOpAddress = 0;
	DecodedOpPageCache* previousPageCache = nullptr;
	DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &previousPageCache);

	if (previousOp) {
		// does previousOp span into address, if so then remove it
        if (previousOpAddress + previousOp->len > address) {
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
            previousOp->detachJitEntry();
#endif
			previousPageCache->ops[previousOpAddress & K_PAGE_MASK] = nullptr;			
			if (preparedRemovalPendingDeallocs) {
				preparedRemovalPendingDeallocs->ops.push_back(previousOp);
			} else {
				pendingDeallocs[KThread::currentThread()->id].ops.push_back(previousOp);
			}
			activeOps--;
			previousOp = getPreviousOp(previousOpAddress, &previousOpAddress, &previousPageCache);
		}
	}
	return previousOp;
}

void DecodedOpCache::threadCleanup(U32 threadId) {
#ifdef BOXEDWINE_MULTI_THREADED
	auto registered = registeredThreads.find(threadId);
	if (registered != registeredThreads.end()) {
		registered->second->decodedOpCacheEpoch.store(0, std::memory_order_release);
		registeredThreads.erase(registered);
	}
	reclaimPendingDeallocs();
#else
	clearPendingDeallocs(threadId);
#endif
}

void DecodedOpCache::clear() {
	removeAll();
	for (auto& it : pendingDeallocs) {
		for (auto& op : it.second.ops) {
			op->dealloc();
		}
	}
	pendingDeallocs.clear();
#ifdef BOXEDWINE_MULTI_THREADED
	for (auto& it : registeredThreads) {
		it.second->decodedOpCacheEpoch.store(0, std::memory_order_release);
	}
	registeredThreads.clear();
#endif
}

#ifdef BOXEDWINE_JIT
void DecodedOpCache::collectAllJitBlocks(std::vector<void*>& out) {
	for (U32 firstIndex = 0; firstIndex < FIRST_INDEX_SIZE; firstIndex++) {
		if (pageData[firstIndex] == emptyPageCacheLevel1) continue;
		for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
			DecodedOpPageCache* page = pageData[firstIndex][secondIndex];
			if (!page) continue;
			for (U32 i = 0; i < K_PAGE_SIZE; i++) {
				DecodedOp* op = page->ops[i];
				if (op) {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
					void* jitCode;
					if (op->flags2 & OP_FLAG2_WASM_JIT_RELOC_HAZARD) {
						jitCode = __atomic_exchange_n(
							&op->pfnJitCode, nullptr, __ATOMIC_SEQ_CST);
					} else {
						jitCode = op->pfnJitCode;
						op->pfnJitCode = nullptr;
					}
#else
					void* jitCode = op->pfnJitCode;
#endif
					if (jitCode) {
						out.push_back(jitCode);
					}
				}
			}
		}
	}
}
#endif

void DecodedOpCache::clearPendingDeallocs(U32 threadId) {
	if (pendingDeallocs.count(threadId)) {
		for (auto& op : pendingDeallocs[threadId].ops) {
			op->dealloc();
		}
		pendingDeallocs.erase(threadId);
	}
}

void DecodedOpCache::retirePendingDeallocs(PendingDecodedOps& pending, size_t previousSize) {
	if (pending.ops.size() == previousSize) {
		return;
	}
#ifdef BOXEDWINE_MULTI_THREADED
	pending.retirementEpoch = epoch.fetch_add(1, std::memory_order_acq_rel) + 1;
	reclaimPendingDeallocs();
#endif
}

#ifdef BOXEDWINE_MULTI_THREADED
void DecodedOpCache::registerThread(CPU* cpu) {
	if (!cpu->thread) {
		kpanic("DecodedOpCache CPU has no thread");
	}
	registeredThreads[cpu->thread->id] = cpu;
	cpu->decodedOpCacheEpoch.store(epoch.load(std::memory_order_acquire), std::memory_order_release);
}

void DecodedOpCache::reclaimPendingDeallocs() {
	for (auto pending = pendingDeallocs.begin(); pending != pendingDeallocs.end();) {
		// A prepared transaction reuses this map entry and its reserved vector
		// across all removal ranges. Even without active CPUs, erasing it after
		// the first range leaves the next range writing through a stale pointer.
		bool canReclaim = &pending->second != preparedRemovalPendingDeallocs &&
			pending->second.retirementEpoch != 0;
		if (canReclaim) {
			for (const auto& registered : registeredThreads) {
				U32 cpuEpoch = registered.second->decodedOpCacheEpoch.load(std::memory_order_acquire);
				if (cpuEpoch && cpuEpoch < pending->second.retirementEpoch) {
					canReclaim = false;
					break;
				}
			}
		}
		if (!canReclaim) {
			++pending;
			continue;
		}
		for (DecodedOp* op : pending->second.ops) {
			op->dealloc();
		}
		pending = pendingDeallocs.erase(pending);
	}
}
#endif

void DecodedOpCache::iterateOps(U32 address, U32 len, OpCacheCallback callback, void* pData) {
	U32 previousOpAddress = 0;
	DecodedOpPageCache* prevPage = nullptr;
	DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &prevPage);

	if (previousOp) {
		// does previousOp span into address
		if (previousOpAddress + previousOp->len > address) {
			callback(previousOpAddress, previousOp, pData);
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
					callback((pageIndex << K_PAGE_SHIFT) + i, page->ops[i], pData);
				}
			}
		}
		address += todo;
		len -= todo;
	}
}

void DecodedOpCache::remove(U32 address, U32 len, bool becauseOfWrite) {
	PendingDecodedOps* pending = preparedRemovalPendingDeallocs;
	if (!pending) {
		KThread* thread = KThread::currentThread();
		if (thread) {
			pending = &pendingDeallocs[thread->id];
		}
	}
	size_t previousPendingSize = pending ? pending->ops.size() : 0;
	DecodedOp* prev = getPreviousOpAndRemoveIfOverlapping(address);
	if (prev) {
		prev->next = preparedRemovalDone ? preparedRemovalDone : DecodedOp::allocDone();
	}
	removeStartAt(address, len, becauseOfWrite);
	if (pending) {
		retirePendingDeallocs(*pending, previousPendingSize);
	}
}

void DecodedOpCache::prepareRemoveRanges(const std::vector<std::pair<U32, U32>>& ranges) {
	KThread* thread = KThread::currentThread();
	if (!thread) {
		throw std::bad_alloc();
	}

	size_t removalCount = 0;
	for (const auto& range : ranges) {
		U32 address = range.first;
		U32 len = range.second;
		if (!len) {
			continue;
		}
		U32 previousOpAddress = 0;
		DecodedOpPageCache* previousPage = nullptr;
		DecodedOp* previousOp = getPreviousOp(address, &previousOpAddress, &previousPage);
		if (previousOp && previousOpAddress + previousOp->len > address) {
			++removalCount;
		}

		U64 cursor = address;
		U64 end = cursor + len;
		while (cursor < end) {
			U32 pageIndex = (U32)(cursor >> K_PAGE_SHIFT);
			U32 offset = (U32)cursor & K_PAGE_MASK;
			U32 todo = (U32)std::min<U64>(end - cursor, K_PAGE_SIZE - offset);
			DecodedOpPageCache* page = getPageCache(pageIndex, false);
			if (page) {
				for (U32 i = offset; i < offset + todo; ++i) {
					if (page->ops[i]) {
						++removalCount;
					}
				}
			}
			cursor += todo;
		}
	}

	auto result = pendingDeallocs.try_emplace(thread->id);
	PendingDecodedOps& pending = result.first->second;
	if (removalCount > pending.ops.max_size() - pending.ops.size()) {
		throw std::length_error("DecodedOp removal is too large");
	}
	pending.ops.reserve(pending.ops.size() + removalCount);
	DecodedOp* done = DecodedOp::allocDone();
	preparedRemovalPendingDeallocs = &pending;
	preparedRemovalDone = done;
}

void DecodedOpCache::finishPreparedRemove() {
	preparedRemovalPendingDeallocs = nullptr;
	preparedRemovalDone = nullptr;
#ifdef BOXEDWINE_MULTI_THREADED
	reclaimPendingDeallocs();
#endif
}

void DecodedOpCache::removeAll() {
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    if (jitPageGroups) {
        for (U32 first = 0; first < FIRST_INDEX_SIZE; ++first) {
            if (jitPageGroups[first] != emptyJitPageGroup) {
                std::free(jitPageGroups[first]);
                jitPageGroups[first] = emptyJitPageGroup;
            }
        }
    }
#endif
	for (U32 firstIndex = 0; firstIndex < FIRST_INDEX_SIZE; firstIndex++) {
		if (pageData[firstIndex] != emptyPageCacheLevel1) {
			for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
				if (pageData[firstIndex][secondIndex]) {
					delete pageData[firstIndex][secondIndex];
				}
			}
			delete[] pageData[firstIndex];
			pageData[firstIndex] = emptyPageCacheLevel1;
		}
		if (writeCounts[firstIndex]) {
			for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
				if (writeCounts[firstIndex][secondIndex]) {
					delete[] writeCounts[firstIndex][secondIndex];
				}
			}
			delete[] writeCounts[firstIndex];
			writeCounts[firstIndex] = nullptr;
		}
	}
#ifdef BOXEDWINE_JIT_ARMV8
    // Page destruction detached every live op before the pointer cells die.
    directJitEntries.clear();
    directJitEntryChunks.clear();
    directJitEntryCount = 0;
#endif
}

void DecodedOpCache::add(DecodedOp* op, U32 address, U32 opCount) {
	// Readers can enter any cached instruction without taking the decoder's
	// mutex. Finish the chain before publishing its first entry, otherwise a
	// thread can reach the still-null tail while we install the remaining ops.
	U32 endAddress = address;
	DecodedOp* tail = op;
	for (U32 remaining = opCount; tail && remaining; --remaining) {
		if (tail->inst == Done) {
			break;
		}
#ifdef __TEST
		if (tail->inst == TestEnd) {
			break;
		}
#endif
		endAddress += tail->len;
		if (remaining == 1 || !tail->next) {
			if (!(tail->flags & OP_FLAG_END_OF_LONG_CHAIN)) {
				tail->next = get(endAddress);
			}
			break;
		}
		tail = tail->next;
	}

	U32 pageIndex = address >> K_PAGE_SHIFT;
	DecodedOpPageCache* page = getPageCache(pageIndex, true);
	U32 offset = address & K_PAGE_MASK;

#ifdef BOXEDWINE_MULTI_THREADED
	reclaimPendingDeallocs();
#else
	clearPendingDeallocs(KThread::currentThread()->id);
#endif

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
#ifdef __TEST
        if (testAfterOpPublished) {
            testAfterOpPublished(address, op);
        }
#endif
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
        if (page->jitEntries) {
            op->bindJitEntry(&page->jitEntries[offset ^ page->jitEntryXorMask]);
        }
#endif
#ifdef BOXEDWINE_JIT_ARMV8
        auto direct = directJitEntries.find(address);
        if (direct != directJitEntries.end()) {
            op->bindDirectJitEntry(&direct->second.binding);
        }
#endif
		address += op->len;
		activeOps++;
		opCount--;		
		offset += op->len;
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
	DecodedOpPageCache* result = pageData[firstIndex][secondIndex];

	if (!result && create) {
		BOXEDWINE_CRITICAL_SECTION;
			
		if (!result) {
			result = pageData[firstIndex][secondIndex];
		}
		if (!result) {
			if (pageData[firstIndex] == emptyPageCacheLevel1) {
				pageData[firstIndex] = new DecodedOpPageCache*[SECOND_INDEX_SIZE];
				memset(pageData[firstIndex], 0, sizeof(DecodedOpPageCache*) * SECOND_INDEX_SIZE);
			}
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

	if (first && first[secondIndex]) {
		memset(first[secondIndex], 0, K_PAGE_SIZE);
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
