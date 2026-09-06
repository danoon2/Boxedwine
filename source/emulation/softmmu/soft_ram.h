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

#ifndef __SOFT_RAM_H__
#define __SOFT_RAM_H__

#include "platformBoxedwine.h"

#ifdef BOXEDWINE_64
#define RAM_TYPE U64
#else
#define RAM_TYPE U32
#endif
struct RamPage {
	RAM_TYPE value = 0;
};

class LinearMemoryBacking;

RamPage ramPageAlloc();
RamPage ramPageAllocNativeContinuous(U8* native, U32 pageCount);
RamPage ramPageAllocNative(U8* native);
U8* ramPageGet(RamPage page);
void ramPageRelease(RamPage page);
void ramPageRetain(RamPage page);
U32 ramPageUseCount(RamPage page);
void ramPageMarkSystem(RamPage page, bool isSystem);
bool ramPageIsSystem(RamPage page);
bool ramPageIsNative(RamPage page);

// Native 64-bit JIT memory aperture. Pool-backed RAM is shared between the normal
// soft-MMU address and a 4 GiB guest-address-shaped alias.
void ramPageConfigureLinearMemory(bool disabled);
bool ramPageUseLinearMemory();
bool ramPageUseLinearMemoryAdjacent();
bool ramPageUseLinearMemoryBacking();
bool ramPageUseLinearMemoryFileCacheBlock();
constexpr U32 LINEAR_MEMORY_RECOMPILE_FAULTS = 3;
U32 ramPageLinearMemoryPageCount();
U64 ramPageLinearMemoryApertureSize();
bool ramPageAllocLinearMemoryBlock(RamPage* pages, U32 pageCount);
std::shared_ptr<LinearMemoryBacking> ramPageCreateLinearMemoryBacking();
bool ramPageAllocLinearMemoryBackingBlock(std::shared_ptr<LinearMemoryBacking>& backing, U32 guestPage, RamPage* pages, U32 pageCount);
U8* ramPageReserveLinearMemoryData(U64 dataSize, U8** linearMemoryAddress);
void ramPageReleaseLinearMemoryData(U8* data, U64 dataSize);
U8* ramPageReserveLinearMemory();
void ramPageReleaseLinearMemory(U8* address);
void ramPageResetLinearMemory(U8* address);
void ramPageRemoveLinearMemory(U8* address, U32 guestPage, U32 pageCount);
U32 ramPageUpdateLinearMemory(U8* address, U32 guestPage, const RamPage* pages, U32 pageCount, bool canRead, bool canWrite, U32 currentMapping);
void shutdownRam();

inline U8* ramPageGet(RamPage page) {
	return (U8*)(page.value << 12);
}

#endif
