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

#ifndef __KARRAY_H__
#define __KARRAY_H__

#include "platform.h"

template <typename T> 
class KArray {
public:
    KArray() : _data(0), _reserved(0), _size(0) {}
    KArray(U32 reserved) {
        this->_data = new T[reserved];
        this->_size = 0;
        this->_reserved = reserved;
    }
    ~KArray() {
        if (this->_data) {
            delete[] this->_data;
        }
    }
    void for_each(std::function<void(T& )> f) {
        for (U32 i=0;i<this->_size;i++) {
            f(_data[i]);
        }
    }
    void add(const T& t) {
        if (this->_size==this->_reserved) {
            if (this->_reserved)
                this->_reserved*=2;
            else
                this->_reserved = 8;
            T* tmp = new T[this->_reserved];
            if (this->_data) {
                for (U32 i=0;i<this->_size;i++) {
                    tmp[i] = this->_data[i];
                }
                delete[] this->_data;
            }
            this->_data = tmp;
        }
        this->_data[this->_size] = t;
        this->_size++;
    }
    void removeAt(int idx) {
#ifdef _DEBUG
        if (idx>=this->size())
            kpanic("KArray index out of bounds");
#endif
        this->_size--;
        for (U32 i=idx;i<this->_size;i++) {
            this->_data[i] = this->_data[i+1];
        }
        this->_data[this->_size] = T();
    }
    void remove(const T& t) {
        for (U32 i=0;i<this->_size;i++) {
            if (this->_data[i]==t) {
                removeAt(i);
                break;
            }
        }
    }
    void removeAll() {
        for (U32 i=0;i<this->_size;i++) {
            this->_data[i] = T();
        }
        this->_size = 0;
    }

    T& operator [](int idx) {
#ifdef _DEBUG
        if (idx>=this->size())
            kpanic("KArray index out of bounds");
#endif
        return _data[idx];
    }

    const T& operator [](int idx) const {
#ifdef _DEBUG
        if (idx>=this->size)
            kpanic("KArray index out of bounds");
#endif
        return _data[idx];
    }
    U32 size() const {return this->size();}

private:
    T* _data;
    U32 _reserved;
    U32 _size;
};

#endif
