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

#ifndef __BTMEMORY_H__
#define __BTMEMORY_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCodeChunk;

#define K_MAX_X86_OP_LEN 15

class BtMemory {
public:	
	BtMemory(KMemory* memory);
	~BtMemory();	

	U8* getExistingHostAddress(U32 eip);
	U8* allocateExcutableMemory(U32 size, U32* allocatedSize);
	void freeExcutableMemory(U8* hostMemory, U32 size);
	void executableMemoryReleased();
	bool isAddressExecutable(U8* address);
	bool isEipPageCommitted(U32 page);
	void setEipPageCommitted(U32 page) { this->committedEipPages[page] = true; }		

	std::shared_ptr<BtCodeChunk> getCodeChunkContainingEip(U32 eip);

	void addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);
	void removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);

	class AllocatedMemory {
	public:
		AllocatedMemory(U8* memory, U32 size) : memory(memory), size(size) {}
		U8* memory;
		U32 size;
	};
	std::list<AllocatedMemory> allocatedExecutableMemory;

#define EXECUTABLE_MIN_SIZE_POWER 7
#define EXECUTABLE_MAX_SIZE_POWER 22
#define EXECUTABLE_SIZES 16

	std::list<U8*> freeExecutableMemory[EXECUTABLE_SIZES];		
	KMemory* memory;

	bool committedEipPages[K_NUMBER_OF_PAGES];

	U8*** eipToHostInstructionPages;
	BOXEDWINE_MUTEX mutex;

protected:
	void clearCodePageFromCache(U32 page);
};

#endif
#endif