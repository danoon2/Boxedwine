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

#ifndef __BHEAP_H__
#define __BHEAP_H__

#define BHEAP_NUMBER_OF_BUCKETS 9
#define BHEAP_MIN_BUCKET 4

class BHeap {
public:
	U32 alloc(KMemory* memory, KThread* thread, U32 len);
	void free(KMemory* memory, U32 address);
	void freeAll(KMemory* memory);
private:
	std::vector<U32> bucket[BHEAP_NUMBER_OF_BUCKETS];
	std::vector<U32> pages;
};

#endif