#pragma once

#include <type_traits>

template<typename T>
class PtrPool {
private:
    std::queue<T*> queue;
    std::vector<T*> allocated;
    BOXEDWINE_MUTEX mutex;
    int blockSize;

public:
    PtrPool(int blockSize=10000) : blockSize(blockSize) {}
    ~PtrPool() {deleteAll();}

    /// Return an object from the pool.
    T* get() {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        T* newData;

        if (!queue.empty()) {
            newData = queue.front();
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

    /// Mark the given object for reuse in the future.
    inline void put(T* obj) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        if constexpr (!std::is_integral<T>::value) {
            obj->reset();
        }
        queue.push(obj);
    }

    void deleteAll() {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        for (auto& t : allocated) {
            delete[] t;
        }
        allocated.clear();
        queue = {};
    }
};

