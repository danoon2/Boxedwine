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

#ifndef __CODEPAGE_DATA_H__
#define __CODEPAGE_DATA_H__

#define MAX_DYNAMIC_COUNT 0xff

#include "soft_ram.h"
#include <map>

class KMemory;

class DecodedOpJIT {
public:
	DecodedOpJIT() : op(nullptr), eip(0), eipLen(0) {}
	DecodedOpJIT(DecodedOp* op, U32 eip, U32 eipLen) : op(op), eip(eip), eipLen(eipLen) {}

	DecodedOp* op;
	U32 eip;
	U32 eipLen;
};

class DecodedOpPageCache {
public:
	DecodedOpPageCache();
	~DecodedOpPageCache();

	U8* writeCounts = nullptr;
	DecodedOp* ops[K_PAGE_SIZE];
};

class DecodedOpCache {
public:
	DecodedOpCache();
	~DecodedOpCache();

	DecodedOp* get(U32 address);
	DecodedOp** getLocation(U32 address);
	DecodedOp* getPreviousOpAndRemoveIfOverlapping(U32 address, bool* removedCurrentJitBlock = nullptr);
	bool remove(U32 address, U32 len, bool becauseOfWrite);	
	void add(DecodedOp* op, U32 address, bool followOpNext);
	bool isAddressDynamic(U32 address, U32 len);
#ifdef BOXEDWINE_DYNAMIC
	static BOXEDWINE_MUTEX lock;

	void addJITCode_nolock(DecodedOp* op, U32 eip, U32 len);
	bool hasJITCode(U32 eip, U32 len);
	bool removeJITCode(U32 eip, U32 len);
	void markOpNoJit(U32 address);
#endif
	void threadCleanup(U32 threadId);
	void clear();

private:
	void removeAll();
	bool removeStartAt(U32 address, U32 len, bool becauseOfWrite);
	DecodedOp* getPreviousOp(U32 address, U32* foundAddress, DecodedOpPageCache** foundPage);
	DecodedOpPageCache* getPageCache(U32 pageIndex, bool create);
	DecodedOpPageCache** pageData[0x400];
	void clearPendingDeallocs(U32 threadId);

	std::map<U32, DecodedOpJIT> jitCode;

	// this will hold DecodedOp's that are ready for dealloc, but are delayed in case the current thread is referencing them while removing them
	// Without this, there will be timing issues that only occasionally show up in debug mode, but become more obvious when running games multiple
	// times, like with automation
	std::map<U32, std::vector<DecodedOp*>> pendingDeallocs;
};

#endif