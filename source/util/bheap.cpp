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
#include "bheap.h"

static int powerOf2(U32 requestedSize) {
    U32 powerOf2Size = 1;
	U32 size = 2;
    while (size < requestedSize) {
		size <<= 1;
        powerOf2Size++;
    }
    return powerOf2Size;
}

void BHeap::freeAll(KMemory* memory) {
	for (auto& page : pages) {
		memory->unmap(page, K_PAGE_SIZE);
	}
	for (U32 i = 0; i < BHEAP_NUMBER_OF_BUCKETS; i++) {
		bucket[i].clear();
	}
}

U32 BHeap::alloc(KMemory* memory, KThread* thread, U32 len) {
	U32 index = powerOf2(len + 4);
	if (index < BHEAP_MIN_BUCKET) {
		index = BHEAP_MIN_BUCKET;
	}
	if (index >= BHEAP_MIN_BUCKET + BHEAP_NUMBER_OF_BUCKETS) {
		len += 4;
		len = (len + K_PAGE_MASK) & ~K_PAGE_MASK;
		U32 address = memory->mmap(thread, 0, len, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS | K_MAP_PRIVATE | K_MAP_BOXEDWINE, -1, 0);
		memory->writed(address, len);
		return address + 4;
	}
	index = index - BHEAP_MIN_BUCKET;
	if (bucket[index].size() > 0) {
		U32 result = bucket[index].back();
		bucket[index].pop_back();
		return result;
	}
	U32 address = memory->mmap(thread, 0, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS | K_MAP_PRIVATE | K_MAP_BOXEDWINE, -1, 0);
	U32 size = 1 << (index + BHEAP_MIN_BUCKET);

	pages.push_back(address);
	for (U32 start = address; start < address + K_PAGE_SIZE; start += size) {
		memory->writed(start, index);
		bucket[index].push_back(start + 4);
	}
	return alloc(memory, thread, len);
}

void BHeap::free(KMemory* memory, U32 address) {
	if (!address) {
		return;
	}
	U32 index = memory->readd(address - 4);
	if (index >= BHEAP_NUMBER_OF_BUCKETS) {
		memory->unmap(address - 4, index);
		return;
	}
	memory->memset(address, 0, (1 << (index + BHEAP_MIN_BUCKET)) - 4);
	bucket[index].push_back(address);
}