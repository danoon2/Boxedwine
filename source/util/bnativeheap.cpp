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
#include "bnativeheap.h"

#define BNATIVEHEAD_64K_BLOCK_SIZE (64 * 1024)

static int powerOf2(U32 requestedSize) {
    U32 powerOf2Size = 1;
	U32 size = 2;
    while (size < requestedSize) {
		size <<= 1;
        powerOf2Size++;
    }
    return powerOf2Size;
}

void BNativeHeap::freeAll() {
	for (auto& block : blocks) {
		Platform::releaseNativeMemory(block, BNATIVEHEAD_64K_BLOCK_SIZE);
	}
	blocks.clear();
	buckets.clear();
	for (auto& it : largeBlocks) {
		Platform::releaseNativeMemory(it.key, it.value);
	}
	largeBlocks.clear();
}

bool BNativeHeap::containsAddress(void* p) {
	for (void* block : blocks) {
		U8* address = (U8*)block;
		if (p >= address && p < address + BNATIVEHEAD_64K_BLOCK_SIZE) {
			return true;
		}
	}
	for (auto& it : largeBlocks) {
		if (p >= it.key && p < it.key + it.value) {
			return true;
		}
	}
	return false;
}

void* BNativeHeap::alloc(U32 len) {
	U32 index = powerOf2(len + 4);
	if (index < 4) {
		index = 4;
	}
	U32 size = 1 << index;
	if (size >= BNATIVEHEAD_64K_BLOCK_SIZE) {
		U32 count = (len + 4 + BNATIVEHEAD_64K_BLOCK_SIZE - 1) / BNATIVEHEAD_64K_BLOCK_SIZE;
		
		U8* result = Platform::alloc64kBlock(count, true);
		*((U32*)result) = count * BNATIVEHEAD_64K_BLOCK_SIZE;
		largeBlocks.set(result, count * BNATIVEHEAD_64K_BLOCK_SIZE);
		return result + 4;
	}
	if (buckets.contains(index) && buckets[index].size()) {
		void* result = buckets[index].back();
		buckets[index].pop_back();
		memset(result, 0, len);
		return result;
	}
	U8* address = Platform::alloc64kBlock(1, true);

	blocks.push_back(address);

	for (U8* start = address; start < address + BNATIVEHEAD_64K_BLOCK_SIZE; start += size) {
		*((U32*)start) = index;
		buckets[index].push_back(start + 4);
	}
	return alloc(len);
}

void BNativeHeap::free(void* address) {
	if (!address) {
		return;
	}
	U32 index = *(((U32*)address) - 1);
	if (index >= BNATIVEHEAD_64K_BLOCK_SIZE) {
		U8* rawAddress = ((U8*)address) - 4;
		Platform::releaseNativeMemory(rawAddress, index);
		largeBlocks.remove(rawAddress);
		return;
	}	
	buckets[index].push_back(address);
}