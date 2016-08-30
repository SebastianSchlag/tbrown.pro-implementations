/**
 * Implementation of a Record Manager with several memory reclamation schemes.
 * This file provides an (atomic) array list class for use by the Record Manager.
 * 
 * Copyright (C) 2016 Trevor Brown
 * Contact (tabrown [at] cs [dot] toronto [dot edu]) with any questions or comments.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARRAYLIST_H
#define	ARRAYLIST_H

#include <cassert>
#include <iostream>
#include <atomic>
#include "machineconstants.h"
#include "globals.h"
using namespace std;

// this list allows multiple readers, but only ONE writer.
// i don't know if it is linearizable; maybe linearize at __size.load()/store()
template <typename T>
class AtomicArrayList {
private:
    atomic_int __size;
    atomic_uintptr_t *data;
public:
    const int capacity;
    AtomicArrayList(const int _capacity) : capacity(_capacity) {
        VERBOSE DEBUG COUTATOMIC("constructor AtomicArrayList capacity="<<capacity<<endl);
        __size.store(0, memory_order_relaxed);
        data = new atomic_uintptr_t[capacity];
    }
    ~AtomicArrayList() {
        delete[] data;
    }
    inline T* get(const int ix) {
        return (T*) data[ix].load(memory_order_relaxed);
    }
    inline int size() {
        return __size.load(memory_order_relaxed); // note: this must be seq_cst if membars are not manually added
    }
    inline void add(T * const obj) {
        int sz = __size.load(memory_order_relaxed);
        assert(sz < capacity);
        SOFTWARE_BARRIER;
        data[sz].store((uintptr_t) obj, memory_order_relaxed);
        SOFTWARE_BARRIER;
        __size.store(sz+1, memory_order_relaxed); // note: this must be seq_cst if membars are not manually added
    }
    inline void erase(const int ix) {
        int sz = __size.load(memory_order_relaxed);
        assert(ix >= 0 && ix < sz);
        if (ix != sz-1) data[ix].store(data[sz-1].load(memory_order_relaxed), memory_order_relaxed);
        __size.store(sz-1, memory_order_relaxed); // note: this must be seq_cst if membars are not manually added
    }
    inline void erase(T * const obj) {
        int ix = getIndex(obj);
        if (ix != -1) erase(ix);
    }
    inline int getIndex(T * const obj) {
        int sz = __size.load(memory_order_relaxed); // note: this must be seq_cst if membars are not manually added
        for (int i=0;i<sz;++i) {
            if (data[i].load(memory_order_relaxed) == (uintptr_t) obj) return i;
        }
        return -1;
    }
    inline bool contains(T * const obj) {
        return (getIndex(obj) != -1);
    }
    inline void clear() {
        SOFTWARE_BARRIER;
        __size.store(0, memory_order_relaxed); // note: this must be seq_cst if membars are not manually added
        SOFTWARE_BARRIER;
    }
    inline bool isFull() {
        return __size.load(memory_order_relaxed) == capacity; // note: this must be seq_cst if membars are not manually added
    }
    inline bool isEmpty() {
        return __size.load(memory_order_relaxed) == 0; // note: this must be seq_cst if membars are not manually added
    }
};

template <typename T>
class ArrayList {
private:
    int __size;
    T **data;
public:
    const int capacity;
    ArrayList(const int _capacity) : capacity(_capacity) {
        __size = 0;
        data = new T*[capacity];
    }
    ~ArrayList() {
        delete[] data;
    }
    inline T* get(const int ix) {
        return data[ix];
    }
    inline int size() {
        return __size;
    }
    inline void add(T * const obj) {
        assert(__size < capacity);
        data[__size++] = obj;
    }
    inline void erase(const int ix) {
        assert(ix >= 0 && ix < __size);
        data[ix] = data[--__size];
    }
    inline void erase(T * const obj) {
        int ix = getIndex(obj);
        if (ix != -1) erase(ix);
    }
    inline int getIndex(T * const obj) {
        for (int i=0;i<__size;++i) {
            if (data[i] == obj) return i;
        }
        return -1;
    }
    inline bool contains(T * const obj) {
        return (getIndex(obj) != -1);
    }
    inline void clear() {
        __size = 0;
    }
    inline bool isFull() {
        return __size == capacity;
    }
    inline bool isEmpty() {
        return __size == 0;
    }
};


#endif	/* ARRAYLIST_H */

