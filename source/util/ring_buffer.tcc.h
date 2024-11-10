//          Copyright Jean Pierre Cimalando 2018-2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// SPDX-License-Identifier: BSL-1.0

//#include "ring_buffer/ring_buffer.h"

template <bool Atomic>
inline size_t Ring_Buffer_Ex<Atomic>::capacity() const
{
    return cap_ - 1;
}

//------------------------------------------------------------------------------
template <class Mutex>
inline Soft_Ring_Buffer_Ex<Mutex>::Soft_Ring_Buffer_Ex(size_t capacity)
    : rb_(capacity)
{
}

template <class Mutex>
inline Soft_Ring_Buffer_Ex<Mutex>::~Soft_Ring_Buffer_Ex()
{
}

template <class Mutex>
inline size_t Soft_Ring_Buffer_Ex<Mutex>::capacity() const
{
    return rb_.capacity();
}

//------------------------------------------------------------------------------
template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::get(T &x)
{
    return get(&x, 1);
}

template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::get(T *x, size_t n)
{
    static_assert(std::is_trivially_copyable<T>::value, "ring_buffer: T must be trivially copyable");
    RB *self = static_cast<RB *>(this);
    return self->getbytes_(x, n * sizeof(T));
}

template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::peek(T &x)
{
    return peek(&x, 1);
}

template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::peek(T *x, size_t n)
{
    static_assert(std::is_trivially_copyable<T>::value, "ring_buffer: T must be trivially copyable");
    RB *self = static_cast<RB *>(this);
    return self->peekbytes_(x, n * sizeof(T));
}

template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::put(const T &x)
{
    return put(&x, 1);
}

template <class RB>
template <class T>
inline bool Basic_Ring_Buffer<RB>::put(const T *x, size_t n)
{
    static_assert(std::is_trivially_copyable<T>::value, "ring_buffer: T must be trivially copyable");
    RB *self = static_cast<RB *>(this);
    return self->putbytes_(x, n * sizeof(T));
}
