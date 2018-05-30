/*
  A simple ring buffer implementation as C++ template.
  
  Copyright (c) 2011 Hannes Flicka
  Licensed under the terms of the MIT license (given below).
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <algorithm>
#include <memory.h>


template<typename T> class ringbuffer {
public:
    /**
      * create a ringbuffer with space for up to size elements.
      */
    explicit ringbuffer(size_t size)
            : size(size)
            , begin(0)
            , end(0)
            , wrap(false)
    {
        buffer = new T[size];
    }

    /**
      * copy constructor
      */
    ringbuffer(const ringbuffer<T> & rb)
    {
        this(rb.size);
        begin = rb.begin;
        end = rb.end;
        memcpy(buffer, rb.buffer, sizeof(T) * size);
    }

    /**
      * destructor
      */
    ~ringbuffer()
    {
        delete[] buffer;
    }

    size_t write(const T * data, size_t n)
    {
        n = std::min(n, getFree());

        if (n == 0) {
            return n;
        }

        const size_t first_chunk = std::min(n, size - end);
        memcpy(buffer + end, data, first_chunk * sizeof(T));
        end = (end + first_chunk) % size;

        if (first_chunk < n) {
            const size_t second_chunk = n - first_chunk;
            memcpy(buffer + end, data + first_chunk, second_chunk * sizeof(T));
            end = (end + second_chunk) % size;
        }

        if (begin == end) {
            wrap = true;
        }

        return n;
    }

    size_t read(T * dest, size_t n)
    {
        n = std::min(n, getOccupied());

        if (n == 0) {
            return n;
        }

        if (wrap) {
            wrap = false;
        }

        const size_t first_chunk = std::min(n, size - begin);
        memcpy(dest, buffer + begin, first_chunk * sizeof(T));
        begin = (begin + first_chunk) % size;

        if (first_chunk < n) {
            const size_t second_chunk = n - first_chunk;
            memcpy(dest + first_chunk, buffer + begin, second_chunk * sizeof(T));
            begin = (begin + second_chunk) % size;
        }
        return n;
    }

    size_t getOccupied() {
        if (end == begin) {
            return wrap ? size : 0;
        } else if (end > begin) {
            return end - begin;
        } else {
            return size + end - begin;
        }
    }

    size_t getFree() {
        return size - getOccupied();
    }
private:
    T * buffer;
    size_t size;
    size_t begin;
    size_t end;
    bool wrap;
};

#endif // RINGBUFFER_H
