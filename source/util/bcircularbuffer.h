#ifndef __BCIRCULAR_BUFFER_H__
#define __BCIRCULAR_BUFFER_H__

template <class T>
class BCircularBuffer {
private:
    uint32_t head = 0;
    uint32_t tail = 0;
    uint32_t bufferSize = 0;
    T* buffer = nullptr;
    
    uint32_t getOffset(uint32_t index) {
        return (tail - 1 - index + bufferSize) & (bufferSize - 1);
    }

    void ensure(uint32_t additionalSize) {
        uint32_t len = size();
        if (len + additionalSize + 1 > bufferSize) {
            uint32_t newBufferSize = bufferSize * 2;
            T* newBuffer = new T[newBufferSize];
            uint32_t todo = len;
            if (todo + head > bufferSize) {
                todo = bufferSize - 1;
            }
            std::move(buffer+head, buffer + todo, newBuffer);
            uint32_t amountCopied = todo;
            todo = len - amountCopied;
            if (todo) {
                std::move(buffer, buffer + todo, newBuffer + amountCopied);
            }
            delete[] buffer;
            buffer = newBuffer;
            bufferSize = newBufferSize;
            head = 0;
            tail = len;
        }
    }

public:
    BCircularBuffer() {
        buffer = new T[32];
        bufferSize = 32;
    }

    BCircularBuffer(const BCircularBuffer& b) {
        buffer = new T[b.bufferSize];
        bufferSize = b.bufferSize;
        head = b.head;
        tail = b.tail;
        std::copy(b.buffer, b.buffer + bufferSize, buffer);
    }

    ~BCircularBuffer() {
        if (buffer) {
            delete[] buffer;
        }
    }

    void clear() noexcept { 
        head = 0; 
        tail = 0; 
    }

    void pushBack(const T& item) {
        ensure(1);
        buffer[tail] = item;
        tail = (tail + 1) & (bufferSize - 1);
    }

    void remove(const T& item) {
        uint32_t len = size();
        for (uint32_t i = 0; i < len; i++) {
            uint32_t offset = getOffset(i);            
            if (item == buffer[offset]) {
                for (; i < len - 1; i++) {
                    buffer[getOffset(i)] = std::move(buffer[getOffset(i+1)]);
                }
                tail = (tail - 1) & (bufferSize - 1);
            }
        }
    }

    void iterate(std::function<bool(T& item)> callback) {
        uint32_t len = size();
        for (uint32_t i = 0; i < len; i++) {
            if (!callback(buffer[getOffset(i)])) {
                break;
            }
        }
    }

    bool popFront(T& item) {
        if (isEmpty()) {
            return false;
        }
        item = buffer[head];
        head = (head + 1) & (bufferSize - 1);
        return true;
    }

    bool get(uint32_t index, T& item) const noexcept {
        if (index >= size()) {
            return false;
        }

        *item = buffer[getOffset(index)];
        return true;
    }

    uint32_t size() const {
        if (isEmpty()) {
            return 0;
        }
        return bufferSize - ((head - tail) & (bufferSize - 1));
    }

    bool isEmpty() const { 
        return head == tail; 
    }
};

#endif