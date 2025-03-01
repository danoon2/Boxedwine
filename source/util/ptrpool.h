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

#pragma once

#include <type_traits>

template<typename T, bool needsMutex = true>
class PtrPool {
private:
    std::queue<T*> queue;
    std::vector<T*> allocated;
    BOXEDWINE_MUTEX mutex;
    int blockSize;

    T* internalGet() {
        if (!queue.empty()) {
            T* newData = queue.front();
            queue.pop();
            return newData;
        }
        if (blockSize == 0) {
            return nullptr;
        }
        T* t = new T[blockSize];
        for (int i = 1; i < blockSize; i++) {
            queue.push(&t[i]);
        }
        allocated.push_back(t);
        return &t[0];
    }

    inline void internalPut(T* obj) {
        if constexpr (!std::is_integral<T>::value) {
            obj->reset();
        }
        queue.push(obj);
    }

    void internalDeleteAll() {
        for (auto& t : allocated) {
            delete[] t;
        }
        allocated.clear();
        queue = {};
    }
public:
    PtrPool(int blockSize=10000) : blockSize(blockSize) {}
    // currently on raspberry pi, calling deleteAll in the destructor hangs, it's probably because the global PtrPool get destructed in a different order, maybe they should be pointers that get deleting in KSystem::destroy
    ~PtrPool() {}

    /// Return an object from the pool.
    T* get() {
        if constexpr (needsMutex) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            return internalGet();
        } else {
            return internalGet();
        }
    }

    /// Mark the given object for reuse in the future.
    inline void put(T* obj) {
        if constexpr (needsMutex) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            internalPut(obj);
        } else {
            internalPut(obj);
        }        
    }

    void deleteAll() {
        if constexpr (needsMutex) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            internalDeleteAll();
        } else {
            internalDeleteAll();
        }
    }
};

