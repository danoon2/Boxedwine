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

#ifndef __DYNAMIC_MEMORY_H__
#define __DYNAMIC_MEMORY_H__

class DynamicMemoryData {
public:
    DynamicMemoryData(void* p, U32 len) : p(p), len(len) {}

    void* p;
    U32 len;
};

class DynamicMemory {
public:
    DynamicMemory() : dynamicExecutableMemoryPos(0), dynamicExecutableMemoryLen(0) {}
    ~DynamicMemory() {
        for (U32 i = 0; i < this->dynamicExecutableMemory.size(); i++) {
            Platform::releaseNativeMemory(this->dynamicExecutableMemory[i].p, this->dynamicExecutableMemory[i].len);
        }
    }
    std::vector<DynamicMemoryData> dynamicExecutableMemory;
    U32 dynamicExecutableMemoryPos;
    U32 dynamicExecutableMemoryLen;
};


#endif