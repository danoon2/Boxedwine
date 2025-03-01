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

#ifndef __KLIST_H__
#define __KLIST_H__

// specialized list so that no memory is allocated when an object is added or iterated
// 
// general the node will live as a member variable on the object it will contain
// because of this, it is bad to use BoxedPtr with this class because the node would then hold an extra reference
template <typename T>
class KList;

template <typename T>  
class KListNode {
public:
    KListNode(T data) : data(data) {}
    T data;

    void remove() {        
        if (!this->list) {
            return;
        }
        if (this->prev) {
            this->prev->next = this->next;
        } else {
            this->list->first = this->next;
        }
        if (this->next) {
            this->next->prev = this->prev;
        } else {
            this->list->last = this->prev;
        }
        this->list->count--;
        this->list = nullptr;
        this->next = nullptr;
        this->prev = nullptr;
    }
    
    bool isInList() {return list!= nullptr;}

    KListNode<T>* getNext() {return this->next;}
private:
    friend KList<T>;
    KListNode<T>* prev = nullptr;
    KListNode<T>* next = nullptr;
    KList<T>* list = nullptr;
}; 

template <typename T>  
class KList {
public:
    KList() = default;
    ~KList() {
        KListNode<T>* n = first;
        while (n) {
            KListNode<T>* next = n->next;
            n->next = nullptr;
            n->prev = nullptr;
            n->list = nullptr;
            n = next;
        }
    }
    bool isEmpty() {return first== nullptr;}
    void addToBack(KListNode<T>* node) {
        if (node->list) {
            if (node->list == this) {
                kwarn("Node already in list");
            } else {
                kpanic("Node already in list");
            }
        }
        if (!this->last) {
            first = node;
            last = node;
        } else {
            node->prev = last;
            last->next = node;
            last = node;
        }
        node->list = this;
        count++;
    } 
    void addToFront(KListNode<T>* node) {
        if (node->list) {
            kpanic("Node already in list");
        }
        if (!this->first) {
            first = node;
            last = node;
        } else {
            node->next = first;
            first->prev = node;
            first = node;
        }
        node->list = this;
        count++;
    }
    void for_each(std::function<void(KListNode<T>*)> f) {
        KListNode<T>* node = first;
        while (node) {
            KListNode<T>* next = node->next;
            f(node);
            node = next;
        }
    }

    KListNode<T>* front() {return first;}
    KListNode<T>* back() {return last;}
    U32 size() {return this->count;}

private:
    friend KListNode<T>;
    KListNode<T>* first = nullptr;
    KListNode<T>* last = nullptr;
    U32 count = 0;
};

#endif
